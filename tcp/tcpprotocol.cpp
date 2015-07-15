#include "tcpprotocol.h"
#include <QDateTime>
#include <QDataStream>

QMap<int, unsigned short> TcpProtocol::getAviableMeasuteTimes()
{
    QMap<int, unsigned short> aviableMeasureTimes;

    aviableMeasureTimes.insert(5, 4);
    aviableMeasureTimes.insert(10, 8);
    aviableMeasureTimes.insert(15, 12);
    aviableMeasureTimes.insert(20, 16);
    aviableMeasureTimes.insert(50, 20);
    aviableMeasureTimes.insert(100, 24);
    aviableMeasureTimes.insert(200, 28);

    return aviableMeasureTimes;
}

MachineHeader  TcpProtocol::readMachineHeader(QByteArray &message, bool *ok)
{
    MachineHeader header;
    //проверка на соответсвие формату
    if(!(message.startsWith("#!") && message.size() >= 30 && message.mid(26, 2) == "!#"))
    {
        if(ok)
            ok[0] = 0;
        return header;
    }
    else
        if(ok)
            ok[0] = 1;


    QDataStream ds(&message, QIODevice::ReadOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds.skipRawData(2);

    ds >> header.type;
    ds >> header.time;
    ds >> header.metaType;
    ds >> header.metaLength;
    ds >> header.dataType;
    ds >> header.dataLenght;

    ds.skipRawData(4);

    return header;
}

QByteArray TcpProtocol::createMessage(QVariantMap meta, QByteArray data, unsigned int metaType, unsigned int binaryType)
{
#ifdef USE_QTJSON
    QJsonObject JsonObj = QJsonObject::fromVariantMap(meta);
    QJsonDocument doc(JsonObj);
    QByteArray serializedMeta = doc.toJson();
#else
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки
    QByteArray serializedMeta = serializer.serialize(meta);
#endif

    //создание заголовка
    MachineHeader header;
    header.type = 0x0140;
    header.metaType = metaType;
    header.metaLength = serializedMeta.size() + 2; //отступы тоже включаются в размер метаданных
    header.dataType = binaryType;
    header.dataLenght = data.size();
    QByteArray machineHeader = TcpProtocol::writeMachineHeader(header);

    QByteArray prepairedMessage = machineHeader + serializedMeta + "\r\n" + data;
    return prepairedMessage;
}

bool TcpProtocol::parceMesssage(QByteArray message, QVariantMap &meta, QByteArray &data, bool headerOnly)
{
    MachineHeader header = readMachineHeader(message);

    //Парсинг сообщения в соотвествии с форматом
    switch(header.metaType)
    {
        //мета в формате QJson
        case 1:
        {
            message.remove(0,30);
            QByteArray JSonMeta = message.mid(0, header.metaLength);
            //обрезание окончания(если есть)
            while(JSonMeta.endsWith("\r\n"))
                JSonMeta.chop(2);

#ifdef USE_QTJSON
            QJsonDocument doc = QJsonDocument::fromJson(JSonMeta);
            if(doc.isNull())
                return 0;
            meta = doc.toVariant().toMap();
#else
            QJson::Parser parser;
            bool ok;
            meta = parser.parse(JSonMeta, &ok).toMap();
            if(!ok)
                return 0;
#endif
            if(!headerOnly)
                data = message.mid(header.metaLength, header.dataLenght);
            return 1;
        }
        default:
            return 0;
    }
}

QByteArray TcpProtocol::writeMachineHeader(MachineHeader header)
{
    QByteArray machineHeader;
    QDataStream ds(&machineHeader, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.writeRawData("#!",2);
    int currDateTime = quint32((QDateTime::currentDateTime()
                               .toMSecsSinceEpoch())/ 1000);

    ds << header.type;
    ds << currDateTime;
    ds << header.metaType;
    ds << header.metaLength;
    ds << header.dataType;
    ds << header.dataLenght;

    ds.writeRawData("!#\r\n", 4);

    return machineHeader;
}

QVariantMap TcpProtocol::wrapErrorInfo(QVariantMap error_info)
{
    error_info["type"] = "reply";
    error_info["reply_type"] = "error";

    //если код ошибки не указан, то она воспринимается как неизвестная
    if(!error_info.contains("error_code"))
        error_info["error_code"] = QString("%1").arg(UNKNOWN_ERROR);

    return error_info;
}

QVariantMap TcpProtocol::unwrapErrorInfo(QVariantMap error_info)
{
    error_info.remove("type");
    error_info.remove("reply_type");

    return error_info;
}

bool TcpProtocol::checkMessageForError(QVariantMap message)
{
    if(message["reply_type"].toString() == "error")
        return 1;
    else
        return 0;
}

QByteArray TcpProtocol::createMessageWithPoints(QVariantMap meta, QVector<Event> events, unsigned int metaType, unsigned int binaryType)
{
    //сохранение данных в бинарном виде
    QByteArray binaryData;
    QDataStream ds(&binaryData, QIODevice::WriteOnly);
    switch (binaryType)
    {
        //сериализация данных обычным способом
        case DIRECT_BINARY:
        {
            meta["binary_data_structure"] = QString("Direct: data(unsigned short(%1 bytes)), time(int(%2 bytes)), "
                                             "valid(bool(%3 bytes))").arg(sizeof(unsigned short))
                                             .arg(sizeof(int)).arg(sizeof(bool));
            for(int i = 0; i < events.size(); i++)
            {
                ds.writeRawData((const char*)(&(events[i].data)), sizeof(unsigned short));
                ds.writeRawData((const char*)(&(events[i].time)), sizeof(int));
                ds.writeRawData((const char*)(&(events[i].valid)), sizeof(bool));
            }
            break;
        }
        //сериализация с помощью Qt
        case QDATASTREAM_BINARY:
        {
            meta["binary_data_structure"] = "Qt4_8: QDataStream. See src 'event.h'";
            ds << events;
            break;
        }
        default:
            break;
    }
    meta["binary_size"] = QString().number(binaryData.size());

    return createMessage(meta, binaryData, metaType, binaryType);
}

bool TcpProtocol::parceMessageWithPoints(MachineHeader messageHeader, QVariantMap messageMeta, QByteArray messageData, QVector<Event> &events)
{
    //проверка метаданных
    switch(messageHeader.dataType)
    {
        case UNDEFINED_BINARY:
            return false;
        case DIRECT_BINARY:
        {
            QDataStream ds(&messageData, QIODevice::ReadOnly);

            events.clear();
            while(!ds.atEnd())
            {
                Event curr_ev;
                ds.readRawData((char*)(&(curr_ev.data)), sizeof(unsigned short));
                ds.readRawData((char*)(&(curr_ev.time)), sizeof(int));
                ds.readRawData((char*)(&(curr_ev.valid)), sizeof(bool));

                events.push_back(curr_ev);
            }
            return true;
        }
        case QDATASTREAM_BINARY:
        {
            QDataStream ds(&messageData, QIODevice::ReadOnly);
            ds.setVersion(QDataStream::Qt_4_8);
            ds >> events;
            return true;
        }
        default:
            return false;
    }
}

bool TcpProtocol::parceMessageWithPoints(QByteArray message, QVariantMap &meta, QVector<Event> &events)
{
    bool ok;
    MachineHeader header = TcpProtocol::readMachineHeader(message, &ok);

    if(!ok)
        return false;

    QByteArray data;

    if(!TcpProtocol::parceMesssage(message, meta, data))
        return false;

    return parceMessageWithPoints(header, meta, data, events);
}

MachineHeader::MachineHeader()
{
    time = 0;
    type = 0;
    metaType = 0;
    metaLength = 0;
    dataType = 0;
    dataLenght = 0;
}
