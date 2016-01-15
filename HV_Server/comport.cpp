#include "comport.h"

ComPort::ComPort(IniManager *manager, QObject *parent) : QThread(parent)
{
    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");

    connect(this, SIGNAL(error(QVariantMap)),
            this, SLOT(handleErrorImpl(QVariantMap)));

    serialPort = 0;

    busyFlag = 0;
    this->manager = manager;
}

ComPort::~ComPort()
{
    disconnect(this, SIGNAL(error(QVariantMap)),
            this, SLOT(handleErrorImpl(QVariantMap)));


    if(serialPort)
    {
        serialPort->close();
        serialPort->deleteLater();
    }
}

bool ComPort::handleError(QVariantMap err)
{
    if(err["error_code"].toUInt() == COM_PORT_ERROR )
    {
//        QSerialPort::SerialPortError comErr = (QSerialPort::SerialPortError)err["com_port_err"].toUInt();

//        switch (comErr) {
//            case QSerialPort::SerialPortError::NoError:
//                return true;

//            case QSerialPort::SerialPortError::DeviceNotFoundError:
//                LOG(WARNING) << tr("Catch error: Device not found. Please check device adress in config file.");
//                return false;
//                break;


//            default:
//                return false;
//        }

    }
    return false;
}

void ComPort::onPortClose()
{
    QVariantMap err;
    err["error_code"] = COM_PORT_CLOSE;
    emit error(err);

    LOG(INFO) << tr("port %1 is closing").arg(serialPort->portName()).toStdString();
}

void ComPort::onPortError(QSerialPort::SerialPortError portError)
{
    if(portError == QSerialPort::NoError)
        return;


    QVariantMap err;

    // Оборачивание ошибки и посылка ее в обработчик
    err["error_code"] = COM_PORT_ERROR;
    err["com_port_err"] = portError;
    err["description"] = serialPort->errorString();

    emit error(err);

    LOG(WARNING) << tr("port %1 has error: %2").arg(serialPort->portName())
                 .arg(serialPort->errorString()).toStdString();
}

void ComPort::handleErrorImpl(QVariantMap err)
{
    if(!handleError(err))
        emit unhandledError(err);
    else
        emit ready();
}

bool ComPort::waitForMessageReady(int timeout)
{
    QEventLoop el;
    connect(this, SIGNAL(receiveFinished()), &el, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout);

    el.exec();

    if(timer.isActive())
    {
        timer.stop();
        return true;
    }
    else
    {
        LOG(WARNING) << "waitForMessageReady : Timeout.";
        return false;
    }
}

void ComPort::run()
{
#ifndef VIRTUAL_MODE
    serialPort = new QSerialPort();

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(serialPort, SIGNAL(aboutToClose()), this, SLOT(onPortClose()));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onPortError(QSerialPort::SerialPortError)));
#endif
}

void ComPort::readMessage()
{
    if(!serialPort->canReadLine())
        return;

    curr_data = serialPort->readLine();
    curr_data.chop(2);

    emit receiveFinished();
}
