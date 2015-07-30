#include "tcpserver.h"

TcpServer::TcpServer(int port, QObject *parent) : QObject(parent)
{
//Подключение сервера к порту
    networkSession = NULL;
    clientConnection = 0;
    this->port = port;

    connect(this, SIGNAL(error(QVariantMap)), this, SLOT(on_error(QVariantMap)));

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
    {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
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

        serverReady();
    }
    else
    {
        sessionOpened();
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
}

void TcpServer::sessionOpened()
{
    // Save the used configuration
    if (networkSession)
    {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer;
    if (!tcpServer->listen(QHostAddress::Any, port))
    {
        //LOG(ERROR) << QString("Unable to start the server: %1.").arg(tcpServer->errorString()).toStdString();
        //QCoreApplication::quit();
        return;
    }

    serverReady();
}

void TcpServer::processNewConnection()
{
    if(clientConnection && clientConnection->isOpen())
    {
        connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
        clientConnection->disconnectFromHost();
//        QTcpSocket *newConnection = tcpServer->nextPendingConnection();
//        connect(newConnection, SIGNAL(disconnected()), newConnection, SLOT(deleteLater()));

//        QVariantMap err;
//        err["error_code"] = MULTIPLE_CONNECTION;
//        err["desctiption"] = "Server already has active connection. Closing this connection";
//        err = TcpProtocol::wrapErrorInfo(err);

//        sendMessage(err, QByteArray(), NULL, newConnection);
//        newConnection->disconnectFromHost();
    }

    clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readMessage()));
    emit newConnection(clientConnection->peerName(), clientConnection->peerPort());
}

void TcpServer::serverReady()
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendReady()));
    connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
    timer->start(100);
}

void TcpServer::sendReady()
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();



    //LOG(INFO) << tr("The server is running on IP: %1 port: %2")
    //                     .arg(ipAddress).arg(tcpServer->serverPort()).toStdString();

    emit serverReady(ipAddress, port);
}

void TcpServer::readMessage()
{
    MachineHeader header;
    QVariantMap meta;
    QByteArray data;
    bool ok;
    bool hasMore;

    readMessageFromStream(clientConnection, header, meta, data, ok, hasMore);

    if(ok)
        emit receiveMessage(header, meta, data);

    if(hasMore)
        readMessage();
}

void TcpServer::sendMessage(QVariantMap message, QByteArray binaryData,  bool *ok, QTcpSocket *socket)
{
    if(!socket)
        socket = clientConnection;
    if(!socket)
        return;
    //    connect(tcpSocket, SIGNAL(disconnected()),
    //            tcpSocket, SLOT(deleteLater()));

    QByteArray prepairedMessage = TcpProtocol::createMessage(message, binaryData);
    sendRawMessage(prepairedMessage, socket);
}

void TcpServer::sendRawMessage(QByteArray message, QTcpSocket *socket)
{
    socket->write(message);
    //tcpSocket->disconnectFromHost();
}

void TcpServer::on_error(QVariantMap info)
{
    //дописывание типа посылки
    QVariantMap infoFull = TcpProtocol::wrapErrorInfo(info);
    //отправка сообщения об ошибке
    sendMessage(infoFull, QByteArray());
}

