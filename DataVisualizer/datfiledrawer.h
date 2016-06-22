#ifndef DATFILEDRAWER_H
#define DATFILEDRAWER_H

#include <QObject>
#include <filedrawer.h>

typedef struct {
   char time_header[4];
   char bn[2];
   unsigned short board_number;
} THEADER;

typedef struct {
   char channel[4];
   float tcal[1024];
} TCHEADER;

typedef struct {
   char event_header[4];
   unsigned int ev_serial_number;
   unsigned short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short minute;
   unsigned short second;
   unsigned short millisec;
   unsigned short reserved;
   char bn[2];
   unsigned short board_number;
   char tc[2];
   unsigned short trigger_cell;
} EHEADER;

typedef struct {
   char channel[4];
   unsigned short data[1024];
} CHANNEL;

class DatFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    DatFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QSettings *settings,
                  QObject *parent = 0);
    ~DatFileDrawer(){delete[] tch;}
    // FileDrawer interface
public slots:
    void setMetaDataToTable(){}
    void setVisible(bool visible, GraphMode graphMode);
    void setColor(QColor color);
    void update();

    void initFile();

private slots:
    ///
    /// \todo Добавить вывод количества событий
    void drawPart(QCPRange range);

private:
    bool loaded;

    quint64 total_events;
    quint64 binary_starts;

    QVector<QDateTime> timeMap;
    QDateTime lastEventTimeStamp;

    THEADER th;
    TCHEADER *tch;

    /// \brief Вспомогательный график для APDFileDrawer::graph.
    /// Содержит границы данных и имеет прозрачный цвет. Используется для
    /// автомасштабирования.
    QCPGraph *graphBorder;
    /// \brief Указатель на график.
    QCPGraph *graph;

    QCPGraph *barGraph;

    QDateTime getEventTimeStamp(int i);
    quint64 getEventSize();
    double getEventAmplitude(int i, quint64 &time);

    quint64 eventSize;
    QDateTime getEventTimeFromHeader(EHEADER eh);

    quint64 redrawLastTime;

    QSettings *settings;
    double FREQ_SAMP;
    double FREQ_CAL;
    int N_CHN;
    int PROCESS_CHANNEL;
    int BATCH;
    int BASELINE_SIZE;
    bool INVERSED;
    int MAX_EVENTS;
    int HIST_FLUSH_STEP;
    double HIST_MIN_VAL;
    double HIST_MAX_VAL;
};

#endif // DATFILEDRAWER_H
