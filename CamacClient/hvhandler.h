#ifndef HVHANDLER_H
#define HVHANDLER_H

#include <QObject>
#include <QEventLoop>
#include "serverhandler.h"
#include <inimanager.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

/*!
 * \brief Класс обработки сервера HV.
 * \details Класс представляет собой оболочку для общения с сервером HVServer
 * на уровне функций и сигналов. Класс содержит функции для создания сообщений
 * серверу и функции для приема сообщений и преорбразования их в сигналы.
 */
class HVHandler : public ServerHandler
{
    Q_OBJECT
public:
    /*!
     * \param ip Адрес сервера HV.
     * \param port Порт сервера HV.
     */
    explicit HVHandler(QString ip = QString(), int port = -1, QObject *parent = 0);

signals:
    /*!
     * \brief Испускается при получении с сервера ответа с напряжением.
     * \param meta Метаданные ответа.
     */
    void getVoltageDone(QVariantMap meta);

    /*!
     * \brief Испускается при получении с сервера ответа об установке напряжения.
     * \param meta Метаданные ответа.
     */
    void setVoltageDone(QVariantMap meta);

#ifdef TEST_MODE
    void sendTestJsonMessage(QByteArray message);
#endif

public slots:
    /*!
     * \brief Проинициализировать сервер.
     * \details Метод запрашивает у сервера состояние вольтметра. Если вольтметры не
     * инициализированы, то они проходят инициализацию. При успешной инициализации
     * испускаются сигналы ServerHandler::ready и ServerHandler::serverInited.
     * \warning Метод может занимать до 20 секунд.
     */
    virtual void initServer(bool *ok = 0);

    /*!
     * \brief Установить напряжение на блоке.
     * При успешном выполнении испускается сигнал HVHandler::setVoltageDone.
     * Метод выполняется практически мгновенно.
     * \param block Номер блока (1 или 2).
     * \param value Устанавлимое напряжение в вольтах.
     */
    void setVoltage(int block, double value);

    /*!
     * \brief Считать напряжение с блока.
     * При успешном выполнении испускается сигнал HVHandler::getVoltageDone.
     * Выполнение занимает примерно 5 секунд.
     * \param Номер блока (1 или 2).
     */
    void getVoltage(int block);

protected slots:
    /*!
     * \brief Метод перехватывает сообщения и парсит ответы.
     */
    virtual void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);


    // TcpBase interface
protected:
    bool handleError(QVariantMap err);
};

#endif // HVHANDLER_H
