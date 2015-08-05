#ifndef ONLINEFORM_H
#define ONLINEFORM_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <online.h>
#include <QJsonDocument>

#include <datavisualizerform.h>

#include "ccpc7handlerform.h"
#include "hvhandlerform.h"

#ifdef TEST_MODE
#include <QDebug>
#endif

namespace Ui {
class OnlineForm;
}

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

    void on_pauseButton_clicked();

    void on_resumeButton_clicked();

    void on_sendComment_clicked();

    void on_stopButton_clicked();

    void setScenarioStage(int stage);

    void on_iterationsBox_valueChanged(int arg1);

    void on_sessionEdit_editingFinished();

    void on_groupEdit_editingFinished();

private:
    CCPC7Handler *ccpc7Handler;
    HVHandler *hvHandler;

    /*!
     * \brief curr_scenario_raw
     * \details строка с текущим исходником сценария. Нужна, чтобы записать
     * в метаинформацию сбора
     */
    QString curr_scenario_raw;

    DataVisualizerForm *dataVisualizerForm;

    int curr_scenario_process_time;
    QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> curr_scenario;

    bool surnameOk;
    bool sessionOk;
    bool groupOk;
    bool scenarioOk;
    bool processingOk;

    void updateEnabledButton();

    Online *online;
    IniManager *settingsManager;
    Ui::OnlineForm *ui;

    QStringListModel *model;
    void visualizeScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant>> scenario);

    static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath);
};

#endif // ONLINEFORM_H
