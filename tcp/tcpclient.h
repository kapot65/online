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
 * \brief Базовый класс для общения с Tcp сервером
 * \details Класс имеет функционал подключения, переподключения и сборки сообщений.
 * */
class TcpClient : public QThread, public TcpBase
{
    Q_OBJECT
public:
    /*!
     * \details При указании адресса и порта клиент автоматически подключается к серверу
     * */
    explicit TcpClient(QString peerName = QString(), int peerPort = -1, QObject *parent = 0);
    ~TcpClient();

    /*!
     * \brief Ручная проверка подключения к серверу.
     * */
    bool isSocketConnected(){return connectedToPeer;}
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
    /*!
     * \brief Вырабатывается успешной сборке сообщения от сервера.
     * \param machineHeader машинный хедер
     * \param metaData преобразованные метаданные
     * \param binaryData бинарные данные
     * */
    void receiveMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);

#ifdef TEST_MODE
    /*!
     * \brief Сигнал предназначен для тестовой визуализации пересылаемых пакетов
     * \warning сигнал только для тестового режима программы
     * \param machineHeader машинный хедер
     * */
    void testReseivedMessage(QByteArray message);
#endif

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
     * \brief saveLastMessage
     * \details Сохраняет метаданные последнего полученного сообщения в TcpClient::lastMessage
     * подключено к сигналу TcpClient::receiveMessage c помощью Qt::DirectConnection
     * \param machineHeader
     * \param metaData
     * \param binaryData
     */
    void saveLastMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);


    /*!
     * \brief Обработка ошибки сокета
     * \warning функция ничего не делает
     * \param err
     */
    void processError(QAbstractSocket::SocketError err){}

    /*!
     * \brief открытие сессии
     * \details взято из примера Qt
     */
    void sessionOpened();

    /*!
     * \brief Чтение сообщение от сервера
     * \details в этой фунции происходит склеивание сообщения и вызов сигнала
     * TcpClient::receiveMessage
     * Если сообщение не соответсвует формату, то оно игнорируется
     */
    void readMessage();

protected:
    /*!
     * \brief Последнее полученное письмо
     */
    QVariantMap lastMessage;
    /*!
     * \brief lastError
     */

private:
    /*!
     * \brief tcpSocket
     */
    QTcpSocket *tcpSocket;

    /*!
     * \brief networkSession
     */
    QNetworkSession *networkSession;

    /*!
     * \brief Флаг подключения к серверу
     */
    bool connectedToPeer;

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
