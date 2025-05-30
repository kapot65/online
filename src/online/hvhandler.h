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
 * \todo Убрать старые функции. Добавить принятие ошибок.
 */
class HVHandler : public ServerHandler
{
    Q_OBJECT
public:
    /*!
     * \param ip Адрес сервера HV.
     * \param port Порт сервера HV.
     */
    explicit HVHandler(IniManager *manager, QObject *parent = 0);

    /*!
     * \note После получения метаданных, поле lastVoltageAndCheckMeta очищается.
     */
    QVariantMap getLastVoltageAndCheckMeta();

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

    /*!
     * \brief Испускается при получении с сервера ответа об установке напряжения.
     * \param meta Метаданные ответа.
     */
    void setVoltageAndCheckDone(QVariantMap meta);

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
    virtual void initServer(bool *ok = 0){TcpProtocol::setOk(true, ok); emit ready(); emit serverInited();}

    /*!
     * \brief Установить напряжение на блоке.
     * При успешном выполнении испускается сигнал HVHandler::setVoltageDone.
     * Метод выполняется практически мгновенно.
     * \param block Номер блока (1 или 2).
     * \param value Устанавлимое напряжение в вольтах.
     */
    void setVoltage(int block, double value);

    /*!
     * \brief Установить напряжение на блоке и проверить установку.
     * При успешном выполнении испускается сигнал setVoltageAndCheckDone.
     * После испускания сигнала завершения метаданные можно взять функцией getLastVoltageAndCheckMeta
     * \param block Номер блока (1 или 2).
     * \param value Устанавлимое напряжение в вольтах.
     * \param max_error Допустимое отклонение от выставляемого напряжения в вольтах
     * \param timeout Таймаут в секундах
     */
    void setVoltageAndCheck(int block, double value, double max_error = 3, int timeout = 20);

protected slots:
    /*!
     * \brief Метод перехватывает сообщения и парсит ответы.
     */
    virtual void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);


    // TcpBase interface
protected:
    bool handleError(QVariantMap err);

private:
    QVariantMap lastVoltageAndCheckMeta;
};

#endif // HVHANDLER_H
