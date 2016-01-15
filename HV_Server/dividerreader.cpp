#include "dividerreader.h"

DividerReader::DividerReader(QString DividerName, IniManager *manager, QObject *parent) : ComPort(manager, parent)
{
    inited = 0;
    classInited = 0;
    stopFlag = 0;

    this->divierReaderName = DividerName;
}

bool DividerReader::init()
{
    classInited = 0;

    //Считывание параметров из ini файла

    if(!manager->getSettingsValue(divierReaderName, "NormingCoefficient").isValid())
    {
        LOG(ERROR) << QString("Parameter <NormingCoefficient> in %1 not set").arg(divierReaderName).toStdString();
        QCoreApplication::exit();
        return false;
    }

    this->dividerNormCoeff = manager->getSettingsValue(divierReaderName, "NormingCoefficient").toDouble();

    QVariant portName = manager->getSettingsValue(divierReaderName, "COM");
    if(!portName.isValid() || portName.toString() == "SET VALUE")
    {
        LOG(ERROR) << QString("Setting \"COM\" in \"%1\" has not set. Stop server.").arg(divierReaderName).toStdString();
        qApp->exit(1);
        return false;
    }

    this->portName = portName.toString();

    classInited = true;
    return true;
}

bool DividerReader::initVoltmeter()
{
    busyFlag = 1;
    inited = 0;

    QEventLoop el;
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &timer, SLOT(stop()));


#ifndef VIRTUAL_MODE
    //Очистка буффера вольтметра
    serialPort->readAll();

    serialPort->write("SYST:REM\r\n"); // Switch voltmeter to remote control (once)
    timer.start(1000);
    el.exec();

    if(!checkError().isEmpty())
        return false;

    serialPort->write("CONF:VOLT:DC 10,0.00001\r\n"); // 10 V, 6 1/2 digit
    timer.start(1000);
    el.exec();

    if(!checkError().isEmpty())
        return false;

    serialPort->write("DET:BAND 3\r\n"); // Slow input signal change
    timer.start(1000);
    el.exec();

    if(!checkError().isEmpty())
        return false;

    serialPort->write("INP:IMP:AUTO ON\r\n"); // > 10 GOm impedance
    timer.start(3000);                      // and long delay for relay
    el.exec();

    if(!checkError().isEmpty())
        return false;

    serialPort->write("VOLT:NPLC 100\r\n"); // Slow measurement
    timer.start(2000);
    el.exec();

    if(!checkError().isEmpty())
        return false;
#else
    timer.start(3000);
    el.exec();
#endif

    inited = 1;
    emit initVoltmeterDone();

    busyFlag = 0;
    return true;
}

void DividerReader::getVoltage()
{
    if(busyFlag)
        return;

    if(!checkError().isEmpty())
        return;

    busyFlag = 1;

#ifndef VIRTUAL_MODE
    serialPort->write("READ?\r\n");

    if(!waitForMessageReady(8000))
    {
        LOG(WARNING) << tr("%1 (%2)getVoltage() -> timeout").arg(metaObject()->className()).arg(divierReaderName).toStdString();
        busyFlag = 0;
        return;
    }

    double raw_voltage = curr_data.toDouble();
    curr_data.clear();

#else
    QEventLoop el;
    QTimer::singleShot(2000, &el, SLOT(quit()));
    el.exec();
    double raw_voltage = -0.05;
#endif

    LOG(INFO) << QString("%1 (%2 * %3)").arg(raw_voltage * dividerNormCoeff)
                                             .arg(raw_voltage)
                                             .arg(dividerNormCoeff)
                                             .toStdString();
    busyFlag = 0;

    lastVoltage = raw_voltage * dividerNormCoeff;
    lastVoltageTimestamp = QDateTime::currentDateTime();

    emit getVoltageDone(raw_voltage * dividerNormCoeff);
}

void DividerReader::getLastVoltage(double &lastVoltage, QDateTime &lastVoltageTimestamp)
{
    lastVoltage = this->lastVoltage;
    lastVoltageTimestamp = this->lastVoltageTimestamp;
}

bool DividerReader::openPort()
{
    serialPort->setPortName(portName);
    if(serialPort->open(QIODevice::ReadWrite))
    {
        LOG(INFO) << tr("%1 Controller connected to port %2").arg(divierReaderName).arg(portName).toStdString();
        return true;
    }
    else
        return false;
}

void DividerReader::run()
{
    //Проверка инициализации класса
    if(!classInited)
    {
        LOG(ERROR) << tr("Attempt to start %1 (%2) class, while it has not been inited."
                         " Please use init() function before use class.").arg(metaObject()->className())
                      .arg(divierReaderName).toStdString();
        return;
    }

    //Инициализация предка
    ComPort::run();

#ifdef TEST_MODE
    qDebug() << tr("%1 working in thread: ").arg(divierReaderName) << QThread::currentThreadId();
#endif


    //Подключение к порту
#ifndef VIRTUAL_MODE
        openPort();
#else
        LOG(INFO) << tr("%1 Controller working in virtual mode").arg(divierReaderName).toStdString();
#endif

    //Инициализация вольтметра
    if(initVoltmeter())
    {
        //старт мониторинга напряжения
        monitorVoltage();
    }

    exec();
}

QString DividerReader::checkError()
{
#ifndef VIRTUAL_MODE
    serialPort->write("SYST:ERR?\r\n");

    if(!waitForMessageReady(8000))
    {
        QVariantMap err;
        err["err_code"] = AGILENT34401A_ERROR;
        err["AgilentError"] = "timeout";

        emit error(err);

        return "timeout";
    }
    else
    {
        QString currError = curr_data;
        curr_data.clear();

        if(currError.startsWith("+0"))
        {
#ifdef TEST_MODE
            LOG(INFO) << "checkError() -> no error";
#endif
            return QString();
        }
        else
        {
            LOG(WARNING) << tr("%1 (%2) checkError() -> %3").arg(metaObject()->className(),
                                                                 divierReaderName,
                                                                 currError).toStdString();

            QVariantMap err;
            err["err_code"] = AGILENT34401A_ERROR;
            err["AgilentError"] = currError;

            emit error(err);

            return currError;
        }
    }
#else
    return QString();
#endif
}

void DividerReader::monitorVoltage()
{
    while(!stopFlag)
    {
        getVoltage();
    }
}

bool DividerReader::handleError(QVariantMap err)
{
#ifdef TEST_MODE
    LOG(WARNING) << "Catch error:";
#endif

    if(ComPort::handleError(err))
        return true;

    unsigned int errType = err["error_code"].toUInt();

    switch (errType)
    {
        case COM_PORT_CLOSE:
#ifdef TEST_MODE
            LOG(WARNING) << "COM_PORT_CLOSE";
#endif
            //2 попытки открыть порт
            if((!openPort()) || (!openPort()))
            {
                LOG(WARNING) << tr("%1 (%2)Cant open twice").arg(metaObject()->className()).arg(divierReaderName).toStdString();
                return false;
            }
            break;

        case AGILENT34401A_ERROR:
        {
            //Пропуск ошибок неинициализированного вольтметра
            if(!inited)
            {
                return true;
            }



            QString voltmeterError = err["AgilentError"].toString();

#ifdef TEST_MODE
            LOG(WARNING) << tr("AGILENT34401A_ERROR: %1").arg(voltmeterError).toStdString();
#endif

            if(voltmeterError == "timeout")
                return true;

            //Сброс состояния удаленного управления. Попытка исправления путем реинициализации вольтметра.
            if(voltmeterError.contains("+550"))
                return initVoltmeter();

            return false;
        }

        default:
            break;
    }

    LOG(INFO) << tr("%1 (%2)getVoltage() catch error").arg(metaObject()->className()).arg(divierReaderName).toStdString();
    return false;
}

DividerReader::~DividerReader()
{
    stopFlag = true;
}
