#include "tcpbase.h"
#include <easylogging++.h>
#include <QEventLoop>
#include <QTimer>

TcpBase::TcpBase(QObject *parent): QThread(parent)
{
#ifdef TEST_MODE
    qDebug() << "TcpBase working in " << QThread::currentThreadId() <<" thread.";
#endif
    continue_message = 0;
    connection = 0;

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(saveLastMessage(MachineHeader,QVariantMap,QByteArray)),
            Qt::DirectConnection);
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
        emit testReseivedMessage(TcpProtocol::createMessage(meta));
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
