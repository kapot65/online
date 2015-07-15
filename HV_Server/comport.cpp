#include "comport.h"

ComPort::ComPort(IniManager *manager, QObject *parent) : QThread(parent)
{
    this->manager = manager;
    serialPort = new QSerialPort(this);

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readMessage()));
}

ComPort::~ComPort()
{
    serialPort->close();
}
