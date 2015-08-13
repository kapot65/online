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
    bool haveOpenedConnection(){return (connection && connection->isOpen());}

protected:
    /*!
     * \brief Последнее полученное сообщение.
     */
    QVariantMap lastMessage;


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

protected slots:

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
