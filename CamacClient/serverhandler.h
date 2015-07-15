#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <QObject>
#include "tcpclient.h"
#ifdef TEST_MODE
#include <QDebug>
#endif

/*!
 * \brief The ServerHandler class
 * \details Родительский класс для обработки сообщeний с сервера
 * Имеет сигналы инициализации, готовности, ошибки
 */
class ServerHandler : public TcpClient
{
    Q_OBJECT
public:
    explicit ServerHandler(QString ip = QString(), int port = -1, QObject *parent = 0);
    ~ServerHandler();

    /*!
     * \brief ручная проверка на ошибку
     */
    inline bool hasError(){return errorFlag;}

    /*!
     * \brief getLastError
     * \return возвращает описание последней ошибки
     */
    inline QVariantMap getLastError(){return lastError;}

    /*!
     * \brief ручная проверка на инициализацию сервера
     * \return
     */
    inline bool hasInited(){return initFlag;}

signals:
    /*!
     * \brief сигнал вырабатывается при готовности класса после создания
     * или при устранении ошибки
     */
    void ready();

    /*!
     * \brief Сигнализирует о наличии ошибки класса
     * \param err описание ошибки
     */
    void error(QVariantMap err);

    /*!
     * \brief serverInited
     */
    void serverInited();

    /*!
     * \brief Отправляет сообщение, необработанное классом
     * \param machineHeader
     * \param metaData
     * \param binaryData
     */
    void unhandledMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);

public slots:
    /*!
     * \brief Виртуальная функция инициализации сервера
     */
    virtual void initServer() = 0;

    /*!
     * \brief Функция переподключения сервера
     * \details Повторяет фунции TcpClient::setPeer и TcpClient::connectToServer()
     */
    void reconnect(QString ip, int port);

protected slots:
    /*!
     * \brief Виртуальный метод обработки сообщения
     */
    virtual void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData) = 0;

private slots:
    void on_Connected(QString ip, int port);
    void on_Disconnected();
    void on_ServerInited();
    void checkMessageForError(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);

private:
    /*!
     * \brief описание последней ошибки
     */
    QVariantMap lastError;

    /*!
     * \brief флаг наличия ошибки
     */
    bool errorFlag;

    /*!
     * \brief флаг прохождения инициализации
     */
    bool initFlag;
};
#endif // SERVERHANDLER_H
