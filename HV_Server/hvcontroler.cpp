#include "hvcontroler.h"

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

HVControler::HVControler(IniManager *manager, QString controllerName, QObject *parent) : ComPort(manager, parent)
{
    busyFlag = 0;

    //загрузка настроек из ini файла
    if(!loadSettings(controllerName, manager))
    {
        LOG(ERROR) << tr("Can't load settings for \"%1\". Stop server.")
                      .arg(controllerName)
                      .toStdString();
        qApp->exit(1);
    }

    serialPort->setPortName(portName);

    if(serialPort->open(QIODevice::ReadWrite))
        LOG(INFO) << "HV Controller connected to port " << portName.toStdString();
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

    QByteArray command;
    command.push_back(QString("#01%1\r").arg(voltage_normalised, 6, 'f', 3, '0').toLatin1());

    serialPort->write(command);

    emit setVoltageDone();

    busyFlag = 0;
}
