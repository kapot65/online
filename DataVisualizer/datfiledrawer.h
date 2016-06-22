#ifndef DATFILEDRAWER_H
#define DATFILEDRAWER_H

#include <QObject>
#include <filedrawer.h>

#define FREQ_SAMP 5.12
#define FREQ_CAL  0.100   // GHz
#define N_CHN 1 // number of saved channels
#define BATCH 100

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
    DatFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);

    // FileDrawer interface
public slots:
    void setMetaDataToTable(){}
    void setVisible(bool visible, GraphMode graphMode);
    void setColor(QColor color){}
    void update();

    void initFile();

private slots:
    void drawPart(QCPRange range);

private:
    bool loaded;

    quint64 total_events;
    quint64 binary_starts;

    QVector<QDateTime> timeMap;
    QDateTime lastEventTimeStamp;

    THEADER th;
    TCHEADER tch[N_CHN];

    /// \brief Вспомогательный график для APDFileDrawer::graph.
    /// Содержит границы данных и имеет прозрачный цвет. Используется для
    /// автомасштабирования.
    QCPGraph *graphBorder;

    /// \brief Указатель на график.
    QCPGraph *graph;
    QDateTime getEventTimeStamp(int i);
    quint64 getEventSize();
};

#endif // DATFILEDRAWER_H
