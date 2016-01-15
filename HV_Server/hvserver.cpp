#include "hvserver.h"
#include "hvmaincontroller.h"

#ifdef TEST_MODE
    #include <QDebug>
#endif

HVServer::HVServer(IniManager *manager, int port, QObject *parent): TcpServer(port, parent)
{
    voltage1Block = -1;
    voltage2Block = -1;

    this->manager = manager;

    //создание классов для взаимодействия с основным блоком напряжения
    divider1 = new DividerReader("Divider1", manager);
    divider1->init();
    divider1->start();
    connect(divider1, SIGNAL(getVoltageDone(double)), this, SLOT(onDivider1GetVoltageDone(double)), Qt::QueuedConnection);

    bool ok;
    hvControlerDivider1 = new HvMainController(manager, "HVController1", &voltage1Block, &ok);
    hvControlerDivider1->start();
    connect(this, SIGNAL(setDivider1Voltage(double)), hvControlerDivider1, SLOT(setVoltage(double)), Qt::QueuedConnection);
    connect(hvControlerDivider1, SIGNAL(setVoltageDone()), this, SLOT(onDivider1SetVoltageDone()));
    connect(this, SIGNAL(setDivider1VoltageAndCheck(QVariantMap)),
            hvControlerDivider1, SLOT(setVoltageAndCheck(QVariantMap)),
            Qt::QueuedConnection);
    connect(hvControlerDivider1, SIGNAL(voltageSetAndCheckDone(QVariantMap)),
            this, SLOT(onBlock1SetVoltageAndCheckDone(QVariantMap)),
            Qt::QueuedConnection);


    //создание классов для взаимодействия с блоком смещения
    divider2 = new DividerReader("Divider2", manager);
    divider2->init();
    divider2->start();
    connect(divider2, SIGNAL(getVoltageDone(double)), this, SLOT(onDivider2GetVoltageDone(double)), Qt::QueuedConnection);

    hvControlerDivider2 = new HVControler(manager, "HVController2", &voltage2Block, &ok);
    hvControlerDivider2->start();
    connect(this, SIGNAL(setDivider2Voltage(double)), hvControlerDivider2, SLOT(setVoltage(double)), Qt::QueuedConnection);
    connect(hvControlerDivider2, SIGNAL(setVoltageDone()), this, SLOT(onDivider2SetVoltageDone()));
    connect(this, SIGNAL(setDivider2VoltageAndCheck(QVariantMap)),
            hvControlerDivider2, SLOT(setVoltageAndCheck(QVariantMap)),
            Qt::QueuedConnection);
    connect(hvControlerDivider2, SIGNAL(voltageSetAndCheckDone(QVariantMap)),
            this, SLOT(onBlock2SetVoltageAndCheckDone(QVariantMap)),
            Qt::QueuedConnection);


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

    hvControlerDivider2->exit();
    hvControlerDivider2->deleteLater();
}

void HVServer::dividerGetVoltageDone(QString dividerName, double voltage)
{
    QVariantMap message;
    message.insert("type", "answer");
    message.insert("block", dividerName);
    message.insert("answer_type", "get_voltage");
    message.insert("voltage", QString("%1").arg(voltage));

#ifdef TEST_MODE
    qDebug() << QThread::currentThreadId() << tr("Send message get_voltage: divider - %1, voltage - %2.")
                                                .arg(dividerName).arg(voltage);
#endif
    sendMessage(message);
}

void HVServer::setVoltageAndCheckDone(QString blockName, QVariantMap status)
{
    status["type"] =  "answer";
    status["block"] = blockName;
    status["answer_type"] =  "set_voltage_and_check";

    sendMessage(status);
}

void HVServer::dividerSetVoltageDone(QString dividerName)
{
    QVariantMap message;
    message.insert("type", "answer");
    message.insert("block", dividerName);
    message.insert("answer_type", "set_voltage");
    message.insert("status", "ok");

#ifdef TEST_MODE
    qDebug() << QThread::currentThreadId() << tr("Send message set_voltage: divider - %1.")
                                                .arg(dividerName);
#endif
    sendMessage(message);
}

void HVServer::onDivider2GetVoltageDone(double voltage)
{
    voltage2Block = voltage - voltage1Block;

#ifdef TEST_MODE
    LOG(INFO) << "Voltage 2 block:" << voltage2Block;
#endif

    //dividerGetVoltageDone("2", voltage);
}

void HVServer::onDivider1GetVoltageDone(double voltage)
{
    voltage1Block = voltage;

#ifdef TEST_MODE
    LOG(INFO) << "Voltage 1 block:" << voltage1Block;
#endif

    //dividerGetVoltageDone("1", voltage);
}

void HVServer::onDivider1SetVoltageDone()
{
    dividerSetVoltageDone("1");
}

void HVServer::onBlock1SetVoltageAndCheckDone(QVariantMap answer)
{
    setVoltageAndCheckDone("1", answer);
}

void HVServer::onDivider2SetVoltageDone()
{
    dividerSetVoltageDone("2");
}

void HVServer::onBlock2SetVoltageAndCheckDone(QVariantMap answer)
{
    setVoltageAndCheckDone("2", answer);
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

    if(commandType == "set_voltage_and_check")
    {
#ifdef TEST_MODE
        qDebug() << "Receive set_voltage_and_check message.";
#endif
        //проверка необходимых параметров
        int block = message["block"].toInt();
        if(block != 1 && block != 2)
        {
            sendUnknownBlockError(message["block"].toString());
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

        switch(block)
        {
            case 1:
                emit setDivider1VoltageAndCheck(message);
                break;
            case 2:
                emit setDivider2VoltageAndCheck(message);
                break;
        }
    }
    else
        if(commandType == "set_voltage")
        {
    #ifdef TEST_MODE
            qDebug() << "Receive set_voltage message.";
    #endif
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
