#include "tcpclient.h"

TcpClient::TcpClient(QString peerName, int peerPort, QObject *parent) : QThread(parent)
{
    continue_message = 0;

    networkSession = NULL;
    tcpSocket = NULL;

    connectedToPeer = 0;

    if(peerName != QString() && peerPort != -1)
    {
        setPeer(peerName, peerPort);
        connectToServer();
    }

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(saveLastMessage(MachineHeader,QVariantMap,QByteArray)),
            Qt::DirectConnection);
}

TcpClient::~TcpClient()
{
    delete networkSession;
    delete tcpSocket;
}

void TcpClient::setPeer(QString peerName, int peerPort)
{
    this->peerName = peerName;
    this->peerPort = peerPort;
}

void TcpClient::connectToServer()
{
    if(networkSession)
        delete networkSession;
    if(tcpSocket)
        delete tcpSocket;

    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
    {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered)
        {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        networkSession->open();
    }

    tcpSocket->connectToHost(peerName, peerPort);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
}

void TcpClient::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
    {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered)
        {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        networkSession->open();
    }

    tcpSocket->connectToHost(this->peerName, this->peerPort);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
}

void TcpClient::readMessage()
{
    //чтение посылки
    QByteArray message;
    if(!continue_message)
    {
        message += tcpSocket->read(30);
        bool ok;
        //попытка считать бинарный заголовок
        //попытка пробуется на каждом пакете, чтобы избежать поломки сервера
        //в случае когда в бинарном хедере и в фактическом сообщении различаются длины
        header = TcpProtocol::readMachineHeader(message, &ok);
        if(ok)
        {
            fullMessage.clear();
            continue_message = 1;
        }
        else
            if(!continue_message)
            {
                //поток чем-то забит
                LOG(ERROR) << "Error parcing tcp stream: can not parse binary header. Clearing all stream.";
                tcpSocket->readAll();
                return;
            }
    }

    if(continue_message)
    {
        message += tcpSocket->read((header.metaLength + header.dataLenght + 30) -
                                   (fullMessage.size() + message.size()));
        fullMessage.push_back(message);

        if(fullMessage.size() >= header.metaLength + header.dataLenght + 30)
        {
#ifdef TEST_MODE
            emit testReseivedMessage(fullMessage);
#endif
            continue_message = 0;

            QVariantMap meta;
            QByteArray data;

            //попытка распарсить сообщение
            if(!(TcpProtocol::parceMesssage(fullMessage, meta, data)))
            {
                //создание описания ошибки
//                QVariantMap error_info;
//                error_info["stage"] = "parsing message";
//                error_info["description"] =  "unable to parse metadata";
//                emit error(error_info);
                return;
            }

            emit receiveMessage(header, meta, data);

            if(tcpSocket->size())
                readMessage();
        }
    }
}

void TcpClient::onSocketConnected()
{
    peerName = tcpSocket->peerName();
    peerPort = tcpSocket->peerPort();

    connectedToPeer = 1;

    emit socketConnected(peerName, peerPort);
}

void TcpClient::onSocketDisconnected()
{
    connectedToPeer = 0;
    emit socketDisconnected();
}

void TcpClient::saveLastMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    lastMessage = metaData;
}

void TcpClient::sendMessage(QVariantMap message, QByteArray binaryData,  bool *ok)
{
    if(!connectedToPeer)
    {
        if(ok)
            ok[0] = 0;
        return;
    }

    QByteArray preparedMessage = TcpProtocol::createMessage(message, binaryData);

    //    connect(tcpSocket, SIGNAL(disconnected()),
    //            tcpSocket, SLOT(deleteLater()));

    tcpSocket->write(preparedMessage);
    //tcpSocket->disconnectFromHost();
}
