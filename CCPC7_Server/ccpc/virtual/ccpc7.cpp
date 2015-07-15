#include <ccpc7.h>
#include <QtNetwork>
#include <QEventLoop>
#include "commandhandler.h"

using namespace ccpc;

CamacImplCCPC7::CamacImplCCPC7(QString ip, int host, QObject *parent) : QObject(parent)
{
    errorCounter = 0;

    networkSession = NULL;
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError()));
    connect(tcpSocket, SIGNAL(connected()), this, SIGNAL(connected()));

    //connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));

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
    tcpSocket->connectToHost(ip, host);

    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &timer, SLOT(deleteLater()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(onConnected()));
    timer.start(400);
}
CamacImplCCPC7::~CamacImplCCPC7()
{
    tcpSocket->abort();
    tcpSocket->disconnectFromHost();
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
    QVariantMap message = CommandHandler::camacOpToMessage(op);

    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки
    QByteArray serializedMessage = serializer.serialize(message);

    //создание бинарного хедера
    MachineHeader header;
    header.type = 0;
    header.metaType = 0;
    header.metaLength = serializedMessage.size();
    header.dataType = 0;
    header.dataLenght = 0;

    QByteArray machineHeader = TcpProtocol::writeMachineHeader(header);

    QEventLoop pause;
    connect(tcpSocket, SIGNAL(readyRead()), &pause, SLOT(quit()));

    tcpSocket->write(machineHeader + serializedMessage);

    //ожидание ответа
    pause.exec();

    QJson::Parser parser;
    QByteArray answer = tcpSocket->readAll();

    if(answer.startsWith("#!"))
    {
        answer.remove(0, 30);
    }

    QVariantMap parsedMessage = parser.parse(answer).toMap();

    op =  CommandHandler::messageToCamacOp(parsedMessage);
    /*
    //считывание ответа
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    in >> op_message;

    op = op_message.toCamacOp();
    */
}
void CamacImplCCPC7::processError()
{
    errorCounter++;
    error = tcpSocket->errorString();
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
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull); // в настройки

    message.insert("type", "command");
    message.insert("command_type", "init");

    QByteArray serializedMessage = serializer.serialize(message);

    //создание бинарного хедера
    MachineHeader header;
    header.type = 0;
    header.metaType = 0;
    header.metaLength = serializedMessage.size();
    header.dataType = 0;
    header.dataLenght = 0;

    QByteArray machineHeader = TcpProtocol::writeMachineHeader(header);

    tcpSocket->write(machineHeader + serializedMessage);
}
