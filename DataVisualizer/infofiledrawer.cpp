#include "infofiledrawer.h"

InfoFileDrawer::InfoFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    file->close();
    update();
}

InfoFileDrawer::~InfoFileDrawer()
{
}

void InfoFileDrawer::setMetaDataToTable()
{
    //заполнение метаданных
    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(2);
    table->insertRow(table->rowCount());
    setMetaTableText(0, 0, tr("тип файла"));
    setMetaTableText(1, 0, tr("файл с комментариями к сбору"));

    table->insertRow(table->rowCount());
    if(meta.contains("date"))
    {
        setMetaTableText(0, table->rowCount() - 1, tr("дата"));
        setMetaTableText(1, table->rowCount() - 1, meta["date"].toString());
    }

    table->insertRow(table->rowCount());
    if(meta.contains("operator"))
    {
        setMetaTableText(0, table->rowCount() - 1, tr("оператор"));
        setMetaTableText(1, table->rowCount() - 1, meta["operator"].toString());
    }

    table->insertRow(table->rowCount());
    if(meta.contains("description"))
    {
        setMetaTableText(0, table->rowCount() - 1, tr("комментарий к началу сбора"));
        setMetaTableText(1, table->rowCount() - 1, meta["description"].toString());
    }

    QVariantList comments = meta["comments"].toList();

    for(int i = 0; i < comments.size(); i++)
    {
        table->insertRow(table->rowCount());
        if(!comments[i].canConvert(QVariant::Map) ||
           !(comments[i].toMap().contains("comment") && comments[i].toMap().contains("date_time")))
        {
            setMetaTableText(0, table->rowCount() - 1, tr("неизвестный тип комментария"));
#ifdef USE_QTJSON
            setMetaTableText(1, table->rowCount() - 1, QJsonDocument::fromVariant(comments[i]).toJson());
#else
            setMetaTableText(1, table->rowCount() - 1, QJson::Serializer().serialize(comments[i]));
#endif
        }
        else
        {
            QVariantMap curr_comment = comments[i].toMap();
            setMetaTableText(0, table->rowCount() - 1, curr_comment["date_time"].toString());
            setMetaTableText(1, table->rowCount() - 1, curr_comment["comment"].toString());
        }
    }
    table->resizeColumnsToContents();
}

void InfoFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    if(graphMode == ABSOLUTE_TIME)
    {
        for(int i = 0; i < items.size(); i++)
        {
            items[i]->setVisible(visible);
        }
    }
    else
    {
        for(int i = 0; i < items.size(); i++)
        {
            items[i]->setVisible(false);
        }
    }
    isVisible = visible;
    //plot->replot();
}

void InfoFileDrawer::update()
{
    QFileInfo fi(*file);
    if(fi.lastModified() == fileLastModified)
        return; //файл не обновлялся

    fileLastModified = fi.lastModified();

    //file->open(QIODevice::ReadOnly);
    file->close();
    file->open(QIODevice::ReadOnly);
    //file->seek(0);
    fileBuffer = file->readAll();

    //удаление старых объектов
//    for(int i = 0 ; i < items.size(); i++)
//    {
//        plot->removeItem(items[i]);
//    }
    //определение текущего состояния видисомти графика
    bool curr_visibility;
    if(items.isEmpty())
        curr_visibility = 0;
    else
        curr_visibility = items[0]->visible();
    //переоткрытие сообщения
    TcpProtocol::parceMessage(fileBuffer, meta, data);

    QVariantList comments = meta["comments"].toList();

    double x = 0.1;
    double y = 0;

    for(int i = 0; i < comments.size(); i++)
    {
        if(comments[i].canConvert(QVariant::Map) &&
           (comments[i].toMap().contains("comment") && comments[i].toMap().contains("date_time")))
        {
            QVariantMap curr_comment = comments[i].toMap();

            //отрисовка комментария на графике
            //добавление бокса с текстом
//            QCPItemText *textItem = new QCPItemText(plot);
//            plot->addItem(textItem);
//            textItem->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
//            textItem->position->setType(QCPItemPosition::ptAxisRectRatio);
//            textItem->position->setCoords(x, y);
//            x += 0.1;
//            textItem->setText(curr_comment["comment"].toString());
//            //textItem->setFont(QFont(font().family(), 16));
//            textItem->setPen(QPen(Qt::black));
//            textItem->setVisible(curr_visibility);
//            items.push_back(textItem);
//            textItem->setParent(this);

//            // add the arrow:
//            QCPItemLine *arrow = new QCPItemLine(plot);
//            plot->addItem(arrow);
//            arrow->start->setParentAnchor(textItem->bottom);
//            QDateTime dtime = QDateTime::fromString(curr_comment["date_time"].toString(), Qt::ISODate);
//            arrow->end->setCoords(dtime.toMSecsSinceEpoch(), 0);
//            arrow->setHead(QCPLineEnding::esSpikeArrow);
//            arrow->setVisible(curr_visibility);
//            items.push_back(arrow);
//            arrow->setParent(this);
        }
    }
    emit updated();
    //setVisible(isVisible);
}
