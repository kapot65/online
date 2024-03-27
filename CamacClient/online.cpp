#include "online.h"
#include "qprocess.h"
#include <QMessageBox>

Online::Online(IniManager *settingsManager, CCPC7Handler *ccpcHandler, HVHandler *hvHandler, QObject *parent) : QObject(parent)
{
    this->settingsManager = settingsManager;

    qRegisterMetaType<MachineHeader>("MachineHeader");

    folderOk = 0;

    catchUnhandlerErrorFlag = 0;

    session = "def_session";
    group = "def_group";
    iteration = 0;

    //создание папки temp
    QDir dir("temp");

#if QT_VERSION >= 0x050000
    dir.removeRecursively();
#else
    dir.rmpath("temp");
#endif

    QDir().mkdir("temp");

    //настройка паузы
    connect(this, SIGNAL(stop_pauseLoop()), &pauseLoop, SLOT(quit()));
    connect(this, SIGNAL(stop_scenario()), &pauseLoop, SLOT(quit()));

    if(!settingsManager->getSettingsValue(metaObject()->className(), "output_folder").isValid())
        settingsManager->setSettingsValue(metaObject()->className(), "output_folder", "output");

    this->ccpcHandler = ccpcHandler;
    connect(ccpcHandler, SIGNAL(error(QVariantMap)), this, SLOT(storeCCPCError(QVariantMap)));
    lastCCPCError = CLIENT_NO_ERROR;

    this->hvHandler = hvHandler;
    connect(hvHandler, SIGNAL(error(QVariantMap)), this, SLOT(storeHVError(QVariantMap)));
    lastHVError = CLIENT_NO_ERROR;

    if(!settingsManager->getSettingsValue(metaObject()->className(), "checkVoltageTimeout").isValid())
        settingsManager->setSettingsValue(metaObject()->className(), "checkVoltageTimeout", 20);

    checkVoltageTimeout = settingsManager->getSettingsValue(metaObject()->className(), "checkVoltageTimeout").toInt();

    if(!settingsManager->getSettingsValue(metaObject()->className(), "checkVoltageError").isValid())
        settingsManager->setSettingsValue(metaObject()->className(), "checkVoltageError", 5);

    checkVoltageError = settingsManager->getSettingsValue(metaObject()->className(), "checkVoltageError").toDouble();

    if(!settingsManager->getSettingsValue(metaObject()->className(), "waitSecAfterVoltage").isValid())
        settingsManager->setSettingsValue(metaObject()->className(), "waitSecAfterVoltage", 5);

    waitSecAfterVoltage = settingsManager->getSettingsValue(metaObject()->className(), "waitSecAfterVoltage").toDouble();

    connect(ccpcHandler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(processUnhandledError(QVariantMap)));
    connect(hvHandler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(processUnhandledError(QVariantMap)));

    connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>,QByteArray)),
            this, SLOT(savePoint(MachineHeader,QVariantMap,QVector<Event>,QByteArray)));
}

Online::~Online()
{

}

bool Online::prepareFolder(QString session, QString group, int iteration, bool add_time)
{    
    currSubFolder = tr("%1/%2/%3").arg(session).arg(group).arg(iteration);

    this->session = session;
    this->group = group;
    this->iteration = iteration;

    if(add_time)
        currSubFolder = tr("%1(%2)").arg(currSubFolder)
                                    .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

    if(!QDir().mkpath("temp/" + currSubFolder))
    {
        folderOk = 0;
        emit sendInfoMessage(tr("Cant prepare subfolder %1.\n").arg(currSubFolder));
        return false;
    }

    folderOk = 1;
    emit foldersPrepaired("temp/" + currSubFolder);
    return true;
}

bool Online::init(QString ccpcIp, int ccpcPort, QString HVIp, int HVPort)
{
    emit workStatusChanged(true);

    bool ok = initImpl(ccpcIp, ccpcPort, HVIp, HVPort);

    emit workStatusChanged(false);

    return ok;
}

bool Online::initImpl(QString ccpcIp, int ccpcPort, QString HVIp, int HVPort)
{
    //Переподключение к CCPC
    emit sendInfoMessage("Reconnecting to CCPC server.\n");

    if(!ccpcHandler->haveOpenedConnection() || ccpcHandler->hasError())
    {
        ccpcHandler->reconnect(ccpcIp, ccpcPort);
        if(!ccpcHandler->waitForConnect())
        {
            lastCCPCError = CLIENT_DISCONNECT;

            LOG(ERROR) << "Failed to connect to ccpc server";
            emit sendInfoMessage("Failed to connect to ccpc server.\n");

            return false;
        }
    }

    //Инициализация CCPC
    emit sendInfoMessage("Initializing CCPC server.\n");

    if(!ccpcHandler->hasInited())
    {
        bool ok;
        ccpcHandler->initServer(&ok);
        if(!ok || ccpcHandler->hasError())
        {
            lastCCPCError = SERVER_INIT_ERROR;

            emit sendInfoMessage("Failed to initialize CCPC server.\n");

            return false;
        }
    }

    //Переподключение HV
    emit sendInfoMessage("Reconnecting to HV server.\n");

    if(!hvHandler->haveOpenedConnection() || hvHandler->hasError())
    {
        hvHandler->reconnect(HVIp, HVPort);
        if(!hvHandler->waitForConnect())
        {
            LOG(ERROR) << "Failed to connect to hv server";
            emit sendInfoMessage("Failed to connect to hv server.\n");

            lastHVError = CLIENT_DISCONNECT;
            return false;
        }
    }

    //Инициализация HV
    emit sendInfoMessage("Initializing HV server.\n");

    if(!hvHandler->hasInited())
    {
        bool ok;
        hvHandler->initServer(&ok);

        if(!ok || hvHandler->hasError())
        {
            lastHVError = SERVER_INIT_ERROR;

            emit sendInfoMessage("Failed to initialize HV server.\n");

            return false;
        }
    }

    catchUnhandlerErrorFlag = false;
    return true;
}

void Online::stopHvMonitor(HVMonitor *hvMonitor)
{
    hvMonitor->stopMonitoring();
#ifdef TEST_MODE
    qDebug() << "Waiting until hvMonitor thread stops.";
#endif
    QEventLoop el;
    connect(hvMonitor, SIGNAL(finished()), &el, SLOT(quit()));
    if(hvMonitor->isRunning())
        el.exec();
#ifdef TEST_MODE
    qDebug() << "hvMonitor thread stops.";
#endif
}

bool Online::processScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
    emit workStatusChanged(true);
    bool ok = processScenarioImpl(scenario);
    emit workStatusChanged(false);
    return ok;
}

bool Online::processScenarioImpl(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{   
    if(!folderOk)
    {
        //проверка подготовленности папки
        emit sendInfoMessage(tr("Folder %1 has not prepaired. Stop processing.\n").arg(currSubFolder));
#ifdef TEST_MODE
        qDebug()<<tr("Folder %1 has not prepaired. Stop processing.").arg(currSubFolder);
#endif
        folderOk = 0;
        return false;
    }

    //создание файла info
    updateInfo();

    //подготовка файла с напряжением
    HVMonitor *hvMonitor = new HVMonitor(currSubFolder, hvHandler);
    connect(hvMonitor, SIGNAL(finished()), hvMonitor, SLOT(deleteLater()));

    hvMonitor->start();

    double last_hv_1 = -1;
    double last_hv_2 = -1;

    need_pause = 0;
    stop_flag = 0;

    emit scenario_start();

    int i = 0;

    auto decrement = [&] () {
        i = qMin(qMax(0, i-1), scenario.size() - 1);
        QPair<SCENARIO_COMMAND_TYPE, QVariant> command = scenario[i];
        emit at_step(i, approximateOperationTime(command));
    };

    auto increment = [&] () {
        i = qMin(qMax(0, i+1), scenario.size() - 1);
        QPair<SCENARIO_COMMAND_TYPE, QVariant> command = scenario[i];
        emit at_step(i, approximateOperationTime(command));
    };

    QObject::connect(this, &Online::step_back, decrement);
    QObject::connect(this, &Online::step_front, increment);

    for(; i < scenario.size(); i++)
    {
        if(catchUnhandlerErrorFlag)
        {
            catchUnhandlerErrorFlag = false;

            auto step = scenario[qMax(0, i - 1)];

            switch (step.first) {
                case ACQUIRE_POINT:
                case ACQUIRE_MULTIPOINT:
                    // перенабираем последнюю и предпоследнюю точку
                    // (предпоследняя могла быть неполная)
                    i = qMax(0, i - 4);
                    break;
                default:
                    // остальные шаги достаточно просто повторить
                    i = qMax(0, i - 1);
                    break;
            }
        }

        //получение примерного времени выполнения шага
        QPair<SCENARIO_COMMAND_TYPE, QVariant> command = scenario[i];
        emit at_step(i, approximateOperationTime(command));

        if(need_pause)
        {
            emit sendInfoMessage("Pause now.\n");
            emit workStatusChanged(false);
            pauseLoop.exec();
            emit workStatusChanged(true);
        }

        if(stop_flag)
        {
            emit sendInfoMessage("Stop acquisition by user request.\n");
            stopHvMonitor(hvMonitor);
            emit scenario_done();
            folderOk = 0;
            return true;
        }

        switch (command.first)
        {
        case SET_VOLTAGE:
            {
                QVariantMap args = command.second.toMap();
                if(!args.contains("block") || !args.contains("voltage"))
                {
                    LOG(ERROR) << tr("Process scenario error: args 'block'"
                                     "or 'voltage' not set (operation #%1: SET_VOLTAGE)."
                                     "Stop processing")
                                  .arg(i).toStdString();
                    stopHvMonitor(hvMonitor);
                    folderOk = 0;
                    return false;
                }

                QEventLoop el;
                emit setting_voltage(args["block"].toInt(), args["voltage"].toDouble());
                connect(hvHandler, SIGNAL(setVoltageDone(QVariantMap)), &el, SLOT(quit()));
                connect(hvHandler, SIGNAL(unhandledError(QVariantMap)), &el, SLOT(quit()));
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));

                hvHandler->setVoltage(args["block"].toInt(), args["voltage"].toDouble());
                el.exec();

                switch (args["block"].toInt())
                {
                    case 1:
                        last_hv_1 = args["voltage"].toDouble();
                        break;
                    case 2:
                        last_hv_2 = args["voltage"].toDouble();
                        break;
                }

                break;
            } 
        case SET_VOLTAGE_AND_CHECK:
            {
                QVariantMap args = command.second.toMap();
                if(!args.contains("block") || !args.contains("voltage"))
                {
                    LOG(ERROR) << tr("Process scenario error: args 'block'"
                                     "or 'voltage' not set (operation #%1: SET_VOLTAGE_AND_CHECK)."
                                     "Stop processing")
                                  .arg(i).toStdString();
                    stopHvMonitor(hvMonitor);
                    folderOk = 0;
                    return false;
                }

                QEventLoop el;
                emit setting_voltage(args["block"].toInt(), args["voltage"].toDouble());
                connect(hvHandler, SIGNAL(setVoltageAndCheckDone(QVariantMap)), &el, SLOT(quit()));
                connect(hvHandler, SIGNAL(unhandledError(QVariantMap)), &el, SLOT(quit()));
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));


                int block = args["block"].toInt();

                double voltage = args["voltage"].toDouble();
                double voltageError = checkVoltageError;

                //Если заданное на основном блоке напряжение равно нулю, то берется грубая ошибка,
                //т.к. вблизи нуля коррекция напряжения не производится
                if((block == 1) && (voltage == 0))
                    voltageError = 10;

                hvHandler->setVoltageAndCheck(block, voltage, voltageError, checkVoltageTimeout);
                el.exec();

                QVariantMap answerMeta = hvHandler->getLastVoltageAndCheckMeta();

                if(answerMeta.isEmpty())
                    continue; //Поймана необрабатываемая ошибка остановка будет произведена
                              //функцией processUnhandledError на следующем этапе

                if(answerMeta["status"].toString() != "ok")
                {
                    switch (QMessageBox::question(NULL, tr("Failed to set HV"),
                                          tr("Failed to set up HV %1. "
                                             "Acquisition will be paused\n"
                                             "Desired HV: %2 V\n"
                                             "Real HV: %3 V\n"
                                             "Error: %4 В\n"
                                             "Operation status: %5")
                                             .arg(answerMeta["block"].toInt())
                                             .arg(args["voltage"].toDouble())
                                             .arg(answerMeta["voltage"].toDouble())
                                             .arg(answerMeta["error"].toDouble())
                                             .arg(answerMeta["status"].toString()),
                                             QMessageBox::Ok | QMessageBox::Ignore | QMessageBox::Retry))
                    {
                        case QMessageBox::Ok:
                            i--;
                            pause();
                            break;
                        case QMessageBox::Ignore:
                            break;
                        case QMessageBox::Retry:
                            i--;
                            break;
                    }
                }

                switch (args["block"].toInt())
                {
                    case 1:
                        last_hv_1 = args["voltage"].toDouble();
                        break;
                    case 2:
                        last_hv_2 = args["voltage"].toDouble();
                        break;
                }

                // Дополнительное ожидание времени после выставления напряжения
                LOG(INFO) << tr("Setting voltage done. Waiting additional %1 seconds")
                             .arg(waitSecAfterVoltage);
                QTimer::singleShot(waitSecAfterVoltage * 1000, &el, SLOT(quit()));
                el.exec();
                LOG(INFO) << tr("Finish waiting");

                break;
            }

        case ACQUIRE_POINT:
        case ACQUIRE_MULTIPOINT:
            {
                QVariantMap meta = command.second.toMap();
                if(!meta.contains("time"))
                {
                    LOG(ERROR) << tr("Process scenario error: "
                                     "meta doesnt contains field 'time' "
                                     "(operation #%1: ACQUIRE_POINT)."
                                     "Stop processing")
                                  .arg(i).toStdString();
                    stopHvMonitor(hvMonitor);
                    folderOk = 0;
                    return false;

                }
                int time = meta["time"].toInt();

                emit acquiring_point(time);
                //формирование метаданных
                QVariantMap ext_metadata;
                ext_metadata["HV1_value"] = tr("%1").arg(last_hv_1);
                ext_metadata["HV2_value"] = tr("%1").arg(last_hv_2);

                if(meta.contains("index"))
                    ext_metadata["point_index"] = meta["index"];

                ext_metadata["session"] = session;
                ext_metadata["group"] = group;
                ext_metadata["iteration"] = iteration;

                QEventLoop el;
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));
                connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>,QByteArray)), &el, SLOT(quit()));
                connect(ccpcHandler, SIGNAL(unhandledError(QVariantMap)), &el, SLOT(quit()));


                if(command.first == ACQUIRE_MULTIPOINT)
                    ccpcHandler->acquirePoint(time, ext_metadata, true);
                else
                    ccpcHandler->acquirePoint(time, ext_metadata, false);

                el.exec();

                //если выход прошел по стопу - остановка сбора точки
                if(stop_flag)
                {
                    QEventLoop elLastPoint;
                    connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>,QByteArray)),
                            &elLastPoint, SLOT(quit()));
                    connect(ccpcHandler, SIGNAL(unhandledError(QVariantMap)),
                            &elLastPoint, SLOT(quit()));

                    ccpcHandler->breakAcquisition();

                    //ожидание загрузки точки
                    elLastPoint.exec();
                }
                break;
            }

        case WAIT:
            {
                bool ok;
                int msec = command.second.toInt(&ok);
                if(!ok)
                {
                    LOG(ERROR) << tr("Process scenario error: "
                                     "can not iterpretate parameter "
                                     "as int (operation #%1: WAIT)."
                                     "Stop processing")
                                  .arg(i).toStdString();
                    stopHvMonitor(hvMonitor);
                    folderOk = 0;
                    return false;
                }

                emit waiting(msec);
                QEventLoop el;
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));
                QTimer::singleShot(msec, &el, SLOT(quit()));

                el.exec();
                break;
            }
        case BREAK:
            stopHvMonitor(hvMonitor);
            emit scenario_done();
            folderOk = 0;
            return true;
        }
    }

    //ожидание остановки потока мониторинга hv
    stopHvMonitor(hvMonitor);

    emit scenario_done();
    folderOk = 0;
    return true;
}

int Online::checkVoltageTimeout = 0;

int Online::approximateScenarioTime(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
    double timeSec = 0;

    for(int i = 0; i < scenario.size(); i++)
    {
        if(scenario[i].first == BREAK)
            break;

        timeSec += approximateOperationTime(scenario[i]);
    }

    return timeSec;
}

double Online::approximateOperationTime(QPair<SCENARIO_COMMAND_TYPE, QVariant> step)
{
    double timeMSec = 0;
    switch (step.first)
    {
        case ACQUIRE_POINT:
        case ACQUIRE_MULTIPOINT:
        {
            int acqTimeSec = step.second.toMap()["time"].toInt();

            //поправка времени для делимости на блоки по 5с
            if(step.first == ACQUIRE_MULTIPOINT)
                if(acqTimeSec%5)
                    acqTimeSec += 5 - acqTimeSec%5;

            timeMSec += acqTimeSec * 1000;

            //время на передачу данных
            timeMSec += 500;


            break;
        }
        case WAIT:
        {
            timeMSec += step.second.toInt();
            break;
        }
        case SET_VOLTAGE:
        {
            timeMSec += 200;
            break;
        }
        case SET_VOLTAGE_AND_CHECK:
        {
            timeMSec += 15000;
            break;
        }
        case BREAK:
        {
            return 0;
        }
    }

    return timeMSec / 1000;
}

QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > Online::constructReverseScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
   QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > reverse_scenario;

   for(int i = scenario.size() - 1; i >= 1;)
   {
       bool firstCondition = (scenario[i].first == ACQUIRE_POINT) ||
                             (scenario[i].first == ACQUIRE_MULTIPOINT);

       if(i >= 2 && firstCondition && scenario[i - 1].first == SET_VOLTAGE_AND_CHECK &&
          scenario[i - 2].first == SET_VOLTAGE_AND_CHECK) {
           reverse_scenario.push_back(scenario[i - 2]);
           reverse_scenario.push_back(scenario[i - 1]);
           reverse_scenario.push_back(scenario[i]);
           i -= 3;
       } else if(firstCondition && scenario[i - 1].first == SET_VOLTAGE_AND_CHECK) {
           reverse_scenario.push_back(scenario[i - 1]);
           reverse_scenario.push_back(scenario[i]);
           i -= 2;
       } else if(i >= 3 && firstCondition && scenario[i - 1].first == WAIT &&
                 scenario[i - 2].first == SET_VOLTAGE && scenario[i - 3].first == SET_VOLTAGE) {
           reverse_scenario.push_back(scenario[i - 3]);
           reverse_scenario.push_back(scenario[i - 2]);
           reverse_scenario.push_back(scenario[i - 1]);
           reverse_scenario.push_back(scenario[i]);
           i -= 4;
       } else if(i >= 2 && firstCondition && scenario[i - 1].first == WAIT &&
                 scenario[i - 2].first == SET_VOLTAGE) {
           reverse_scenario.push_back(scenario[i - 2]);
           reverse_scenario.push_back(scenario[i - 1]);
           reverse_scenario.push_back(scenario[i]);
           i -= 3;
       } else {
           //при конструировании произошла ошибка
           reverse_scenario.clear();
           return reverse_scenario;
       }
   }

   return reverse_scenario;
}

QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > Online::parseScenario(QString scenario_string, bool *ok, bool noShiftBlock)
{
    QStringList elements = scenario_string.split(QRegExp("\\s+"));

    QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario;
    int command_number = 0;
    int point_index = 0;
    for(int i = 0; i < elements.size(); i++, command_number++)
    {
        if(elements[i].isEmpty())
            continue;

        if(!elements[i].compare("POINT", Qt::CaseInsensitive))
        {
            //проверка граничности
            if( elements.size() < i + 3 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            bool parseOk;
            int time = elements[i+1].toInt(&parseOk);

            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            double voltageMain = elements[i+2].toDouble(&parseOk);
            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as double")
                              .arg(command_number).arg(elements[i]).arg(elements[i+2]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            double voltageShift = elements[i+3].toDouble(&parseOk);
            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as double")
                              .arg(command_number).arg(elements[i]).arg(elements[i+3]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            QVariantMap args;

            args["block"] = "1";
            args["voltage"] = voltageMain;

            scenario.push_back(qMakePair(SET_VOLTAGE_AND_CHECK, QVariant(args)));


            if(!noShiftBlock) {
                args["block"] = "2";
                args["voltage"] = voltageShift;

                scenario.push_back(qMakePair(SET_VOLTAGE_AND_CHECK, QVariant(args)));
            }

            args.clear();

            args["time"] = tr("%1").arg(time);
            args["index"] = tr("%1").arg(point_index++);

            scenario.push_back(qMakePair(ACQUIRE_MULTIPOINT, QVariant(args)));

            i += 3;
            continue;

        }

        if(!elements[i].compare("SET_VOLTAGE", Qt::CaseInsensitive))
        {
            //cчитывание аргументов
            //проверка граничности
            if( elements.size() < i + 2 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            bool parseOk;
            int block = elements[i+1].toInt(&parseOk);
            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }
            //проверка правильности блока
            if(block !=1 && block !=2)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): block number is not correct."
                                 "Aviable block numbers: 1,2")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            double voltage = elements[i+2].toDouble(&parseOk);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as double")
                              .arg(command_number).arg(elements[i]).arg(elements[i+2]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            QVariantMap args;
            args["block"] = block;
            args["voltage"] = voltage;

            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (SET_VOLTAGE, args));
            i += 2;
            continue;
        }

        if(!elements[i].compare("SET_VOLTAGE_AND_CHECK", Qt::CaseInsensitive))
        {
            //cчитывание аргументов
            //проверка граничности
            if( elements.size() < i + 2 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            bool parseOk;
            int block = elements[i+1].toInt(&parseOk);
            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }
            //проверка правильности блока
            if(block !=1 && block !=2)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): block number is not correct."
                                 "Aviable block numbers: 1,2")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            double voltage = elements[i+2].toDouble(&parseOk);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as double")
                              .arg(command_number).arg(elements[i]).arg(elements[i+2]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            QVariantMap args;
            args["block"] = block;
            args["voltage"] = voltage;

            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (SET_VOLTAGE_AND_CHECK, args));
            i += 2;
            continue;
        }

        if((!elements[i].compare("ACQUIRE_POINT", Qt::CaseInsensitive)) ||
           (!elements[i].compare("ACQUIRE_MULTIPOINT", Qt::CaseInsensitive)))

        {
            bool multipoint = !elements[i].compare("ACQUIRE_MULTIPOINT", Qt::CaseInsensitive);
            //проверка граничности
            if( elements.size() < i + 1 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            //block
            bool parseOk;
            QVariantMap meta;
            if(!multipoint)
                meta["time"] = TcpProtocol::correctMeasureTime(elements[i+1].toInt(&parseOk));
            else
                meta["time"] = elements[i+1].toInt(&parseOk);

            meta["index"] = tr("%1").arg(point_index++);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            SCENARIO_COMMAND_TYPE command = ACQUIRE_POINT;
            if(multipoint)
                command = ACQUIRE_MULTIPOINT;

            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (command, meta));
            i += 1;
            continue;
        }
        if(!elements[i].compare("WAIT", Qt::CaseInsensitive))
        {
            //проверка граничности
            if( elements.size() < i + 1 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            //block
            bool parseOk;
            int msec = elements[i+1].toInt(&parseOk);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                TcpProtocol::setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (WAIT, msec));
            i += 1;
            continue;
        }
        if(!elements[i].compare("BREAK", Qt::CaseInsensitive))
        {
            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (WAIT, QVariant()));
            continue;
        }

        //если эдемент не совпал ни с одно из команд
        LOG(ERROR) << tr("Error in parsing scenario: unknown command #%1 (%2)."
                         "This error may be caused by incorrect arguments number in"
                         "previous commands")
                      .arg(command_number).arg(elements[i]).toStdString();
        TcpProtocol::setOk(0, ok);
        return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
    }

    TcpProtocol::setOk(1, ok);
    return scenario;
}

void Online::addFileToScenario(QString filename, QByteArray data)
{
    if(filename.isEmpty())
    {
        filename = tr("undefined_file_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }

    //обновление файла метаинформации
    QFile binaryFile("temp/" + currSubFolder + "/" + filename);
    binaryFile.open(QIODevice::WriteOnly);
    binaryFile.write(data);
    binaryFile.close();
}

void Online::processUnhandledError(QVariantMap info)
{
    catchUnhandlerErrorFlag = true;
    if (
            onlineFormUi != nullptr &&
            onlineFormUi->autoResumeBox->isChecked() &&
            info["error_code"].toInt() == 6
        ) {

        QProcess::startDetached(
            tr("zenity --warning --text \"%1 %2 (resuming)\" &")
                .arg(QTime::currentTime().toString())
                .arg(info["description"].toString()));

//        connect(th, &QThread::finished, th, &QThread::deleteLater);
//        th->start();
        LOG(ERROR) << "Catched unhandled error. Auto Resume";
    } else {
        QMessageBox::warning(NULL, tr("Поймана необрабатываемая ошибка"),
                             tr("В процессе работы цикла поймана необрабатываемая ошибка: %1.\n"
                                "Сбор поставлен на паузу.").arg(info["description"].toString()));
        LOG(ERROR) << "Catched unhandled error. Pause Acquisition";
        pause();
    }
}

void Online::flushInfoFile()
{
    QFile infoFile("temp/" + currSubFolder + "/meta");
    infoFile.open(QIODevice::WriteOnly);
    infoFile.write(TcpProtocol::createMessage(info));
    infoFile.close();
}

void Online::updateInfo(QVariant infoBlock, bool addAsComment)
{
    //создание структуры файла с комментариями
    if(info.isEmpty())
    {
        info["type"] = "info_file";
        info["comments"] = QVariantList();
        info["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        info["format_description"] = "https://docs.google.com/document/d/12qmnZRO55y6zr08Wf-BQYAmklqgf5y3j_gD_VkNscXc/edit?usp=sharing";
        info["programm_revision"] = APP_REVISION;
    }

    if(infoBlock.isValid())
    {
        if(!addAsComment &&
           infoBlock.toMap().contains("name") &&
           infoBlock.toMap().contains("value"))
        {
            info[infoBlock.toMap()["name"].toString()] = infoBlock.toMap()["value"];
        }
        else
        {
            QVariantList updated_comments = info["comments"].toList();
            switch (infoBlock.type())
            {
                case QVariant::Map:
                    updated_comments.push_back(infoBlock);
                    break;
                case QVariant::String:
                    {
                        QVariantMap comment;
                        comment["date_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        comment["comment"] = infoBlock;
                        updated_comments.push_back(comment);
                        break;
                    }
                default:
                    updated_comments.push_back(infoBlock);
                    break;
            }
            info["comments"] = updated_comments;
        }
    }

    //обновление файла метаинформации
    flushInfoFile();
}

void Online::clearInfo(){

    info["end_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    flushInfoFile();
    info.clear();
}

void Online::pause()
{
    emit sendInfoMessage("Pause signal received. Will stop at next step.\n");
    //добавление комментария о паузе в информацию
    updateInfo(tr("Сбор поставлен на паузу."), true);
    emit paused();
    need_pause = 1;
}

void Online::resume()
{
    emit sendInfoMessage("Resume signal received. Resumming.\n");
    updateInfo(tr("Сбор продолжен."), true);
    need_pause = 0;
    emit stop_pauseLoop();
}

void Online::stop()
{
    updateInfo(tr("Сбор остановлен оператором."), true);
    stop_flag = 1;
    emit stop_scenario();
}

void Online::storeCCPCError(QVariantMap info)
{
    info["error_type"] = "CCPC error";
    info["timestamp"] = QDateTime::currentDateTime();
    updateInfo(info, true);
}

void Online::storeHVError(QVariantMap info)
{
    info["error_type"] = "HV error";
    info["timestamp"] = QDateTime::currentDateTime();
    updateInfo(info, true);
}

void Online::savePoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> events, QByteArray data)
{
    QVariantMap ext_meta = meta["external_meta"].toMap();

    //создание имени по имеющимся метаданным
    QString filename = "p";
    if(ext_meta.contains("point_index"))
        filename = tr("%1%2").arg(filename).arg(ext_meta["point_index"].toInt());
    else
        filename = tr("%1%2").arg(filename).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    if(meta.contains("acquisition_time"))
        filename += tr("(%1s)").arg(meta["acquisition_time"].toInt());
    if(ext_meta.contains("HV1_value") && ext_meta["HV1_value"].toInt() != -1)
        filename += tr("(HV1=%1)").arg(ext_meta["HV1_value"].toInt());
    if(ext_meta.contains("HV2_value") && ext_meta["HV2_value"].toInt() != -1)
        filename += tr("(HV2=%1)").arg(ext_meta["HV2_value"].toInt());


    auto filepath = tr("temp/%1/%2").arg(currSubFolder).arg(filename);
    QFile pointFile(filepath);
    if (pointFile.exists()) {
        auto overwritePath = tr("temp/%1/_dup").arg(currSubFolder);
        QDir().mkpath(overwritePath);
        QDir().rename(filepath, tr("%1/%2-%3").arg(overwritePath, filename, QUuid::createUuid().toString()));
    }

    pointFile.open(QIODevice::WriteOnly);

    QByteArray file_data = TcpProtocol::createMessage(meta, data, machineHeader.metaType, machineHeader.dataType);
    pointFile.write(file_data);
    pointFile.close();
}
