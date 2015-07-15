#include "hvcontroler.h"

HVControler::HVControler(IniManager *manager, QObject *parent) : ComPort(manager, parent)
{
    busyFlag = 0;

    c0 = 0.24124;
    c1 = 0.0098257;

    QVariant portName = manager->getSettingsValue("HVController", "COM");
    if(!portName.isValid() || portName.toString() == "SET VALUE")
    {
        LOG(ERROR) << "Setting \"COM\" in \"HVController\" has not set. Stop server.";
        qApp->exit(1);
        return;
    }
    else
        serialPort->setPortName(portName.toString());

    if(serialPort->open(QIODevice::ReadWrite))
        LOG(INFO) << "HV Controller connected to port " << portName.toString().toStdString();
}

void HVControler::readMessage()
{
    QByteArray data = serialPort->readAll();
    //LOG(DEBUG) << QString(data).toStdString();
}

void HVControler::setVoltage(double voltage)
{
    busyFlag = 1;

    //преобразование напряжения;
    double voltage_normalised = c0 + c1*voltage;

    if(voltage_normalised < 0.)
    {
        voltage_normalised = 0;
        LOG(WARNING) << "Voltage " << QString().number(voltage).toStdString() << " out of range. Changed to minimal";
    }

    if(voltage_normalised > 9.2)
    {
        voltage_normalised = 9.2;
        LOG(WARNING) << "Voltage " << QString().number(voltage).toStdString() << " out of range. Changed to maximal";
    }

    QByteArray command;
    command.push_back(QString("#01%1\r").arg(voltage_normalised, 6, 'f', 3, '0').toLatin1());

    serialPort->write(command);

    emit setVoltageDone();

    busyFlag = 0;
}
