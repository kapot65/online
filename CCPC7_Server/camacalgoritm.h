#ifndef CAMACALGORITM_H
#define CAMACALGORITM_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QtEndian>
#include <QFile>
#include <QTime>
#include <QDebug>

#include <QTimer>
#include <QEventLoop>

#include <easylogging++.h>
#include <ccpc7.h>

#include "camacserversettings.h"
#include <tcpprotocol.h>
#include <event.h>

/*
#if QT_VERSION >= 0x040800
class Sleeper: public QThread
{
public:
    static void ssleep(unsigned int sec) {return sleep(sec);}
    static void mSleep(unsigned int sec) {return msleep(sec);}
};
#endif
*/

class CamacAlgoritm : public QObject
{
    Q_OBJECT
public:
    explicit CamacAlgoritm(QObject *parent = 0);

protected slots:
    void on_breakAcquisition();
    //ручная запись и считывание из MADC
    void testMADC();

signals:
    void currentEventCount(int count);

protected:
    ccpc::CamacImplCCPC7 *camac;
    CamacServerSettings *settings;

    QVector<Event> acquirePoint(unsigned short measureTime, bool *manuallyBreak = NULL);
    void resetCounters();
    unsigned int getCounterValue(int counterNum, int channelNum, bool withReset);

    //функции CAMAC
    void C();
    void Z();
    ccpc::CamacOp NAF(int n, int a, int f, unsigned short &data);

private:
    bool breakFlag;

#if QT_VERSION >= 0x040800
    QTimer *timer;
    QEventLoop *eventLoop;
#endif

    void disableMeasurement();
    void enableMeasurement();

    //функции MADS
    void writeMADC(unsigned short &data, long &time);
    void readMADC(unsigned short &data, long &time, bool &valid); //READ 12 BIT DATA & 28 BIT TIME & VALID BIT FROM THE MEMORY AND INCREMENT ADDRESS
    void readMADCData(unsigned short &data, long &time, bool &valid); //READ 12 BIT DATA & VALID BIT FROM THE MEMORY AND INCREMENT ADDRESS
    void setMADCAddr(long &addr, unsigned short &measureTime); //SET 18 BIT ADDRESS AND MEASUREMENT TIME IN SEC
    void getMADCAddr(long &addr, bool &addrOverflow, bool &endOfMeasurement);//GET ADDRESS, ADDRESS-OVERFLOW AND END-OF-MEASURE BITS

    //вспомогательные функции
    bool checkBit(unsigned short var, int pos);
    void replaceBit(long &var, int pos, bool bit);
    void replaceBit(unsigned short &var, int pos, bool bit);

    //доступные времена сбора
    QMap<int, unsigned short> aviableMeasureTimes;
};

#endif // CAMACALGORITM_H
