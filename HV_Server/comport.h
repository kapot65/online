#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <tcpserver.h>
#include <inimanager.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <easylogging++.h>
#include <tcpbase.h>
#include <tcpprotocol.h>

/*!
 * \brief Класс для общения с устройствами через COM порт в отдельном потоке.
 * \todo Объединить этот класс с TcpBase. В базовом классе должны быть функции передачи и обработки ошибок.
 */
class ComPort: public QThread
{
    Q_OBJECT
public:
    /*!
     * \param manager Менеджер настроек.
     */
    ComPort(IniManager *manager, QObject *parent = 0);
    ~ComPort();

    /*!
     * \brief Проверка занятости интерфейса.
     * \return Флаг занятости.
     */
    bool checkBusy(){return busyFlag;}

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

signals:
    /*!
     * \brief Испускается, когда вольтметр заканчивает писать ответ. Конец ответа
     * определяется наличием символов "\r\n" на конце.
     */
    void receiveFinished();

    /*!
     * \brief Сигнал для передачи сообщений об ошибках в орбаботчики.
     * \param err описание ошибки. Для корректной работы должно содержать
     * поле "error code" подробнее о кодах ошибок можно посмотреть в /ref errType.
     */
    void error(QVariantMap err);

    void unhandledError(QVariantMap err);

    void ready();

protected slots:
    /*!
     * \brief Читает сообщение с порта. После каждого прочитанного сообщения
     * испускает сигнал DividerReader::receiveFinished.
     * Прикреплен к сигналу QSerialPort::readyRead.
     */
    virtual void readMessage();

    /*!
     * \brief Вызывается при закрытии порта. Пишет в консоль сообщение о закрытии.
     */
    void onPortClose();

    /*!
     * \brief Вызывается при ошибке на порте. Пишет текст ошибки в консоль.
     */
    void onPortError(QSerialPort::SerialPortError portError);

private slots:
    /*!
     * \brief Частный слот для обработки ошибок.
     * Вызывает виртуальную фунцию TcpBase::handleError, и, если она не справляется с ошибкой - испускает сигнал TcpBase::unhandledError.
     * \param err Описание ошибки.
     */
    void handleErrorImpl(QVariantMap err);

protected:
    /*!
     * \brief Ожидание готовности сообщения
     * \param timeout Таймаут в миллисекундах.
     * \return true - сообщение получено. false - выход по таймауту.
     */
    bool waitForMessageReady(int timeout = 5000);

    /*!
     * \brief Указатель на менеджер настроек.
     */
    IniManager *manager;

    /*!
     * \brief Указатель на класс порта.
     */
    QSerialPort *serialPort;

    /*!
     * \brief Флаг занятости контроллера.
     */
    bool busyFlag;

    /*!
     * \brief Текущее сообщение с порта.
     * \note После каждой команды нужно очищать.
     * \warning Является законченным, только после испускания сигнала DividerReader::receiveFinished.
     */
    QByteArray curr_data;

    virtual void run();
};

#endif // COMPORT_H
