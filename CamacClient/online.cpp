#include "online.h"

Online::Online(IniManager *settingsManager, CCPC7Handler *ccpcHandler, HVHandler *hvHandler, QObject *parent) : QObject(parent)
{
    this->settingsManager = settingsManager;

    qRegisterMetaType<MachineHeader>("MachineHeader");

    folderOk = 0;

    //создание папки temp
    QDir dir("temp");
    dir.removeRecursively();
    QDir().mkdir("temp");

    //настройка паузы
    connect(this, SIGNAL(stop_pauseLoop()), &pauseLoop, SLOT(quit()));

    if(!settingsManager->getSettingsValue("Online", "voltage_reload_time").isValid())
        settingsManager->setSettingsValue("Online", "voltage_reload_time", 5);

    if(!settingsManager->getSettingsValue("Online", "output_folder").isValid())
        settingsManager->setSettingsValue("Online", "output_folder", "output");

    this->ccpcHandler = ccpcHandler;
    connect(ccpcHandler, SIGNAL(error(QVariantMap)), this, SLOT(handleCCPCError(QVariantMap)));
    lastCCPCError = CLIENT_NO_ERROR;

    this->hvHandler = hvHandler;
    connect(hvHandler, SIGNAL(error(QVariantMap)), this, SLOT(handleHVError(QVariantMap)));
    lastHVError = CLIENT_NO_ERROR;

    connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>)),
            this, SLOT(savePoint(MachineHeader,QVariantMap,QVector<Event>)));
}

Online::~Online()
{

}

bool Online::prepareFolder(QString session, QString group, int iteration, bool add_time)
{
    currSubFolder = tr("%1/%2/%3").arg(session).arg(group).arg(iteration);
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
    if(!ccpcHandler->isSocketConnected() || ccpcHandler->hasError())
    {
        ccpcHandler->reconnect(ccpcIp, ccpcPort);

        QEventLoop el;
        connect(ccpcHandler, SIGNAL(socketConnected(QString,int)), &el, SLOT(quit()));
        connect(ccpcHandler, SIGNAL(error(QVariantMap)), &el, SLOT(quit()));
        el.exec();

        if(!ccpcHandler->isSocketConnected())
        {
            lastCCPCError = CLIENT_DISCONNECT;
            LOG(ERROR) << "Failed to connect to ccpc server";
            return false;
        }
    }

    if(!ccpcHandler->hasInited())
    {
        QEventLoop el;
        bool ok;
        ok = connect(ccpcHandler, SIGNAL(serverInited()), &el, SLOT(quit()));
        ok = connect(ccpcHandler, SIGNAL(error(QVariantMap)), &el, SLOT(quit()));

        ccpcHandler->initServer();
        el.exec();

        if(ccpcHandler->hasError())
        {
            lastCCPCError = SERVER_INIT_ERROR;
            return false;
        }
    }


    if(!hvHandler->isSocketConnected() || hvHandler->hasError())
    {
        QEventLoop el;
        connect(hvHandler, SIGNAL(socketConnected(QString,int)), &el, SLOT(quit()));
        connect(hvHandler, SIGNAL(error(QVariantMap)), &el, SLOT(quit()));

        hvHandler->reconnect(HVIp, HVPort);

        el.exec();
        if(!hvHandler->isSocketConnected())
        {
            LOG(ERROR) << "Failed to connect to hv server";
            lastHVError = CLIENT_DISCONNECT;
            return false;
        }
    }

    if(!hvHandler->hasInited())
    {
        hvHandler->initServer();
        if(hvHandler->hasError())
        {
            lastHVError = SERVER_INIT_ERROR;
            return false;
        }
    }

    return true;
}

void Online::stopHvMonitor(HVMonitor *hvMonitor)
{
#ifndef VIRTUAL_MODE
    hvMonitor->stopHvMonitorFlag = 1;
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
#endif
}

bool Online::processScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
    if(!folderOk)
    {
        //проверка подлготовленности папки
        emit sendInfoMessage(tr("Folder %1 has not prepaired. Stop processing.\n").arg(currSubFolder));
#ifdef TEST_MODE
        qDebug()<<tr("Folder %1 has not prepaired. Stop processing.").arg(currSubFolder);
#endif
        folderOk = 0;
        return false;
    }

    //создание файла info
    updateInfo();


#ifdef TEST_MODE
    qDebug()<<QThread::currentThreadId();
#endif

    //подготовка файла с напряжением
    HVMonitor *hvMonitor = new HVMonitor(currSubFolder, hvHandler);
    connect(hvMonitor, SIGNAL(finished()), hvMonitor, SLOT(deleteLater()));
#ifndef VIRTUAL_MODE
    hvMonitor->start();
#endif

    double last_hv_1 = -1;
    double last_hv_2 = -1;

    need_pause = 0;
    stop_flag = 0;

    for(int i = 0; i < scenario.size(); i++)
    {
        emit at_step(i);

        if(stop_flag)
        {
            emit sendInfoMessage("Stop acquisition by user request.\n");
            stopHvMonitor(hvMonitor);
            folderOk = 0;
            return true;
        }
        if(need_pause)
        {
            emit sendInfoMessage("Pause now.\n");
            pauseLoop.exec();
        }

        auto command = scenario[i];
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
#ifndef VIRTUAL_MODE
                connect(hvHandler, SIGNAL(setVoltageDone(QVariantMap)), &el, SLOT(quit()));
                connect(hvHandler, SIGNAL(error(QVariantMap)), &el, SLOT(quit()));
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));

                hvHandler->setVoltage(args["block"].toInt(), args["voltage"].toDouble());
#else
                QTimer::singleShot(100, &el, SLOT(quit()));
#endif
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
        case ACQUIRE_POINT:
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
                ext_metadata["acquisition_time"] = tr("%1").arg(time);
                ext_metadata["HV1_value"] = tr("%1").arg(last_hv_1);
                ext_metadata["HV2_value"] = tr("%1").arg(last_hv_2);
                if(meta.contains("index"))
                    ext_metadata["point_index"] = meta["index"];

                QEventLoop el;
                connect(this, SIGNAL(stop_scenario()), &el, SLOT(quit()));
#ifndef VIRTUAL_MODE
                connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>)), &el, SLOT(quit()));
                connect(ccpcHandler, SIGNAL(error(QVariantMap)), &el, SLOT(quit()));


                ccpcHandler->acquirePoint(time, ext_metadata);
#else
                QTimer::singleShot(time * 1000, &el, SLOT(quit()));
#endif
                el.exec();

#ifndef VIRTUAL_MODE
                //если выход прошел по стопу - остановка сбора точки
                if(stop_flag)
                {
                    QEventLoop elLastPoint;
                    connect(ccpcHandler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>)),
                            &elLastPoint, SLOT(quit()));
                    connect(ccpcHandler, SIGNAL(error(QVariantMap)),
                            &elLastPoint, SLOT(quit()));

                    ccpcHandler->breakAcquisition();

                    //ожидание загрузки точки
                    elLastPoint.exec();
                }
#endif
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
                QTimer::singleShot(msec, Qt::CoarseTimer, &el, SLOT(quit()));

                el.exec();
                break;
            }

        case BREAK:
            emit breaking();
            stopHvMonitor(hvMonitor);
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

int Online::approximateScenarioTime(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
    int timeMSec = 0;

    for(int i = 0; i < scenario.size(); i++)
    {
        switch (scenario[i].first)
        {
            case ACQUIRE_POINT:
            {
                //время на передачу данных
                timeMSec += 500;
                timeMSec += scenario[i].second.toMap()["time"].toInt() * 1000;
                break;
            }
            case WAIT:
            {
                timeMSec += scenario[i].second.toInt();
                break;
            }
            case SET_VOLTAGE:
            {
                timeMSec += 200;
                break;
            }
            case BREAK:
            {
                return timeMSec / 1000;
            }
        }
    }

    return timeMSec / 1000;
}

QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > Online::constructReverseScenario(QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario)
{
   QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > reverse_scenario;

   for(int i = scenario.size() - 1; i >= 2; i -= 3)
   {
       if(scenario[i].first == ACQUIRE_POINT &&
          scenario[i - 1].first == WAIT &&
          scenario[i - 2].first == SET_VOLTAGE)
       {
           reverse_scenario.push_back(scenario[i - 2]);
           reverse_scenario.push_back(scenario[i - 1]);
           reverse_scenario.push_back(scenario[i]);
       }
       else
       {
           //при конструировании произошла ошибка
           reverse_scenario.clear();
           return reverse_scenario;
       }
   }

   return reverse_scenario;
}

void Online::setOk(bool answer, bool *ok)
{
    if(ok)
        ok[0] = answer;
}

QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > Online::parseScenario(QString scenario_string, bool *ok)
{
    QStringList elements = scenario_string.split(QRegExp("\\s+"));

    QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> > scenario;
    int command_number = 0;
    int point_index = 0;
    for(int i = 0; i < elements.size(); i++, command_number++)
    {
        if(!elements[i].compare("SET_VOLTAGE", Qt::CaseInsensitive))
        {
            //cчитывание аргументов
            //проверка граничности
            if( elements.size() < i + 2 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            //block
            bool parseOk;
            int block = elements[i+1].toInt(&parseOk);
            //проверка правильности парсинга
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }
            //проверка правильности блока
            if(block !=1 && block !=2)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): block number is not correct."
                                 "Aviable block numbers: 1,2")
                              .arg(command_number).arg(elements[i]).toStdString();
                setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            double voltage = elements[i+2].toDouble(&parseOk);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as double")
                              .arg(command_number).arg(elements[i]).arg(elements[i+2]).toStdString();
                setOk(0, ok);
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
        if(!elements[i].compare("ACQUIRE_POINT", Qt::CaseInsensitive))
        {
            //проверка граничности
            if( elements.size() < i + 1 )
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): not enough arguments")
                              .arg(command_number).arg(elements[i]).toStdString();
                setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            //сборка аргументов
            //block
            bool parseOk;
            QVariantMap meta;
            meta["time"] = tr("%1").arg(elements[i+1].toInt(&parseOk));
            meta["index"] = tr("%1").arg(point_index++);
            if(!parseOk)
            {
                LOG(ERROR) << tr("Error in command #%1 (%2): can not interpretate"
                                 "'%3' as int")
                              .arg(command_number).arg(elements[i]).arg(elements[i+1]).toStdString();
                setOk(0, ok);
                return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
            }

            scenario.push_back(QPair<SCENARIO_COMMAND_TYPE, QVariant>
                               (ACQUIRE_POINT, meta));
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
                setOk(0, ok);
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
                setOk(0, ok);
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
        setOk(0, ok);
        return QVector<QPair<SCENARIO_COMMAND_TYPE, QVariant> >();
    }

    setOk(1, ok);
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

void Online::updateInfo(QVariant infoBlock, bool addAsComment)
{
    //создание структуры файла с комментариями
    if(info.isEmpty())
    {
        info["type"] = "info_file";
        info["comments"] = QVariantList();
        info["date"] = QDate::currentDate().toString(Qt::ISODate);
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
    QFile infoFile("temp/" + currSubFolder + "/meta");
    infoFile.open(QIODevice::WriteOnly);
    infoFile.write(TcpProtocol::createMessage(info, QByteArray()));
    infoFile.close();
//#ifndef USE_QTJSON
//    QJson::Serializer serializer;
//    serializer.setIndentMode(QJson::IndentFull); // в настройки
//#endif

//    QFile infoFile("temp/" + currSubFolder + "/info.json");
//    infoFile.open(QIODevice::WriteOnly);

//#ifdef USE_QTJSON
//    infoFile.write(QJsonDocument::fromVariant(map).toJson());
//#else
//    bool ok;
//    serializer.serialize(map, (QIODevice*)(&infoFile), &ok);
//#endif
//    infoFile.close();
}

void Online::pause()
{
    emit sendInfoMessage("Pause signal received. Will stop at next step.\n");
    //добавление комментария о паузе в информацию
    updateInfo(tr("Сбор поставлен на паузу."), true);
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

void Online::handleCCPCError(QVariantMap info)
{
    info["error_type"] = "CCPC error";
    info["timestamp"] = QDateTime::currentDateTime();
    updateInfo(info, true);
}

void Online::handleHVError(QVariantMap info)
{
    info["error_type"] = "HV error";
    info["timestamp"] = QDateTime::currentDateTime();
    updateInfo(info, true);
#ifdef TEST_MODE
    qDebug() << "handling hv server error by re initing";
    qDebug() << info;
#endif
    QString ip = settingsManager->getSettingsValue("HV_handler", "ip").toString();
    int port = settingsManager->getSettingsValue("HV_handler", "port").toInt();
    hvHandler->reconnect(ip, port);
    hvHandler->initServer();
}

void Online::savePoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> data)
{
    QVariantMap ext_meta = meta["external_meta"].toMap();

    //создание имени по имеющимся метаданным
    QString filename = "p";
    if(ext_meta.contains("point_index"))
        filename = tr("%1%2").arg(filename).arg(ext_meta["point_index"].toInt());
    else
        filename = tr("%1%2").arg(filename).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    if(ext_meta.contains("acquisition_time"))
        filename += tr("(%1s)").arg(ext_meta["acquisition_time"].toInt());
    if(ext_meta.contains("HV1_value") && ext_meta["HV1_value"].toInt() != -1)
        filename += tr("(HV1=%1)").arg(ext_meta["HV1_value"].toInt());
    if(ext_meta.contains("HV2_value") && ext_meta["HV2_value"].toInt() != -1)
        filename += tr("(HV2=%1)").arg(ext_meta["HV2_value"].toInt());


    QFile pointFile(tr("temp/%1/%2").arg(currSubFolder).arg(filename));
    pointFile.open(QIODevice::WriteOnly);

    QByteArray file_data = TcpProtocol::createMessageWithPoints(meta, data,
                                                                machineHeader.metaType,
                                                                machineHeader.dataType);

    pointFile.write(file_data);

    pointFile.close();
}

HVMonitor::HVMonitor(QString subFolder, HVHandler *hvHandler):QThread(0)
{
    this->subFolder = subFolder;
    this->hvHandler = hvHandler;
    stopHvMonitorFlag = 0;

    last_divider1_voltage = -1;
    last_divider2_voltage = -1;

    connect(hvHandler, SIGNAL(getVoltageDone(QVariantMap)),
            this, SLOT(saveCurrentVoltage(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(getVoltage(int)), hvHandler, SLOT(getVoltage(int)), Qt::QueuedConnection);

    connect(this, SIGNAL(finished()), this, SLOT(beforeClose()));
}

void HVMonitor::prepareVoltageFile(BINARYTYPE type)
{
    if(type != HV_BINARY && type != HV_TEXT_BINARY)
        type = HV_BINARY;

    this->binaryType = type;

    stopHvMonitorFlag = 0;
    voltageFile = new QFile(tr("temp/%1/voltage").arg(subFolder));
    bool ok = voltageFile->open(QIODevice::WriteOnly);

    //создание метаданных
    QVariantMap meta;
    meta["type"] = "voltage";
    switch (type)
    {
        case HV_BINARY:
            meta["format_description"] = "https://drive.google.com/open?id=1FY_twMu3VFa-WNzFpMab-d2Fgb_QlAZtM1EQV0Io7i0";
            break;
        case HV_TEXT_BINARY:
            meta["format_description"] = "https://drive.google.com/open?id=1onpiq0FB7m1B86fy2zy3pzfw-8j9YxzDWZFoOKG3ySk";
            break;
    }
    meta["programm_revision"] = APP_REVISION;

    QByteArray serializedMeta = TcpProtocol::createMessage(meta, QByteArray(), JSON_METATYPE, type);
    hvFileMachineHeader = TcpProtocol::readMachineHeader(serializedMeta);

    voltageFile->write(serializedMeta);
    if(!ok)
    {
        LOG(ERROR) << tr("Could not create file: %1. Will try again at next point.")
                      .arg(voltageFile->errorString()).toStdString();
        voltageFile->deleteLater();
    }
}

void HVMonitor::closeVoltageFile()
{
    stopHvMonitorFlag = 1;

    voltageFile->close();
    voltageFile->deleteLater();
}

void HVMonitor::insertVoltageBinary(QVariantMap &message)
{
    //запись напряжения в файл
    //обновление бинарного хедера
    hvFileMachineHeader.dataLenght += (sizeof(unsigned char)
                                       + sizeof(unsigned long long int)
                                       + sizeof(double));
    hvFileMachineHeader.dataType = HV_BINARY;
    voltageFile->seek(0);
    voltageFile->write(TcpProtocol::writeMachineHeader(hvFileMachineHeader));

    //записывание бинарных данных
    voltageFile->seek(voltageFile->size());
    unsigned char block = message["block"].toInt();
    unsigned long long int time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    double voltage = message["voltage"].toDouble();
    voltageFile->write((const char*)&block, sizeof(block));
    voltageFile->write((const char*)&time, sizeof(time));
    voltageFile->write((const char*)&voltage, sizeof(voltage));
}

void HVMonitor::insertVoltageText(QVariantMap &message)
{
    //запись напряжения в файл
    //обновление бинарного хедера
    QByteArray voltageLine = tr("%1 %2 %3\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                                             .arg(message["block"].toInt())
                                             .arg(message["voltage"].toDouble()).toLatin1();

    hvFileMachineHeader.dataLenght += voltageLine.size();
    hvFileMachineHeader.dataType = HV_TEXT_BINARY;
    voltageFile->seek(0);
    voltageFile->write(TcpProtocol::writeMachineHeader(hvFileMachineHeader));

    //записывание бинарных данных
    voltageFile->seek(voltageFile->size());
    voltageFile->write(voltageLine);
}

void HVMonitor::saveCurrentVoltage(QVariantMap message)
{
    if(!message.contains("block") || !message.contains("voltage"))
        return;

    if(message["block"].toString() == "1")
        last_divider1_voltage = message["voltage"].toDouble();

    if(message["block"].toString() == "2")
        last_divider2_voltage = message["voltage"].toDouble();

    switch (hvFileMachineHeader.dataType)
    {
        case HV_BINARY:
            insertVoltageBinary(message);
            break;
        case HV_TEXT_BINARY:
            insertVoltageText(message);
            break;
    }
}

void HVMonitor::beforeClose()
{
#ifdef TEST_MODE
    qDebug()<<"closing hv monitor thread";
#endif

    closeVoltageFile();
    stopHvMonitorFlag = 1;
}

void HVMonitor::run()
{
#ifdef TEST_MODE
    qDebug()<<QThread::currentThreadId();
#endif

    prepareVoltageFile();

    QEventLoop el;
    connect(hvHandler, SIGNAL(getVoltageDone(QVariantMap)), &el, SLOT(quit()));
    connect(this, SIGNAL(destroyed()), &el, SLOT(quit()));

    while(!stopHvMonitorFlag)
    {
        if(hvHandler->hasError())
        {
#ifdef TEST_MODE
            qDebug()<<"Error occured in hv handler. Waiting while it will be solved";
#endif
            QEventLoop elError;
            connect(hvHandler, SIGNAL(ready()), &elError, SLOT(quit()));
            elError.exec();
#ifdef TEST_MODE
            qDebug()<<"Solved error in hv handler.";
#endif
        }

        emit getVoltage(1);

        el.exec();
        if(stopHvMonitorFlag)
            return;

        emit getVoltage(2);
        el.exec();
        if(stopHvMonitorFlag)
            return;
    }
}
