#include "ccpc7handlerform.h"
#include "ui_ccpc7handlerform.h"
#include <QComboBox>

CCPC7HandlerForm::CCPC7HandlerForm(CCPC7Handler *ccpc7Handler, DataVisualizerForm *dataVisualizerForm, IniManager *settingsManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CCPC7HandlerForm)
{
    ui->setupUi(this);

    ui->acquisitionTimeBox->addItem("5", 5);
    ui->acquisitionTimeBox->addItem("10", 10);
    ui->acquisitionTimeBox->addItem("15", 15);
    ui->acquisitionTimeBox->addItem("20", 20);
    ui->acquisitionTimeBox->addItem("50", 50);
    ui->acquisitionTimeBox->addItem("100", 100);
    ui->acquisitionTimeBox->addItem("200", 200);

    this->settingsManager = settingsManager;
    this->ccpc7Handler = ccpc7Handler;
    this->dataVisualizerForm = dataVisualizerForm;

    haveWarning = 0;

    //установка ui
    ui->camacIpEdit->setText(settingsManager->getSettingsValue("CCPC7Handler", "ip").toString());
    ui->camacPortEdit->setValue(settingsManager->getSettingsValue("CCPC7Handler", "port").toInt());

    if(settingsManager->getSettingsValue("CCPC7Handler", "countersFilePath").isValid())
        ui->countersFileEdit->setText(settingsManager->getSettingsValue("CCPC7Handler", "countersFilePath").toString());

    if(settingsManager->getSettingsValue("CCPC7Handler", "countersReloadTime").isValid())
        ui->reloadCountersBox->setValue(settingsManager->getSettingsValue("CCPC7Handler", "countersReloadTime").toInt());

    connect(this, SIGNAL(serverSettingsChanged()), this, SLOT(on_serverSettingsChanged()));
    connect(ui->initButton, SIGNAL(clicked()), this->ccpc7Handler, SLOT(initServer()));
    connect(ui->breakAcquisitionButton, SIGNAL(clicked()), this->ccpc7Handler, SLOT(breakAcquisition()));
    connect(ui->resetCountersButton, SIGNAL(clicked()), this->ccpc7Handler, SLOT(resetCounters()));
}

CCPC7HandlerForm::~CCPC7HandlerForm()
{
    delete ui;
}

void CCPC7HandlerForm::on_camacIpEdit_editingFinished()
{
    settingsManager->setSettingsValue("CCPC7Handler", "ip", ui->camacIpEdit->text());
    emit serverSettingsChanged();
}

void CCPC7HandlerForm::on_camacPortEdit_editingFinished()
{
    settingsManager->setSettingsValue("CCPC7Handler", "port", ui->camacPortEdit->value());
    emit serverSettingsChanged();
}

void CCPC7HandlerForm::on_serverSettingsChanged()
{
    ui->ccpcStatusLabel->setText("Settings have been changed. Please reconnect device");
    LOG(WARNING) << "Settings have been changed.";
    haveWarning = 1;
    emit sendWarning("Settings have been changed.");
}

void CCPC7HandlerForm::on_countersFileEdit_editingFinished()
{
    settingsManager->setSettingsValue("CCPC7Handler", "countersFilePath", ui->countersFileEdit->text());
}

void CCPC7HandlerForm::on_reloadCountersBox_editingFinished()
{
    settingsManager->setSettingsValue("CCPC7Handler", "countersReloadTime", ui->reloadCountersBox->value());
}

void CCPC7HandlerForm::on_countersFileSelect_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    ui->countersFileEdit->text());
    ui->countersFileEdit->setText(fileName);
    on_countersFileEdit_editingFinished();
}

void CCPC7HandlerForm::on_acquirePointButton_clicked()
{
    connect(ccpc7Handler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>)),
            this, SLOT(drawAcquiredPoint(MachineHeader,QVariantMap,QVector<Event>)));

#if QT_VERSION >= 0x050300
    ccpc7Handler->acquirePoint(ui->acquisitionTimeBox->currentData().toInt());
#else
    ccpc7Handler->acquirePoint(ui->acquisitionTimeBox->currentText().toInt());
#endif
}

void CCPC7HandlerForm::on_reconnectButton_clicked()
{
    ccpc7Handler->reconnect(ui->camacIpEdit->text(),
                            ui->camacPortEdit->value());
    haveWarning = 0;
}

void CCPC7HandlerForm::on_monitorReceiveCountersValue(QVariantMap message)
{
    int counter = message.value("counter").toInt();
    QTime time = message.value("time").toTime();
    QList<QVariant> channels_id = message.value("channels_id").toList();
    QList<QVariant> channels_value = message.value("channels_value").toList();

    for(int i = 0 ; i < channels_id.size(); i++)
    {
         countersFile->write(QString("%1\t%2\t%3\t%4\n").arg(time.toString())
                                       .arg(counter)
                                       .arg(channels_id[i].toInt())
                                       .arg(channels_value[i].toInt()).toLatin1());
         countersFile->flush();
    }
}

//void CCPC7HandlerForm::drawAcquiredPoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> events)
//{
//    disconnect(ccpc7Handler, SIGNAL(pointAcquired(MachineHeader,QVariantMap,QVector<Event>)),
//            this, SLOT(drawAcquiredPoint(MachineHeader,QVariantMap,QVector<Event>)));

//    GraphUnit* newGraph = graphViewer->createGraph(QString("Point %1").arg(QTime::currentTime().toString()),
//                                                   GRAPH); // поменять GRAPH на HIST

//    //копирование метаинформации в описание графика
//#ifdef USE_QTJSON
//    newGraph->addDescription(QJsonDocument::fromVariant(meta).toJson());
//#else
//    newGraph->addDescription(QJson::Serializer().serialize(meta));
//#endif

//    //извлечение точек из events
//    QVector<QPointF> points;
//    for(int  i = 0 ; i < events.size(); i++)
//        points.push_back(QPointF(events[i].time, events[i].data));

//    //добавление точек на график
//    newGraph->addPoints(points);
//    //пометить на возможность ручного удаления (больше график програмно не будет использоваться)
//    newGraph->markForManualDelete(true);
//}

void CCPC7HandlerForm::on_writeCountersButton_clicked(bool checked)
{
    static bool first_time_here = true;
    static QTimer timer;
    //подключение таймера
    if(first_time_here)
    {
        first_time_here = false;
        connect(&timer, SIGNAL(timeout()), this, SLOT(sendGetCountersValue()));
    }

    if(checked)
    {
        //пока без графика, т.к. не реализована возможность постройки больше чем одного графика
//        currGraphUnit = graphViewer->createGraph(QString("Counters_%1").arg(QTime::currentTime().toString()),
//                                                 GRAPH);
//        currGraphUnit->addDescription(QString("Counters graph.\n"
//                                              "  Start time: %1\n").arg(QTime::currentTime().toString()));
//        currPointNum = 0;

        //запись производится в файл
        if(QFile::exists(ui->countersFileEdit->text()))
        {
            if(QMessageBox::question(this, "Перезаписать файл?",
                                  QString("Файл %1 уже существует. Перезаписать его?").arg(ui->countersFileEdit->text()),
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::No)
            {
                ui->writeCountersButton->setChecked(0);
                return;
            }
        }

        //открытие файла
        countersFile = new QFile(ui->countersFileEdit->text(), this);
        countersFile->open(QIODevice::WriteOnly);

        //проверка открытия файла
        if(!QFile::exists(ui->countersFileEdit->text()))
        {
            ui->writeCountersButton->setChecked(0);
            ui->ccpcStatusLabel->setText("Output file cant be created. Please change filename.");
            return;
        }


        timer.start(ui->reloadCountersBox->value() * 1000);
        connect(ccpc7Handler, SIGNAL(counterAcquired(QVariantMap)),
                this, SLOT(on_monitorReceiveCountersValue(QVariantMap)));
    }
    else
    {
        countersFile->close();
        countersFile->deleteLater();

        disconnect(ccpc7Handler, SIGNAL(counterAcquired(QVariantMap)),
                   this, SLOT(on_monitorReceiveCountersValue(QVariantMap)));

        timer.stop();

        //заменить на canBeManuallyDeleted
        //currGraphUnit->markForManualDelete(true);
    }
}

void CCPC7HandlerForm::sendGetCountersValue()
{
    bool reset_after = 1;

    QList<QVariant> channels1Counter;

    if(ui->counter10->isChecked())
        channels1Counter.push_back("0");
    if(ui->counter11->isChecked())
        channels1Counter.push_back("1");
    if(ui->counter12->isChecked())
        channels1Counter.push_back("2");
    if(ui->counter13->isChecked())
        channels1Counter.push_back("3");

    if(!channels1Counter.isEmpty())
        ccpc7Handler->getCountersValue(1, channels1Counter, reset_after);

    QList<QVariant> channels2Counter;

    if(ui->counter20->isChecked())
        channels2Counter.push_back("0");
    if(ui->counter21->isChecked())
        channels2Counter.push_back("1");
    if(ui->counter22->isChecked())
        channels2Counter.push_back("2");
    if(ui->counter23->isChecked())
        channels2Counter.push_back("3");

    if(!channels1Counter.isEmpty())
    {
        QEventLoop el;
        connect(ccpc7Handler, SIGNAL(counterAcquired(QVariantMap)), &el, SLOT(quit()));
        el.exec();
    }

    if(!channels2Counter.isEmpty())
        ccpc7Handler->getCountersValue(2, channels2Counter, reset_after);
}
