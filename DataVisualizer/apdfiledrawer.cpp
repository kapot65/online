#include "apdfiledrawer.h"

APDFileDrawer::APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
: FileDrawer(table, plot, filename, parent)
{
    loaded = 0;

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


    histSetWidget = new QWidget();
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

    graphSetWidget = new QWidget();
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
            amplHist.hist->setVisible(visible);
            break;
        case INTERVAL:
            intervalHist.hist->setVisible(visible);
            break;
    }
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

QPair<QVector<double>, QVector<double> > APDFileDrawer::getAmplHistValues() const
{
    return amplHist.histValues;
}

QVector<int> APDFileDrawer::getVal() const
{
    return val;
}

std::vector<quint64> APDFileDrawer::getTime() const
{
    return time;
}


void APDFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    isVisible = visible;

    graphBorder->setVisible(false);
    intervalHist.hist->setVisible(false);
    amplHist.hist->setVisible(false);
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

    amplHist.hist->setPen(QPen(color));
    intervalHist.hist->setPen(QPen(color));
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

        while(!ts.atEnd())
        {
            //получение значений из файла
            quint64 timeVal;
            int inter, ampl, w;
            ts >> timeVal >> ampl >> inter >> w;

            if(!timeVal && time.size())
                break;

            if(ampl < minAmpl)
                minAmpl = ampl;
            if(ampl > maxAmpl)
                maxAmpl = ampl;

            if(inter < minInterval)
                minInterval = inter;
            if(inter > maxInterval)
                maxInterval = inter;

            //запись значений в вектора
            time.push_back(timeVal * timeCoef);
            val.push_back(ampl);
            interval.push_back(inter);
            width.push_back(w * timeCoef);
        }

        graph = plot->addGraph();
        graph->setPen(QPen(color));
        graph->setLineStyle(QCPGraph::lsStepLeft);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));

        graph2 = plot->addGraph();
        graph2->setPen(QPen(color));
        graph2->setLineStyle(QCPGraph::lsNone);
        graph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));

        graphBorder = plot->addGraph();
        graphBorder->addData(0, minAmpl);
        graphBorder->addData(time[time.size() - 1], qMax((int)qAbs(minAmpl), (int)maxAmpl));
        graphBorder->setPen(QPen(QColor(0,0,0,0)));

        minAmpl = -512;
        maxAmpl = 0;
        generateHistFromData(val, amplHist.histValues.first, amplHist.histValues.second,
                             minAmpl, maxAmpl,  qAbs(maxAmpl - minAmpl));

        amplHist.hist = createGraphHistFromData(plot, amplHist.histValues.first, amplHist.histValues.second, minAmpl, maxAmpl);
        amplHist.hist->setVisible(false);
        plot->addPlottable(amplHist.hist);


        generateHistFromData<int>(interval, intervalHist.histValues.first, intervalHist.histValues.second,
                                  minInterval, maxInterval, qMin(qMax(128, qAbs((int)maxInterval - (int)minInterval) / 4096), 8192));
        intervalHist.hist = createGraphHistFromData(plot, intervalHist.histValues.first, intervalHist.histValues.second,
                                          minInterval, maxInterval);
        intervalHist.hist->setVisible(false);
        plot->addPlottable(intervalHist.hist);

        //установка родителей графикам
        graph->setParent(this);
        graphBorder->setParent(this);
        amplHist.hist->setParent(this);
        intervalHist.hist->setParent(this);
    }
    emit updated();
}

template<typename T>
void APDFileDrawer::getMinMaxInd(T &vector, double min, double max,
                                 quint64 &minInd, quint64 &maxInd)
{
    minInd = 0;
    maxInd = vector.size() - 1;

    bool minFound = false;
    bool maxFound = false;
    for(quint64 i = 0; i < vector.size(); i++)
    {
        if(!minFound && vector[i] > min)
        {
            minFound = true;
            if(i == 0)
                minInd = 0;
            else
                minInd = i - 1;
        }

        if(!maxFound && vector[i] > max)
        {
            maxFound = true;
            maxInd = i;
            break;
        }
    }
}

void APDFileDrawer::sendHistEventsInWindow(QCPRange range, APDHist &apdHist)
{
    if(apdHist.hist->visible())
    {
        //подсчет количества событий в окне
        quint64 minInd;
        quint64 maxInd;

        getMinMaxInd<QVector<double> >(apdHist.histValues.second, range.lower, range.upper,
                                      minInd, maxInd);

        quint64 sum = 0;
        for(quint64 i = minInd; i < maxInd; i++)
            sum += apdHist.histValues.first[i];

        emit sendTextInfo(QFileInfo(*file).fileName(),
                          tr("Событий в окне: %1 ")
                          .arg(sum));
    }
}

void APDFileDrawer::drawPart(QCPRange range)
{
    if(!time.size())
        return;

    sendHistEventsInWindow(range, amplHist);

    sendHistEventsInWindow(range, intervalHist);

    if(graph->visible() || graph2->visible())
    {
        //определение максимального количества точек, которые могут быть отображены на экране
        int maxPoints = plot->width();

        //получение индекса точек, попадающих в границу
        /*
        std::vector<int>::iterator itMin = std::lower_bound(time.begin(), time.end(), min);
        std::vector<int>::iterator itMax = std::upper_bound(time.begin(), time.end(), max);

        int minInd = itMin - time.begin();
        int maxInd = itMax - time.begin();
        */
        quint64 minInd;
        quint64 maxInd;


        getMinMaxInd<std::vector<quint64> >(time, range.lower, range.upper, minInd, maxInd);

        int step = qMax((quint64)1, (maxInd - minInd)/maxPoints);

        QVector<double> x, y;

        //закрашивание собвтий, если их мало
        if(graph->visible())
        {
            ///\todo добавить порог по событиям в настройки
            if(maxInd - minInd < 100)
                graph->setBrush(QBrush(color, Qt::DiagCrossPattern));
            else
                graph->setBrush(QBrush());

            for(quint64 i = minInd; i < maxInd; i+= step)
            {
                x.push_back(time[i]);
                y.push_back(val[i]);

                x.push_back(time[i] + width[i]);
                y.push_back(0);
            }

            if(time.size() != 0)
            {
                x.push_front(0);
                y.push_front(0);

                x.push_back(time[time.size() - 1]);
                y.push_back(val[time.size() - 1]);
                x.push_back(time[time.size() - 1] + width[time.size() - 1]);
                y.push_back(0);
            }

            graph->setData(x, y);
            plot->replot();
        }

        if(graph2->visible())
        {
            for(quint64 i = minInd; i < maxInd; i+= step)
            {
                x.push_back(time[i]);
                y.push_back(qAbs(val[i]));
            }

            graph2->setData(x, y);
            plot->replot();
        }

        int eventsInWindow = (maxInd - minInd)/2;
        emit sendTextInfo(QFileInfo(*file).fileName(),
                          tr("Событий в окне: %1 (%2 показано, %3 скрыто)")
                          .arg(eventsInWindow)
                          .arg(eventsInWindow/step)
                          .arg((int)(((double)eventsInWindow)*(1. - 1./(double)step))));
    }
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
