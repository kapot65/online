#ifndef COMMANDHANDLER
#define COMMANDHANDLER

#include <QThread>
#include <QDataStream>
#include <QVector>
#include <QEventLoop>
#include <stdlib.h>
#include <QCoreApplication>
#include <QDate>
#include <QTime>

#include <ccpc7.h>

#include <tcpprotocol.h>
#include "camacalgoritm.h"
#include "camacserversettings.h"

/*!
 * \brief Класс обработки запросов камака.
 * Команды выполняются с помощью Класса CamacAlgoritm.
 * Работает вместе с CamacServer.
 */
class CommandHandler : public CamacAlgoritm
{
    Q_OBJECT
public:
    CommandHandler(CamacServerSettings *settings, METATYPE metatype, QObject *parent = 0);

    /*!
     * \brief Ручная проверка занятости устройства.
     * \return true - устройво занято, false - устройство свободно.
     */
    bool checkBusy() {return busyFlag;}

    /*!
     * \brief Проверка инициализации камака.
     * Если инициализация не пройдена, высылает ошибку.
     * \return Инициализированность устройтсва.
     */
    bool checkInit();

    /*!
     * \brief Преобразование команды NAF из QVariantMap в ccpc::CamacOp.
     * \param message Команда NAF в формате QVariantMap.
     * \return Команда NAF в формате ccpc::CamacOp.
     */
    static ccpc::CamacOp messageToCamacOp(QVariantMap message);

    /*!
     * \brief Обратное к CommandHandler::messageToCamacOp преобразование.
     * \param op Команда NAF в формате ccpc::CamacOp.
     * \return Команда NAF в формате QVariantMap.
     */
    static QVariantMap camacOpToMessage(ccpc::CamacOp op);

signals:
    void error(QVariantMap info);
    void breakAcquisition();
    void sendMessage(QVariantMap message, QByteArray binaryData);
    void sendRawMessage(QByteArray message);

public slots:
    /*!
     * \brief Инициализация устройства.
     * \param message Метаданные команды.
     */
    void processInit(QVariantMap message);

    /*!
     * \brief Сбор точки.
     * \param message Метаданные команды.
     * \todo VIRTUAL_MODE сделать обработку флага manually_break
     */
    void processAcquirePoint(QVariantMap message);

    /*!
     * \brief Ручная остановка сбора.
     * \param message Метаданные команды.
     */
    void processBreakAcquisition(QVariantMap message);


    /*!
     * \brief Проведение команды NAF.
     * \param message Метаданные команды.
     */
    void ProcessNAF(QVariantMap message);

    /*!
     * \brief Обнуление каунтеров.
     * \param message Метаданные команды.
     */
    void processResetCounters(QVariantMap message);

    /*!
     * \brief Считывание данных с каунтеров.
     * \param message Метаданные команды.
     */
    void processGetCountersValue(QVariantMap message);

    /*!
     * \brief Посылка клиенту информации о текущем статусе набора
     * слот соединен с CamacAlgoritm::currentEventCount
     * \param count текущий счет
     * \param currentTime текущее время в секундах
     * \param totalTime полное время в секундах
     */
    void processCurrentEventCount(long count, int currentTime, int totalTime);

private:
    ///Путь к временной папке.
    QString tempFolder;

    ///Флаг занятости устройства.
    bool busyFlag;

    /*!
     * \brief Формат метаданных, передаваемый сервером
     */
    METATYPE metatype;

#ifdef VIRTUAL_MODE
    ///
    /// \brief Флаг инициализированности сервера.
    /// \note работает только в режиме VIRTUAL_MODE
    ///
    bool initFlag;
#endif
};


#endif // COMMANDHANDLER

