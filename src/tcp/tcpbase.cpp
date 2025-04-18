#include "tcpbase.h"
#include <easylogging++.h>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>

TcpBase::TcpBase(IniManager *manager, QObject *parent) : QThread(parent)
{
#ifdef TEST_MODE
    qDebug() << "TcpBase working in " << QThread::currentThreadId() <<" thread.";
#endif
    continue_message = 0;
    connection = 0;

    this->manager = manager;

    //считывание полей из конфигурацтонного файла
    if(!manager->getSettingsValue(metaObject()->className(), "binary_meta").isValid())
        manager->setSettingsValue(metaObject()->className(), "binary_meta", false);


    if(manager->getSettingsValue(metaObject()->className(), "binary_meta").toBool())
        metaType = QDATASTREAM_METATYPE;
    else
        metaType = JSON_METATYPE;


    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(saveLastMessage(MachineHeader,QVariantMap,QByteArray)),
            Qt::DirectConnection);

    connect(this, SIGNAL(error(QVariantMap)),
            this, SLOT(handleErrorImpl(QVariantMap)));
}

void TcpBase::readMessageFromStream(QIODevice *dev,
                                    MachineHeader &header,
                                    QVariantMap &meta,
                                    QByteArray &data,
                                    bool &ok,
                                    bool &hasMore)
{
    //чтение посылки
    QByteArray message;
    if(!continue_message)
    {
        if(dev->bytesAvailable() < 30)
        {
            ok = false;
            hasMore = false;
            return;
        }

        message += dev->read(30);
        //попытка считать бинарный заголовок
        //попытка пробуется на каждом пакете, чтобы избежать поломки сервера
        //в случае когда в бинарном хедере и в фактическом сообщении различаются длины
        this->header = TcpProtocol::readMachineHeader(message, &ok);
        if(ok)
        {
            fullMessage.clear();
            continue_message = 1;
        }
        else
            if(!continue_message)
            {
                //поток чем-то забит
                //очистка всего потока
                LOG(ERROR) << "Error parcing tcp stream: can not parse binary header. Clearing all stream.";
                dev->readAll();
                hasMore = false;
                return;
            }
    }

    if(continue_message)
    {
        message += dev->read((this->header.metaLength + this->header.dataLenght + 30) -
                                   (fullMessage.size() + message.size()));
        fullMessage.push_back(message);

        if(fullMessage.size() >= this->header.metaLength + this->header.dataLenght + 30)
        {
            //Сообщение собрано. Попытка распарсить его.
            continue_message = 0;

            //попытка распарсить сообщение
            if(!(TcpProtocol::parceMessage(fullMessage, meta, data)))
                ok = false;
            else
                ok = true;

            if(dev->bytesAvailable())
                hasMore = true;
            else
                hasMore = false;

            header = this->header;
        }
    }
}

bool TcpBase::handleError(QVariantMap err)
{
    return false;
}

void TcpBase::handleErrorImpl(QVariantMap err)
{
    if(!handleError(err))
        emit unhandledError(err);
    else
       emit ready();
}

void TcpBase::readMessage()
{
    MachineHeader header;
    QVariantMap meta;
    QByteArray data;
    bool ok;
    bool hasMore;

    readMessageFromStream(connection, header, meta, data, ok, hasMore);

    if(ok)
    {
        emit receiveMessage(header, meta, data);
#ifdef TEST_MODE
        emit testReseivedMessage(TcpProtocol::createMessage(meta, QByteArray(), metaType));
#endif
    }

    if(hasMore && connection->bytesAvailable())
        readMessage();
}

bool TcpBase::waitForMessage(int timeout)
{
    QEventLoop el;
    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)), &el, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout);
    el.exec();

    if(timer.isActive())
    {
        timer.stop();
        return true;
    }
    else
        return false;
}

bool TcpBase::waitForConnect(int timeout)
{
    if(haveOpenedConnection())
        return true;

    QEventLoop el;
    connect(connection, SIGNAL(connected()), &el, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));

    timer.setSingleShot(true);
    timer.start(timeout);
    el.exec();

    if(timer.isActive())
    {
        timer.stop();
        return true;
    }
    else
        return false;
}

void TcpBase::saveLastMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    lastMessage = metaData;
}

void TcpBase::sendMessage(QVariantMap message, QByteArray binaryData,  bool *ok, QTcpSocket *socket)
{
    QByteArray prepairedMessage = TcpProtocol::createMessage(message, binaryData, metaType);
    sendRawMessage(prepairedMessage, ok, socket);
}

void TcpBase::sendRawMessage(QByteArray message, bool *ok, QTcpSocket *socket)
{
    lastSentMessage = message;

    if(!socket)
        socket = connection;
    if(!socket || !socket->isOpen())
    {
        if(!socket)
        {
            LOG(WARNING) << tr("%1:Socket doesn't exist. Sending Message to nowhere.").arg(metaObject()->className()).toStdString();
            TcpProtocol::setOk(false, ok);
            return;
        }

        if(!socket->isOpen())
            LOG(ERROR) << tr("Catch socket not open error on %1").arg(metaObject()->className()).toStdString();

        TcpProtocol::setOk(false, ok);
        return;
    }

    if(socket->write(message) != -1)
        TcpProtocol::setOk(true, ok);
    else
    {
        LOG(ERROR) << tr("Catch socket->write error on %1: %2").arg(metaObject()->className()).arg(socket->errorString()).toStdString();
        TcpProtocol::setOk(false, ok);
    }
}
