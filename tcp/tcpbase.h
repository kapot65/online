#ifndef TCPBASE_H
#define TCPBASE_H

#include <tcpprotocol.h>
#include <QThread>
#include <QTcpSocket>

/*!
 * \brief В этом классе находятся реализации методов, общих для
 * TcpClient и TcpServer
 */
class TcpBase : public QThread
{
    Q_OBJECT
public:
    TcpBase(QObject *parent = 0);

    /*!
     * \brief Ожидание готовности сообщения
     * \param timeout Таймаут в миллисекундах.
     * \return true - сообщение получено. false - выход по таймауту.
     */
    bool waitForMessage(int timeout = 10000);

    /*!
     * \brief Ожидание соединения.
     * \param timeout Таймаут в милисекундах.
     */
    bool waitForConnect(int timeout = 30000);

    /*!
     * \brief Ручная проверка подключения.
     * */
    bool haveOpenedConnection(){return (connection && connection->state() == QAbstractSocket::ConnectedState);}

protected:
    /*!
     * \brief Последнее полученное сообщение.
     */
    QVariantMap lastMessage;

    /*!
     * \brief Последнее посланное сообщение в сыром виде.
     */
    QByteArray lastSentMessage;


    void readMessageFromStream(QIODevice *dev,
                               MachineHeader &header,
                               QVariantMap &meta,
                               QByteArray &data,
                               bool &ok,
                               bool &hasMore);

    /*!
     * \brief текущее соединение
     */
    QTcpSocket *connection;

    /*!
     * \brief Текущее собираемое сообщение.
     */
    QByteArray fullMessage;

    /*!
     * \brief Флаг продолжения чтения сообщения.
     */
    bool continue_message;

    /*!
     * \brief Машиночитаемый заголовок текущего сообщения.
     */
    MachineHeader header;

signals:
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
     */
    void testReseivedMessage(QByteArray message);
#endif

    /*!
     * \brief Сигнал для передачи сообщений об ошибках в орбаботчики.
     * \param err описание ошибки. Для корректной работы должно содержать
     * поле "error code" подробнее о кодах ошибок можно посмотреть в /ref errType.
     */
    void error(QVariantMap err);

    void unhandledError(QVariantMap err);

    /*!
     * \brief сигнал вырабатывается при готовности класса после создания
     * или при устранении ошибки
     */
    void ready();

protected:
    /*!
     * \brief Обработчик ошибок.
     * \note Для корректной работы, необходимо, чтобы перед кодом каждой реализации была вставка
     *
     *        if(baseClass::handleError)
     *          return true;
     *
     * или подобная ей. Здесь baseClass - ближайший предок класса. Таким образом не будет нарушена рекурсивная последовательность вызовов обработчиков ошибок.
     *
     * \param err Описание ошибки.
     * \return true - ошибка обработана, false - обработчик не смог обработать ошибку
     */
    virtual bool handleError(QVariantMap err);

private slots:
    /*!
     * \brief Частный слот для обработки ошибок.
     * Вызывает виртуальную фунцию TcpBase::handleError, и, если она не справляется с ошибкой - испускает сигнал TcpBase::unhandledError.
     * \param err Описание ошибки.
     */
    void handleErrorImpl(QVariantMap err);

protected slots:
    void sendMessage(QVariantMap message, QByteArray binaryData = QByteArray(), bool *ok = NULL,
                     QTcpSocket *socket = 0);

    /*!
     * \brief лучше пользоваться TcpBase::sendMessage()
     * \todo Добавить сигнализирование и обработку ошибок.
     */
    void sendRawMessage(QByteArray message, bool *ok = NULL, QTcpSocket *socket = 0);

    /*!
     * \brief Чтение сообщения
     * \details В этой фунции происходит склеивание сообщения и вызов сигнала
     * TcpClient::receiveMessage
     * Если сообщение не соответсвует формату, то оно игнорируется
     */
    virtual void readMessage();

    /*!
     * \brief Сохраняет метаданные последнего полученного сообщения в TcpClient::lastMessage
     * подключено к сигналу TcpBase::receiveMessage c помощью Qt::DirectConnection
     * \param machineHeader
     * \param metaData
     * \param binaryData
     */
    void saveLastMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);
};

#endif // TCPBASE_H
