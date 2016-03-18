#ifndef CAMACSERVER_H
#define CAMACSERVER_H

#include <QDataStream>
#include <QVector>
#include <QtNetwork>
#include <stdlib.h>
#include <QCoreApplication>

#include "camacalgoritm.h"
#include "commandhandler.h"
#include "camacserversettings.h"
#include <tcpserver.h>
#include <tcpprotocol.h>


/*!
 * \brief Сервер для выполнения операций на камаке. Работает с классом CCPC7Handler.
 * \details Класс создает Tcp сервер и обрабатывает приходящие на него сообщения.
 * Успешно разобранные сообщения класс отправляет через сигналы в класс CommandHandler.
 */
class CamacServer : public TcpServer
{
    Q_OBJECT
public:
    /*!
     * \brief Считывает настройки из ini файла. Создает сервер, устанваливает связи.
     * Если в процессе создания будет ошибка - она запишется в лог файл.
     * \param port Порт, который будет слушать сервер.
     * \param settings Менеджер настроек CamacServer.
     */
    CamacServer(int port, CamacServerSettings *settings, QObject *parent = 0);
    ~CamacServer();

private slots:
    /*!
    * \brief Обработка сообщения. В этом методе происходит деление сообщений
    * на команды и ответы. Если сообщение не подходит ни под один из критериев, отправителю посылается
    * сообщение с ошибкой анализа сообщения. Далее, команды обрабатываются методом CamacServer::processCommand, а
    * ответы - методом CamacServer::processReply.
    * \param header Машинный заголовок сообщения.
    * \param meta Метаданные сообщения.
    * \param data Бинарные данные сообщения.
    * \todo Извлечь метод в TcpBase.
    */
    void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);

signals:
    /*!
     * \brief Сигнал связан с CommandHandler::processInit
     * \param message Метаданные запроса.
     */
    void init(QVariantMap message);

    /*!
     * \brief Сигнал связан с CommandHandler::processAcquirePoint
     * \param message Метаданные запроса.
     */
    void acquirePoint(QVariantMap message);

    /*!
     * \brief Сигнал связан с CommandHandler::processBreakAcquisition
     * \param message Метаданные запроса.
     */
    void breakAcquisition(QVariantMap message);

    /*!
     * \brief Сигнал связан с CommandHandler::ProcessNAF
     * \param message Метаданные запроса.
     */
    void NAF(QVariantMap message);

    /*!
     * \brief Сигнал связан с CommandHandler::processResetCounters
     * \param message Метаданные запроса.
     */
    void resetCounters(QVariantMap message);

    /*!
     * \brief Сигнал связан с CommandHandler::processGetCountersValue
     * \param message Метаданные запроса.
     */
    void getCountersValue(QVariantMap message);

private:

    /*!
     * \brief Обработка команды.
     * \param message Метаданные запроса.
     * \todo Убрать в TcpBase, например.
     */
    void processCommand(QVariantMap message);

    /*!
     * \brief Обработка ответа.
     * \details Функция отправляет сообщение о том, что на сервере не предусмотрена обработка ответа.
     * \param message Метаданные запроса.
     * \todo Убрать в TcpBase, например.
     */
    void processReply(QVariantMap message);

    ///Обработчик команд.
    CommandHandler *cmdHandler;

    ///Менеджер настроек CamacServer.
    CamacServerSettings *settings;
};

#endif // CAMACSERVER_H
