#include "datavisualizerform.h"
#include "ui_datavisualizerform.h"

DataVisualizerForm::DataVisualizerForm(bool interactive, QSettings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataVisualizerForm)
{
    qsrand(QTime::currentTime().msecsSinceStartOfDay());
    this->interactive = interactive;

    //создание файла с настройками, если не указан главный файл настроек
    if(settings == 0)
    {
        this->settings = new QSettings("DataVisualizerSettings.ini",
                                       QSettings::IniFormat);
    }
    else
        this->settings = settings;

    ui->setupUi(this);

    CustomPlotZoom *zPlot = new CustomPlotZoom(this);
    zPlot->setZoomMode(true);
    plot = zPlot;
    ui->plotFrame->layout()->addWidget(plot);
    QSizePolicy pol;
    pol.setHorizontalPolicy(QSizePolicy::Expanding);
    pol.setVerticalPolicy(QSizePolicy::Expanding);
    plot->setSizePolicy(pol);

    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllEntries | QDir::NoDot);
    model->setResolveSymlinks(true);
    ui->fileBrowser->setModel(model);
    QAbstractItemDelegate *del = new CustomItemDelegate(model, &opened_files, this);
    ui->fileBrowser->setItemDelegate(del);


    timeAxis = plot->axisRect()->addAxis(QCPAxis::atBottom);
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(updateTimeAxis(QCPRange)),Qt::DirectConnection);
    timeAxis->setLabel(tr("Время"));
    timeAxis->setTickLabelType(QCPAxis::ltDateTime);
    timeAxis->setDateTimeFormat("hh:mm:ss");

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    connect(ui->graphButton, SIGNAL(clicked()), this, SLOT(change_mode()));
    connect(ui->histButton, SIGNAL(clicked()), this, SLOT(change_mode()));
    connect(ui->graphRelativeTimeButton, SIGNAL(clicked()),
            this, SLOT(change_mode()));
}

DataVisualizerForm::~DataVisualizerForm()
{
    delete ui;
}

void DataVisualizerForm::openDir(QString dir)
{
    ui->fileBrowser->setRootIndex(model->setRootPath(dir));
}

void DataVisualizerForm::visualizeFile(QString filepath)
{
    if(opened_files.contains(filepath))
    {
        opened_files[filepath]->setVisible(!opened_files[filepath]->visible(), getCurrentGraphMode());

        //проверка существования файла (на случай если hv файл еще не успел записаться)
        if(opened_files.contains(filepath))
            opened_files[filepath]->setMetaDataToTable();
        plot->rescaleAxes(true);
        plot->replot();
    }
}

void DataVisualizerForm::clear()
{
    if(opened_files.isEmpty())
        return;

    QMap<QString, FileDrawer*>::iterator it;
    for(it = opened_files.begin(); it != opened_files.end(); it++)
    {
        it.value()->deleteLater();
    }

    opened_files.clear();
    ui->metaTable->clearSelection();
    ui->metaTable->clearFocus();
    plot->clearGraphs();
    plot->clearItems();
    plot->clearPlottables();
    plot->replot();
}

GraphMode DataVisualizerForm::getCurrentGraphMode()
{
    if(ui->graphRelativeTimeButton->isChecked())
        return RELATIVE_TIME;
    if(ui->graphButton->isChecked())
        return ABSOLUTE_TIME;
    if(ui->histButton->isChecked())
        return HISTOGRAMM;
}

void DataVisualizerForm::updateTimeAxis(QCPRange range)
{
    GraphMode mode = getCurrentGraphMode();
    switch (mode)
    {
        case ABSOLUTE_TIME:
            timeAxis->setRange(range.lower / 1000., range.upper / 1000.);
            break;
        case RELATIVE_TIME:
            timeAxis->setRange(range.lower / qPow(10,9), range.upper / qPow(10,9));
            break;
        default:
            break;
    }
    //преобразование времени MSecsSinceEpoch во время QCustomPlot

}

void DataVisualizerForm::on_colorEditButton_clicked()
{
    QString dir = model->rootPath() + "/" +
                  ui->fileBrowser->currentIndex().data().toString();

    if(!opened_files.contains(dir))
        return;

    FileDrawer *fd = opened_files[dir];

    QColor new_color = QColorDialog::getColor(fd->getColor(), this,
                                              tr("выберите цвет"),
                                              QColorDialog::ShowAlphaChannel);
    fd->setColor(new_color);

    plot->replot();
}

void DataVisualizerForm::change_mode()
{
    QMap<QString, FileDrawer*>::iterator it;

    GraphMode currGraphMode = getCurrentGraphMode();

    switch (currGraphMode)
    {
        case RELATIVE_TIME:
            timeAxis->setDateTimeFormat("mm:ss");
            timeAxis->setVisible(true);
            break;

        case ABSOLUTE_TIME:
            timeAxis->setDateTimeFormat("hh:mm:ss");
            timeAxis->setVisible(true);
            break;

        case HISTOGRAMM:
            timeAxis->setVisible(false);
            break;
    }

    for(it = opened_files.begin(); it != opened_files.end(); it++)
        it.value()->setVisible(it.value()->visible(), currGraphMode);

    plot->rescaleAxes(true);
    plot->replot();
}

FileDrawer::FileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent) : QObject(parent)
{
    this->table = table;
    this->plot = plot;

    isVisible = false;
    color = getRandomColor();

    file = new QFile(filename, this);
    file->open(QIODevice::ReadOnly);
    TcpProtocol::parceMessage(file->readAll(), meta, data, 1);
    file->close();
    file->open(QIODevice::ReadOnly);

    connect(this, SIGNAL(updated()), plot, SLOT(replot()));
}

FileDrawer::~FileDrawer()
{
    file->close();
}

void FileDrawer::setMetaTableText(int col, int row, QString text)
{
    table->setItem(row,col, new QTableWidgetItem());
    table->item(row,col)->setText(text);
}

InfoFileDrawer::InfoFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    file->close();
    update();
}

InfoFileDrawer::~InfoFileDrawer()
{
    for(int i = 0 ; i <  items.size(); i++)
        plot->removeItem(items[i]);
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
            setMetaTableText(1, table->rowCount() - 1, QJsonDocument::fromVariant(comments[i]).toJson());
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
    for(int i = 0 ; i < items.size(); i++)
    {
        plot->removeItem(items[i]);
    }
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
            QCPItemText *textItem = new QCPItemText(plot);
            plot->addItem(textItem);
            textItem->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            textItem->position->setType(QCPItemPosition::ptAxisRectRatio);
            textItem->position->setCoords(x, y);
            x += 0.1;
            textItem->setText(curr_comment["comment"].toString());
            //textItem->setFont(QFont(font().family(), 16));
            textItem->setPen(QPen(Qt::black));
            textItem->setVisible(curr_visibility);
            items.push_back(textItem);

            // add the arrow:
            QCPItemLine *arrow = new QCPItemLine(plot);
            plot->addItem(arrow);
            arrow->start->setParentAnchor(textItem->bottom);
            QDateTime dtime = QDateTime::fromString(curr_comment["date_time"].toString(), Qt::ISODate);
            arrow->end->setCoords(dtime.toMSecsSinceEpoch(), 0);
            arrow->setHead(QCPLineEnding::esSpikeArrow);
            arrow->setVisible(curr_visibility);
            items.push_back(arrow);
        }
    }
    emit updated();
    //setVisible(isVisible);
}

PointFileDrawer::PointFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    loaded = 0;
    update();
}

PointFileDrawer::~PointFileDrawer()
{
    for(int i = 0; i < graph_absolute.size(); i++)
    {
        plot->removePlottable(graph_absolute[i]);
        plot->removePlottable(graph_relative[i]);
        plot->removePlottable(bars[i]);
    }
}

void PointFileDrawer::setMetaDataToTable()
{
    //заполнение метаинформации
    table->clearContents();
    table->setRowCount(8);
    table->setColumnCount(2);
    setMetaTableText(0, 0, tr("тип файла"));
    setMetaTableText(1, 0, tr("точка"));
    setMetaTableText(0, 1, tr("дата сбора"));
    setMetaTableText(1, 1, meta["date"].toString());
    setMetaTableText(0, 2, tr("время начала сбора"));
    setMetaTableText(1, 2, meta["start_time"].toString());
    setMetaTableText(0, 3, tr("время конца сбора"));
    setMetaTableText(1, 3, meta["end_time"].toString());
    setMetaTableText(0, 4, tr("количество событий"));
    setMetaTableText(1, 4, meta["total_events"].toString());
    setMetaTableText(0, 5, tr("время сбора"));
    setMetaTableText(1, 5, meta["external_meta"].toMap()
                           ["acquisition_time"].toString());
    setMetaTableText(0, 6, tr("установленное напряжение на блоке 1"));
    setMetaTableText(1, 6, meta["external_meta"].toMap()
                           ["HV1_value"].toString());
    setMetaTableText(0, 7, tr("установленное напряжение на блоке 2"));
    setMetaTableText(1, 7, meta["external_meta"].toMap()
                           ["HV2_value"].toString());
    table->resizeColumnsToContents();
}

void PointFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    //скрытие всех графиков
    for(int i = 0; i < graph_absolute.size(); i++)
        graph_absolute[i]->setVisible(false);
    for(int i = 0; i < graph_relative.size(); i++)
        graph_relative[i]->setVisible(false);
    for(int i = 0; i < bars.size(); i++)
        bars[i]->setVisible(false);


    //показ нужного графика
    switch (graphMode)
    {
        case ABSOLUTE_TIME:
        {
            for(int i = 0; i < graph_absolute.size(); i++)
                graph_absolute[i]->setVisible(visible);
            break;
        }
        case RELATIVE_TIME:
        {
            for(int i = 0; i < graph_relative.size(); i++)
                graph_relative[i]->setVisible(visible);
            break;
        }
        case HISTOGRAMM:
        {
            for(int i = 0; i < bars.size(); i++)
                bars[i]->setVisible(visible);
        }
        default:
            break;
    }

    isVisible = visible;
}

void PointFileDrawer::setColor(QColor color)
{
    this->color = color;

    for(int i = 0; i < graph_absolute.size(); i++)
        graph_absolute[i]->setPen(color);

    for(int i = 0; i < graph_relative.size(); i++)
        graph_relative[i]->setPen(color);

    for(int i = 0; i < bars.size(); i++)
        bars[i]->setBrush(color);

}

QMap<int, unsigned short> PointFileDrawer::aviableMeasureTimes = TcpProtocol::getAviableMeasuteTimes();

void PointFileDrawer::update()
{
    //файл рисуется только один раз, т.к. он не обновляется
    QByteArray rawData = file->readAll();

    if(!loaded)
    {
        loaded = 1;

        QVariantMap meta;
        QVector<Event> events;

        //парсинг сообщения
        if(!TcpProtocol::parceMessageWithPoints(rawData, meta, events) || events.isEmpty())
        {
            loaded = 0;
            return;
        }

        //преобразование формата для QCustomPlot
        QVector<double> time(events.size());
        QVector<double> time_abs(events.size());
        QVector<double> event_data(events.size());

        //получение абсолютного времени из метаданных
        QDate date = QDate::fromString(meta["date"].toString(), "yyyy.MM.dd");
        QTime start_time = QTime::fromString(meta["start_time"].toString(), "hh:mm:ss.zzz");
        QTime end_time = QTime::fromString(meta["end_time"].toString(), "hh:mm:ss.zzz");

        QDateTime start_datetime(date, start_time);
        QDateTime end_datetime(date, end_time);

        int acquisitionTime = 5;
        //попытка получить точное время из метаданных
        if(meta["external_meta"].toMap().contains("acquisition_time"))
        {
            acquisitionTime = meta["external_meta"].toMap()["acquisition_time"].toInt();
            acquisitionTime = aviableMeasureTimes.lowerBound(acquisitionTime).key();
        }
        else
        {
            double raw_time = (double)(end_datetime.toMSecsSinceEpoch()
                                       - start_datetime.toMSecsSinceEpoch())/1000.;
            acquisitionTime = aviableMeasureTimes.lowerBound(raw_time + 1).key();
        }

        //коэффициент нормирования относительного времени
        double coeff = TcpProtocol::madsTimeToNSecCoeff(acquisitionTime);
        //коэффициент перевода относительного времени в миллисекунды
        double coeff_abs = coeff/qPow(10,6);

        //создание графиков с абсолютным и относительным временем
        for(int i =0; i< events.size(); i++)
        {
            time[i] = coeff*events[i].time;
            time_abs[i] = (double)start_datetime.toMSecsSinceEpoch()
                                         + coeff_abs*events[i].time;
            event_data[i] = events[i].data;
            //вычисление абсолютного времени
        }

        graph_absolute.push_back(plot->addGraph());
        graph_absolute.last()->addData(time_abs, event_data);
        graph_absolute.last()->setVisible(false);
        graph_absolute.last()->setPen(QPen(color));
        graph_absolute.last()->setLineStyle(QCPGraph::lsNone);
        graph_absolute.last()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        graph_relative.push_back(plot->addGraph());
        graph_relative.last()->addData(time, event_data);
        graph_relative.last()->setVisible(false);
        graph_relative.last()->setPen(QPen(color));
        graph_relative.last()->setLineStyle(QCPGraph::lsNone);
        graph_relative.last()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

        //создание гистограммы
        int max_size = 4096;
        int bins = 128;

        float diff = ((float)bins / (double)max_size);
        QVector<double> hist(bins);
        QVector<double> bin(bins);

        for(int i = 0; i < events.size(); i++)
        {
            hist[qMin((int)((double)events[i].data * diff), max_size - 1)]++;
        }

        for (int i = 0; i < bins; i++)
            bin[i] = (((double)i + 0.5) / (double)bins) * (double)max_size;

        bars.push_back(new QCPBars(plot->xAxis, plot->yAxis));
        plot->addPlottable(bars.last());
        bars.last()->setPen(Qt::NoPen);
        bars.last()->setWidth((1. / (double)bins) * (double)max_size);
        bars.last()->setData(bin, hist);
        bars.last()->setVisible(false);
        bars.last()->setBrush(QBrush(color));
    }

    emit updated();
}

QColor FileDrawer::getRandomColor()
{
    // this are the numbers of the QT default colors
    static int QtColours[]= { 3, 2, 7, 13, 8, 14, 9,
                              15, 10, 16, 11, 17, 12,
                              18, 5, 4, 6, 19, 0, 1 };


    QColor color(QtColours[qrand() % sizeof(QtColours)]);
    color.setAlpha(150);

#ifdef TEST_MODE
    qDebug() << color;
#endif

    return color;
}

VoltageFileDrawer::VoltageFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    graph_block_1 = plot->addGraph();

    graph_block_1->setName(tr("напряжение блок 1"));
    graph_block_2 = plot->addGraph();

    graph_block_2->setName(tr("напряжение блок 2"));

    setColor(color);
    update();
}

VoltageFileDrawer::~VoltageFileDrawer()
{
    plot->removeGraph(graph_block_1);
    plot->removeGraph(graph_block_2);
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

CustomItemDelegate::CustomItemDelegate(QFileSystemModel *model, QMap<QString, FileDrawer *> *opened_files,
                                       QObject *parent): QStyledItemDelegate(parent)
{
    this->model = model;
    this->opened_files = opened_files;
}

void CustomItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QRect rect = opt.rect;
    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    QString filepath = model->rootPath() + "/" + index.data().toString();

    if(opened_files->contains(filepath))
    {
        FileDrawer *currFileDrawer = (*opened_files)[filepath];

        if(currFileDrawer->visible())
            painter->fillRect(QRect(rect.x() + rect.height(), rect.y() , rect.width() - rect.height(), rect.height()), Qt::gray);
        else
            painter->fillRect(QRect(rect.x() + rect.height(), rect.y() , rect.width() - rect.height(), rect.height()), Qt::white);

        painter->fillRect(QRect(rect.x(), rect.y(), rect.height(), rect.height()), currFileDrawer->getColor());
    }

    if(opt.state & QStyle::State_Selected)
    {
        painter->drawRect(QRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2));
    }

    painter->drawText(QRect(rect.x() + rect.height(), rect.y() , rect.width() - rect.height(), rect.height()),
                      opt.displayAlignment, index.data().toString());
}

void DataVisualizerForm::on_fileBrowser_doubleClicked(const QModelIndex &index)
{
    QString filepath = model->rootPath() + "/" + index.data().toString();

    QFileInfo fileInfo(filepath);

    if(fileInfo.isDir())
        openDir(filepath);
    else
        visualizeFile(filepath);
}

void DataVisualizerForm::on_fileBrowser_clicked(const QModelIndex &index)
{
    QString filepath = model->rootPath() + "/" + index.data().toString();

    if(!(opened_files.contains(filepath)))
    {
        //определение APD файла
        /// \todo Изменить проверку на APD файл.
        if(index.data().toString() == "APD_126_10_ev")
        {
            APDFileDrawer *apdfd = new APDFileDrawer(ui->metaTable, plot, filepath, this);
            opened_files[filepath] = apdfd;
        }
        else
        {
            //определение типа файла
            QFile file(filepath);
            file.open(QIODevice::ReadOnly);
            QByteArray fileData = file.readAll();
            file.close();

            QVariantMap meta;
            QByteArray data;
            if(TcpProtocol::parceMessage(fileData, meta, data, 1))
            {
                //обработка точки
                if(meta["reply_type"].toString() == "aquired_point")
                {
                    PointFileDrawer *pfd = new PointFileDrawer(ui->metaTable, plot, filepath, this);
                    opened_files[filepath] = pfd;
                }
                else
                    if(meta["type"] == "info_file")
                    {
                        InfoFileDrawer *ifd = new InfoFileDrawer(ui->metaTable, plot, filepath, this);
                        opened_files[filepath] = ifd;

                        //настройка автообновления файла
                        if(interactive)
                        {
                            QTimer *timer = new QTimer(ifd);
                            connect(timer, SIGNAL(timeout()), ifd, SLOT(update()), Qt::QueuedConnection);
                            timer->start(5000); // в настройки
                        }
                    }
                    else
                        if(meta["type"] == "voltage")
                        {
                            VoltageFileDrawer *vfd = new VoltageFileDrawer(ui->metaTable, plot, filepath, this);
                            opened_files[filepath] = vfd;
                            if(interactive)
                            {
                                QTimer *timer = new QTimer(vfd);
                                connect(timer, SIGNAL(timeout()), vfd, SLOT(update()), Qt::QueuedConnection);
                                timer->start(5000); // в настройки
                            }
                        }
                        else
                            //файл не подошел ни под один
                            //из распознаемых типов
                            return;
            }
        }
    }

    //проверка существования файла (на случай если hv файл еще не успел записаться)
    if(opened_files.contains(filepath))
        opened_files[filepath]->setMetaDataToTable();
    return;
}


APDFileDrawer::APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
: FileDrawer(table, plot, filename, parent)
{
    loaded = 0;

    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(drawPart(QCPRange)));

    update();
}

APDFileDrawer::~APDFileDrawer()
{
    plot->removeGraph(graph);
}

void APDFileDrawer::setMetaDataToTable()
{
    table->clearContents();
    table->setRowCount(1);
    table->setColumnCount(2);
    setMetaTableText(0, 0, tr("тип файла"));
    setMetaTableText(1, 0, tr("файл APD"));
}

void APDFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    isVisible = visible;
    switch(graphMode)
    {
        case RELATIVE_TIME:
            graph->setVisible(visible);
            break;
        default:
            graph->setVisible(false);
            break;
    }
}

void APDFileDrawer::setColor(QColor color)
{
    this->color = color;
    graph->setPen(QPen(color));
}

void APDFileDrawer::update()
{
    //считывается только один раз
    if(!loaded)
    {
        //парсинг файла
        QTextStream ts(file);

        QString buf;
        while(!ts.atEnd())
        {
            //считывание времени
            ts >> buf;
            time.push_back(buf.toInt());

            //считывание амплитуды
            ts >> buf;
            val.push_back(buf.toInt());

            //пропуск интервала
            ts >> buf;

            //считывание ширины
            ts >> buf;
            width.push_back(buf.toInt());
        }

        graph = plot->addGraph();
        graph->setPen(QPen(color));
        graph->setLineStyle(QCPGraph::lsStepLeft);
    }
}

void APDFileDrawer::drawPart(QCPRange range)
{
    if(!time.size())
        return;

    //определение максимального количества точек, которые могут быть отображены на экране
    int maxPoints = plot->width();


    int min = range.lower;
    int max = range.upper;

    //получение индекса точек, попадающих в границу
    std::vector<int>::iterator itMin = std::lower_bound(time.begin(), time.end(), min);
    std::vector<int>::iterator itMax = std::upper_bound(time.begin(), time.end(), max);

    int minInd = itMin - time.begin();
    int maxInd = itMax - time.begin();

    int step = qMax(1, (maxInd - minInd)/maxPoints);

    QVector<double> x, y;

    x.push_back(0);
    y.push_back(0);
    for(int i = minInd; i < maxInd; i+= step)
    {
        x.push_back(time[i]);
        y.push_back(val[i]);

        x.push_back(time[i] + width[i]);
        y.push_back(0);
    }
    x.push_back(time[time.size() - 1]);
    y.push_back(0);

    graph->setData(x, y);
    plot->replot();
}
