#include <ccpc7.h>
#include <QtNetwork>
#include <QEventLoop>
//#include "commandhandler.h"

using namespace ccpc;

CamacImplCCPC7::CamacImplCCPC7(QString ip, int host, QObject *parent) : QObject(parent)
{
    errorCounter = 0;

    networkSession = NULL;
    connection = new QTcpSocket(this);
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError()));
    connect(connection, SIGNAL(connected()), this, SIGNAL(connected()));

    //connect(connection, SIGNAL(readyRead()), this, SLOT(readMessage()));

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
    connection->connectToHost(ip, host);

    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &timer, SLOT(deleteLater()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(onConnected()));
    timer.start(400);
}
CamacImplCCPC7::~CamacImplCCPC7()
{
    connection->abort();
    connection->disconnectFromHost();
    if(networkSession)
        networkSession->close();
}

void CamacImplCCPC7::onConnected()
{
    emit connected();
}

void CamacImplCCPC7::exec(CamacOp &op)
{
    /*
    Message op_message;
    op_message.fromCamacOp(op);

    //добавить передачу данных
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << op_message;
    out.device()->seek(0);
    */
    QVariantMap message ;//= CommandHandler::camacOpToMessage(op);
    QByteArray fullMessage = TcpProtocol::createMessage(message);

    QEventLoop pause;
    connect(connection, SIGNAL(readyRead()), &pause, SLOT(quit()));

    connection->write(fullMessage);

    //ожидание ответа
    pause.exec();

    QByteArray answer = connection->readAll();
    QByteArray data;
    TcpProtocol::parceMessage(answer, message, data);
    //op =  CommandHandler::messageToCamacOp(message);
}
void CamacImplCCPC7::processError()
{
    errorCounter++;
    error = connection->errorString();
    emit have_error("CAMAC Client:" + error);
}
void CamacImplCCPC7::sessionOpened()
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

}

void CamacImplCCPC7::init()
{
    //создание посылки
    QVariantMap message;
    message.insert("type", "command");
    message.insert("command_type", "init");

    QByteArray fullMessage = TcpProtocol::createMessage(message);

    connection->write(fullMessage);
}
