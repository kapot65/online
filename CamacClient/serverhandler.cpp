#include "serverhandler.h"
#include <QEventLoop>
#include <QTimer>

ServerHandler::ServerHandler(IniManager *manager, QObject *parent) : TcpClient(manager, parent)
{
    initFlag  = 0;

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(processMessage(MachineHeader,QVariantMap,QByteArray)));

    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(checkMessageForError(MachineHeader,QVariantMap,QByteArray)));

    connect(this, SIGNAL(socketConnected(QString,int)),
            this, SLOT(on_Connected(QString,int)), Qt::DirectConnection);
    connect(this, SIGNAL(socketDisconnected()), this, SLOT(on_Disconnected()));
    connect(this, SIGNAL(serverInited()), this, SLOT(on_ServerInited()));
}

void ServerHandler::reconnect(QString ip, int port)
{
    this->setPeer(ip, port);
    this->connectToServer();

    initFlag = 0;
}

void ServerHandler::on_Connected(QString ip, int port)
{
    lastError = QVariantMap();
    emit ready();
}

void ServerHandler::on_Disconnected()
{
    lastError = QVariantMap();
    lastError["error_code"] = QString("%1").arg(CLIENT_DISCONNECT);
    initFlag = 0;

    emit error(lastError);
}

void ServerHandler::on_ServerInited()
{
    initFlag = 1;
}

void ServerHandler::checkMessageForError(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    if(TcpProtocol::checkMessageForError(metaData))
    {
        lastError = TcpProtocol::unwrapErrorInfo(metaData);
        emit error(lastError);
    }
}

ServerHandler::~ServerHandler()
{

}

bool ServerHandler::hasError()
{
    if(!haveOpenedConnection())
        return true;

    if(connection->error() != QAbstractSocket::UnknownSocketError)
        return true;
    else
        return false;
}
