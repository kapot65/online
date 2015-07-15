#include "hvhandler.h"

HVHandler::HVHandler(QString ip, int port, QObject *parent) : ServerHandler(ip, port, parent)
{
}

void HVHandler::initServer()
{
    if(hasError())
        return;

#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    //инструмент для ожидания ответа
    QEventLoop el;
    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            &el, SLOT(quit()));

    //создание посылки
    QVariantMap message;
    message.insert("type", "command");
    message.insert("block", "1");

    //проверка, инициализирован ли уже вольтметр
    message["command_type"] = "check_init";

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif
    sendMessage(message);
    el.exec();

    if(!lastMessage["inited"].toBool())
    {
        message["command_type"] = "init";
#ifdef TEST_MODE
        #ifdef USE_QTJSON
        serializedMessage = QJsonDocument::fromVariant(message).toJson();
        #else
        serializedMessage = serializer.serialize(message);
        #endif
        emit sendTestJsonMessage(serializedMessage);
#endif
        sendMessage(message);
            el.exec();
    }

    message.insert("type", "command");
    message["block"] = "2";
    message["command_type"] = "check_init";
#ifdef TEST_MODE
    #ifdef USE_QTJSON
    serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif
    sendMessage(message);
        el.exec();

    if(!lastMessage["inited"].toBool())
    {
        message["command_type"] =  "init";
#ifdef TEST_MODE
        #ifdef USE_QTJSON
        serializedMessage = QJsonDocument::fromVariant(message).toJson();
        #else
        serializedMessage = serializer.serialize(message);
        #endif
        emit sendTestJsonMessage(serializedMessage);
#endif
        sendMessage(message);
        el.exec();
    }

    emit serverInited();
    emit ready();
}

void HVHandler::setVoltage(int block, double value)
{
    if(block != 1 && block != 2)
        return;

    if(hasError())
        return;

    QString divider;
    if(block == 1)
        divider = "1";
    else
        divider = "2";

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "set_voltage");
    message.insert("block", divider);
    message.insert("voltage", QString("%1").arg(value));

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

void HVHandler::getVoltage(int block)
{
    if(!hasInited())
    {
        QVariantMap error_info;
        error_info["description"] = "HV Has not inited yet!";
        emit error(error_info);
        return;
    }

    if(block != 1 && block != 2)
        return;

    if(hasError())
        return;

    QString divider;
    if(block == 1)
        divider = "1";
    else
        divider = "2";

    //создание посылки
    QVariantMap message;
#ifdef TEST_MODE
    #ifndef USE_QTJSON
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки
    #endif
#endif

    message.insert("type", "command");
    message.insert("command_type", "get_voltage");
    message.insert("block", divider);

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

void HVHandler::processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    if(metaData.value("answer_type").toString() == "init")
    {
        static bool divider1_inited = false;
        static bool divider2_inited = false;

        if(metaData.value("block").toString() == "1")
            divider1_inited = true;
        if(metaData.value("block").toString() == "2")
            divider2_inited = true;

        if(divider1_inited && divider2_inited)
            emit serverInited();
    }
    else
        if(metaData.value("answer_type").toString() == "set_voltage")
        {
            emit setVoltageDone(metaData);
        }
        else
            if(metaData.value("answer_type").toString() == "get_voltage")
            {
                emit getVoltageDone(metaData);
            }
            else
            {
                emit unhandledMessage(machineHeader, metaData, binaryData);
            }
}
