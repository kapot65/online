#include "comport.h"

ComPort::ComPort(IniManager *manager, QObject *parent) : QThread(parent)
{
    this->manager = manager;
    serialPort = new QSerialPort(this);

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(serialPort, SIGNAL(aboutToClose()), this, SLOT(onPortClose()));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onPortError(QSerialPort::SerialPortError)));
}

ComPort::~ComPort()
{
    serialPort->close();
}

void ComPort::onPortClose()
{
    LOG(INFO) << tr("port %1 is closing").arg(serialPort->portName()).toStdString();
}

void ComPort::onPortError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::NoError)
        return;

    LOG(INFO) << tr("port %1 has error: %2").arg(serialPort->portName())
                 .arg(serialPort->errorString()).toStdString();
}
