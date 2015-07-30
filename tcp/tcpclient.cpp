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
    MachineHeader header;
    QVariantMap meta;
    QByteArray data;
    bool ok;
    bool hasMore;

    readMessageFromStream(tcpSocket, header, meta, data, ok, hasMore);

    if(ok)
    {
        emit receiveMessage(header, meta, data);
#ifdef TEST_MODE
        emit testReseivedMessage(TcpProtocol::createMessage(meta));
#endif
    }

    if(hasMore)
        readMessage();
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
