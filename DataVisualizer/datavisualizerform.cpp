#include "datavisualizerform.h"
#include <ui_DataVisualizerForm.h>
#include <QFileDialog>

#include <QTimer>

DataVisualizerForm::DataVisualizerForm(bool interactive, QSettings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataVisualizerForm)
{
    qsrand(QTime::currentTime().second());
    this->interactive = interactive;

    //создание файла с настройками, если не указан главный файл настроек
    if(settings == 0)
    {
        this->settings = new QSettings("DataVisualizerSettings.ini",
                                       QSettings::IniFormat);
    }
    else
        this->settings = settings;

    //считывание настроек
    settings->beginGroup(metaObject()->className());
    bool hide_abs_time = settings->value("hide_abs_time", false).toBool();
    settings->endGroup();

    ui->setupUi(this);

    ui->metaTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


#ifndef APD_MODE
    ui->saveGraphs->setVisible(false);
#endif

    CustomPlotZoom *zPlot = new CustomPlotZoom(this);
    zPlot->setZoomMode(true);
    plot = zPlot;
    ui->plotFrame->layout()->addWidget(plot);
    QSizePolicy pol;
    pol.setHorizontalPolicy(QSizePolicy::Expanding);
    pol.setVerticalPolicy(QSizePolicy::Expanding);
    plot->setSizePolicy(pol);

    ruler = new Ruler(this, plot, this);

    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
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

    if(hide_abs_time)
    {
        ui->graphRelativeTimeButton->setChecked(true);
        ui->graphButton->setEnabled(false);
        ui->graphButton->setVisible(false);
        change_mode();
    }
}

DataVisualizerForm::~DataVisualizerForm()
{
    delete ui;
}

void DataVisualizerForm::openDir(QString dir)
{
    ui->fileBrowser->setRootIndex(model->setRootPath(dir));
    //resizeColumnsToContents(ui->fileBrowser);
}

void DataVisualizerForm::visualizeFile(QString filepath)
{
    if(opened_files.contains(filepath))
    {
        opened_files[filepath]->setVisible(!opened_files[filepath]->visible(), getCurrentGraphMode());

        //проверка существования файла (на случай если hv файл еще не успел записаться)
        plot->rescaleAxes(true);
        plot->replot();
    }
}

void DataVisualizerForm::clear()
{
    clearText();
    ui->metaTable->clear();

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

void DataVisualizerForm::resizeColumnsToContents(QTreeWidget *treeWidget_)
{
    int cCols = treeWidget_->columnCount();
    int cItems = treeWidget_->topLevelItemCount();
    int w;
    int col;
    int i;
    for( col = 0; col < cCols; col++ ) {
        w = treeWidget_->header()->sectionSizeHint( col );
        for( i = 0; i < cItems; i++ )
            w = qMax( w, treeWidget_->topLevelItem( i )->text( col ).size()*7 + (col == 0 ? treeWidget_->indentation() : 0) );
        treeWidget_->header()->resizeSection( col, w );
    }
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
    QString dir = model->filePath(ui->fileBrowser->currentIndex());

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

CustomItemDelegate::CustomItemDelegate(QFileSystemModel *model, QMap<QString, FileDrawer *> *opened_files,
                                       QObject *parent): QStyledItemDelegate(parent)
{
    this->model = model;
    this->opened_files = opened_files;
}

void CustomItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.data().toString() != model->fileName(index))
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }


    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QRect rect = opt.rect;
    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    QString filepath = model->filePath(index);

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
        painter->setPen(QPen(QBrush(Qt::black), 3));
        painter->drawRect(QRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, rect.height() - 2));
        painter->setPen(QPen());
    }

    painter->drawText(QRect(rect.x() + rect.height(), rect.y() , rect.width() - rect.height(), rect.height()),
                      opt.displayAlignment, index.data().toString());
}

void DataVisualizerForm::clearText()
{
    curr_info.clear();
    ui->infoLabel->clear();
}

void DataVisualizerForm::updateEventsInfo(QCPRange range)
{
    clearText();

    QMap<QString, FileDrawer*>::iterator it;

    for(it = opened_files.begin(); it != opened_files.end(); it++)
    {
        it.value()->sendHistEventsInWindow(range);
    }
}

void DataVisualizerForm::on_fileBrowser_doubleClicked(const QModelIndex &index)
{
    clearText();

    QString filepath = model->filePath(index);

    QFileInfo fileInfo(filepath);

    if(fileInfo.isDir())
    {
        //openDir(filepath);
    }
    else
        visualizeFile(filepath);
}

void DataVisualizerForm::on_fileBrowser_clicked(const QModelIndex &index)
{
    ui->metaTable->clear();

    QString filepath = model->filePath(index);

    if(!(opened_files.contains(filepath)))
    {
        //определение APD файла
        QFileInfo fileInfo(filepath);
        if((QFile::exists(fileInfo.absolutePath() + "/setup_raw") ||
           QFile::exists(fileInfo.absolutePath() + "/setup") ||
           QFile::exists(fileInfo.absolutePath() + "/block_stat")) &&
           (fileInfo.baseName() != "setup_raw") &&
           (fileInfo.baseName() != "setup") &&
           (fileInfo.baseName() != "block_stat") &&
            fileInfo.suffix().isEmpty() && //Проверка расширения файла
            !fileInfo.fileName().contains("amplHist")) //Файл не сгененрирован автоматически.
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
    {
        opened_files[filepath]->setMetaDataToTable();

        connect(opened_files[filepath], SIGNAL(sendTextInfo(QString,QString)),
                this, SLOT(updateText(QString,QString)), Qt::UniqueConnection);
    }
    return;
}

#ifdef APD_MODE
void DataVisualizerForm::on_saveGraphs_clicked()
{
    QString dir = settings->value("lastSavedFile").toString();
    dir = QFileDialog::getSaveFileName(this, tr("Выберите файл для сохранения"), dir);

    if(dir.isEmpty())
        return;

    settings->setValue("lastSavedFile", dir);

    QStringList names;
    QVector<std::vector<quint64> > times;
    QVector<QVector<int> > values;
    QVector<QPair<QVector<double>, QVector<double> > > amplitudes;

    //загрузка открытых значений
    for(QMap<QString, FileDrawer*>::iterator it = opened_files.begin(); it!= opened_files.end(); it++)
    {
        bool visible = it.value()->visible();
        QString className = it.value()->metaObject()->className();
        if(visible && className == "APDFileDrawer")
        {
            names.push_back(it.key());
            times.push_back(((APDFileDrawer*)(it.value()))->getTime());
            values.push_back(((APDFileDrawer*)(it.value()))->getVal());
            amplitudes.push_back(((APDFileDrawer*)(it.value()))->getAmplHistValues());
        }
    }

    //создание метаданных
    QVariantMap meta;
    meta["type"] = "APDGeneratedFile";

    for(int i = 0; i < names.size(); i++)
        meta[tr("batchAt%1").arg(i)] = QFileInfo(names[i]).fileName();


    //создание данных файла гистограммы
    QByteArray histFileData;
    int i = 0;
    for(bool end = 0; !end; i++)
    {
        bool haveElement = false;

        for(int j = 0; j < names.size(); j++)
        {
            if(j == 0 && i < amplitudes[0].first.size())
                histFileData += tr("%1\t").arg(amplitudes[0].second[i]).toLatin1();

            if(i < amplitudes[j].first.size())
            {
                haveElement = true;
                histFileData += tr("%1").arg(amplitudes[j].first[i]).toLatin1();

                if(j != names.size() - 1)
                  histFileData += "\t";
            }
        }

        histFileData += "\n";

        if(!haveElement)
            end = 1;
    }

    meta["fileType"] = "histogramms";
    histFileData = TcpProtocol::createMessage(meta, histFileData);

    //запись в файл
    QFile histFile(dir + "_hist");
    histFile.open(QIODevice::WriteOnly);
    histFile.write(histFileData);
    histFile.close();

    //создание данных файла амплитуд
    //сортировка данных
    QMap<quint64, QVector<qint64> > sortedValues;

    for(int j = 0; j < names.size(); j++)
    {
        for(int i = 0; i < times[j].size(); i++ )
        {
            if(!sortedValues.contains(times[j][i]))
               sortedValues[times[j][i]] = QVector<qint64>(names.size());

            sortedValues[times[j][i]][j] = values[j][i];
        }
    }

    //вывод значений
    QByteArray amplFileData;
    for(QMap<quint64, QVector<qint64> >::iterator it = sortedValues.begin(); it != sortedValues.end(); it++)
    {
        amplFileData += tr("%1\t").arg(it.key()).toLatin1();
        int size = it.value().size();
        for(int k = 0; k < size; k++)
        {
            if(it.value()[k] == 0)
                amplFileData += " ";
            else
                amplFileData += tr("%1").arg(it.value()[k]).toLatin1();

            if(k != size - 1)
              amplFileData += "\t";
        }

        amplFileData += "\n";
    }

    meta["fileType"] = "time";
    amplFileData = TcpProtocol::createMessage(meta, amplFileData);

    //запись в файл
    QFile amplFile(dir + "_time");
    amplFile.open(QIODevice::WriteOnly);
    amplFile.write(amplFileData);
    amplFile.close();
}
#endif

void DataVisualizerForm::updateText(QString sender, QString info)
{
    curr_info[sender] = info;

    QString text;
    for(QMap<QString, QString>::iterator it = curr_info.begin(); it != curr_info.end(); it++)
        text += tr("%1: %2\n").arg(it.key(), it.value());

    ui->infoLabel->setText(text);
}

Ruler::Ruler(DataVisualizerForm *form, QCustomPlot *plot, QObject *parent) : QObject(parent)
{
    this->form = form;
    this->plot = plot;
    x1 = 0;
    x2 = 0;
    y = 0;

    mouseJustMovedFlag = 0;

    text = 0;
    graph = 0;

    connect(plot, SIGNAL(mouseRelease(QMouseEvent*)),
            this, SLOT(processMouseRelease(QMouseEvent*)));

    connect(plot, SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(processMouseMove(QMouseEvent*)));
}

void Ruler::processMouseMove(QMouseEvent *ev)
{
    if((ev->buttons() & Qt::RightButton) ||
       (ev->buttons() & Qt::LeftButton))
    {
        mouseJustMovedFlag = 1;
        clearRuler();
    }

}

void Ruler::clearRuler()
{
    x1 = 0;
    x2 = 0;
    y = 0;

    if(text)
    {
        plot->removeItem(text);
        text = 0;
    }

    if(graph)
    {
        graph->removeDataAfter(0);
    }

    plot->replot();
}

void Ruler::processMouseRelease(QMouseEvent *ev)
{
    if((ev->button() == Qt::RightButton) || mouseJustMovedFlag)
    {
        mouseJustMovedFlag = 0;
        clearRuler();
    }
    else
        if(ev->button() == Qt::LeftButton)
        {
            if(!graph)
            {
                graph = plot->addGraph();
                graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
                connect(graph, SIGNAL(destroyed()), this, SLOT(onGraphDestroyed()));
            }

            if(x1 == 0)
            {
                clearRuler();

                y = plot->yAxis->pixelToCoord(ev->pos().y());
                x1 = plot->xAxis->pixelToCoord(ev->pos().x());

                graph->addData(x1, y);
                plot->replot();
            }
            else
                if(x2 == 0)
                {
                    x2 = plot->xAxis->pixelToCoord(ev->pos().x());

                    graph->addData(x2, y);

                    if(text)
                    {
                        plot->removeItem(text);
                        text = 0;
                    }

                    switch(form->getCurrentGraphMode())
                    {
                        case ABSOLUTE_TIME:
                        case RELATIVE_TIME:
                        {
                            text = new QCPItemText(plot);
                            connect(text, SIGNAL(destroyed()), this, SLOT(onTextDestroyed()));

                            plot->addItem(text);

                            text->setText(tr("\n%1 нс").arg(qAbs(x2 - x1)));
                            text->position->setCoords((x2 + x1) / 2., y);

                            break;
                        }

                        case HISTOGRAMM:
                        {
                            text = new QCPItemText(plot);
                            connect(text, SIGNAL(destroyed()), this, SLOT(onTextDestroyed()));

                            plot->addItem(text);

                            text->setText(tr("\n Окно обновлено: (%1, %2)").arg(qMin(x1, x2)).arg(qMax(x1, x2)));
                            text->position->setCoords((x2 + x1) / 2., y);

                            form->updateEventsInfo(QCPRange(qMin(x1, x2), qMax(x1, x2)));

                            break;
                        }
                    }



                    plot->replot();
                }
                else
                {
                    x1 = 0;
                    x2 = 0;
                    y = 0;
                    processMouseRelease(ev);
                }
        }
}

void Ruler::onGraphDestroyed()
{
    graph = 0;
}

void Ruler::onTextDestroyed()
{
    text = 0;
}
