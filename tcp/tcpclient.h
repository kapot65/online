#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkSession>
#include <QSettings>
#include <QtNetwork/QNetworkConfigurationManager>
#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include <QFile>
#include <QTime>
#include <tcpprotocol.h>
#include <tcpbase.h>
#include <easylogging++.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

/*!
 * \brief Базовый класс для общения с Tcp сервером TcpServer.
 * \details Класс имеет функционал подключения, переподключения и сборки сообщений.
 */
class TcpClient : public TcpBase
{
    Q_OBJECT
public:
    /*!
     * \param peerName Адресс сервера.
     * \param peerPort Порт сервера.
     * \details При указании адресса и порта клиент автоматически подключается к серверу.
     * \param parent
     */
    explicit TcpClient(IniManager *manager, QObject *parent = 0);
    ~TcpClient();

    /*!
     * \brief Указание параметров подключения к серверу.
     * \detais После установки параметров, для переподключения
     * необходимо вызвать функцию TcpClient::connectToServer
     * \param peerName ip адрес сервера
     * \param peerPort порт сервера
     * */
    void setPeer(QString peerName, int peerPort);

signals:
    /*!
     * \brief Вырабатывается при успешном подключении к серверу.
     * */
    void socketConnected(QString name, int port);
    /*!
     * \brief Вырабатывается при отключении от сервера.
     * */
    void socketDisconnected();

public slots:
    /*!
     * \brief Переподключает клиент к серверу.
     * \details При успешном переподключении вырабатывает сигнал TcpClient::socketConnected
     * */
    void connectToServer();

private slots:

    void onSocketConnected();
    void onSocketDisconnected();

    /*!
     * \brief Обработка ошибки сокета
     * \warning функция ничего не делает
     * \todo Добавить реализацию.
     * \param err
     */
    void processError(QAbstractSocket::SocketError err){}

private:
    QNetworkSession *networkSession;

    /*!
     * \brief ip адрес сервера
     */
    QString peerName;

    /*!
     * \brief порт сервера
     */
    int peerPort;

    // TcpBase interface
protected:
    virtual bool handleError(QVariantMap err);
};

#endif // TCPCLIENT_H
