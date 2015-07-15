#include "commandhandler.h"

bool CommandHandler::checkInit()
{
    //инициализация проверяется по указателю на камак
    if(camac == NULL)
    {
        busyFlag = 0;
        //создание описания ошибки
        QVariantMap errorParams;
        errorParams["error_code"] = QString("%1").arg(SERVER_INIT_ERROR);
        errorParams.insert("stage", QString("check init"));
        errorParams.insert("description", "CCPC hasn't set yet");

        emit error(errorParams);
        return 0;
    }
    else
        return 1;
}

void CommandHandler::ProcessNAF(QVariantMap message)
{
    if(!checkInit())
        return;

    busyFlag = 1;

    ccpc::CamacOp op = messageToCamacOp(message);

    //выполнение операции
    camac->exec(op);

    //создание посылки
    QVariantMap messageToSend = camacOpToMessage(op);

    messageToSend.insert("type", "reply");
    messageToSend.insert("reply_type", "NAF");
    messageToSend.insert("status", "ok");

    emit sendMessage(messageToSend, QByteArray());

    busyFlag = 0;
}

void CommandHandler::processResetCounters(QVariantMap message)
{
    if(!checkInit())
        return;

    busyFlag = 1;

    resetCounters();

    QVariantMap messageToSend;
    messageToSend.insert("type", "reply");
    messageToSend.insert("reply_type", "reset_counters");
    messageToSend.insert("status", "ok");

    emit sendMessage(messageToSend, QByteArray());

    busyFlag = 0;
}

void CommandHandler::processGetCountersValue(QVariantMap message)
{
    if(!checkInit())
        return;

    busyFlag = 1;

    if(!message.value("counter_id").isValid() || !message.value("channels_id").isValid())
    {
        //создание описания ошибки
        QVariantMap errorParams;
        errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
        errorParams.insert("stage", QString("get_counters_value"));
        errorParams.insert("description", "counter_id or channels_id not set");

        emit error(errorParams);
        busyFlag = 0;
        return;
    }

    bool withReset = 0;
    if(message.value("channels_id").isValid())
        withReset = message.value("reset_after").toBool();

    int counter = message.value("counter_id").toInt();

    QList<QVariant> channels_id = message.value("channels_id").toList();
    QList<QVariant> channels_value;

    for(int i  = 0 ; i < channels_id.size(); i++)
    {
        channels_value.push_back(QString().number(getCounterValue(counter, channels_id[i].toInt(), withReset)));
    }


    QVariantMap messageToSend;
    messageToSend.insert("type", "reply");
    messageToSend.insert("reply_type", "get_counters_value");
    messageToSend.insert("status", "ok");
    messageToSend.insert("channels_id", channels_id);
    messageToSend.insert("channels_value", channels_value);
    messageToSend.insert("time", QTime::currentTime());
    messageToSend.insert("counter", QString().number(counter));

    emit sendMessage(messageToSend, QByteArray());

    busyFlag = 0;
}

QVariantMap CommandHandler::camacOpToMessage(ccpc::CamacOp op)
{
    QVariantMap message;

    message.insert("type", "command");
    message.insert("command_type", "NAF");

    message.insert("n", QString().number(op.n));
    message.insert("a", QString().number(op.a));
    message.insert("f", QString().number(op.f));

    message.insert("q", QString().number(op.q));
    message.insert("x", QString().number(op.x));

    QList<QVariant> data;
    for(int i = 0; i < op.data.size(); i++)
    {
        data.push_back(QVariant(QString().number((int)op.data[i])));
    }
    message.insert("data", QVariant(data));

    message.insert("mode", QString().number((int)(op.mode)));
    message.insert("dir", QString().number((int)(op.dir)));

    return message;
}

ccpc::CamacOp CommandHandler::messageToCamacOp(QVariantMap message)
{
    ccpc::CamacOp op;

    //преобразование в тип CamacOp
    op.n = message.value("n").toInt();
    op.a = message.value("a").toInt();
    op.f = message.value("f").toInt();

    QList<QVariant> data = message.value("data").toList();
    if(!data.isEmpty())
    {
        op.data.clear();
        for(int i = 0; i < data.size(); i++)
        {
            op.data.push_back(data[i].toLongLong());
        }
    }

    op.q = message.value("q").toInt();
    op.x = message.value("x").toInt();

    op.mode = ccpc::CamacMode(message.value("mode").toInt());
    op.dir = ccpc::CamacDataDir(message.value("dir").toInt());

    return op;
}

void CommandHandler::processInit(QVariantMap message)
{
    busyFlag = 1;
    // Инициализация Камака
    bool reset = (bool)(!camac);
    if(!camac)
        delete camac;

#ifdef Q_OS_WIN

    if((!settings->getSettingsValue("vCamac", "ip").isValid()) || (!settings->getSettingsValue("vCamac", "port").isValid()))
    {
        //создание описания ошибки
        QVariantMap errorParams;
        errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
        errorParams.insert("stage", QString("init"));
        errorParams.insert("description", "settings for virtual camac not set");

        emit error(errorParams);
        busyFlag = 0;
        return;
    }

    camac = new ccpc::CamacImplCCPC7(settings->getSettingsValue("vCamac", "ip").toString(),
                                     settings->getSettingsValue("vCamac", "port").toInt(), this);


    QEventLoop pause;
    connect(camac, SIGNAL(connected()), &pause, SLOT(quit()));
    pause.exec();

    camac->init();
#elif defined(Q_OS_LINUX)

    camac = new ccpc::CamacImplCCPC7();

#endif

    Z();
    C();

    //проталкивание созданного объекта в главный поток
    //camac->moveToThread(QApplication::instance()->thread());

    //создание сообщения об инициализации
    QVariantMap messageToSend;
    messageToSend.insert("type", "reply");
    messageToSend.insert("reply_type", "init");
    messageToSend.insert("status", "ok");
    messageToSend.insert("reseted", QString().number(!reset));

    emit sendMessage(messageToSend, QByteArray());

    busyFlag = 0;
}

void CommandHandler::processAcquirePoint(QVariantMap message)
{
    if(!checkInit())
        return;

    busyFlag = 1;

    if(!message.value("acquisition_time").isValid())
    {
        //создание описания ошибки
        QVariantMap errorParams;
        errorParams["error_code"] = QString("%1").arg(INCORRECT_MESSAGE_PARAMS);
        errorParams.insert("stage", QString("acquire point"));
        errorParams.insert("description", "parameter acquisition_time not found");

        emit error(errorParams);
        busyFlag = 0;
        return;
    }

    unsigned short acqisitionTime = message.value("acquisition_time").toInt();


    bool manuallyBreak;
    QDate acqDate = QDate::currentDate();
    QTime acqTimeStart = QTime::currentTime();
    QVector<Event> events = acquirePoint(acqisitionTime, &manuallyBreak);
    QTime acqTimeEnd = QTime::currentTime();

    //создание сообщения
    QVariantMap messageToSend;
    messageToSend.insert("type", "reply");
    messageToSend.insert("date", acqDate.toString("yyyy.MM.dd"));
    messageToSend.insert("start_time", acqTimeStart.toString("hh:mm:ss.zzz"));
    messageToSend.insert("end_time", acqTimeEnd.toString("hh:mm:ss.zzz"));
    messageToSend.insert("reply_type", "aquired_point");
    messageToSend.insert("status", "ok");
    messageToSend.insert("total_events", QString().number(events.size()));
    //протаскивание мета информации
    if(message.contains("external_meta"))
        messageToSend["external_meta"] = message["external_meta"];

    if(manuallyBreak)
        messageToSend.insert("breaked", "true");

    QByteArray prepairedMessage = TcpProtocol::createMessageWithPoints(messageToSend, events,
                                                                       JSON_METATYPE, DIRECT_BINARY);
    emit sendRawMessage(prepairedMessage);
    busyFlag = 0;
}

void CommandHandler::processBreakAcquisition(QVariantMap message)
{
    if(!checkInit())
        return;

    busyFlag = 1;

    emit breakAcquisition();

    //создание сообщения об инициализации
    QVariantMap messageToSend;
    messageToSend.insert("type", "reply");
    messageToSend.insert("reply_type", "break_acquisition");
    messageToSend.insert("status", "ok");

    emit sendMessage(messageToSend, QByteArray());

    busyFlag = 0;
}

CommandHandler::CommandHandler(CamacServerSettings *settings, QObject *parent): CamacAlgoritm(parent)
{
    camac = 0;
    busyFlag = 0;
    this->settings = settings;

    connect(this, SIGNAL(breakAcquisition()), this, SLOT(on_breakAcquisition()));
}
