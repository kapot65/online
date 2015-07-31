#ifndef CCPC7HANDLER_H
#define CCPC7HANDLER_H

#include <serverhandler.h>
#include <event.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

/*!
 * \brief Класс обработки сервера CCPC7
 * \details Класс представляет собой оболочку для общения с сервером CCPC7
 * на уровне функций и сигналов. Класс содержит функции для создания сообщений
 * серверу и функции для приема сообщений и преорбразования их в сигналы
 * \todo добавить обработку ошибки занятости сервера
 */
class CCPC7Handler : public ServerHandler
{
    Q_OBJECT
public:
    /*!
     * \brief CCPC7Handler
     * \details Если заданы параметры, то подключение происходит в контсрукторе.
     * \param ip
     * \param port
     * \param parent
     */
    explicit CCPC7Handler(QString ip = QString(), int port = -1, QObject *parent = 0);
    ~CCPC7Handler();

signals:
    /*!
     * \brief Сигнал успешного прерывания сбора.
     */
    void acquisitionBreaked();

    /*!
     * \brief Сигнал успешного сбора точки
     * \param machineHeader бинарный хедер
     * \param metaData метаданные
     * \param events массив событий
     */
    void pointAcquired(MachineHeader machineHeader, QVariantMap metaData, QVector<Event> events);

    /*!
     * \brief Сигнал успешного считывания каунтера.
     * \param metaData метаданные
     */
    void counterAcquired(QVariantMap metaData);

    /*!
     * \brief Сигнал успешного сброса каунтеров.
     */
    void countersResetted();

#ifdef TEST_MODE
    /*!
     * \brief Посылает тестовое Json сообщение на главную форму
     * \warning тестовая функция
     */
    void sendTestJsonMessage(QByteArray message);
#endif

public slots:
    /*!
     * \brief Метод для инициализации сервера.
     * \details Создает Json сообщение и посылает его на сервер.
     * В случае успешного инициализирования присылает сигнал ServerHandler::serverInited
     * Сигнал сообщения должен придти практически мгновенно
     */
    virtual void initServer();

    /*!
     * \brief Метод для сбора точки.
     * \details Создает Json сообщение и посылает его на сервер.
     * В случае успешного инициализирования присылает сигнал CCPC7Handler::pointAcquired
     * Сигнал приходит не сразу, а с задержкой, близкой к времени сбора
     */
    void acquirePoint(int time, QVariant external_meta = 0);

    /*!
     * \brief Метод для ручного прерывания сбора.
     * \details Создает Json сообщение и посылает его на сервер.
     * В случае успешного инициализирования присылает сигнал CCPC7Handler::acquisitionBreaked
     * Сигнал сообщения должен придти практически мгновенно.
    */
    void breakAcquisition();

    /*!
     * \brief Метод для сброса каунтеров.
     * \details Создает Json сообщение и посылает его на сервер.
     * В случае успешного инициализирования присылает сигнал CCPC7Handler::countersResetted
     * Сигнал сообщения должен придти практически мгновенно.
     */
    void resetCounters();

    /*!
     * \brief Метод для сбора информации с каунтера.
     * \param counter номер каунтера (1 или 2)
     * \param channels_id номера нужных каналов (0, 1, 2, 3)
     * \param reset_after нужно ли обнулять каунтер после сбора
     */
    void getCountersValue(int counter, QList<QVariant> channels_id, bool reset_after);

protected slots:
    /*!
     * \brief Обработчик сообщений, поступающих с сервера.
     * \details Парсит Json сообщение и в соотвествии с ним выдает следующие сигналы:
     * CCPC7Handler::pointAcquired - при сообщении о собранной точке
     * CCPC7Handler::counterAcquired - при сообщении о собранной информации с каунтера
     * ServerHandler::serverInited - при инициализации сервера
     * CCPC7Handler::acquisitionBreaked - при ручном прерывании сбора
     * CCPC7Handler::countersResetted - при сбросе каунтеров (ручном)
     * CCPC7Handler::unhandledMessage - если сообщение не подходит ни под одну из перечисленных категорий;
     * \warning сообщения об ошибках будут вызывать сигнал CCPC7Handler::unhandledMessage
     * \param mashineHeader
     * \param metaData
     * \param binaryData
     */
    virtual void processMessage(MachineHeader mashineHeader, QVariantMap metaData, QByteArray binaryData);
};

#endif // CCPC7HANDLER_H
