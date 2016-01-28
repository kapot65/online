#include "hvmonitor.h"
#include <QTimer>

HVMonitor::HVMonitor(QString subFolder, HVHandler *hvHandler):QThread(0)
{
    this->subFolder = subFolder;
    this->hvHandler = hvHandler;

    connect(this, SIGNAL(finished()), this, SLOT(beforeClose()));
}

void HVMonitor::prepareVoltageFile(BINARYTYPE type)
{
    if(type != HV_BINARY && type != HV_TEXT_BINARY)
        type = HV_BINARY;

    this->binaryType = type;

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

    int block = message["block"].toInt();

    if(block != 1 && block != 2)
        return;

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
    disconnect(hvHandler, SIGNAL(getVoltageDone(QVariantMap)),
            this, SLOT(saveCurrentVoltage(QVariantMap)));

    closeVoltageFile();
}

void HVMonitor::run()
{
    prepareVoltageFile();

    connect(hvHandler, SIGNAL(getVoltageDone(QVariantMap)),
            this, SLOT(saveCurrentVoltage(QVariantMap)), Qt::QueuedConnection);

    QEventLoop el;
    connect(this, SIGNAL(stop()), &el, SLOT(quit()));
    el.exec();
}


