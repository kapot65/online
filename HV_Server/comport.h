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

protected slots:
    /*!
     * \brief Прикреплен к сигналу QSerialPort::readyRead
     */
    virtual void readMessage() = 0;

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
     * \brief Указатель на менеджер настроек.
     */
    IniManager *manager;

    /*!
     * \brief Указатель на класс порта.
     */
    QSerialPort *serialPort;
};

#endif // COMPORT_H
