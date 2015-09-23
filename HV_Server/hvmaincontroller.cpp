#include "hvmaincontroller.h"

HvMainController::HvMainController(IniManager *manager, QString controllerName, bool *ok, QObject *parent)
    : HVControler(manager, controllerName, ok, parent), CCPCCommands()
{
    if(!manager->getSettingsValue(controllerName, "ControlerCCPCId").isValid())
    {
        processSettingError("ControlerCCPCId", controllerName);
        initSuccesfullFlag = false;
    }
    else
    {
        controllerId = manager->getSettingsValue(controllerName, "ControlerCCPCId").toInt();

        if(controllerId < 0 || controllerId > 24)
            initSuccesfullFlag = false;
        else
        {
#ifndef VIRTUAL_MODE

            camac = new ccpc::CamacImplCCPC7();

            //Инициализация блока управления.
            long data = 0x16;
            ccpc::CamacOp op = NAF(controllerId, 1, 16, data);

            if(!(op.q && op.x))
            {
                initSuccesfullFlag = false;
                return;
            }

            data = 0x1E;
            NAF(controllerId, 1, 16, data);


            //Установка нулевого напряжения.
            data = 0xffffff;
            NAF(controllerId, 0, 16, data);

            long databuf;
            //Проверка того, что напряжение установилось
            NAF(controllerId, 0, 0, databuf);

            if(data != databuf)
            {
                initSuccesfullFlag = false;
                return;
            }
#endif
            initSuccesfullFlag = true;
        }
    }
}

void HvMainController::setVoltage(double voltage)
{
    if(!initSuccesfullFlag)
    {
        LOG(ERROR) << QString("%1 has not inited succesfully. Exit setting voltage step.").arg(controllerName).toStdString();
        return;
    }
}

long HvMainController::encodeVoltage(double voltage)
{
    if(voltage > 4 || voltage < 0)
    {
        LOG(WARNING) << tr("Bad voltage value: %1. Changing to zero.").arg(voltage).toStdString();
        return 0xffffff;
    }

    QString voltageString = tr("%1").arg((voltage * 100000));

    QString voltageOutput;
    for(int i = voltageString.size(); i >=0 ; i--)
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

