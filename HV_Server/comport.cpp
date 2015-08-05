#include "comport.h"

ComPort::ComPort(IniManager *manager, QObject *parent) : QThread(parent)
{
    busyFlag = 0;

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

bool ComPort::waitForMessageReady(int timeout)
{
    QEventLoop el;
    connect(this, SIGNAL(receiveFinished()), &el, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));

    timer.start(timeout);
    el.exec();

    if(timer.isActive())
    {
        timer.stop();
        return true;
    }
    else
        return false;
}

void ComPort::readMessage()
{
    curr_data += serialPort->readAll();
    if(curr_data.endsWith("\r") || curr_data.endsWith("\n"))
    {
        //обрезание спецсимволов
        while(curr_data.size() && !QChar(curr_data[curr_data.size() - 1]).isDigit())
            curr_data.chop(1);

        emit receiveFinished();
    }
}
