#ifndef ONLINE_H
#define ONLINE_H

#include <QObject>
#include <ccpc7handler.h>
#include <hvhandler.h>
#include <QTimer>
#include <QDir>
#include <QVariant>
#include <QVariantMap>
#include <QDataStream>
#include <QDateTime>

#include "hvmonitor.h"

#ifdef VIRTUAL_MODE
#include <QTimer>
#endif

#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include <easylogging++.h>

#ifdef TEST_MODE
#include <QDebug>
#endif

/*!
 * \brief Типы команд из сценария
 * \details В файле сценария названия команд совпадают с
 * названиями в этом перечислении.
 */
enum SCENARIO_COMMAND_TYPE
{
    SET_VOLTAGE = 0, ///< Установить напряжение.
    SET_VOLTAGE_AND_CHECK = 1, ///< Установить напряжение у убедится в успешности установки.
    ACQUIRE_POINT = 2, ///< Провести набор точки.
    ACQUIRE_MULTIPOINT = 3, ///< Провести набор точки с разделением на блоки по 5с.
    WAIT = 4, ///< Ждать.
    BREAK = 5 ///< Остановить сценарий.
};

/*!
 * \brief Класс для выполнения сценариев.
 * \todo Добавить в название индекс по базовому напряжению.
 */
class Online : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Конструктор
     * \param settingsManager Менеджер настроек.
     * \param ccpcHandler Обработчик сервера CCPC.
     * \param hvHandler Обработчик сервера HV.
     */
    explicit Online(IniManager *settingsManager,
                    CCPC7Handler *ccpcHandler,
                    HVHandler *hvHandler,
                    QObject *parent = 0);
    ~Online();

    /*!
     * \brief Возвращает относительный путь к текущей подпапке.
     * \return Относительный путь к текущей подпапке.
     */
    QString getCurrSubFolder(){return currSubFolder;}

    /*!
     * \brief Подготавливает структуру папки для записи в нее данных.
     * \param session Имя сеанса. Может быть записано в виде относительного пути.
     * \param group Имя группы. Может быть записано в виде относительного пути.
     * \param iteration Номер итерации.
     * \param add_time Флаг на добавление времени к названием (для их уникальности).
     * \return Успешность подготовки папки.
     */
    bool prepareFolder(QString session, QString group, int iteration, bool add_time = true);

    /*!
     * \brief Инициализация серверов устройств.
     * \param ccpcIp Ip адрес CCPC.
     * \param ccpcPort порт CCPC.
     * \param HVIp Ip адрес HV.
     * \param HVPort порт HV.
     * \return Успешность инициализации серверов.
     * \todo добавить таймауты.
     */
    bool init(QString ccpcIp, int ccpcPort, QString HVIp, int HVPort);

    /*!
     * \brief Подфункция для Online::init
     */
    bool initImpl(QString ccpcIp, int ccpcPort, QString HVIp, int HVPort);

    /*!
     * \brief Выполняет сценарий и записывает результат во временную папку.
     * \param scenario Сценарий.
     * \return Успешность выполнения сценария.
     */
    bool processScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario);

    /*!
     * \brief Подфункция для Online::processScenario
     */
    bool processScenarioImpl(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario);

    /*!
     * \brief Метод вычисляет примерное время обработки сценария.
     * \param scenario Сценарий.
     * \return Время выполнения сценария в секундах.
     */
    static int approximateScenarioTime(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario);

    /*!
     * \brief Метод вычисляет примерное время обработки операции сценария.
     * \param step операция сценария
     * \return Примерное время выполнения в секундах
     */
    static double approximateOperationTime(QPair<SCENARIO_COMMAND_TYPE, QVariant> step);

    /*!
     * \brief Метод конструирует сценарий обратной сборки.
     * Если методу не удается построить обратный сценарий - он выдает пустой сценарий.
     * \warning Может преобразовать только примитивные сценарии.
     * \return Сценарий. Вектор пар. Первый элемет в паре содержит тип команды сценария.
     * Второй элемент - ее аргуметны.
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > constructReverseScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario);

    /*!
     * \brief Метод парсит сценарий. Если происходит ошибка парсинга - записывает ошибку\
     * в лог и возвращает пустой сценарий.
     * \param scenario_string Строка содержащая сырой сценарий.
     * \param ok Флаг успешности парсинга.
     * \return Сценарий. Вектор пар. Первый элемет в паре содержит тип команды сценария.
     * Второй элемент - ее аргуметны.
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > parseScenario(QString scenario_string, bool *ok = 0, bool noShiftBlock=true);

signals:
    /*!
     * \brief Сигнал испускаестя при поставовке сценария на паузу с помощью функции Online::pause.
     */
    void paused();

    /*!
     * \brief Папка подготовлена. Сигнал испускается методом Online::prepareFolder.
     * \param путь к папке.
     */
    void foldersPrepaired(QString dir);

    /*!
     * \brief Сценарий выполнен. Испускается методом Online::processScenario.
     */
    void scenario_done();

    /*!
     * \brief Сценарий начал исполняться. Испускается методом Online::processScenario.
     */
    void scenario_start();

    /*!
     * \brief Сценарий находится на шаге step. Испускается методом Online::processScenario.
     * \param step текущий шаг.
     */
    void at_step(int step, int step_time = 0);

    /*!
     * \brief Испускается при запросе на остановку сценария через Online::stop.
     * Останавливает все QEventLoop, расставленные в алгоритме.
     */
    void stop_scenario();

    /*!
     * \brief отправляет сообщение на форму
     */
    void sendInfoMessage(QString message);

    /*!
     * \brief Установливается напряжение. Испускается методом Online::processScenario.
     * \param block Блок напряжения.
     * \param voltage Напряжение.
     */
    void setting_voltage(int block, double voltage);

    /*!
     * \brief Проводится сбор точки. Испускается методом Online::processScenario.
     * \param time Время сбора в секундах.
     */
    void acquiring_point(int time);

    /*!
     * \brief Ожидание. Испускается методом Online::processScenario.
     * \param msec Время ожидания в милисееундах.
     */
    void waiting(int msec);

    /*!
     * \brief Остановка паузы. Испускается слотом Online::resume.
     */
    void stop_pauseLoop();

    /*!
     * \brief Изменение режима работы класса.
     * \param working true - класс в работе, false - класс бездействует.
     */
    void workStatusChanged(bool working);

public slots:
    /*!
     * \brief updateInfo
     * \details Добавляет комментарий к сбору.
     * Если параметр addAsComment - false, то infoBlock должен иметь тип QVariantMap и иметь поля
     * name и value. Если addAsComment - true, то при типе infoBlock - QString комментарий запишется со временем,
     * если тип другой - то комментарий запишется в оригинальном виде через QJson.
     * \param infoBlock блок информации
     * \param addAsComment если параметр - 0, то комментарий запишется на главный уровень, если 1 - то
     * запишется в массив комментариев.
     */
    void updateInfo(QVariant infoBlock = QVariant(), bool addAsComment = false);

    /*!
     * \brief Записывает end_time и удаляет информацию из буффера
     */
    void clearInfo();

    /*!
     * \brief Остановка сценария на паузу. Возобновляется с помощью слота Online::resume.
     */
    void pause();

    /*!
     * \brief Возобновление работы после паузы.
     */
    void resume();

    /*!
     * \brief Остановка выполнения сценария.
     * \warning Останавливается моментально, не дожидаясь выполнения шага до конца.
     * \todo Начало падать при стопе.
     */
    void stop();

    /*!
     * \brief Добавление в папку в выходными данными сценария произвольного файла.
     * \param filename Имя файла.
     * \param data Данные файла.
     * \todo Сделать возможность создания папок.
     */
    void addFileToScenario(QString filename, QByteArray data);

private slots:
    /*!
     * \brief Обработка ошибки, которую не смогли обработать CCPC7Handler и HVHandler.
     */
    void processUnhandledError(QVariantMap info);

    /*!
     * \brief Метод записывает
     * код и время ошибки CCPC в метаданные проекта.
     * \param info Метаданные сообщения об ошибке.
     * \todo Сделать обработку ошибок.
     */
    void storeCCPCError(QVariantMap info);

    /*!
     * \brief Метод записывает
     * код и время ошибки HV в метаданные проекта.
     * \param info Метаданные сообщения об ошибке.
     */
    void storeHVError(QVariantMap info);

    /*!
     * \brief Записывает полученную точку в файл.
     * \param machineHeader Машинный заголовок сообщения с точкой.
     * \param meta Метаданные сообщения с точкой.
     * \param data Данные о набранных событиях.
     * \todo Изменить поле "reply_type" на финальное.
     */
    void savePoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> data);

private:
    /*!
     * \brief Безопасная остановка потока мониторинга HV. Метод устанавливает флаг
     * остановки сервера и дожидается конца его работы.
     * \param hvMonitor Указатель на поток мониторинга HV.
     */
    void stopHvMonitor(HVMonitor *hvMonitor);

    /*!
     * \brief Текущий относительный путь к папке с данными сбора.
     */
    QString currSubFolder;

    /*!
     * \brief Указатель на обработчик CCPC.
     */
    CCPC7Handler *ccpcHandler;

    /*!
     * \brief Последняя ошибка обработчика CCPC.
     */
    ERROR_TYPE lastCCPCError;

    /*!
     * \brief Указатель на обработчик HV.
     */
    HVHandler *hvHandler;

    /*!
     * \brief Последняя ошибка обработчика HV.
     */
    ERROR_TYPE lastHVError;

    /*!
     * \brief Цикл паузы.
     */
    QEventLoop pauseLoop;

    /*!
     * \brief Флаг необходимости паузы.
     */
    bool need_pause;

    /*!
     * \brief Флаг необходимости остановки сценария.
     */
    bool stop_flag;

    /*!
     * \brief Папка подготовлена.
     */
    bool folderOk;

    bool catchUnhandlerErrorFlag;

    /*!
     * \brief Объект для описания информации, записанной вручную.
     */
    QVariantMap info;

    /*!
     * \brief Менеджер настроек.
     */
    IniManager *settingsManager;

    /*!
     * \brief Таймаут проверки напряжения в секундах для HVHandler::setVoltageAndCheck. Задается в конфигурационном файле.
     */
    static int checkVoltageTimeout;

    /*!
     * \brief Максимальное допустимое отклонение напряжения от заданного для функции HVHandler::setVoltageAndCheck. Задается в конфигурационном файле.
     */
    double checkVoltageError;
    void flushInfoFile();

    /*!
     * \brief Время ожидания в секундах после выставления напряжения
     */
    int waitSecAfterVoltage;
};


#endif // ONLINE_H
