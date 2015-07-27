#include "hvserver.h"

HVServer::HVServer(IniManager *manager, int port, QObject *parent): TcpServer(port, parent)
{
    this->manager = manager;

    divider1 = new DividerReader("Divider1", manager);
    divider1->start();

    connect(this, SIGNAL(initDivider1()), divider1, SLOT(initVoltmeter()), Qt::QueuedConnection);
    connect(divider1, SIGNAL(initVoltmeterDone()), this, SLOT(onInitDivider1Done()), Qt::QueuedConnection);
    connect(this, SIGNAL(getDivider1Voltage()), divider1, SLOT(getVoltage()), Qt::QueuedConnection);
    connect(divider1, SIGNAL(getVoltageDone(double)), this, SLOT(onDivider1GetVoltageDone(double)), Qt::QueuedConnection);

    hvControlerDivider1 = new HVControler(manager, "HVController1");
    hvControlerDivider1->start();
    connect(this, SIGNAL(setDivider1Voltage(double)), hvControlerDivider1, SLOT(setVoltage(double)), Qt::QueuedConnection);
    connect(hvControlerDivider1, SIGNAL(setVoltageDone()), this, SLOT(onDivider1SetVoltageDone()));


    divider2 = new DividerReader("Divider2", manager);
    divider2->start();

    connect(this, SIGNAL(initDivider2()), divider2, SLOT(initVoltmeter()), Qt::QueuedConnection);
    connect(divider2, SIGNAL(initVoltmeterDone()), this, SLOT(onInitDivider2Done()), Qt::QueuedConnection);
    connect(this, SIGNAL(getDivider2Voltage()), divider2, SLOT(getVoltage()), Qt::QueuedConnection);
    connect(divider2, SIGNAL(getVoltageDone(double)), this, SLOT(onDivider2GetVoltageDone(double)), Qt::QueuedConnection);

    hvControlerDivider2 = new HVControler(manager, "HVController2");
    hvControlerDivider2->start();
    connect(this, SIGNAL(setDivider2Voltage(double)), hvControlerDivider2, SLOT(setVoltage(double)), Qt::QueuedConnection);
    connect(hvControlerDivider2, SIGNAL(setVoltageDone()), this, SLOT(onDivider2SetVoltageDone()));

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(processMessage(MachineHeader,QVariantMap,QByteArray)));

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(processMessage(MachineHeader,QVariantMap,QByteArray)));
}

HVServer::~HVServer()
{
    divider1->exit();
    divider1->deleteLater();

    divider2->exit();
    divider2->deleteLater();

    hvControlerDivider1->exit();
    hvControlerDivider1->deleteLater();
}

void HVServer::dividerGetVoltageDone(QString dividerName, double voltage)
{
    QVariantMap message;
    message.insert("type", "answer");
    message.insert("block", dividerName);
    message.insert("answer_type", "get_voltage");
    message.insert("voltage", QString("%1").arg(voltage));

    sendMessage(message);
}

void HVServer::initDividerDone(QString dividerName)
{
    QVariantMap message;
    message.insert("type", "answer");
    message.insert("block", dividerName);
    message.insert("answer_type", "init");
    message.insert("status", "ok");

    sendMessage(message);
}

void HVServer::dividerSetVoltageDone(QString dividerName)
{
    QVariantMap message;
    message.insert("type", "answer");
    message.insert("block", dividerName);
    message.insert("answer_type", "set_voltage");
    message.insert("status", "ok");

    sendMessage(message);
}

void HVServer::onInitDivider2Done()
{
    initDividerDone("2");
}

void HVServer::onDivider2GetVoltageDone(double voltage)
{
    dividerGetVoltageDone("2", voltage);
}

void HVServer::onInitDivider1Done()
{
    initDividerDone("1");
}

void HVServer::onDivider1GetVoltageDone(double voltage)
{
    dividerGetVoltageDone("1", voltage);
}

void HVServer::onDivider1SetVoltageDone()
{
    dividerSetVoltageDone("1");
}

void HVServer::onDivider2SetVoltageDone()
{
    dividerSetVoltageDone("2");
}

void HVServer::processMessage(MachineHeader header, QVariantMap meta, QByteArray data)
{
    //определение типа посылки
    QString messageType = meta["type"].toString();
    if(messageType == "command")
    {
        processCommand(meta);
    }
    else
        if(messageType == "reply")
        {
            processReply(meta);
        }
        else
        {
            //создание описания ошибки
            QVariantMap errorParams;
            errorParams["error_code"] = QString("%1").arg(UNKNOWN_MESSAGE_ERROR);
            errorParams.insert("stage", "process message type");
            errorParams.insert("description", QString("unknown message type: %1").arg(messageType));

            emit error(errorParams);
            return;
        }
}

void HVServer::sendBusyMessage(QString block)
{
    QVariantMap errorParams;
    errorParams["error_code"] = QString("%1").arg(SERVER_BUSY_ERROR);
    errorParams.insert("stage", "check busy");
    errorParams.insert("description", QString("%1 busy").arg(block));

    emit error(errorParams);
}

void HVServer::sendInitError(QString block)
{
    QVariantMap errorParams;
    errorParams["error_code"] = QString("%1").arg(SERVER_INIT_ERROR);
    errorParams.insert("stage", "check init");
    errorParams.insert("description", QString("%1 has not inited yet").arg(block));

    emit error(errorParams);
}

void HVServer::sendUnknownBlockError(QString block)
{
    QVariantMap errorParams;
    errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
    errorParams.insert("stage", "process block id");
    errorParams.insert("description", QString("unknown block: %1").arg(block));

    emit error(errorParams);
}

void HVServer::sendUnknownCommandError(QString commandType)
{
    QVariantMap errorParams;
    errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
    errorParams.insert("stage", "process command_type");
    errorParams.insert("description", QString("unknown command_type: %1").arg(commandType));

    emit error(errorParams);
}

void HVServer::processCommand(QVariantMap message)
{
    QString commandType = message.value("command_type").toString();
    QString block = message.value("block").toString();

    if(commandType == "check_init")
    {
        QVariantMap answer;
        answer.insert("block", block);
        answer.insert("type", "reply");

        if(block == "1")
        {
            bool inited = divider1->checkInited();
            answer.insert("inited", inited);

            sendMessage(answer);
        }
        else
            if(block == "2")
            {
                bool inited = divider2->checkInited();
                answer.insert("inited", inited);

                sendMessage(answer);
            }
            else
            {
                sendUnknownBlockError(block);
                return;
            }
    }
    else
        if(commandType == "init")
        {
            if(block == "1")
                emit initDivider1();
            else
                if(block == "2")
                    emit initDivider2();
                else
                {
                    //создание описания ошибки
                    sendUnknownBlockError(block);
                    return;
                }
        }
        else
            if(commandType == "get_voltage")
            {
                if(block == "1")
                {
                    if(divider1->checkBusy())
                    {
                        sendBusyMessage("divider1 get block");
                        return;
                    }
                    if(!divider1->checkInited())
                    {
                        sendInitError("1");
                        return;
                    }
                    emit getDivider1Voltage();
                }
                else
                    if(block == "2")
                    {
                        if(divider2->checkBusy())
                        {
                            sendBusyMessage("divider2 get block");
                            return;
                        }
                        if(!divider2->checkInited())
                        {
                            sendInitError("2");
                            return;
                        }
                        emit getDivider2Voltage();
                    }
                    else
                    {
                        sendUnknownBlockError(block);
                        return;
                    }
            }
            else
                if(commandType == "set_voltage")
                {
                    //проверка необходимых параметров
                    int block = message["block"].toInt();
                    if(block != 1 && block != 2)
                    {
                        sendUnknownBlockError(message["block"].toString());
                        return;
                    }

                    if(!message.contains("voltage"))
                    {
                        QVariantMap errorParams;
                        errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
                        errorParams.insert("description", QString("voltage field incorrect").arg(commandType));

                        emit error(errorParams);
                        return;
                    }

                    //проверка занятости
                    bool busy;
                    switch(block)
                    {
                        case 1:
                            busy = hvControlerDivider1->checkBusy();
                            break;
                        case 2:
                            busy = hvControlerDivider2->checkBusy();
                            break;
                    }
                    if(busy)
                    {
                        sendBusyMessage(tr("divider%1 set block").arg(block));
                        return;
                    }


                    double voltage = message.value("voltage").toDouble();

                    switch(block)
                    {
                        case 1:
                            emit setDivider1Voltage(voltage);
                            break;
                        case 2:
                            emit setDivider2Voltage(voltage);
                            break;
                    }
                }
                else
                {
                    //создание описания ошибки
                    sendUnknownCommandError(commandType);
                    return;
                }
}

void HVServer::processReply(QVariantMap message)
{
    //не предусмотрены действия на reply
    QVariantMap errorInfo;
    errorInfo["error_code"] = QString("%1").arg(UNKNOWN_MESSAGE_ERROR);
    errorInfo.insert("stage", "process reply");
    errorInfo.insert("description","no action prescribed for this reply");
    emit error(errorInfo);
}
