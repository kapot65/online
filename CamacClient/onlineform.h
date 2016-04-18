#ifndef ONLINEFORM_H
#define ONLINEFORM_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <online.h>

#include <QStringListModel>

#include <datavisualizerform.h>

#include "ccpc7handlerform.h"
#include "hvhandlerform.h"

#ifdef TEST_MODE
#include <QDebug>
#endif

#include <QCompleter>

namespace Ui {
class OnlineForm;
}

/*!
 * \brief Класс для вывода времени, прошедшего с начала итерации на форму OnlineForm.
 * Работает в отдельном потоке. Стартуется через QThread::start. Отсчет завершается с помощью
 * метода OnlineFormTimeWatcher::stopTimer.
 */
class OnlineFormTimeWatcher : public QThread
{
    Q_OBJECT
public:
    /*!
     * \param timeLabel QLabel, куда будет выводится отсчет
     */
    OnlineFormTimeWatcher(QLabel *timeLabel,
                          QObject *parent = NULL);
    void stopTimer(){scenarioRunning = false;}

private:
    QLabel *timeLabel;
    bool scenarioRunning;
    // QThread interface
protected:
    virtual void run();
};

class ScenarioStepTicker : public QThread
{
    Q_OBJECT
public:
    ScenarioStepTicker(int stage_time, QProgressBar *bar, QObject *parent = 0);

public slots:
    void stopTimer();

signals:
    void stop();
    void setBarValue(int val);

private:
    QProgressBar *bar;
    bool stopFlag;
    int stage_time;

    // QThread interface
protected:
    void run();
};

/*!
 * \brief Окно для проведения набора в режиме Онлайн.
 * \todo Сделать уведомление об инициализации вольтметров.
 * \todo Центрировать текущий шаг в таблице.
 */
class OnlineForm : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineForm(CCPC7Handler *ccpc7Handler, HVHandler *hvHandler,
                        DataVisualizerForm *dataVisualizerForm, Online *online,
                        IniManager *settingsManager, QWidget *parent = 0);
    ~OnlineForm();

private slots:
    void on_openScenarioButton_clicked();

    void on_operatorSurnameEdit_editingFinished();

    void on_startButton_clicked();

    void onPauseApplied();

    void onResumeApplied();

    void on_sendComment_clicked();

    void on_stopButton_clicked();

    void setScenarioStage(int stage, int stage_time);

    void on_iterationsBox_valueChanged(int arg1);

    void on_sessionEdit_editingFinished();

    void on_groupEdit_editingFinished();

    void on_checkUserForNextStep_stateChanged(int arg1);

    void refreshGroupCompleter();

    void processWorkStatus(bool working);

    void processInfoMessage(QString message);

    /*!
     * \brief Обработка начала выполнения итерации сценария. Соединен с
     * сигналом Online::scenario_start.
     * \details Начинает отсчет времени с начала итерации сценария
     */
    void processScenarioStart();

    /*!
     * \brief Обработка завершения выполнения итерации сценария. Соединен с
     * сигналом Online::scenario_done.
     * \details Заканчивает отсчет времени с начала итерации сценария
     */
    void processScenarioDone();


private:
    /*!
     * \brief Проинициализировать и обновить список фамилий операторов для автодополнения.
     * \param operatorSurname Фамилия оператора. Если пустая строка - то фамилия не будет добавлена в список
     */
    void refreshNameCompleter(QString operatorSurname = QString());

    /*!
     * \brief Поиск максимального индекса в папке с данными.
     * \param output_folder Путь к данным.
     * \return Текущий максимальный индекс.
     */
    int findMaxIndexInFolder(QString output_folder);

    /*!
     * \brief Отобразить на дисплее путь, куда будет записана текущая итерация.
     * Функция должна вызываться только после функции Online::prepareFolder.
     */
    void displayCurrentSetFolder();

    QCompleter *nameCompleter;
    QCompleter *groupCompleter;


    CCPC7Handler *ccpc7Handler;
    HVHandler *hvHandler;

    bool stopFlag;

    /*!
     * \brief curr_scenario_raw
     * \details строка с текущим исходником сценария. Нужна, чтобы записать
     * в метаинформацию сбора
     */
    QString curr_scenario_raw;

    DataVisualizerForm *dataVisualizerForm;

    int curr_scenario_process_time;
    QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > curr_scenario;

    bool surnameOk;
    bool sessionOk;
    bool groupOk;
    bool scenarioOk;
    bool processingOk;

    void updateEnabledButton();

    /*!
     * \brief Сохранение данных набора в окончательную папку.
     * \todo Перенести в Online.
     */
    void flushData(QString output_folder);

    Online *online;
    IniManager *settingsManager;
    Ui::OnlineForm *ui;


    /*!
     * \brief Таймер очистки информационного лейбла.
     */
    QTimer infoMessageWipeTimer;

    OnlineFormTimeWatcher *timeWatcher;
    ScenarioStepTicker *ticker;

    QStringListModel *model;
    void visualizeScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario);

    static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath);
};

#endif // ONLINEFORM_H
