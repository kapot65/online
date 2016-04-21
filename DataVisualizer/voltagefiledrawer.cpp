#include "voltagefiledrawer.h"

VoltageFileDrawer::VoltageFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    graph_block_1 = plot->addGraph();
    graph_block_1->setParent(this);
    graph_block_1->setName(tr("напряжение блок 1"));

    graph_block_2 = plot->addGraph();
    graph_block_2->setParent(this);
    graph_block_2->setName(tr("напряжение блок 2"));

    setColor(color);
    update();
}


VoltageFileDrawer::~VoltageFileDrawer()
{
}

void VoltageFileDrawer::setMetaDataToTable()
{
    table->clearContents();
    table->setRowCount(1);
    table->setColumnCount(2);
    setMetaTableText(0, 0, tr("тип файла"));
    setMetaTableText(1, 0, tr("файл напряжения"));
}

void VoltageFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    if(graphMode == ABSOLUTE_TIME)
    {
        graph_block_1->setVisible(visible);
        graph_block_2->setVisible(visible);
    }
    else
    {
        graph_block_1->setVisible(false);
        graph_block_2->setVisible(false);
    }

    isVisible = visible;
}

void VoltageFileDrawer::setColor(QColor color)
{
    this->color = color;

    graph_block_1->setPen(color);

    QPen dashPen;
    dashPen.setStyle(Qt::DashLine);
    dashPen.setColor(color);
    graph_block_2->setPen(dashPen);
}

void VoltageFileDrawer::update()
{
    QFileInfo fi(*file);
    if(fi.lastModified() == fileLastModified)
        return; //файл не обновлялся

    //считывание хедеров
    if(file->pos() == 0)
    {
        //проверка готовности бинарного хедера
        if(file->bytesAvailable() < 30)
            return;

        //считывание бинарного заголовка
        rawMachineHeader = file->read(30);
        bool ok;
        header = TcpProtocol::readMachineHeader(rawMachineHeader, &ok);
        if(!ok)
        {
            file->seek(0);
            return;
        }
    }
    if(file->pos() == 30)
    {
        if(file->bytesAvailable() < header.metaLength)
            return;

        //парсинг мета хедера
        QByteArray headers = rawMachineHeader + file->read(header.metaLength);
        QByteArray data;
        TcpProtocol::parceMessage(headers, meta, data, true);
    }


    while(file->bytesAvailable() >= sizeof(unsigned char)
                                + sizeof(unsigned long long int)
                                + sizeof(double))
    {
        unsigned char block;
        unsigned long long int time;
        double value;
        //добаление значений на график
        switch(header.dataType)
        {
            case HV_BINARY:
                file->read((char*)&block, sizeof(unsigned char));
                file->read((char*)&time, sizeof(unsigned long long int));
                file->read((char*)(&value), sizeof(double));
            case HV_TEXT_BINARY:
            {
                QStringList lines = QString(file->readLine()).split(' ');
                time = QDateTime().fromString(lines[0], Qt::ISODate).toMSecsSinceEpoch();
                block = lines[1].toInt();
                value = lines[2].toDouble();
            }
        }

        switch (block)
        {
            case 1:
                graph_block_1->addData(time, value);
            break;

            case 2:
                graph_block_2->addData(time, value);
            break;

            default:
                break;
        }
    }

    emit updated();
}
