#include "tcpclient.h"

TcpClient::TcpClient(QString peerName, int peerPort, QObject *parent) : TcpBase(parent)
{
    continue_message = 0;
    networkSession = NULL;

    if(peerName != QString() && peerPort != -1)
    {
        setPeer(peerName, peerPort);
        connectToServer();
    }
}

TcpClient::~TcpClient()
{
    if(networkSession)
        networkSession->deleteLater();
    if(connection)
        connection->deleteLater();
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
    if(connection)
        delete connection;

    connection = new QTcpSocket;
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    connect(connection, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

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

    connection->connectToHost(peerName, peerPort);
    connect(connection, SIGNAL(readyRead()), this, SLOT(readMessage()));
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

    connection->connectToHost(this->peerName, this->peerPort);
    connect(connection, SIGNAL(readyRead()), this, SLOT(readMessage()));
}

void TcpClient::onSocketConnected()
{
    peerName = connection->peerName();
    peerPort = connection->peerPort();

    emit socketConnected(peerName, peerPort);
}

void TcpClient::onSocketDisconnected()
{
    emit socketDisconnected();
}

void TcpClient::sendMessage(QVariantMap message, QByteArray binaryData,  bool *ok)
{
    if(!haveOpenedConnection())
    {
        TcpProtocol::setOk(0, ok);
        return;
    }

    QByteArray preparedMessage = TcpProtocol::createMessage(message, binaryData);

    if(connection->write(preparedMessage) == -1)
    {
        TcpProtocol::setOk(0, ok);
        return;
    }

    TcpProtocol::setOk(1, ok);
}
