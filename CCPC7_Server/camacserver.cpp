#include "camacserver.h"
#ifdef TEST_MODE
    #include <QDebug>
#endif

CamacServer::CamacServer(int port, CamacServerSettings *settings): TcpServer(port)
{
    this->settings = settings;

//установка соответсвующего уровня логирования
#if __cplusplus == 201103L
    el::Loggers::setLoggingLevel(settings->getLogLevel());
#else
    //easyloggingpp::Loggers::setLoggingLevel(settings->getLogLevel());
#endif

    cmdHandler = new CommandHandler(settings, this);

    //соединение внутренних сигналов со слотами
    connect(this, SIGNAL(receiveMessage(MachineHeader,QVariantMap,QByteArray)),
            this, SLOT(processMessage(MachineHeader,QVariantMap,QByteArray)));

    connect(cmdHandler, SIGNAL(error(QVariantMap)), this, SLOT(on_error(QVariantMap)), Qt::QueuedConnection);

    connect(cmdHandler, SIGNAL(sendMessage(QVariantMap,QByteArray)),
            this, SLOT(sendMessage(QVariantMap,QByteArray)), Qt::QueuedConnection);
    connect(cmdHandler, SIGNAL(sendRawMessage(QByteArray)),
            this, SLOT(sendRawMessage(QByteArray)), Qt::QueuedConnection);

    connect(this, SIGNAL(breakAcquisition(QVariantMap)), cmdHandler, SLOT(processBreakAcquisition(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(acquirePoint(QVariantMap)), cmdHandler, SLOT(processAcquirePoint(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(init(QVariantMap)), cmdHandler, SLOT(processInit(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(NAF(QVariantMap)), cmdHandler, SLOT(ProcessNAF(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(resetCounters(QVariantMap)), cmdHandler, SLOT(processResetCounters(QVariantMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(getCountersValue(QVariantMap)), cmdHandler, SLOT(processGetCountersValue(QVariantMap)), Qt::QueuedConnection);
}
CamacServer::~CamacServer()
{
    settings->deleteLater();
}

void CamacServer::processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData)
{
    //определение типа посылки
    QString messageType = metaData["type"].toString();
    if(messageType == "command")
    {
        processCommand(metaData);
    }
    else
        if(messageType == "reply")
        {
            processReply(metaData);
        }
        else
        {
            //создание описания ошибки
            QVariantMap errorParams;
            errorParams["error_code"] = QString("%1").arg(UNKNOWN_MESSAGE_ERROR);
            errorParams.insert("stage", "process message type");
            errorParams.insert("description", QString("unknown message type: %1").arg(messageType));

            emit error(errorParams);
            return;
        }
}

void CamacServer::processCommand(QVariantMap message)
{
    QString commandType = message.value("command_type").toString();

    if(commandType == "break_acquisition")
    {
#ifdef TEST_MODE
        qDebug() << tr("receive break_acquisition message");
#endif

        emit breakAcquisition(message);
    }
    else
        if(cmdHandler->checkBusy())
        {
            //устройство занято
            //создание описания ошибки
            QVariantMap errorParams;
            errorParams["error_code"] = QString("%1").arg(SERVER_BUSY_ERROR);
            errorParams.insert("stage", "check busy");
            errorParams.insert("description", QString("CAMAC is busy now"));

            emit error(errorParams);
            return;
        }
        else
        {
#ifdef TEST_MODE
            qDebug() << tr("receive %1 message").arg(commandType);
#endif
            if(commandType == "NAF")
            {

                emit NAF(message);
            }
            else
                if(commandType == "init")
                {
                    emit init(message);
                }
                else
                    if(commandType == "acquire_point")
                    {
                        emit acquirePoint(message);
                    }
                    else
                        if(commandType == "reset_counters")
                        {
                            emit resetCounters(message);
                        }
                        else
                            if(commandType == "get_counters_value")
                            {
                                emit getCountersValue(message);
                            }
                            else
                            {
                                //создание описания ошибки
                                QVariantMap errorParams;
                                errorParams["error_code"] = QString("%1").arg(UNKNOWN_MESSAGE_ERROR);
                                errorParams.insert("stage", "process command_type");
                                errorParams.insert("description", QString("unknown command_type: %1").arg(commandType));

                                emit error(errorParams);
                                return;
                            }
        }
}
void CamacServer::processReply(QVariantMap message)
{
    //не предусмотрены действия на reply
    QVariantMap errorInfo;
    errorInfo["error_code"] = QString("%1").arg(UNKNOWN_MESSAGE_ERROR);
    errorInfo.insert("stage", "process reply");
    errorInfo.insert("description","no action prescribed for this reply");
    emit error(errorInfo);
}
