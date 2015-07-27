#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <tcpserver.h>
#include <inimanager.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <easylogging++.h>

class ComPort: public QThread
{
    Q_OBJECT
public:
    ComPort(IniManager *manager, QObject *parent = 0);
    ~ComPort();

protected slots:
    virtual void readMessage() = 0;
    void onPortClose();
    void onPortError(QSerialPort::SerialPortError error);

protected:
    IniManager *manager;
    QSerialPort *serialPort;
};

#endif // COMPORT_H
