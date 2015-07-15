#ifndef CAMACSERVER_H
#define CAMACSERVER_H

#include <QDataStream>
#include <QVector>
#include <QtNetwork>
#include <stdlib.h>
#include <QCoreApplication>
#include <QJson/Parser>
#include <QJson/Serializer>

#include "camacalgoritm.h"
#include "commandhandler.h"
#include "camacserversettings.h"
#include <tcpserver.h>
#include <tcpprotocol.h>


class CamacServer : public TcpServer
{
    Q_OBJECT
public:
    CamacServer(int port, CamacServerSettings *settings);
    ~CamacServer();

private slots:
    void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);

signals:
    void serverError(QString error);

    void init(QVariantMap message);
    void acquirePoint(QVariantMap message);
    void breakAcquisition(QVariantMap message);
    void NAF(QVariantMap message);
    void resetCounters(QVariantMap message);
    void getCountersValue(QVariantMap message);

private:
    void processCommand(QVariantMap message);
    void processReply(QVariantMap message);


    CommandHandler *cmdHandler;
    CamacServerSettings *settings;
};

#endif // CAMACSERVER_H
