#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <tcpserver.h>
#include <inimanager.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <easylogging++.h>

/*!
 * \brief Класс для общения с устройствами через COM порт в отдельном потоке.
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

signals:
    /*!
     * \brief Испускается, когда вольтметр заканчивает писать ответ. Конец ответа
     * определяется наличием символов "\r\n" на конце.
     */
    void receiveFinished();

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
    void onPortError(QSerialPort::SerialPortError error);

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
};

#endif // COMPORT_H
