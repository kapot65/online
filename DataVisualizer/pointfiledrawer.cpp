#include "pointfiledrawer.h"

PointFileDrawer::PointFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent)
    : FileDrawer(table, plot, filename, parent)
{
    loaded = 0;
    update();
}

PointFileDrawer::~PointFileDrawer()
{
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
        graph_absolute.last()->setParent(this);

        graph_relative.push_back(plot->addGraph());
        graph_relative.last()->addData(time, event_data);
        graph_relative.last()->setVisible(false);
        graph_relative.last()->setPen(QPen(color));
        graph_relative.last()->setLineStyle(QCPGraph::lsNone);
        graph_relative.last()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
        graph_relative.last()->setParent(this);

        //создание гистограммы
        QVector<unsigned short> data(events.size());
        for(int i = 0; i < events.size(); i++)
           data[i] = events[i].data;
        bars.push_back(createHist<unsigned short>(data, plot));
        plot->addPlottable(bars.last());
        bars.last()->setVisible(false);
        bars.last()->setParent(this);
    }

    emit updated();
}
