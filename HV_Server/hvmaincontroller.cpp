#include "hvmaincontroller.h"

HvMainController::HvMainController(IniManager *manager, QString controllerName, double *voltage, bool *ok, QObject *parent)
    : HVControler(manager, controllerName, voltage, ok, parent), CCPCCommands()
{
    bool coefOk = true;

    if(!manager->getSettingsValue(controllerName, "ControlerCCPCId").isValid())
    {
        processSettingError("ControlerCCPCId", controllerName);
        coefOk = false;
    }

    if(!manager->getSettingsValue(controllerName, "a0").isValid())
    {
        processSettingError("a0", controllerName);
        coefOk = false;
    }

    if(!manager->getSettingsValue(controllerName, "a1").isValid())
    {
        processSettingError("a1", controllerName);
        coefOk = false;
    }

    if(!manager->getSettingsValue(controllerName, "initialShift").isValid())
    {
        manager->setSettingsValue(controllerName, "initialShift", 100);
    }
    initialShift = manager->getSettingsValue(controllerName, "initialShift").toDouble();


    if(!coefOk)
    {
        initSuccesfullFlag = false;
        TcpProtocol::setOk(false, ok);
    }
    else
    {
        controllerId = manager->getSettingsValue(controllerName, "ControlerCCPCId").toInt();
        a0 = manager->getSettingsValue(controllerName, "a0").toDouble();
        a1 = manager->getSettingsValue(controllerName, "a1").toDouble();

        if(controllerId < 0 || controllerId > 24)
            initSuccesfullFlag = false;
        else
        {
#ifndef VIRTUAL_MODE
            camac = new ccpc::CamacImplCCPC7;
            setVoltage(0, initSuccesfullFlag);
#else
            initSuccesfullFlag = true;
#endif
            if(initSuccesfullFlag)
                TcpProtocol::setOk(true, ok);
            else
                TcpProtocol::setOk(false, ok);
        }
    }
}

HvMainController::~HvMainController()
{
#ifndef TEST_MODE
    delete camac;
#endif
}

void HvMainController::setVoltage(double voltage)
{
    if(!initSuccesfullFlag)
    {
        LOG(ERROR) << QString("%1 has not inited succesfully. Exit setting voltage step.").arg(controllerName).toStdString();
        return;
    }
    busyFlag = 1;

#ifndef VIRTUAL_MODE
    bool ok;
    setVoltage(voltage, ok); 
#endif
    settedVoltage = voltage;

    emit setVoltageDone();

    busyFlag = 0;
}

void HvMainController::setVoltage(double voltage, bool &ok)
{
    if(voltage < initialShift)
    {
        //обнуление напряжения на блоке смещения
        lastCorrectionVoltage = 0;
        //установка напряжения
        setVoltageShift(voltage - initialShift);
        setVoltageBase(0, ok);
        return;
    }
    else
    {
        setVoltageShift(0); // установка части напряжения через com порт

        //установка оставшейся части через ЦАП

        double voltageBase = voltage - initialShift;

        setVoltageBase(voltageBase, ok);
    }
}

long HvMainController::encodeVoltage(double voltage)
{
    if(voltage > 26000 || voltage < 0)
    {
        LOG(WARNING) << tr("Bad voltage value: %1. Changing to zero.").arg(voltage).toStdString();
        return 0xffffff;
    }


    //Преобразование напряжения в единицы измерения ccpc
    voltage = a0 + a1 * voltage;

    QString voltageString = tr("%1").arg((voltage * 100000), 6, 'f', 0, QChar('0'));

    QString voltageOutput;
    for(int i = 0; i < voltageString.size() ; i++)
    {
        char currChar = voltageString.at(i).toLatin1();
        switch(currChar)
        {
            case '0':
                voltageOutput += "1111";
                break;
            case '1':
                voltageOutput += "1110";
                break;
            case '2':
                voltageOutput += "1101";
                break;
            case '3':
                voltageOutput += "1100";
                break;
            case '4':
                voltageOutput += "1011";
                break;
            case '5':
                voltageOutput += "1010";
                break;
            case '6':
                voltageOutput += "1001";
                break;
            case '7':
                voltageOutput += "1000";
                break;
            case '8':
                voltageOutput += "0111";
                break;
            case '9':
                voltageOutput += "0110";
                break;
        }
    }

    long out = voltageOutput.toLong(0, 2);
    return out;
}


void HvMainController::setVoltageShift(double voltage)
{
#ifdef TEST_MODE
    qDebug() << tr("setting shift: %1 v").arg(voltage);
#endif
    double settedVoltageBuf = settedVoltage;
    if(serialPort)
        HVControler::setVoltage(initialShift + lastCorrectionVoltage + voltage);
    lastCorrectionVoltage += voltage;
    settedVoltage = settedVoltageBuf;
}

void HvMainController::setVoltageBase(double voltage, bool &ok)
{
    //Инициализация блока управления.
    long data = 0x16;
    ccpc::CamacOp op = NAF(controllerId, 1, 16, data);

    if(!(op.q && op.x))
    {
        ok = false;
        return;
    }

    data = 0x1E;
    NAF(controllerId, 1, 16, data);


    //Установка нулевого напряжения.
    data = encodeVoltage(voltage);
    NAF(controllerId, 0, 16, data);

    long databuf;
    //Проверка того, что напряжение установилось
    NAF(controllerId, 0, 0, databuf);

    if(data != databuf)
    {
        ok = false;
        return;
    }

    ok = true;
    return;
}

void HvMainController::correctVoltage()
{
    qDebug() << controllerName << " correctVoltage works in " << QThread::currentThread() << " thread";

    QEventLoop el;
    QTimer timer;
    connect(&timer,SIGNAL(timeout()), &el, SLOT(quit()));

    QList<double> lastVoltage;
    lastVoltage.push_front(actualVoltage[0]);

    timer.start(1000);

    while(!stopFlag)
    {
        el.exec();
        if(settedVoltage != -1)
        {
            //если напряжение изменилось
            if(lastVoltage.first()!= actualVoltage[0])
            {
                lastVoltage.push_front(actualVoltage[0]);
                while(lastVoltage.size() > 3)
                    lastVoltage.pop_back();

                //корректирование производится только в случае,
                //если напряжение выше начального смещения
                if(actualVoltage[0] > initialShift)
                {
                    if(qAbs(lastVoltage[0] - lastVoltage[1]) < (maxCorrection / 20.))
                    {
                        //вычисление корректирующего напряжения

                        double delta = settedVoltage - actualVoltage[0];
                        delta *= correctionCoefficient;

                        if(qAbs(delta) < maxCorrection)
                        {
                            LOG(INFO) << tr("correcting %1 voltage by %2 v").arg(controllerName)
                                           .arg(delta).toStdString();

                            setVoltageShift(delta);
                        }
                        else
                        {
                            LOG(WARNING) << tr("Volatage error (%1) < max correction (%2)").arg(delta).arg(maxCorrection).toStdString();
                        }
                    }
                }
            }
        }

    }
}
