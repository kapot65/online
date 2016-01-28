#include "hvhandler.h"
#include <QTimer>

HVHandler::HVHandler(QString ip, int port, QObject *parent) : ServerHandler(ip, port, parent)
{
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

void HVHandler::setVoltageAndCheck(int block, double value, double max_error, int timeout)
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

    message.insert("type", "command");
    message.insert("command_type", "set_voltage_and_check");
    message.insert("block", divider);
    message.insert("voltage", QString("%1").arg(value));
    message["max_error"] = QString("%1").arg(max_error);
    message["timeout"] = QString("%1").arg(timeout);

#ifdef TEST_MODE
    #ifdef USE_QTJSON
    QByteArray serializedMessage = QJsonDocument::fromVariant(message).toJson();
    #else
    QJson::Serializer serializer;
    QByteArray serializedMessage = serializer.serialize(message);
    #endif
    emit sendTestJsonMessage(serializedMessage);
#endif

    sendMessage(message);
}

void HVHandler::processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    if(metaData["answer_type"].toString() == "set_voltage_and_check")
    {
        lastVoltageAndCheckMeta = metaData;
        emit setVoltageAndCheckDone(metaData);
    }
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
QVariantMap HVHandler::getLastVoltageAndCheckMeta()
{
    QVariantMap buffer = lastVoltageAndCheckMeta;
    lastVoltageAndCheckMeta.clear();
    return buffer;
}

