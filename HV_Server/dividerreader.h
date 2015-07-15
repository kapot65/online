#ifndef DIVIDERREADER_H
#define DIVIDERREADER_H

#include "comport.h"
#include <QObject>

class DividerReader : public ComPort
{
    Q_OBJECT
public:
    explicit DividerReader(QString DividerName, IniManager *manager, QObject *parent = 0);
    ~DividerReader();
    bool checkBusy(){return busy;}
    bool checkInited(){return inited;}

signals:
    void receiveFinished();
    void initVoltmeterDone();
    void getVoltageDone(double voltage);

public slots:
    void initVoltmeter();
    void getVoltage();

protected slots:
    virtual void readMessage();

private:
    bool busy;
    bool inited;

    double dividerNormCoeff;

    QByteArray curr_data;
};

#endif // DIVIDERREADER_H
