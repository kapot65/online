#include "hvcontroler.h"
#include <tcpprotocol.h>

void HVControler::processSettingError(QString setting, QString controllerName)
{
    LOG(ERROR) << tr("Setting \"%1\" in \"%2\" has not set.")
                  .arg(setting)
                  .arg(controllerName)
                  .toStdString();
}

bool HVControler::loadSettings(QString controllerName, IniManager *manager)
{
    int errorCount = 0;

    if(!manager->getSettingsValue(controllerName, "c0").isValid() && errorCount++)
        processSettingError("c0", controllerName);
    c0 = manager->getSettingsValue(controllerName, "c0").toDouble();

    if(!manager->getSettingsValue(controllerName, "c1").isValid() && errorCount++)
        processSettingError("c0", controllerName);
    c1 = manager->getSettingsValue(controllerName, "c1").toDouble();

    if(!manager->getSettingsValue(controllerName, "minTreshold").isValid() && errorCount++)
        processSettingError("c0", controllerName);
    minTreshold = manager->getSettingsValue(controllerName, "minTreshold").toDouble();

    if(!manager->getSettingsValue(controllerName, "maxTreshold").isValid() && errorCount++)
        processSettingError("c0", controllerName);
    maxTreshold = manager->getSettingsValue(controllerName, "maxTreshold").toDouble();

    QVariant portNameVariant = manager->getSettingsValue(controllerName, "COM");
    if((!portNameVariant.isValid() || portNameVariant.toString() == "SET VALUE") && errorCount++)
        processSettingError("COM", controllerName);
    portName = portNameVariant.toString();

    if(errorCount)
        return false;

    return true;
}

HVControler::HVControler(IniManager *manager, QString controllerName, bool *ok, QObject *parent) : ComPort(manager, parent)
{
    this->controllerName = controllerName;

    busyFlag = 0;

#ifdef TEST_MODE
    qDebug() << tr("%1 working in thread: ").arg(controllerName) << QThread::currentThreadId();
#endif

    //загрузка настроек из ini файла
    if(!loadSettings(controllerName, manager))
    {
        LOG(ERROR) << tr("Can't load settings for \"%1\". Stop server.")
                      .arg(controllerName)
                      .toStdString();
        TcpProtocol::setOk(false, ok);
        return;
    }

#ifndef VIRTUAL_MODE
    serialPort->setPortName(portName);

    if(!serialPort->open(QIODevice::ReadWrite))
    {
        LOG(INFO) << "Can not connect to port " << portName.toStdString();
        TcpProtocol::setOk(false, ok);
        return;
    }

    //Проверка подключенности порта
    serialPort->write("$012\r");

    if(waitForMessageReady(5000) &&
       curr_data.startsWith('!') && //проверка доступности команды
       curr_data.size()) ///\bug проверка длины ответа
    {
        curr_data.clear();
        TcpProtocol::setOk(true, ok);
        LOG(INFO) << tr("%1 Controller connected to port %2").arg(controllerName).arg(portName).toStdString();
    }
    else
    {
        //если проверка порта не пройдена
        LOG(ERROR)<<tr("Com port has not pass answer checking: '%1' -> '%2'")
                    .arg("$012\r").arg(QString(curr_data)).toStdString();

        TcpProtocol::setOk(false, ok);
        curr_data.clear();
        return;
    }
#else
    TcpProtocol::setOk(true, ok);
    LOG(INFO) << tr("%1 Controller working in virtual mode").arg(controllerName).toStdString();
#endif
}

void HVControler::setVoltage(double voltage)
{
#ifdef TEST_MODE
    qDebug() << QThread::currentThreadId() << tr("%1: setting %2 volts on block.")
                                                .arg(controllerName).arg(voltage);
#endif

    busyFlag = 1;

    //преобразование напряжения;
    double voltage_normalised = c0 + c1*voltage;

    if(voltage_normalised < minTreshold)
    {
        voltage_normalised = minTreshold;
        LOG(WARNING) << "Voltage " << QString().number(voltage).toStdString() << " out of range. Changed to minimal";
    }

    if(voltage_normalised > maxTreshold)
    {
        voltage_normalised = maxTreshold;
        LOG(WARNING) << "Voltage " << QString().number(voltage).toStdString() << " out of range. Changed to maximal";
    }

#ifndef VIRTUAL_MODE
    QByteArray command;
    command.push_back(QString("#01%1\r").arg(voltage_normalised, 6, 'f', 3, '0').toLatin1());

    serialPort->write(command);

    if(waitForMessageReady() && curr_data == ">")
    {
        LOG(INFO) << tr("Set voltage %1").arg(voltage).toStdString();
        emit setVoltageDone();
    }
    else
        LOG(WARNING) << tr("Cant set voltage: receive error answer - %1")
                        .arg(QString(curr_data)).toStdString();

    curr_data.clear();
#else
    LOG(INFO) << tr("Set voltage %1").arg(voltage).toStdString();
    emit setVoltageDone();
#endif

    busyFlag = 0;

#ifdef TEST_MODE
    qDebug() << QThread::currentThreadId() << tr("%1: setting %2 volts on block done.")
                                                .arg(controllerName).arg(voltage);
#endif
}
