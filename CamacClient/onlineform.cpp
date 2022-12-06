#include "onlineform.h"
#include "ui_onlineform.h"
#include <QTime>
#include <QMovie>
#include <QDirIterator>
#include <QScrollBar>

void OnlineForm::refreshGroupCompleter()
{
    QDir dir(settingsManager->getSettingsValue("Online","output_folder").toString() + QDir::separator() + ui->sessionEdit->text());

    QDirIterator dirIt(dir,
                       QDirIterator::Subdirectories);

    QRegExp rx("*set_*_*");
    rx.setPatternSyntax(QRegExp::Wildcard);

    QStringList dirs;

    while(dirIt.hasNext())
    {
        QString currDir = dir.relativeFilePath(QFileInfo(dirIt.next()).absoluteFilePath());

        if(!rx.exactMatch(currDir) && !dirs.contains(currDir))
            dirs.push_back(currDir);
    }

    if(groupCompleter)
        groupCompleter->deleteLater();

    groupCompleter = new QCompleter(dirs, this);

    groupCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    ui->groupEdit->setCompleter(groupCompleter);
}

void OnlineForm::processWorkStatus(bool working)
{
    if(working)
    {
        ui->stepProgressBar->setEnabled(true);
    }
    else
    {
        ui->stepProgressBar->setEnabled(false);
    }
}

void OnlineForm::processInfoMessage(QString message)
{
    if(!message.endsWith("\n"))
        message += "\n";
    ui->infoBrowser->insertPlainText(message);

    infoMessageWipeTimer.start(10000);
}



OnlineForm::OnlineForm(CCPC7Handler *ccpc7Handler, HVHandler *hvHandler,
                       Online *online,
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

    nameCompleter = NULL;
    groupCompleter = NULL;

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

    if(!settingsManager->getSettingsValue(metaObject()->className(), "noShiftBlock").isValid())
        settingsManager->setSettingsValue(metaObject()->className(), "noShiftBlock", true);

    noShiftBlock = settingsManager->getSettingsValue(metaObject()->className(), "noShiftBlock").toBool();

    this->ccpc7Handler = ccpc7Handler;
    this->hvHandler = hvHandler;
    this->online = online;
    this->settingsManager = settingsManager;

    updateEnabledButton();
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);
    ui->stopButton->setEnabled(false);

    ui->finishOnThisIterationBox->setEnabled(false);

    qRegisterMetaType<QVector<Event> >("QVector<Event>");


    connect(online, SIGNAL(sendInfoMessage(QString)),
            this, SLOT(processInfoMessage(QString)), Qt::QueuedConnection);
    connect(&infoMessageWipeTimer, SIGNAL(timeout()),
            ui->infoBrowser, SLOT(clear()), Qt::QueuedConnection);


    connect(ui->pauseButton, SIGNAL(clicked()), online, SLOT(pause()));
    connect(online, SIGNAL(paused()), this, SLOT(onPauseApplied()), Qt::QueuedConnection);
    connect(ui->resumeButton, SIGNAL(clicked()), online, SLOT(resume()));
    connect(online, SIGNAL(stop_pauseLoop()), this, SLOT(onResumeApplied()), Qt::QueuedConnection);
    connect(online, SIGNAL(at_step(int,int)), this, SLOT(setScenarioStage(int,int)), Qt::QueuedConnection);
    connect(online, SIGNAL(workStatusChanged(bool)), this, SLOT(processWorkStatus(bool)), Qt::QueuedConnection);

    QObject::connect(ui->stepBackButton, &QPushButton::clicked, online, &Online::step_back);
    QObject::connect(ui->stepFrontButton, &QPushButton::clicked, online, &Online::step_front);

    on_checkUserForNextStep_stateChanged(ui->checkUserForNextStep->checkState());

    refreshNameCompleter();
    refreshGroupCompleter();

    connect(ui->sessionEdit, SIGNAL(editingFinished()), this, SLOT(refreshGroupCompleter()));

    //настройка вывода прогресса
    ui->stepProgressBar->setValue(0);
    ui->stepProgressBar->setEnabled(false);

    paramsUpdater = new ParamsUpdater(ui->paramsBrowser, this);
    timeWatcher = new OnlineFormTimeWatcher(paramsUpdater, this);

    connect(online, SIGNAL(scenario_done()),
            this, SLOT(processScenarioDone()), Qt::DirectConnection);
    connect(online, SIGNAL(scenario_start()),
            this, SLOT(processScenarioStart()), Qt::DirectConnection);

    connect(this->ccpc7Handler, SIGNAL(currentAcqStatus(long,int,int)), this, SLOT(showCurrentAcqStatus(long,int,int)), Qt::QueuedConnection);

    ticker = NULL;
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

    curr_scenario = Online::parseScenario(scenarioString, &scenarioOk, noShiftBlock);

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

    QStringList scenarioList;
    for(int i = 0; i < scenario.size(); i++)
    {
        switch (scenario[i].first)
        {
        case SET_VOLTAGE_AND_CHECK:
            {
                QVariantMap args = scenario[i].second.toMap();
                scenarioList.push_back(tr("Set voltage %1 on block #%2 and check")
                                       .arg(args["voltage"].toDouble())
                                       .arg(args["block"].toInt())
                                       );
                break;
            }
        case SET_VOLTAGE:
            {
                QVariantMap args = scenario[i].second.toMap();
                scenarioList.push_back(tr("Set voltage %1 on block #%2")
                                       .arg(args["voltage"].toDouble())
                                       .arg(args["block"].toInt())
                                       );
                break;
            }
        case ACQUIRE_POINT:
            {
                scenarioList.push_back(tr("Acquire point %1 (%2s)")
                                       .arg(scenario[i].second.toMap()["index"].toInt())
                                       .arg(scenario[i].second.toMap()["time"].toInt()));
                break;
            }
        case ACQUIRE_MULTIPOINT:
            {
                scenarioList.push_back(tr("Acquire point splitted by chunks %1 (%2s)")
                                       .arg(scenario[i].second.toMap()["index"].toInt())
                        .arg(scenario[i].second.toMap()["time"].toInt()));
                break;
            }
        case WAIT:
            {
                scenarioList.push_back(tr("Wait %1s")
                                       .arg((double)(scenario[i].second.toInt()) / 1000.));
                break;
            }
        case BREAK:
            {
                scenarioList.push_back(tr("Stop"));
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

int OnlineForm::findMaxIndexInFolder(QString output_folder)
{
    QStringList foldersList = QDir(output_folder).entryList(QDir::Dirs);

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

    return max_idx;
}

void OnlineForm::flushData(QString output_folder)
{
    QString outputCurrFolder = output_folder + "/" +online->getCurrSubFolder();

    QFileInfo fi(outputCurrFolder);
    int max_idx = findMaxIndexInFolder(fi.absoluteDir().absolutePath());

    outputCurrFolder = fi.absolutePath() + QDir::separator() + tr("set_%1").arg(max_idx + 1);

    if(!copyRecursively("temp/" + online->getCurrSubFolder(), outputCurrFolder))
    {
        QMessageBox::warning(this, "Copy error", tr("Can't copy temp/%1 to %2. Please do it"
                                                    "manually and then press ok.")
                                                 .arg(online->getCurrSubFolder()).arg(outputCurrFolder));
    }

    //обновление автодополнения
    refreshGroupCompleter();
}

void OnlineForm::refreshNameCompleter(QString operatorSurname)
{
    QStringList operators = settingsManager->getSettingsValue(metaObject()->className(), "operatorSurnames").toStringList();
    if(!operatorSurname.isEmpty() && !operators.contains(operatorSurname))
        operators.push_back(operatorSurname);
    settingsManager->setSettingsValue(metaObject()->className(), "operatorSurnames", operators);

    if(nameCompleter)
    {
        nameCompleter->deleteLater();
        nameCompleter = NULL;
    }

    nameCompleter = new QCompleter(operators);

    nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    ui->operatorSurnameEdit->setCompleter(nameCompleter);
}

void OnlineForm::displayCurrentSetFolder()
{
    QString output_folder = settingsManager->getSettingsValue("Online","output_folder").toString();

    QFileInfo fi(output_folder + QDir::separator() + online->getCurrSubFolder());
    QString outGroupFolder = fi.absoluteDir().absolutePath();
    int maxInd = findMaxIndexInFolder(outGroupFolder);
    paramsUpdater->updateParam("Путь к выходной директории",
                               QDir(output_folder).relativeFilePath(outGroupFolder + QDir::separator() + tr("set_%1").arg(maxInd + 1)));
}

void OnlineForm::on_startButton_clicked()
{
    ui->paramsBrowser->clear();
    ui->infoBrowser->clear();

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

    //блокировка кнопки открыть
    ui->openScenarioButton->setEnabled(false);
    refreshNameCompleter(ui->operatorSurnameEdit->text());

    processingOk = 0;
    updateEnabledButton();

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

    ui->pauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);

    ui->finishOnThisIterationBox->setChecked(false);
    ui->finishOnThisIterationBox->setEnabled(true);

    stopFlag = false;

    for(int i = 0 ; ((!iterations) || (i < iterations)) && !stopFlag; i++)
    {
        //сохранение информации о сборе, набраной вручную
        QVariantMap operatorInfo;
        operatorInfo["name"] = "operator";
        operatorInfo["value"] = ui->operatorSurnameEdit->text();
        online->updateInfo(operatorInfo);

        if(!ui->acquisitionCommentsBox->toPlainText().isEmpty())
        {
            QVariantMap initialCommentInfo;
            initialCommentInfo["name"] = "description";
            initialCommentInfo["value"] = ui->acquisitionCommentsBox->toPlainText();
            online->updateInfo(initialCommentInfo);
        }

        //заполнение информации об итерации
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

        displayCurrentSetFolder();

        //запись сценария в отдельный файл
        online->addFileToScenario(tr("scenario"), curr_scenario_raw.toLatin1());

        if(ui->finishOnThisIterationBox->isChecked())
            break;

        paramsUpdater->updateParam("Iteration", tr("%1 / %2").arg(i + 1).arg(iterations));

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
            ui->infoBrowser->insertPlainText("\nAcquisition scenario failed.");
            break;
        }

        online->clearInfo();

        //переписывание файлов в конечную папку
        flushData(output_folder);
    }
    ui->finishOnThisIterationBox->setEnabled(false);

    paramsUpdater->updateParam("Iteration", tr("-"));
    ui->stopButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    ui->openScenarioButton->setEnabled(true);
    processingOk = 1;
    updateEnabledButton();
}

void OnlineForm::onPauseApplied()
{
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(true);
}

void OnlineForm::onResumeApplied()
{
    ui->pauseButton->setEnabled(true);
    ui->resumeButton->setEnabled(false);
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
    if(QMessageBox::question(this, tr("Warning"), tr("Stop acquisition?")) == QMessageBox::Yes)
    {
        QTimer::singleShot(0, online, SLOT(stop()));
    }
}

void OnlineForm::setScenarioStage(int stage, int stage_time)
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

    /*
    if(ticker)
    {
        if(ticker->isRunning())
            ticker->stopTimer();

        ticker->deleteLater();
    }

    ticker = new ScenarioStepTicker(stage_time, ui->stepProgressBar, this);
    ticker->start();
    */
}

void OnlineForm::on_iterationsBox_valueChanged(int arg1)
{
    int iterations = ui->iterationsBox->value();

    if(iterations == 1)
        ui->reversAcquisitionBox->setEnabled(false);
    else
        ui->reversAcquisitionBox->setEnabled(true);

    QString totalTime = QDateTime::fromTime_t(curr_scenario_process_time * iterations).toUTC().toString("hh:mm:ss");
    QString iterTime = QDateTime::fromTime_t(curr_scenario_process_time).toUTC().toString("hh:mm:ss");

    paramsUpdater->updateParam("Iteration estimated time", iterTime);

    if(iterations)
        paramsUpdater->updateParam("Estimated time",
                                   tr("%1 (%2 per iteration)")
                                   .arg(totalTime)
                                   .arg(iterTime));
    else
        paramsUpdater->updateParam("Estimated time",
                                   tr("- (%1 per iteration)").arg(iterTime));
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
        disconnect(online, SIGNAL(at_step(int,int)), online, SLOT(pause()));
        break;
    case Qt::Checked:
        connect(online, SIGNAL(at_step(int,int)), online, SLOT(pause()));
    default:
        break;
    }
}

OnlineFormTimeWatcher::OnlineFormTimeWatcher(ParamsUpdater *metaUpdater, QObject *parent) : QThread(parent)
{
    scenarioRunning = false;
    connect(this, SIGNAL(updateTime(QString,QString)),
            metaUpdater, SLOT(updateParam(QString,QString)), Qt::QueuedConnection);
}

void OnlineForm::processScenarioStart()
{
    QEventLoop el;
    connect(timeWatcher, SIGNAL(finished()), &el, SLOT(quit()));

    if(timeWatcher->isRunning())
        el.exec();

    timeWatcher->start();
}

void OnlineForm::processScenarioDone()
{
    timeWatcher->stopTimer();
}

void OnlineForm::showCurrentAcqStatus(long counts, int currentTime, int totalTime)
{
    paramsUpdater->updateParam("Count", tr("%1 (%2/%3 s)").arg(counts).arg(currentTime).arg(totalTime));
}

void OnlineFormTimeWatcher::run()
{
    QTime time;

    time.start();

    QTimer timerSecond;
    QEventLoop el;

    connect(&timerSecond, SIGNAL(timeout()), &el, SLOT(quit()));

    scenarioRunning = true;
    timerSecond.start(1000);
    while(scenarioRunning)
    {
        el.exec();
#if QT_VERSION >= 0x050000
        QString timeStr = QTime::fromMSecsSinceStartOfDay(time.elapsed()).toString(Qt::ISODate);
#else
        QString timeStr = QTime(0,0).addMSecs(time.elapsed()).toString(Qt::ISODate);
#endif
        emit updateTime("Iteration time", timeStr);
    }
}


ScenarioStepTicker::ScenarioStepTicker(int stage_time, QProgressBar *bar, QObject *parent) : QThread(parent)
{
    this->bar = bar;
    this->stage_time = stage_time;
    connect(this, SIGNAL(setBarValue(int)), bar, SLOT(setValue(int)), Qt::QueuedConnection);
}

void ScenarioStepTicker::stopTimer()
{
    stopFlag = true;
    emit stop();
}

void ScenarioStepTicker::run()
{
    stopFlag == false;

    emit setBarValue(0);

    QEventLoop el;
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &el, SLOT(quit()));
    connect(this, SIGNAL(stop()), &el, SLOT(quit()));
    timer.start(1000);

    for(int i = 0; (i < stage_time)&&(!stopFlag); i++)
    {
        el.exec();
        emit setBarValue(((double)(i + 1)/(double)stage_time)*100);
    }
}

ParamsUpdater::ParamsUpdater(QTextBrowser *textBrower, QObject *parent) : QObject(parent)
{
    this->textBroswer = textBrower;
}

void ParamsUpdater::updateParam(QString key, QString value)
{
    metaParams[key] = value;
    updateView();
}

void ParamsUpdater::updateView()
{
    QString text;
    foreach (QString key, metaParams.keys()) {
        text += tr("%1: %2\n").arg(key, metaParams[key]);
    }

    int value = textBroswer->verticalScrollBar()->value();
    textBroswer->setPlainText(text);
    textBroswer->verticalScrollBar()->setValue(value);
}
