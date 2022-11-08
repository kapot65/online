#include "ccpc7handler.h"

CCPC7Handler::CCPC7Handler(IniManager *manager, QObject *parent) : ServerHandler(manager, parent)
{

}

CCPC7Handler::~CCPC7Handler()
{

}

void CCPC7Handler::processMessage(MachineHeader mashineHeader, QVariantMap metaData, QByteArray binaryData)
{
    if(metaData.value("reply_type").toString() == "aquired_point") {
        QVector<Event> events;
        TcpProtocol::parceMessageWithPoints(mashineHeader, metaData, binaryData, events);
        emit pointAcquired(mashineHeader, metaData, events, binaryData);
    } else if(metaData.value("reply_type").toString() == "get_counters_value") {
        emit counterAcquired(metaData);
    } else if(metaData.value("reply_type").toString() == "init") {
        emit serverInited();
    } else if(metaData.value("reply_type").toString() == "break_acquisition") {
        emit acquisitionBreaked();
    } else if(metaData.value("reply_type").toString() == "reset_counters") {
        emit countersResetted();
    } else if(metaData.value("reply_type").toString() == "acquisition_status") {

        long counts = metaData["count"].toLongLong();
        int currentTime = metaData["current_time"].toInt();
        int totalTime =  metaData["total_time"].toInt();

        emit currentAcqStatus(counts, currentTime, totalTime);
    } else {
        emit unhandledMessage(mashineHeader, metaData, binaryData);
    }
}

void CCPC7Handler::initServer(bool *ok)
{
    //создание посылки
    QVariantMap message;
    message.insert("type", "command");
    message.insert("command_type", "init");

    sendMessage(message);
    if(!waitForMessage())
    {
        TcpProtocol::setOk(false, ok);
        return;
    }

    if(!(lastMessage["status"].toString() == "ok" &&
         lastMessage["reply_type"].toString() == "init"))
    {
        TcpProtocol::setOk(false, ok);
        return;
    }

    TcpProtocol::setOk(true, ok);
}

void CCPC7Handler::acquirePoint(int time, QVariant external_meta, bool splitPoint)
{
    if(hasError())
        return;

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "acquire_point");
    message.insert("acquisition_time", time);

    if(splitPoint)
        message["split"] = true;

    if(external_meta != 0)
        message["external_meta"] = external_meta;

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif

    sendMessage(message);
}

void CCPC7Handler::resetCounters()
{
    if(hasError())
        return;

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "reset_counters");

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif

    sendMessage(message);
}

void CCPC7Handler::breakAcquisition()
{
    if(hasError())
        return;

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "break_acquisition");

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif

    sendMessage(message);
}

void CCPC7Handler::getCountersValue(int counter, QList<QVariant> channels_id, bool reset_after)
{
    if(hasError())
        return;

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "get_counters_value");
    message.insert("counter_id", QString().number(counter));
    message.insert("channels_id", channels_id);
    message.insert("reset_after", QString().number(reset_after));

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif

    sendMessage(message);
}



