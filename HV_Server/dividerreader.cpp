#include "dividerreader.h"

DividerReader::DividerReader(QString DividerName, IniManager *manager, QObject *parent) : ComPort(manager, parent)
{
    inited = 0;

    if(!manager->getSettingsValue(DividerName, "NormingCoefficient").isValid())
    {
        LOG(ERROR) << QString("Parameter <NormingCoefficient> in %1 not set").arg(DividerName).toStdString();
        QCoreApplication::exit();
        return;
    }
    this->dividerNormCoeff = manager->getSettingsValue(DividerName, "NormingCoefficient").toDouble();

    QVariant portName = manager->getSettingsValue(DividerName, "COM");
    if(!portName.isValid() || portName.toString() == "SET VALUE")
    {
        LOG(ERROR) << QString("Setting \"COM\" in \"%1\" has not set. Stop server.").arg(DividerName).toStdString();
        qApp->exit(1);
        return;
    }
    else
        serialPort->setPortName(portName.toString());



    if(serialPort->open(QIODevice::ReadWrite))
        LOG(INFO) << "DividerReader Controller connected to port " << portName.toString().toStdString();
}

void DividerReader::initVoltmeter()
{
    busyFlag = 1;

    QEventLoop el;
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &timer, SLOT(stop()));

    serialPort->write("SYST:REM\r\n"); // Switch voltmeter to remote control (once)
    timer.start(1000);
    el.exec();
    serialPort->write("CONF:VOLT:DC 10,0.00001\r\n"); // 10 V, 6 1/2 digit
    timer.start(1000);
    el.exec();
    serialPort->write("DET:BAND 3\r\n"); // Slow input signal change
    timer.start(1000);
    el.exec();
    serialPort->write("INP:IMP:AUTO ON\r\n"); // > 10 GOm impedance
    timer.start(3000);                      // and long delay for relay
    el.exec();
    serialPort->write("VOLT:NPLC 100\r\n"); // Slow measurement
    timer.start(2000);
    el.exec();

    inited = 1;
    emit initVoltmeterDone();
    busyFlag = 0;
}

void DividerReader::getVoltage()
{
    busyFlag = 1;

    QEventLoop el;
    connect(this, SIGNAL(receiveFinished()), &el, SLOT(quit()));

    serialPort->write("READ?\r\n");
    el.exec();

    double raw_voltage = curr_data.toDouble();


    LOG(INFO) << QString("%1 (%2 * %3)").arg(raw_voltage * dividerNormCoeff)
                                             .arg(raw_voltage)
                                             .arg(dividerNormCoeff)
                                             .toStdString();
    curr_data.clear();

    busyFlag = 0;

    emit getVoltageDone(raw_voltage * dividerNormCoeff);
}

DividerReader::~DividerReader()
{
}
