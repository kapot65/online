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

enum SCENARIO_COMMAND_TYPE
{
    SET_VOLTAGE = 0,
    ACQUIRE_POINT = 1,
    WAIT = 2,
    BREAK = 4
};

class HVMonitor : public QThread
{
    Q_OBJECT
public:
    HVMonitor(QString subFolder, HVHandler *hvHandler);
    bool stopHvMonitorFlag;

    double last_divider1_voltage;
    double last_divider2_voltage;

signals:
    void getVoltage(int block);

private slots:
    void saveCurrentVoltage(QVariantMap message);
    void beforeClose();

protected:
    virtual void run();

private:
    QString subFolder;
    HVHandler *hvHandler;

    void prepareVoltageFile();
    void closeVoltageFile();

    MachineHeader hvFileMachineHeader;

    QFile* voltageFile;
    bool voltageStartFlag;
};

class Online : public QObject
{
    Q_OBJECT
public:
    explicit Online(IniManager *settingsManager,
                    CCPC7Handler *ccpcHandler,
                    HVHandler *hvHandler,
                    QObject *parent = 0);
    ~Online();

    /*!
     * \brief getCurrSubFolder
     * Возвращает относительный путь к текущей подпапке.
     * \return
     */
    QString getCurrSubFolder(){return currSubFolder;}

    bool prepareFolder(QString session, QString group, int iteration, bool add_time = true);

    bool init(QString ccpcIp, int ccpcPort, QString HVIp, int HVPort);
    /*!
     * \brief processScenario
     * выполняет сценарий и записывает резудьтат во временную папку
     * \param scenario
     * \return
     */
    bool processScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    /*!
     * \brief approximateScenarioTime
     * Метод вычисляет примерное время обработки сигнала.
     * \param scenario
     * \return время выполнения сценария в секундах
     */
    static int approximateScenarioTime(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    /*!
     * \brief constructReverseScenario
     * Метод конструирует сценарий обратной сборки.
     * Если методу не удается построить обратный сценарий - он выдает пустой сценарий
     * \warning Может преобразовать только примитивные сценарии.
     * \return
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> constructReverseScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);
    /*!
     * \brief parseScenario
     * Метод парсит сценарий. Если происходит ошибка парсинга - записывает ошибку в лог.
     * \param scenario_string
     * \param ok
     * \return
     */
    static QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> parseScenario(QString scenario_string, bool *ok = 0);
signals:
    void foldersPrepaired(QString dir);
    void scenario_done();
    void at_step(int step);
    void stop_scenario();

    /*!
     * \brief отправляет сообщение на форму
     */
    void sendInfoMessage(QString message);

    void setting_voltage(int block, double voltage);
    void acquiring_point(int time);
    void waiting(int msec);
    void breaking();

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
    void pause();
    void resume();
    void stop();
    void addFileToScenario(QString filename, QByteArray data);

private slots:
    void handleCCPCError(QVariantMap info);
    void handleHVError(QVariantMap info);
    void savePoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> data);

private:
    void stopHvMonitor(HVMonitor *hvMonitor);

    QString currSubFolder;
    CCPC7Handler *ccpcHandler;
    ERROR_TYPE lastCCPCError;
    HVHandler *hvHandler;
    ERROR_TYPE lastHVError;

    static void setOk(bool answer, bool *ok);

    QEventLoop pauseLoop;
    bool need_pause;
    bool stop_flag;

    bool folderOk;

    /*!
     * \brief объект для описания информации, записанной вручную
     */
    QVariantMap info;

    IniManager *settingsManager;
};

#endif // ONLINE_H
