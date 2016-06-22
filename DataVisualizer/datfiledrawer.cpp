#include "datfiledrawer.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <QDate>
#include <QTime>

int main_(int argc, const char * argv[])
{
   int ch, i, j, n, ndt;
   double t1, t2, dt, sumdt, sumdt2;
   THEADER th;
   TCHEADER tch[N_CHN];
   EHEADER eh;
   CHANNEL channel[N_CHN];
   double waveform[N_CHN][1024], time[N_CHN][1024];

   if(argc != 3)
    return -1;

   // open file
   std::ifstream fh;
   fh.open(argv[1], std::ios::in | std::ios::binary);

   if(!fh.is_open())
   {
       std::cout << "can't open binary file " << argv[1] << "\n";
       return -1;
   }

   std::ofstream of;
   of.open(argv[2], std::ios::out);
   if(!of.is_open())
   {
       std::cout << "can't open out file " << argv[2] << "\n";
       return -1;
   }

   of << "# peaks, generated from " << argv[1] << "\n";
   of << "# name: out\n";
   of << "# type: matrix\n";
   of << "# rows: 1\n";
   of << "# columns: ";

   auto sizeInsertPosition = of.tellp();

   of << "                \n";

   // read time bin widths

   fh.read((char*)(&th), sizeof(th));
   for (ch=0 ; ch<N_CHN ; ch++)
       fh.read((char*)(&tch[ch]), sizeof(TCHEADER));

   // initialize statistics
   ndt = 0;
   sumdt = sumdt2 = 0;

   for (n= 0 ; ; n++) {
      // read event header
      fh.read((char*)(&eh), sizeof(eh));

      // check for valid event header
      if (memcmp(eh.event_header, "EHDR", 4) != 0) {
         printf("Invalid event header (probably number of saved channels not equal %d)\n", N_CHN);
         return 0;
      }

      // print notification every 100 events
      if (n % 500 == 0 || fh.eof())
         printf("Analyzing event #%d\n", n);

      // stop if end-of-file
      if (fh.eof())
         break;

      // read channel data
      for (ch=0 ; ch<N_CHN ; ch++) {
         fh.read((char*)(&channel[ch]), sizeof(CHANNEL));

         double max = -10.;

         for (i=0 ; i<1024 ; i++) {
            // convert to volts
            waveform[ch][i] = channel[ch].data[i]/65536.0-0.5;

            if(max < std::fabs(waveform[ch][i]))
                max = std::fabs(waveform[ch][i]);

            // calculate time for this cell
            for (j=0,time[ch][i]=0 ; j<i ; j++)
               time[ch][i] += tch[ch].tcal[(j+eh.trigger_cell) % 1024];
         }

         of << max << " ";
      }
   }

   of.seekp(sizeInsertPosition);
   of << n;

   of.close();
   fh.close();

   return 1;
}

DatFileDrawer::DatFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent, false)
{
    loaded = 0;

    graphBorder = 0;
    graph = 0;

    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(drawPart(QCPRange)));
    update();
}

void DatFileDrawer::setVisible(bool visible, GraphMode graphMode)
{
    if(graphMode == RELATIVE_TIME)
        graphBorder->setVisible(visible);
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

QDateTime DatFileDrawer::getEventTimeStamp(int i)
{
    static quint64 eventSize = getEventSize();
    QDateTime timeStamp;

    file->seek(binary_starts + i*eventSize);
    EHEADER eh;
    file->read((char*)(&eh), sizeof(eh));

    QDate eventDate(eh.year, eh.month, eh.day);
    QTime eventTime(eh.hour, eh.minute, eh.second, eh.millisec);

    timeStamp = QDateTime(eventDate, eventTime);

    return timeStamp;
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

    quint64 eventSize = getEventSize();
    total_events = (file->size() - binary_starts)/eventSize;

    timeMap.clear();

    //создание временной карты
    for(int i = 0; i < total_events; i += BATCH)
        timeMap.push_back(getEventTimeStamp(i));
    lastEventTimeStamp = getEventTimeStamp(total_events - 1);

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
//    if(!time.size())
//        return;

//    sendHistEventsInWindow(range, amplHist);

//    sendHistEventsInWindow(range, intervalHist);

//    if(graph->visible() || graph2->visible())
//    {
//        //определение максимального количества точек, которые могут быть отображены на экране
//        int maxPoints = plot->width();

//        //получение индекса точек, попадающих в границу
//        /*
//        std::vector<int>::iterator itMin = std::lower_bound(time.begin(), time.end(), min);
//        std::vector<int>::iterator itMax = std::upper_bound(time.begin(), time.end(), max);

//        int minInd = itMin - time.begin();
//        int maxInd = itMax - time.begin();
//        */
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
