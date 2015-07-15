#include "hvserverhandler.h"

HVServerHandler::HVServerHandler(QObject *parent) :
    QObject(parent)
{
    manager = new IniManager("HVServerSettings.ini", this);

    hvServer = new HVServer(manager, 33669, this);

    connect(hvServer, SIGNAL(serverReady(QString,int)), this, SLOT(on_ServerReady(QString,int)));
    connect(hvServer, SIGNAL(newConnection(QString,int)), this, SLOT(on_NewConnection(QString,int)));
}

HVServerHandler::~HVServerHandler()
{

}

void HVServerHandler::on_ServerReady(QString ip, int port)
{
    LOG(INFO) << QString("The server is running on %1 (%2)").arg(ip).arg(port).toStdString();
}

void HVServerHandler::on_NewConnection(QString ip, int port)
{
    LOG(INFO) << QString("New connection: %1 (%2)").arg(ip).arg(port).toStdString();
}
