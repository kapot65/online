#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QSettings>
#include <QNetworkConfigurationManager>
#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <QJson/Parser>
#include <QJson/Serializer>
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
    explicit TcpClient(QString peerName = QString(), int peerPort = -1, QObject *parent = 0);
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
     * \brief Собирает и отправляет сообщение на сервер.
     * \param message метаданные посылки
     * \param binaryData бинарные данные
     * \param *ok статус отправки
     * */
    void sendMessage(QVariantMap message, QByteArray binaryData = QByteArray(), bool *ok = NULL);
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

    /*!
     * \brief открытие сессии
     * \details взято из примера Qt
     */
    void sessionOpened();

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
};

#endif // TCPCLIENT_H
