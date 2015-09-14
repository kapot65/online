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
            unsigned short data = 0x16;
            ccpc::CamacOp op = NAF(controllerId, 1, 16, data);

            if(!(op.q && op.x))
            {
                initSuccesfullFlag = false;
                return;
            }

            data = 0x1E;
            NAF(controllerId, 1, 16, data);


            //Установка нулевого напряжения.
            data = 0xffffff
            NAF(controllerId, 0, 16, data);

            unsigned char databuf;
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

