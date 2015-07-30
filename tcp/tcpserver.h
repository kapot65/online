#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTimer>
#include <QDataStream>
#include <QVector>
#include <QtNetwork>
#include <stdlib.h>
#ifdef USE_QTJSON
#include <QJsonDocument>
#else
#include <QJson/Parser>
#include <QJson/Serializer>
#endif
#include "tcpprotocol.h"
#include "tcpbase.h"
//#include <easylogging++.h>

class TcpServer : public QObject, public TcpBase
{
    Q_OBJECT
public:
    explicit TcpServer(int port, QObject *parent = 0);
    //void setPort(int port){this->port = port;}
    int getPort(){return port;}

signals:
    void error(QVariantMap info);
    void serverReady(QString ip, int port);
    void receiveMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);
    void newConnection(QString peerName, int peerPort);

private slots:
    void sessionOpened();
    void readMessage();
    void processNewConnection();
    void sendReady();

private:
    void serverReady();

public slots:
    void sendMessage(QVariantMap message, QByteArray binaryData = QByteArray(), bool *ok = NULL,
                     QTcpSocket *socket = 0);
    void sendRawMessage(QByteArray message, QTcpSocket *socket);

protected:
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
    QTcpSocket *clientConnection;
    int port;

protected slots:
    void on_error(QVariantMap info);
};

#endif // TCPSERVER_H
