#include "apdfiledrawer.h"

APDFileDrawer::APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QSettings *settings, QObject *parent)
: FileDrawer(table, plot, filename, parent, false)
{
    this->settings = settings;

    settings->beginGroup(metaObject()->className());
    if(!settings->contains("BATCH"))
        settings->setValue("BATCH", 1000);
    BATCH = settings->value("BATCH").toInt();
    if(!settings->contains("MAX_EVENTS"))
        settings->setValue("MAX_EVENTS", 2000);
    MAX_EVENTS = settings->value("MAX_EVENTS").toInt();
    if(!settings->contains("HIST_FLUSH_STEP"))
        settings->setValue("HIST_FLUSH_STEP", 10000);
    HIST_FLUSH_STEP = settings->value("HIST_FLUSH_STEP").toInt();
    if(!settings->contains("HIST_MIN_VAL"))
        settings->setValue("HIST_MIN_VAL", 0);
    HIST_MIN_VAL = settings->value("HIST_MIN_VAL").toDouble();
    if(!settings->contains("HIST_MAX_VAL"))
        settings->setValue("HIST_MAX_VAL", 50);
    HIST_MAX_VAL = settings->value("HIST_MAX_VAL").toDouble();
    if(!settings->contains("HIST_MIN_INTER_VAL"))
        settings->setValue("HIST_MIN_INTER_VAL", 0);
    HIST_MIN_INTER_VAL = settings->value("HIST_MIN_INTER_VAL").toDouble();
    if(!settings->contains("HIST_MAX_INTER_VAL"))
        settings->setValue("HIST_MAX_INTER_VAL", 1000000);
    HIST_MAX_INTER_VAL = settings->value("HIST_MAX_INTER_VAL").toDouble();
    settings->endGroup();

    loaded = 0;
    binFile = NULL;

    //загрузка метаданных файла
    loadMetaData();

    histSetWidget = 0;
    graphSetWidget = 0;

    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(drawPart(QCPRange)));

    update();
}

APDFileDrawer::~APDFileDrawer()
{
}

void APDFileDrawer::createHistModeChangeButtons(int col, int row)
{
    setMetaTableText(0, row, tr("Показывать гистограмму по"));

    //создание кнопок выбора гистограммы
    if(histSetWidget)
    {
        delete histSetWidget;
        histSetWidget = 0;
    }


    histSetWidget = new QWidget(table);
    QHBoxLayout *layout = new QHBoxLayout(histSetWidget);
    histSetWidget->setLayout(layout);
    histSetButtons[0] = new QRadioButton("амплитуде", histSetWidget);
    histSetButtons[1] = new QRadioButton("интервалу", histSetWidget);
    connect(histSetButtons[0], SIGNAL(pressed()), this, SLOT(changeHistType()));
    connect(histSetButtons[1], SIGNAL(pressed()), this, SLOT(changeHistType()));
    layout->addWidget(histSetButtons[0]);
    layout->addWidget(histSetButtons[1]);

    connect(histSetWidget, SIGNAL(destroyed(QObject*)),
            this, SLOT(fullDeleteHistTab()), Qt::DirectConnection);

    switch (APDFileDrawer::histType)
    {
        case AMPLITUDE:
            histSetButtons[0]->setChecked(true);
            break;
        case INTERVAL:
            histSetButtons[1]->setChecked(true);
            break;
    }
    table->setCellWidget(col, row, histSetWidget);
}

void APDFileDrawer::createGraphModeChangeButtons(int col, int row)
{
    setMetaTableText(0, col, tr("Строить "));

    //создание кнопок выбора гистограммы
    if(graphSetWidget)
    {
        delete graphSetWidget;
        graphSetWidget = 0;
    }

    graphSetWidget = new QWidget(table);
    QHBoxLayout *layout = new QHBoxLayout(graphSetWidget);
    graphSetWidget->setLayout(layout);
    graphSetButtons[0] = new QRadioButton("график счета", graphSetWidget);
    graphSetButtons[1] = new QRadioButton("двухплотный график", graphSetWidget);
    connect(graphSetButtons[0], SIGNAL(pressed()), this, SLOT(changeGraphType()));
    connect(graphSetButtons[1], SIGNAL(pressed()), this, SLOT(changeGraphType()));
    layout->addWidget(graphSetButtons[0]);
    layout->addWidget(graphSetButtons[1]);

    graphSetWidget->show();

    connect(graphSetWidget, SIGNAL(destroyed(QObject*)),
            this, SLOT(fullDeleteGraphTab()), Qt::DirectConnection);

    switch (APDFileDrawer::graphType)
    {
        case COUNT:
            graphSetButtons[0]->setChecked(true);
            break;
        case PLOT:
            graphSetButtons[1]->setChecked(true);
            break;
    }
    table->setCellWidget(col, row, graphSetWidget);
}

void APDFileDrawer::setMetaDataToTable()
{
    table->clearContents();

    table->setColumnCount(2);
    if(meta.isEmpty())
        table->setRowCount(4);
    else
    {
        table->setRowCount(10);
        int i = 4;
        setMetaTableText(0, i, tr("Время начала сбора"));
        setMetaTableText(1, i, meta["timeStart"].toDateTime().toString());
        i++;
        setMetaTableText(0, i, tr("Время окончания сбора"));
        setMetaTableText(1, i, meta["timeStop"].toDateTime().toString());
        i++;
        setMetaTableText(0, i, tr("Шаг по времени"));
        setMetaTableText(1, i, tr("%1 нс").arg(meta["mainStep"].toDouble() / 10.));
        i++;
        setMetaTableText(0, i, tr("Отступ базовой линии"));
        setMetaTableText(1, i, tr("%1").arg(meta["baseLineOffset"].toDouble()));
        i++;
        setMetaTableText(0, i, tr("Коэффициент усиления"));
        setMetaTableText(1, i, tr("%1").arg(meta["amplifyCoef"].toDouble()));
        i++;
        setMetaTableText(0, i, tr("Порог"));
        setMetaTableText(1, i, tr("%1").arg(meta["threshold"].toDouble()));
    }

    setMetaTableText(0, 0, tr("тип файла"));
    setMetaTableText(1, 0, tr("файл APD"));

    createHistModeChangeButtons(1, 1);
    createGraphModeChangeButtons(2, 1);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();
}

void APDFileDrawer::setHistVisible(bool visible)
{
    switch (APDFileDrawer::histType)
    {
        case AMPLITUDE:
            amplHist->setVisible(visible);
            break;
        case INTERVAL:
            intervalHist->setVisible(visible);
            break;
    }
    plot->rescaleAxes();
}

void APDFileDrawer::setGraphVisible(bool visible)
{
    graphBorder->setVisible(visible);
    switch (APDFileDrawer::graphType)
    {
        case COUNT:
            graph->setVisible(visible);
            break;
        case PLOT:
            graph2->setVisible(visible);
            break;
    }

}

bool APDFileDrawer::getEventData(quint64 number, quint64 &time, short &ampl, uint &inter, short &w)
{
    static int stepSize = sizeof(quint64) + sizeof(inter) + 2*sizeof(short);

    if(binFile->size() < stepSize*(number + 1))
        return false;

    binFile->seek(stepSize*number);
    binFile->read((char*)(&time), sizeof(time));
    binFile->read((char*)(&ampl), sizeof(ampl));
    binFile->read((char*)(&inter), sizeof(inter));
    binFile->read((char*)(&w), sizeof(w));

    return true;
}



QVector<quint64> APDFileDrawer::getTime() const
{
    return timeMap;
}


void APDFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    isVisible = visible;

    graphBorder->setVisible(false);
    intervalHist->setVisible(false);
    amplHist->setVisible(false);
    graph->setVisible(false);
    graph2->setVisible(false);

    switch(graphMode)
    {
    case HISTOGRAMM:

        setHistVisible(visible);
        break;

    case RELATIVE_TIME:
        setGraphVisible(visible);
        break;

        default:
            break;
    }
}

void APDFileDrawer::setColor(QColor color)
{
    this->color = color;
    graph->setPen(QPen(color));
    graph2->setPen(QPen(color));

    amplHist->setPen(QPen(color));
    intervalHist->setPen(QPen(color));
}

void APDFileDrawer::update()
{
    //считывается только один раз
    if(!loaded)
    {
        double minAmpl = 0;
        double maxAmpl = 0;

        double minInterval = 0;
        double maxInterval = 0;

        //парсинг файла
        QTextStream ts(file);

        quint64 timeCoef = 0.1;
        if(meta.contains("mainStep"))
            timeCoef = meta["mainStep"].toDouble() / 10.;

        binFile = new QFile((QFileInfo(*file).absoluteFilePath() + ".tmp"), this);
        binFile->open(QIODevice::WriteOnly);

        //данные для гистограммы
        QVector<short> data;
        QVector<double> binVal;
        QVector<double> bin;
        QVector<uint> dataInter;
        QVector<double> interBinVal;
        QVector<double> interBin;

        quint64 step = 0;
        while(!ts.atEnd())
        {
            //получение значений из файла
            quint64 timeVal;
            uint inter;
            short ampl, w;
            ts >> timeVal >> ampl >> inter >> w;
            ampl = qAbs(ampl);

            bool end = false;
            if(!timeVal && timeMap.size())
                end = true;

            //запись данных в гистограмму
            //гистограмма амплитуд
            if(!(step%HIST_FLUSH_STEP) || end)
            {
                QVector<double> binStep;
                double minVal = HIST_MIN_VAL;
                double maxVal = HIST_MAX_VAL;
                generateHistFromData<short>(data, binStep, bin, minVal, maxVal);
                if(binVal.isEmpty())
                    binVal = binStep;
                else
                    for(int i = 0; i < binVal.size(); i++)
                        binVal[i] += binStep[i];
                data.clear();
            }
            //гистограмма интервалов
            if(!(step%HIST_FLUSH_STEP) || end)
            {
                QVector<double> binStep;
                double minVal = HIST_MIN_INTER_VAL;
                double maxVal = HIST_MAX_INTER_VAL;
                generateHistFromData<uint>(dataInter, binStep, interBin, minVal, maxVal);
                if(interBinVal.isEmpty())
                    interBinVal = binStep;
                else
                    for(int i = 0; i < binVal.size(); i++)
                        interBinVal[i] += binStep[i];
                dataInter.clear();
            }
            if(end) break;

            if(ampl < minAmpl)
                minAmpl = ampl;
            if(ampl > maxAmpl)
                maxAmpl = ampl;

            if(inter < minInterval)
                minInterval = inter;
            if(inter > maxInterval)
                maxInterval = inter;

            quint64 time_normalized = timeVal * timeCoef;
            if(!(step % BATCH))
                timeMap.push_back(time_normalized);
            step++;

            data.push_back(ampl);
            dataInter.push_back(inter);


            //запись значений бинарный файл
            short w_normalized = w * timeCoef;
            binFile->write((char*)(&time_normalized), sizeof(time_normalized));
            binFile->write((char*)(&ampl), sizeof(ampl));
            binFile->write((char*)(&inter), sizeof(inter));
            binFile->write((char*)(&w_normalized), sizeof(w_normalized));
        }

        binFile->close();
        binFile->open(QIODevice::ReadOnly);

        //создание графиков гистограмм
        amplHist = createGraphHistFromData(plot, binVal, bin);
        amplHist->setVisible(false);
        amplHist->setPen(QPen(color));
        amplHist->setLineStyle(QCPGraph::lsStepCenter);
        amplHist->setParent(this);
        plot->addPlottable(amplHist);

        intervalHist = createGraphHistFromData(plot, interBinVal, interBin);
        intervalHist->setVisible(false);
        intervalHist->setPen(QPen(color));
        intervalHist->setLineStyle(QCPGraph::lsStepCenter);
        intervalHist->setParent(this);
        plot->addPlottable(intervalHist);

        //создание двуплотных графиков
        graph = plot->addGraph();
        graph->setVisible(false);
        graph->setPen(QPen(color));
        graph->setLineStyle(QCPGraph::lsStepLeft);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));

        graph2 = plot->addGraph();
        graph2->setVisible(false);
        graph2->setPen(QPen(color));
        graph2->setLineStyle(QCPGraph::lsNone);
        graph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));

        graphBorder = plot->addGraph();
        graphBorder->setVisible(false);
        graphBorder->addData(0, minAmpl);
        graphBorder->addData(timeMap[timeMap.size() - 1], qMax((int)qAbs(minAmpl), (int)maxAmpl));
        graphBorder->setPen(QPen(QColor(0,0,0,0)));

        //установка родителей графикам
        graphBorder->setParent(this);
        graph->setParent(this);
        graphBorder->setParent(this);

        loaded = true;
    }
    emit updated();
}

void APDFileDrawer::drawPart(QCPRange range)
{
    if(!visible())
        return;

    if((QDateTime::currentMSecsSinceEpoch() - redrawLastTime) < 2000)
        return;
    redrawLastTime = QDateTime::currentMSecsSinceEpoch();

    double lower = range.lower;
    double upper = range.upper;

    int minBatchInd = 0;
    for(;(minBatchInd < timeMap.size()) &&
         (timeMap[minBatchInd] <= lower);
        minBatchInd++);

    int maxBatchInd = timeMap.size() - 1;
    for(;(maxBatchInd >= 0) &&
         (timeMap[maxBatchInd] >= upper);
        maxBatchInd--);

    minBatchInd = qMax(0, minBatchInd - 1);
    maxBatchInd = qMin(timeMap.size()-1, maxBatchInd + 1);

    int step = qMax(1, (int)((maxBatchInd - minBatchInd)*BATCH/MAX_EVENTS));

    QVector<double> times;
    QVector<double> amps;
    for(int i = minBatchInd*BATCH; i < maxBatchInd*BATCH; i += step)
    {
        quint64 time;
        uint inter;
        short ampl, w;

        if(getEventData(i, time, ampl, inter, w))
        {
            times.push_back(time);
            amps.push_back(ampl);
        }
        else
            break;
    }

    graph->setData(times, amps);
    graph2->setData(times, amps);
    plot->replot();

//    if(!time.size())
//        return;

//    sendHistEventsInWindow(range, amplHist);

//    sendHistEventsInWindow(range, intervalHist);

//    if(graph->visible() || graph2->visible())
//    {
//        //определение максимального количества точек, которые могут быть отображены на экране
//        int maxPoints = plot->width();

//        //получение индекса точек, попадающих в границу
//        quint64 minInd;
//        quint64 maxInd;


//        getMinMaxInd<std::vector<quint64> >(time, range.lower, range.upper, minInd, maxInd);

//        int step = qMax((quint64)1, (maxInd - minInd)/maxPoints);

//        QVector<double> x, y;

//        //закрашивание собвтий, если их мало
//        if(graph->visible())
//        {
//            ///\todo добавить порог по событиям в настройки
//            if(maxInd - minInd < 100)
//                graph->setBrush(QBrush(color, Qt::DiagCrossPattern));
//            else
//                graph->setBrush(QBrush());

//            for(quint64 i = minInd; i < maxInd; i+= step)
//            {
//                x.push_back(time[i]);
//                y.push_back(val[i]);

//                x.push_back(time[i] + width[i]);
//                y.push_back(0);
//            }

//            if(time.size() != 0)
//            {
//                x.push_front(0);
//                y.push_front(0);

//                x.push_back(time[time.size() - 1]);
//                y.push_back(val[time.size() - 1]);
//                x.push_back(time[time.size() - 1] + width[time.size() - 1]);
//                y.push_back(0);
//            }

//            graph->setData(x, y);
//            plot->replot();
//        }

//        if(graph2->visible())
//        {
//            for(quint64 i = minInd; i < maxInd; i+= step)
//            {
//                x.push_back(time[i]);
//                y.push_back(qAbs(val[i]));
//            }

//            graph2->setData(x, y);
//            plot->replot();
//        }

//        int eventsInWindow = (maxInd - minInd)/2;
//        emit sendTextInfo(QFileInfo(*file).filePath(),
//                          tr("Событий в окне: %1 (%2 показано, %3 скрыто)")
//                          .arg(eventsInWindow)
//                          .arg(eventsInWindow/step)
//                          .arg((int)(((double)eventsInWindow)*(1. - 1./(double)step))));
//    }
}

void APDFileDrawer::loadMetaData()
{
    QFile setupFile(QFileInfo(file->fileName()).absolutePath() + "/setup");
    if(setupFile.exists())
    {
        setupFile.open(QIODevice::ReadOnly);

        QString content(setupFile.readAll());
        QStringList contentList = content.split("\n", QString::SkipEmptyParts);

        for(int i = 0; i < contentList.size(); i++)
        {
            QString buf;

            if(contentList[i].contains("Station StationStart time: "))
            {
                buf = contentList[i].remove("Station StationStart time: ");
                meta["timeStart"] = QDateTime::fromString(buf);
                continue;
            }

            if(contentList[i].contains("Stop time: "))
            {
                buf = contentList[i].remove("Stop time: ");
                meta["timeStop"] = QDateTime::fromString(buf);
                continue;
            }

            if(contentList[i].contains("Threshold = "))
            {
                buf = contentList[i].remove("Threshold = ");
                meta["threshold"] = buf.toDouble();
                continue;
            }

            if(contentList[i].contains("Main step = "))
            {
                buf = contentList[i].remove("Main step = ");


                //перевод в одинаковые единицы измерения
                int coef = 1;
                if(buf.contains(" ns"))
                {
                    buf = buf.remove(" ns");
                    coef = 1;
                }
                if(buf.contains(" mks"))
                {
                    buf = buf.remove(" mks");
                    coef = 1000;
                }

                meta["mainStep"] = (buf.toDouble() * coef) * 10; //наносекунды * 10
                continue;
            }

            if(contentList[i].contains("K = "))
            {
                buf = contentList[i].remove("K = ");
                meta["amplifyCoef"] = buf.toDouble();
                continue;
            }

            if(contentList[i].contains("Base line offset = "))
            {
                buf = contentList[i].remove("Base line offset = ");
                meta["baseLineOffset"] = buf.toDouble();
                continue;
            }
        }

        setupFile.close();
    }
}

void APDFileDrawer::changeHistType()
{
    QEventLoop el;
    QTimer::singleShot(200, &el, SLOT(quit()));
    el.exec();

    if(histSetButtons[0]->isChecked())
        APDFileDrawer::histType = AMPLITUDE;
    if(histSetButtons[1]->isChecked())
        APDFileDrawer::histType = INTERVAL;
}

void APDFileDrawer::changeGraphType()
{
    QEventLoop el;
    QTimer::singleShot(200, &el, SLOT(quit()));
    el.exec();

    if(graphSetButtons[0]->isChecked())
        APDFileDrawer::graphType = COUNT;
    if(graphSetButtons[1]->isChecked())
        APDFileDrawer::graphType = PLOT;
}

void APDFileDrawer::fullDeleteHistTab()
{
    histSetWidget = 0;
}

void APDFileDrawer::fullDeleteGraphTab()
{
    graphSetWidget = 0;
}

APD_HIST_TYPE APDFileDrawer::histType = AMPLITUDE;
APD_GRAPH_TYPE APDFileDrawer::graphType = COUNT;
