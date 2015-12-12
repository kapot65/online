#include "onlineform.h"
#include "ui_onlineform.h"
#include <QTime>

OnlineForm::OnlineForm(CCPC7Handler *ccpc7Handler, HVHandler *hvHandler,
                       DataVisualizerForm *dataVisualizerForm, Online *online,
                       IniManager *settingsManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineForm)
{
    surnameOk = 0;
    scenarioOk = 0;
    sessionOk = 0;
    groupOk = 0;
    processingOk = 1;

    curr_scenario_process_time = 0;

    ui->setupUi(this);

    ui->operatorSurnameLabel->setToolTip(tr("Обязательный параметр."));
    ui->sessionLabel->setToolTip(tr("Обязательный параметр."));
    ui->groupLabel->setToolTip(tr("Обязательный параметр. "
                                  "Если в конце стоит число - будет проводится инкремент по нему."));
    ui->iterationsLabel->setToolTip(tr("При значении равном нулю, будет происходить неограниченное повторение итераций."));

    if(settingsManager->getSettingsValue("OnlineForm", "last_description").isValid())
        ui->acquisitionCommentsBox->setPlainText(settingsManager->getSettingsValue("OnlineForm", "last_description")
                                                 .toString());
    if(settingsManager->getSettingsValue("OnlineForm", "last_session").isValid())
    {
        ui->sessionEdit->setText(settingsManager->getSettingsValue("OnlineForm", "last_session").toString());
        on_sessionEdit_editingFinished();
    }

    this->ccpc7Handler = ccpc7Handler;
    this->hvHandler = hvHandler;
    this->dataVisualizerForm = dataVisualizerForm;
    this->online = online;
    this->settingsManager = settingsManager;

    updateEnabledButton();
    ui->pauseButton->setVisible(false);
    ui->resumeButton->setVisible(false);
    ui->stopButton->setVisible(false);

    ui->scenarioView->setVisible(false);
    ui->finishOnThisIterationBox->setVisible(false);

    qRegisterMetaType<QVector<Event> >("QVector<Event>");
    connect(online, SIGNAL(sendInfoMessage(QString)),
            ui->infoLabel, SLOT(setText(QString)), Qt::QueuedConnection);
    connect(ui->pauseButton, SIGNAL(clicked()), online, SLOT(pause()));
    connect(ui->resumeButton, SIGNAL(clicked()), online, SLOT(resume()));

    connect(online, SIGNAL(foldersPrepaired(QString)),
            dataVisualizerForm, SLOT(openDir(QString)), Qt::QueuedConnection);
    connect(online, SIGNAL(scenario_done()),
            dataVisualizerForm, SLOT(clear()), Qt::QueuedConnection);

    connect(online, SIGNAL(at_step(int)), this, SLOT(setScenarioStage(int)), Qt::QueuedConnection);

    on_checkUserForNextStep_stateChanged(ui->checkUserForNextStep->checkState());
}

OnlineForm::~OnlineForm()
{
    settingsManager->setSettingsValue("OnlineForm", "last_session",
                                      ui->sessionEdit->text());

    settingsManager->setSettingsValue("OnlineForm",
                                      "last_description",
                                      ui->acquisitionCommentsBox->toPlainText());
    delete ui;
}

void OnlineForm::on_openScenarioButton_clicked()
{
    QString dir = settingsManager->getSettingsValue("OnlineForm", "last_scenario_path").toString();
    dir = QFileDialog::getOpenFileName(this, tr("Open file"), dir);

    if(dir.isEmpty())
        return;

    settingsManager->setSettingsValue("OnlineForm", "last_scenario_path", dir);

    QFile scenarioFile(dir);
    scenarioFile.open(QIODevice::ReadOnly);
    QString scenarioString = scenarioFile.readAll();

    curr_scenario = Online::parseScenario(scenarioString, &scenarioOk);

    updateEnabledButton();

    if(!scenarioOk)
    {
        QMessageBox::critical(this, "Error", tr("Errors occured while parsing scenario file"
                                                " (%1). See problems in logfile. Stop processing").arg(dir));
        return;
    }

    curr_scenario_raw = scenarioString;
    //обновление времени
    curr_scenario_process_time = Online::approximateScenarioTime(curr_scenario);
    on_iterationsBox_valueChanged(ui->iterationsBox->value());

    //визуализация сценария
    visualizeScenario(curr_scenario);
}

void OnlineForm::on_operatorSurnameEdit_editingFinished()
{
    if(ui->operatorSurnameEdit->text().isEmpty())
        surnameOk = 0;
    else
        surnameOk = 1;

    updateEnabledButton();
}

void OnlineForm::updateEnabledButton()
{
    if(surnameOk && scenarioOk && processingOk &&
       sessionOk && groupOk)
    {
        ui->iterationsBox->setEnabled(true);
        on_iterationsBox_valueChanged(ui->iterationsBox->value());
        ui->startButton->setEnabled(true);
    }
    else
    {
        ui->iterationsBox->setEnabled(false);
        ui->reversAcquisitionBox->setEnabled(false);
        ui->startButton->setEnabled(false);
    }
}

void OnlineForm::visualizeScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
    ui->scenarioView->clear();
    ui->scenarioView->setVisible(true);

    QStringList scenarioList;
    for(int i = 0; i < scenario.size(); i++)
    {
        switch (scenario[i].first)
        {
        case SET_VOLTAGE:
            {
                QVariantMap args = scenario[i].second.toMap();
                scenarioList.push_back(tr("Выставить напряжение %1 на блоке %2")
                                       .arg(args["voltage"].toDouble())
                                       .arg(args["block"].toInt())
                                       );
                break;
            }
        case ACQUIRE_POINT:
            {
                scenarioList.push_back(tr("Сбор точки %1 (%2s)")
                                       .arg(scenario[i].second.toMap()["index"].toInt())
                                       .arg(scenario[i].second.toMap()["time"].toInt()));
                break;
            }
        case WAIT:
            {
                scenarioList.push_back(tr("Ожидание %1s")
                                       .arg((double)(scenario[i].second.toInt()) / 1000.));
                break;
            }
        case BREAK:
            {
                scenarioList.push_back(tr("Остановка"));
                break;
            }
        }
    }

    //Нумерация операций
    for(int i = 0; i < scenarioList.size(); i++)
    {
        scenarioList[i].push_front(tr("%1. ").arg(i + 1));
    }

    ui->scenarioView->addItems(scenarioList);
}

bool OnlineForm::copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir())
    {
        QDir targetDir = QFileInfo(tgtFilePath).absoluteDir();
#ifdef TEST_MODE
        qDebug()<<QFileInfo(tgtFilePath).fileName();
#endif
        if (!targetDir.mkpath(QFileInfo(tgtFilePath).fileName()))
            return false;

        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }
    return true;
}

void OnlineForm::flushData(QString output_folder)
{
    QString outputCurrFolder = output_folder + "/" +online->getCurrSubFolder();

    QFileInfo fi(outputCurrFolder);
    QStringList foldersList = fi.absoluteDir().entryList(QDir::Dirs);

    //поиск максимального индекса
    int max_idx = 0;
    for(int i = 0; i < foldersList.size(); i++)
    {
        QStringList splittedName = foldersList.at(i).split("_");
        if(splittedName.size() >= 2)
        {
            int currIdx = splittedName.at(1).toInt();

            if(currIdx > max_idx)
                max_idx = currIdx;
        }
    }

    QByteArray binaryTime;
    QDataStream ds(&binaryTime, QIODevice::WriteOnly);
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    ds.writeRawData((const char*)(&(time)), sizeof(qint64));

    QString timeHex = binaryTime.toHex();

    outputCurrFolder = fi.absolutePath() + QDir::separator() + tr("set_%1_%2").arg(max_idx + 1).arg(timeHex);

    if(!copyRecursively("temp/" + online->getCurrSubFolder(), outputCurrFolder))
    {
        QMessageBox::warning(this, "Copy error", tr("Can't copy temp/%1 to %2. Please do it"
                                                    "manually and then press ok.")
                                                 .arg(online->getCurrSubFolder()).arg(outputCurrFolder));
    }
}

void OnlineForm::on_startButton_clicked()
{
    ui->infoLabel->clear();

    dataVisualizerForm->clear();
    QString curr_session = ui->sessionEdit->text();
    QString curr_group = ui->groupEdit->text();

    //проверка правильности указания папки выходных файлов
    QString output_folder = settingsManager->getSettingsValue("Online","output_folder").toString();
    if(!QDir().exists(output_folder))
        QDir().mkpath(output_folder);

    if(!QDir().exists(output_folder))
    {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось создать папку выходных данных %1").arg(output_folder));
        return;
    }

    //блокировка кнопки открыть
    ui->openScenarioButton->setEnabled(false);

    processingOk = 0;
    updateEnabledButton();

    //сохранение информации о сборе, набраной вручную
    QVariantMap operatorInfo;
    operatorInfo["name"] = "operator";
    operatorInfo["value"] = ui->operatorSurnameEdit->text();
    online->updateInfo(operatorInfo);


    //считывание параметров сбора из интерфейса
    int iterations = ui->iterationsBox->value();
    bool useReverseScenario = ui->reversAcquisitionBox->isChecked();

    QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > reverseScenario;
    if(useReverseScenario)
    {
        reverseScenario = Online::constructReverseScenario(curr_scenario);
        if(reverseScenario.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Can not create reverse scenario"));
            return;
        }
    }

    if(!ui->acquisitionCommentsBox->toPlainText().isEmpty())
    {
        QVariantMap initialCommentInfo;
        initialCommentInfo["name"] = "description";
        initialCommentInfo["value"] = ui->acquisitionCommentsBox->toPlainText();
        online->updateInfo(initialCommentInfo);
    }

    QVariantMap descriptionLink;
    descriptionLink["name"] = "format_description";
    descriptionLink["value"] = "https://drive.google.com/open?id=1ATs4Mq3K72TjsNy-1QZvL7WtoB9h90nRpElnLXVqZX0";
    online->updateInfo(descriptionLink);

    QVariantMap revisionInfo;
    revisionInfo["name"] = "programm_revision";
    revisionInfo["value"] = APP_REVISION;
    online->updateInfo(revisionInfo);

#ifndef VIRTUAL_MODE
    if(!online->init(settingsManager->getSettingsValue("CCPC7Handler", "ip").toString(),
                 settingsManager->getSettingsValue("CCPC7Handler", "port").toInt(),
                 settingsManager->getSettingsValue("HV_handler", "ip").toString(),
                 settingsManager->getSettingsValue("HV_handler", "port").toInt()))
    {
        LOG(ERROR) << "Error occured at initialisation step";
        ui->openScenarioButton->setEnabled(true);
        processingOk = 1;
        updateEnabledButton();
        return;
    }
#endif

    ui->pauseButton->setVisible(true);
    ui->stopButton->setVisible(true);

    ui->finishOnThisIterationBox->setChecked(false);
    ui->finishOnThisIterationBox->setVisible(true);

    stopFlag = false;

    for(int i = 0 ; ((!iterations) || (i < iterations)) && !stopFlag; i++)
    {
        //dataVisualizerForm->clear();

        if(!online->prepareFolder(curr_session, curr_group, i))
        {
            QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось создать папку во временной директории."
                                                         "Проверьте корректность написания названия сеанса и группы"));
            LOG(ERROR) << "Error occured at preparation step. Cant create temp folder.";
            ui->openScenarioButton->setEnabled(true);
            processingOk = 1;
            updateEnabledButton();
            return;
        }
        //запись сценария в отдельный файл
        online->addFileToScenario(tr("scenario"), curr_scenario_raw.toLatin1());

        if(iterations > 1)
        {
            QVariantMap iterationInfo;
            QVariantMap iterationParams;
            iterationParams["iteration"] = i + 1;
            iterationParams["reverse"] = (bool)(useReverseScenario && i%2 == 1);
            iterationInfo["name"] = "iteration_info";
            iterationInfo["value"] = iterationParams;
            online->updateInfo(iterationInfo);
        }

        if(ui->finishOnThisIterationBox->isChecked())
            break;

        ui->curr_iteration_label->setText(tr("Current iteration: %1 / %2").arg(i + 1).arg(iterations));

        bool ok;
        if(useReverseScenario && i%2 == 1)
        {
            visualizeScenario(reverseScenario);
            ok = online->processScenario(reverseScenario);
        }
        else
        {
            visualizeScenario(curr_scenario);
            ok = online->processScenario(curr_scenario);
        }

        if(!ok)
        {
            ui->infoLabel->setText(ui->infoLabel->text() + "\nAcquisition scenario failed.");
            break;
        }

        //переписывание файлов в конечную папку
        flushData(output_folder);
    }
    ui->finishOnThisIterationBox->setVisible(false);

    ui->curr_iteration_label->clear();
    ui->stopButton->setVisible(false);
    ui->pauseButton->setVisible(false);
    ui->resumeButton->setVisible(false);

    ui->openScenarioButton->setEnabled(true);
    processingOk = 1;
    updateEnabledButton();
}

void OnlineForm::on_pauseButton_clicked()
{
    ui->pauseButton->setVisible(false);
    ui->resumeButton->setVisible(true);
}

void OnlineForm::on_resumeButton_clicked()
{
    ui->pauseButton->setVisible(true);
    ui->resumeButton->setVisible(false);
}

void OnlineForm::on_sendComment_clicked()
{
    if(ui->commentEdit->text().isEmpty())
        return;

    QVariant comment = QString(ui->commentEdit->text());
    online->updateInfo(comment, 1);

    ui->commentEdit->clear();
}

void OnlineForm::on_stopButton_clicked()
{
    stopFlag = true;
    if(QMessageBox::question(this, tr("Предупреждение"), tr("Вы точно хотите остановить набор?")) == QMessageBox::Yes)
    {
        QTimer::singleShot(0, online, SLOT(stop()));
    }
}

void OnlineForm::setScenarioStage(int stage)
{
#ifdef TEST_MODE
    qDebug()<<"setting stage:" << stage;
#endif
    if(stage >= ui->scenarioView->count() || stage < 0)
        return;

    for(int i = 0; i < ui->scenarioView->count(); i++)
    {
        if(i == stage)
            ui->scenarioView->item(i)->setBackground(QColor(Qt::gray));
        else
            ui->scenarioView->item(i)->setBackground(QColor(Qt::white));
    }

    ui->scenarioView->scrollToItem(ui->scenarioView->item(stage));
}

void OnlineForm::on_iterationsBox_valueChanged(int arg1)
{
    int iterations = ui->iterationsBox->value();

    if(iterations == 1)
        ui->reversAcquisitionBox->setEnabled(false);
    else
        ui->reversAcquisitionBox->setEnabled(true);

    int sec = curr_scenario_process_time * iterations;

    ui->scenario_time_label->setText(tr("Примерное время выполнения: %1ч %2мин %3с")
                                     .arg(sec / (60*60))
                                     .arg(sec / 60)
                                     .arg(sec % 60));
}

void OnlineForm::on_sessionEdit_editingFinished()
{
    if(ui->sessionEdit->text().isEmpty())
        sessionOk = 0;
    else
        sessionOk = 1;

    updateEnabledButton();
}

void OnlineForm::on_groupEdit_editingFinished()
{
    if(ui->groupEdit->text().isEmpty())
        groupOk = 0;
    else
        groupOk = 1;

    updateEnabledButton();
}

void OnlineForm::on_checkUserForNextStep_stateChanged(int arg1)
{
    switch (arg1) {
    case Qt::Unchecked:
        disconnect(online, SIGNAL(at_step(int)), online, SLOT(pause()));
        disconnect(online, SIGNAL(at_step(int)), this, SLOT(on_pauseButton_clicked()));
        break;
    case Qt::Checked:
        connect(online, SIGNAL(at_step(int)), online, SLOT(pause()));
        connect(online, SIGNAL(at_step(int)), this, SLOT(on_pauseButton_clicked()));
    default:
        break;
    }
}
