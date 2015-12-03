#include "hvhandler.h"
#include <QTimer>

HVHandler::HVHandler(QString ip, int port, QObject *parent) : ServerHandler(ip, port, parent)
{
}

void HVHandler::initServer(bool *ok)
{
    if(hasError())
    {
        TcpProtocol::setOk(0, ok);
        return;
    }

    for(int i = 1; i <= 2; i++)
    {
        //создание посылки
        QVariantMap message;
        message["type"] = "command";
        message["block"] = tr("%1").arg(i);

        //проверка, инициализирован ли уже вольтметр
        message["command_type"] = "check_init";

        sendMessage(message);

        if(!waitForMessage())
        {
            TcpProtocol::setOk(0, ok);
            return;
        }

        if(!lastMessage["inited"].toBool())
        {
            message["command_type"] = "init";
            sendMessage(message);
            if(!waitForMessage(20000))
            {
                TcpProtocol::setOk(0, ok);
                return;
            }

            //проверка ответа
            if(lastMessage["status"] != "ok")
            {
                TcpProtocol::setOk(0, ok);
                return;
            }
        }
    }

    TcpProtocol::setOk(true, ok);

    emit serverInited();
    emit ready();
}

void HVHandler::setVoltage(int block, double value)
{
    if(hasError())
        return;

    if(block != 1 && block != 2)
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
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
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
    if(hasError())
        return;

    if(!hasInited())
    {
        QVariantMap error_info;
        error_info["description"] = "HV Has not inited yet!";
        emit error(error_info);
        return;
    }

    if(block != 1 && block != 2)
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
    //serializer.setIndentMode(QJson::IndentFull); // в настройки
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

bool HVHandler::handleError(QVariantMap err)
{
    if(ServerHandler::handleError(err))
        return true;

    unsigned int errCode = err["error_code"].toUInt();
    switch(errCode)
    {
        case SERVER_BUSY_ERROR:
        {
            LOG(WARNING) << tr("%1 cacth err: SERVER_BUSY_ERROR").arg(metaObject()->className()).toStdString();
            //Ожидаем 5 с, затем снова пересылаем пакет
            // М.б. стоит перенести в предка

            QEventLoop el;
            QTimer::singleShot(5000, &el, SLOT(quit()));
            el.exec();

            sendRawMessage(lastSentMessage);

            return true;
        }

        default:
            return false;
    }
}
