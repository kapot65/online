#include "tcpclient.h"

TcpClient::TcpClient(IniManager* manager, QObject *parent) : TcpBase(manager, parent)
{
    continue_message = 0;
    networkSession = NULL;
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
    {
        delete networkSession;
        if(connection)
            delete connection;    }


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
    if(!connection->waitForConnected(5000))
    {
        LOG(WARNING) << tr("%1: Catch socket err: %2").arg(metaObject()->className())
                        .arg(connection->errorString()).toStdString();
        QVariantMap err;
        err["error_code"] = UNKNOWN_ERROR;
        emit error(TcpProtocol::wrapErrorInfo(err));
    }

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

bool TcpClient::handleError(QVariantMap err)
{
    if(TcpBase::handleError(err))
        return true;

    unsigned int errCode = err["error_code"].toUInt();
    switch(errCode)
    {
        case CLIENT_NO_ERROR:
            LOG(WARNING) << tr("%1 cacth err: CLIENT_NO_ERROR").arg(metaObject()->className()).toStdString();
            return true;

        case CLIENT_DISCONNECT:
        {
            LOG(WARNING) << tr("%1 cacth err: CLIENT_DISCONNECT").arg(metaObject()->className()).toStdString();

            //Возможно сервер переподключили вручную
            //Ожидание подключения
            if(!waitForConnect(1000))
            {
                connectToServer();
                return waitForConnect(1000);
            }
            else
            {
                LOG(WARNING) << "Catch manual recconection.";
                return true;
            }
        }

        default:
            return false;
    }
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
