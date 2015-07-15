#ifndef HVCONTROLER_H
#define HVCONTROLER_H

#include "comport.h"


class HVControler : public ComPort
{
    Q_OBJECT
public:
    explicit HVControler(IniManager *manager, QObject *parent = 0);
    bool checkBusy(){return busyFlag;}

signals:
    void setVoltageDone();

public slots:
    void setVoltage(double voltage);

protected slots:
    virtual void readMessage();

private:
    //линейные коэффициенты преобразования
    double c0;
    double c1;

    bool busyFlag;
};

#endif // HVCONTROLER_H
