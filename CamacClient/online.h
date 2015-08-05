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

#ifdef VIRTUAL_MODE
#include <QTimer>
#endif

#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <QJson/Parser>
#include <QJson/Serializer>
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
    ACQUIRE_POINT = 1, ///< Провести набор точки.
    WAIT = 2, ///< Ждать.
    BREAK = 4 ///< Остановить сценарий.
};

/*!
 * \brief Класс для считывания напряжения HV в отдельном потоке.
 * \warning В классе нету проверок вольтметров и таймаутов.
 * \todo Добавить таймауты и улучшить устойчивость к ошибкам.
 * \details Класс, в отдельном потоке отсылает через HVHandler запросы на считывание
 * напряжения и обрабатывает их. За каждую итерацию он отправляет 2 запроса на каждый
 * вольтметр и ждет пока с них обоих не придут ответы. После прихода ответов итерация
 * повторяется. Также класс реагирует на ошибки и ждет, пока они исправятся.
 */
class HVMonitor : public QThread
{
    Q_OBJECT
public:
    /*!
     * \brief Конструктор класса.
     * \param subFolder Подпапка куда будет писаться файл с напряжением.
     * Файл будет назваться "voltage"
     * \param hvHandler
     */
    HVMonitor(QString subFolder, HVHandler *hvHandler);

    /*!
     * \brief Возвращает последнее напряжение считанное с блока.
     * \param block Номер блока (1 - основной, 2 - блок смещения).
     * \return Напряжение в вольтах. При некорректном номере блока выдает -1.
     */
    double getLastDividerVoltage(int block);

signals:
    /*!
     * \brief Сигнал, связанный с HVHandler::getVoltage
     * \param block Номер блока (1 - основной, 2 - блок смещения).
     */
    void getVoltage(int block);

    /*!
     * \brief Сигнал, испускаемый при успешном получении ответов с обоих вольтметров.
     */
    void stepDone();

private slots:
    /*!
     * \brief Сохраняет в файл полученное напряжение.
     * \warning перед сохранением, необходимо подготовить файл
     * методом HVMonitor::prepareVoltageFile.
     * \param message Сообщение, присланное с HVHandler.
     */
    void saveCurrentVoltage(QVariantMap message);

    /*!
     * \brief Слот, вызывающийся перед закрытием потока.
     * Производит остановку основного цикла опроса.
     * \warning Этот слот только устанваливает флаг на остановку.
     * Остановку следует определять по сигналу QThread::finished.
     */
    void beforeClose();

protected:
    /*!
     * \brief Основной цикл опроса вольтметров.
     */
    virtual void run();

private:
    /*!
     * \brief Последние считанные напряжения с вольтметров.
     */
    double last_dividers_voltage[2];

    /*!
     * \brief Флаги опршенности вольтметров. Значение 0 -
     * вольтметр еще не опрошен за текущую итерацию, 1 - уже опрожен.
     * Переменная используется в методе HVMonitor::saveCurrentVoltage для
     * определения условий для испускания сигнала HVMonitor::stepDone
     */
    bool blockDone[2];

    /*!
     * \brief Путь к подпапке, указанный при создании класса.
     */
    QString subFolder;

    /*!
     * \brief Указатель на HVHandler, указанный при создании класса.
     */
    HVHandler *hvHandler;

    /*!
     * \brief Создает и подготавливает файл для записи напряжения.
     * \param type Формат записи данных о напряжении.
     */
    void prepareVoltageFile(BINARYTYPE type = HV_TEXT_BINARY);

    /*!
     * \brief Закрывает файл с напряжением. Используется в слоте HVMonitor::beforeClose
     */
    void closeVoltageFile();

    /*!
     * \brief insertVoltageBinary
     * \details записывает напряжение в блоки в бинарном виде
     * \param message
     */
    void insertVoltageBinary(QVariantMap &message);

    /*!
     * \brief insertVoltageBinary
     * \details записывает напряжение в блоки в текстовом виде
     * \param message
     */
    void insertVoltageText(QVariantMap &message);

    /*!
     * \brief Машинный заголовок для файла с напряжением.
     */
    MachineHeader hvFileMachineHeader;

    /*!
     * \brief Тип данных, в котором будет записываться напряжение.
     */
    BINARYTYPE binaryType;

    /*!
     * \brief Текущий файл с напряжением.
     */
    QFile* voltageFile;
#ifndef VIRTUAL_MODE
public:
#endif
    /*!
     * \brief Флаг на остановку мониторинга напряжения.
     * \todo Убрать в private.
     */
    bool stopHvMonitorFlag;
};

/*!
 * \brief Класс для выполнения сценария.
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
     * \brief Выполняет сценарий и записывает результат во временную папку.
     * \param scenario Сценарий.
     * \return Успешность выполнения сценария.
     */
    bool processScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    /*!
     * \brief Метод вычисляет примерное время обработки сигнала.
     * \param scenario Сценарий.
     * \return Время выполнения сценария в секундах.
     */
    static int approximateScenarioTime(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    /*!
     * \brief Метод конструирует сценарий обратной сборки.
     * Если методу не удается построить обратный сценарий - он выдает пустой сценарий.
     * \warning Может преобразовать только примитивные сценарии.
     * \return Сценарий. Вектор пар. Первый элемет в паре содержит тип команды сценария.
     * Второй элемент - ее аргуметны.
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> constructReverseScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    /*!
     * \brief Метод парсит сценарий. Если происходит ошибка парсинга - записывает ошибку\
     * в лог и возвращает пустой сценарий.
     * \param scenario_string Строка содержащая сырой сценарий.
     * \param ok Флаг успешности парсинга.
     * \return Сценарий. Вектор пар. Первый элемет в паре содержит тип команды сценария.
     * Второй элемент - ее аргуметны.
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> parseScenario(QString scenario_string, bool *ok = 0);

signals:
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
     * \brief Сценарий находится на шаге step. Испускается методом Online::processScenario.
     * \param step текущий шаг.
     */
    void at_step(int step);

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
     * \brief Остановка по сценарию. Испускается методом Online::processScenario.
     */
    void breaking();

    /*!
     * \brief Остановка паузы. Испускается слотом Online::resume.
     */
    void stop_pauseLoop();

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
     * \brief Обработка ошибок CCPC. В данный момент метод только записывает
     * код и время ошибки в метаданные проекта
     * \param info Метаданные сообщения об ошибке.
     * \todo Сделать обработку ошибок.
     */
    void handleCCPCError(QVariantMap info);

    /*!
     * \brief Обработка ошибок CCPC. Метод записывает
     * код и время ошибки в метаданные проекта и переиницализирует сервер.
     * \param info Метаданные сообщения об ошибке.
     */
    void handleHVError(QVariantMap info);

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

    /*!
     * \brief Объект для описания информации, записанной вручную.
     */
    QVariantMap info;

    /*!
     * \brief Менеджер настроек.
     */
    IniManager *settingsManager;
};

#endif // ONLINE_H
