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

HVControler::HVControler(IniManager *manager, QString controllerName, double *voltage, bool *ok, QObject *parent) : ComPort(manager, parent)
{
    this->controllerName = controllerName;

    actualVoltage = voltage;

    busyFlag = 0;

    settedVoltage = -1;

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

    settedVoltage = (voltage_normalised - c0)/c1;

#ifdef TEST_MODE
    qDebug() << QThread::currentThreadId() << tr("%1: setting %2 volts on block done.")
                                                .arg(controllerName).arg(voltage);
#endif
}

void HVControler::setVoltageAndCheck(QVariantMap params)
{
    QVariantMap answer;

    //Проверка корректности входных параметров
    if(!params.contains("voltage"))
        answer["description"] = answer["description"].toString() + tr("params doesnt contains field\"voltage\"\n");
    if(!params.contains("max_error"))
        answer["description"] = answer["description"].toString() + tr("params doesnt contains field\"max_error\"\n");
    if(!params.contains("timeout"))
        answer["description"] = answer["description"].toString() + tr("params doesnt contains field\"timeout\"\n");

    if(answer.contains("description"))
    {
        answer["status"] = "bad_params";
        emit voltageSetAndCheckDone(answer);
        return;
    }

    int timeout = params["timeout"].toInt();
    double voltage = params["voltage"].toDouble();
    double max_error = params["max_error"].toDouble();

    QElapsedTimer timer;
    QEventLoop el;
    QTimer secondsTimer;
    connect(&secondsTimer, SIGNAL(timeout()), &el, SLOT(quit()));
    timer.start();
    secondsTimer.start(1000);

    setVoltage(voltage);

    while(timer.elapsed()/1000 < timeout)
    {
        el.exec();
        if(qAbs(actualVoltage[0] - voltage) < max_error)
        {
            //Нужное наряжение установлено
            answer["status"] = "ok";
            answer["error"] = actualVoltage[0] - voltage;
            answer["voltage"] = actualVoltage[0];

            emit voltageSetAndCheckDone(answer);
            return;
        }
    }

    //Выход из функции по таймауту
    answer["status"] = "timeout";
    answer["error"] = actualVoltage[0] - voltage;
    answer["voltage"] = actualVoltage[0];

    emit voltageSetAndCheckDone(answer);
    return;
}

void HVControler::run()
{
    ComPort::run();

    bool ok;

#ifndef VIRTUAL_MODE
    serialPort->setPortName(portName);

    if(!serialPort->open(QIODevice::ReadWrite))
    {
        LOG(INFO) << "Can not connect to port " << portName.toStdString();
        TcpProtocol::setOk(false, &ok);
        return;
    }

    //Проверка подключенности порта
    serialPort->write("$012\r");

    if(waitForMessageReady(5000) &&
       curr_data.startsWith('!') && //проверка доступности команды
       curr_data.size()) ///\bug проверка длины ответа
    {
        curr_data.clear();
        TcpProtocol::setOk(true, &ok);
        LOG(INFO) << tr("%1 Controller connected to port %2").arg(controllerName).arg(portName).toStdString();
    }
    else
    {
        //если проверка порта не пройдена
        LOG(ERROR)<<tr("Com port has not pass answer checking: '%1' -> '%2'")
                    .arg("$012\r").arg(QString(curr_data)).toStdString();

        QByteArray missedData = serialPort->readAll().replace("\r", "\\r")
                                .replace("\n", "\\n");

        TcpProtocol::setOk(false, &ok);
        curr_data.clear();
        return;
    }
#else
    TcpProtocol::setOk(true, &ok);
    LOG(INFO) << tr("%1 Controller working in virtual mode").arg(controllerName).toStdString();
#endif

    exec();
}

void HVControler::readMessage()
{
    while(serialPort->bytesAvailable())
    {
        curr_data += serialPort->read(1);
        if(curr_data.endsWith('\r'))
        {
            curr_data.chop(1);
            emit receiveFinished();
            break;
        }
    }
}
