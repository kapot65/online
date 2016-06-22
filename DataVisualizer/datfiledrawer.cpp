#include "datfiledrawer.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <QDate>
#include <QTime>

#include <easylogging++.h>

DatFileDrawer::DatFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QSettings *settings, QObject *parent)
    : FileDrawer(table, plot, filename, parent, false)
{
    this->settings = settings;
    settings->beginGroup(metaObject()->className());
    if(!settings->contains("FREQ_SAMP"))
        settings->setValue("FREQ_SAMP", 5.12);
    FREQ_SAMP = settings->value("FREQ_SAMP").toDouble();
    if(!settings->contains("FREQ_CAL"))
        settings->setValue("FREQ_CAL", 0.100);
    FREQ_CAL = settings->value("FREQ_CAL").toDouble();
    if(!settings->contains("N_CHN"))
        settings->setValue("N_CHN", 1);
    N_CHN = settings->value("N_CHN").toInt();
    if(!settings->contains("PROCESS_CHANNEL"))
        settings->setValue("PROCESS_CHANNEL", 0);
    PROCESS_CHANNEL = settings->value("PROCESS_CHANNEL").toInt();
    if(!settings->contains("BASELINE_SIZE"))
        settings->setValue("BASELINE_SIZE", 48);
    BASELINE_SIZE = settings->value("BASELINE_SIZE").toInt();
    if(!settings->contains("BATCH"))
        settings->setValue("BATCH", 500);
    BATCH = settings->value("BATCH").toInt();
    if(!settings->contains("INVERSED"))
        settings->setValue("INVERSED", true);
    INVERSED = settings->value("INVERSED").toBool();
    if(!settings->contains("MAX_EVENTS"))
        settings->setValue("MAX_EVENTS", 2000);
    MAX_EVENTS = settings->value("MAX_EVENTS").toInt();
    if(!settings->contains("HIST_FLUSH_STEP"))
        settings->setValue("HIST_FLUSH_STEP", 10000);
    HIST_FLUSH_STEP = settings->value("HIST_FLUSH_STEP").toInt();
    if(!settings->contains("HIST_MIN_VAL"))
        settings->setValue("HIST_MIN_VAL", 0.0);
    HIST_MIN_VAL = settings->value("HIST_MIN_VAL").toDouble();
    if(!settings->contains("HIST_MAX_VAL"))
        settings->setValue("HIST_MAX_VAL", 0.5);
    HIST_MAX_VAL = settings->value("HIST_MAX_VAL").toDouble();
    settings->endGroup();

    tch = new TCHEADER[N_CHN];

    loaded = 0;
    redrawLastTime = 0;
    graphBorder = 0;
    graph = 0;

    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(drawPart(QCPRange)));
    update();
}

void DatFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    graphBorder->setVisible(false);
    graph->setVisible(false);
    barGraph->setVisible(false);

    this->isVisible = visible;
    switch(graphMode)
    {
        case RELATIVE_TIME:
            graphBorder->setVisible(visible);
            graph->setVisible(visible);
            break;
        case HISTOGRAMM:
            barGraph->setVisible(visible);
            break;
    }
}

void DatFileDrawer::setColor(QColor color)
{

    this->color = color;
    graph->setPen(color);
    barGraph->setPen(color);
}

void DatFileDrawer::update()
{
    //считывается только один раз
    if(!loaded)
    {
        initFile();
        loaded = true;
    }

    emit updated();
}

QDateTime DatFileDrawer::getEventTimeFromHeader(EHEADER eh)
{
    QDateTime timeStamp;

    QDate eventDate(eh.year, eh.month, eh.day);
    QTime eventTime(eh.hour, eh.minute, eh.second, eh.millisec);

    timeStamp = QDateTime(eventDate, eventTime);
    return timeStamp;
}

QDateTime DatFileDrawer::getEventTimeStamp(int i)
{
    QDateTime timeStamp;

    file->seek(binary_starts + i*eventSize);
    EHEADER eh;
    file->read((char*)(&eh), sizeof(eh));

    return getEventTimeFromHeader(eh);
}

double DatFileDrawer::getEventAmplitude(int i, quint64 &time)
{
    file->seek(binary_starts + i*eventSize);

    EHEADER eh;
    file->read((char*)(&eh), sizeof(eh));

    CHANNEL *channel = new CHANNEL[N_CHN];
    double *waveform = new double[N_CHN*1024];

    // read channel data
    for (int ch=0 ; ch<N_CHN ; ch++) {
       file->read((char*)(&channel[ch]), sizeof(CHANNEL));

       // convert to volts
       for (int i=0 ; i<1024 ; i++) {
          waveform[ch*1024 + i] = channel[ch].data[i]/65536.0-0.5;
       }
    }

   //поиск амплитуды
   double baseLine = 0;
   for(int i=0; i < BASELINE_SIZE; i++)
       baseLine += waveform[PROCESS_CHANNEL*1024 + i];
   baseLine /= BASELINE_SIZE;

   double inv = qPow(-1, (double)((bool)INVERSED));
   double extremum = -10.;
   for (i=0 ; i<1024 ; i++)
       if(waveform[PROCESS_CHANNEL*1024 + i]*inv > extremum)
           extremum = waveform[PROCESS_CHANNEL*1024 + i]*inv;
   //extremum *= inv;
   extremum += baseLine;

   time = getEventTimeFromHeader(eh).toMSecsSinceEpoch();

   delete[] waveform;
   delete[] channel;
   return extremum;
}

inline quint64 DatFileDrawer::getEventSize(){
    return (sizeof(EHEADER) + sizeof(CHANNEL)*N_CHN);
}

void DatFileDrawer::initFile()
{
    // чтение хедера
    file->read((char*)(&th), sizeof(th));
    for (int ch = 0 ; ch<N_CHN ; ch++)
        file->read((char*)(&tch[ch]), sizeof(TCHEADER));

    binary_starts = file->pos();
    file->size();

    eventSize = getEventSize();
    total_events = (file->size() - binary_starts)/eventSize;

    timeMap.clear();

    //создание временной карты
    for(int i = 0; i < total_events; i += BATCH)
        timeMap.push_back(getEventTimeStamp(i));
    lastEventTimeStamp = getEventTimeStamp(total_events - 1);

    //создание гистограммы
    QVector<double> data;
    QVector<double> binVal;
    QVector<double> bin;

    for(int i = 0; i < total_events; i += 1)
    {
        quint64 time_buf;
        data.push_back(getEventAmplitude(i, time_buf));
        if(!(i%HIST_FLUSH_STEP) || i == total_events - 1)
        {
            QVector<double> binStep;
            double minVal = HIST_MIN_VAL;
            double maxVal = HIST_MAX_VAL;
            generateHistFromData<double>(data, binStep, bin, minVal, maxVal);
            if(binVal.isEmpty())
                binVal = binStep;
            else
                for(int i = 0; i < binVal.size(); i++)
                    binVal[i] += binStep[i];

            binStep.clear();
        }
    }

    barGraph = createGraphHistFromData(plot, binVal, bin);
    barGraph->setVisible(false);
    barGraph->setPen(QPen(color));
    barGraph->setLineStyle(QCPGraph::lsStepCenter);
    barGraph->setParent(this);
    plot->addPlottable(barGraph);

    //создание двуплотного графика
    graph = plot->addGraph();
    graph->setPen(QPen(color));
    graph->setLineStyle(QCPGraph::lsNone);
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));

    //создание ограничивающего прямоугольника
    graphBorder = plot->addGraph();
    double timeStart = timeMap.first().toMSecsSinceEpoch();
    double timeEnd = lastEventTimeStamp.toMSecsSinceEpoch() - timeStart;
    graphBorder->addData(0, -2.);
    graphBorder->addData(timeEnd*qPow(10,6), 2.);
    graphBorder->setPen(QPen(QColor(0,0,0,0)));
}

void DatFileDrawer::drawPart(QCPRange range)
{
    if(!visible())
        return;
    if((QDateTime::currentMSecsSinceEpoch() - redrawLastTime) < 2000)
        return;
    redrawLastTime = QDateTime::currentMSecsSinceEpoch();

    double timeStart = timeMap.first().toMSecsSinceEpoch();
    QDateTime timeLower = QDateTime::fromMSecsSinceEpoch(quint64(range.lower*qPow(10,-6)) +
                                                         timeMap.first().toMSecsSinceEpoch());
    QDateTime timeUpper = QDateTime::fromMSecsSinceEpoch(quint64(range.upper*qPow(10,-6)) +
                                                         timeMap.first().toMSecsSinceEpoch());

    int minBatchInd = 0;
    for(;(minBatchInd < timeMap.size()) &&
         (timeMap[minBatchInd] <= timeLower);
        minBatchInd++);

    int maxBatchInd = timeMap.size() - 1;
    for(;(maxBatchInd >= 0) &&
         (timeMap[maxBatchInd] >= timeUpper);
        maxBatchInd--);

    minBatchInd = qMax(0, minBatchInd - 1);
    maxBatchInd = qMin(timeMap.size()-1, maxBatchInd + 1);

    int step = qMax(1, (maxBatchInd - minBatchInd)*BATCH/MAX_EVENTS);

    QVector<double> times;
    QVector<double> amps;
    for(int i = minBatchInd*BATCH; i < maxBatchInd*BATCH; i += step)
    {
        quint64 timeMsec;
        double amp = getEventAmplitude(i, timeMsec);

        times.push_back((timeMsec - timeStart)*qPow(10., 6.));
        amps.push_back(amp);
    }

    graph->setData(times, amps);
    plot->replot();
}
