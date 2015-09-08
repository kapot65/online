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
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include "tcpprotocol.h"
#include "tcpbase.h"
//#include <easylogging++.h>

/*!
 * \brief Класс Tcp сервера, работающего по протоколу, реуализованному в TcpProtocol.
 * \details Класс обрабатывает сообщения, приходящие на сервер, производит их парсинг и
 * преобразует в сигналы. Также сервер имеет функции для отправления сообщений обратно.
 * \warning Сервер может работать одновременно только с одним подключением. При новом подключении
 * старое обрывается.
 */
class TcpServer : public TcpBase
{
    Q_OBJECT
public:
    /*!
     * \param port Порт, который будет прослушивать сервер.
     */
    explicit TcpServer(int port, QObject *parent = 0);
    //void setPort(int port){this->port = port;}

    /*!
     * \brief Получить текущий порт.
     * \return Текущий порт.
     */
    int getPort(){return port;}

signals:
    void error(QVariantMap info);
    void serverReady(QString ip, int port);
    void newConnection(QString peerName, int peerPort);

private slots:
    void sessionOpened();
    void processNewConnection();
    void sendReady();

private:
    void serverReady();

public slots:
    void sendMessage(QVariantMap message, QByteArray binaryData = QByteArray(), bool *ok = NULL,
                     QTcpSocket *socket = 0);
    void sendRawMessage(QByteArray message, QTcpSocket *socket = 0);

protected:
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;

    int port;

protected slots:
    void on_error(QVariantMap info);
};

#endif // TCPSERVER_H
