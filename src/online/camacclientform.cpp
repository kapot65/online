#include "camacclientform.h"
#include "ui_camacclientform.h"
#include <QProcess>
#include <QFileDialog>
#include <QCloseEvent>
#include <QtWidgets>

void CamacClientForm::restoreSettings()
{
    manager = new IniManager(tr("%1Settings.ini").arg(BIN_NAME), this);

    if(!manager->settings->value("advanced_mode").isValid())
        manager->settings->setValue("advanced_mode", true);
    if(!manager->settings->value("voltage_only").isValid())
        manager->settings->setValue("voltage_only", true);
}

void CamacClientForm::setCCPC7Handler()
{
    ccpc7Handler = new CCPC7Handler(manager, this);
    ccpc7Handler->start();

    connect(ccpc7Handler, SIGNAL(ready()), this, SLOT(camacMarkReady()));
    connect(ccpc7Handler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(camacMarkError()));
#ifdef TEST_MODE
    connect(ccpc7Handler, SIGNAL(testReseivedMessage(QByteArray)), this, SLOT(showTextOutput(QByteArray)));
#endif
}

void CamacClientForm::setCCPC7HandlerForm()
{
    ccpc7HandlerForm =  new CCPC7HandlerForm(ccpc7Handler, manager, this);
    ui->tabWidget->widget(1)->layout()->addWidget(ccpc7HandlerForm);

    connect(ccpc7HandlerForm, SIGNAL(sendWarning(QString)), this, SLOT(camacMarkWarning()));
}

void CamacClientForm::setHVHandler()
{
    hvHandler = new HVHandler(manager, this);
    hvHandler->start();

    connect(hvHandler, SIGNAL(ready()), this, SLOT(HVMarkReady()), Qt::DirectConnection);
    connect(hvHandler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(HVMarkError()), Qt::DirectConnection);
#ifdef TEST_MODE
    connect(hvHandler, SIGNAL(testReseivedMessage(QByteArray)), this, SLOT(showTextOutput(QByteArray)));
#endif
}

void CamacClientForm::setHVHandlerForm()
{
    hvHandlerForm = new HVHandlerForm(hvHandler, manager, this);

    ui->tabWidget->widget(2)->layout()->addWidget(hvHandlerForm);
    //ui->tabWidget->tabBar()->setTabTextColor(2,Qt::red);

    connect(hvHandlerForm, SIGNAL(sendWarning(QString)), this, SLOT(HVMarkWarning()));
}

CamacClientForm::CamacClientForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CamacClientForm)
{
    ui->setupUi(this);

    //настройка размеров виджета
    QSize size = this->size();
    QList<int> sizes;
    sizes.push_back(size.height());
    sizes.push_back(0);
    ui->splitter->setSizes(sizes);

    //считывание настроек из ini файла
    restoreSettings();

    //настройка обработчика Камак
    setCCPC7Handler();
    setCCPC7HandlerForm();

    //настройка обработчика HV
    setHVHandler();
    setHVHandlerForm();

    if(!manager->settings->value("log_size").isValid())
        manager->settings->setValue("log_size", 10000);

    logOutputMaxSize = manager->settings->value("log_size").toInt();

    //настройка окна онлайн
    online = new Online(manager, ccpc7Handler, hvHandler, this);
    onlineForm = new OnlineForm(ccpc7Handler, hvHandler, online, manager, this);
    ui->tabWidget->widget(0)->layout()->addWidget(onlineForm);

    if(!manager->settings->value("voltage_only").toBool()) {
        onlineForm->setEnabled(false);
        ccpc7HandlerForm->setEnabled(false);
    } else if(!manager->settings->value("advanced_mode").toBool()) {
        ccpc7HandlerForm->setEnabled(false);
        hvHandlerForm->setEnabled(false);
    }

    //настройка главного окошка
    //connect(this, SIGNAL(sendTestCamacMessage(QByteArray)), ccpc7HandlerForm, SLOT(receiveTestJson(QByteArray)));
}

CamacClientForm::~CamacClientForm()
{
    manager->deleteLater();

    delete ui;
}

void CamacClientForm::closeEvent(QCloseEvent *e)
{
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, tr("Confirm"),
                                  tr("Exit programm?"));

#ifdef TEST_MODE
    qDebug()<<button;
#endif
    if(button == QMessageBox::No)
        e->ignore();
    else
        close();
}


#ifdef TEST_MODE
void CamacClientForm::showTextOutput(QByteArray output)
{

    if(ui->output->toPlainText().size() >= logOutputMaxSize)
        ui->output->clear();

    ui->output->insertPlainText(QString("%1\n%2\n")
                                .arg(QTime::currentTime().toString())
                                .arg(TcpProtocol::toDebug(output)));
}
#endif

void CamacClientForm::camacMarkError()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(1,Qt::darkRed);
#endif
}

void CamacClientForm::camacMarkWarning()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(1,Qt::darkYellow);
#endif
}

void CamacClientForm::camacMarkReady()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(1,Qt::darkGreen);
#endif
}

void CamacClientForm::HVMarkError()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(2,Qt::darkRed);
#endif
}

void CamacClientForm::HVMarkWarning()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(2,Qt::darkYellow);
#endif
}

void CamacClientForm::HVMarkReady()
{
#if QT_VERSION >= 0x050300
    ui->tabWidget->tabBar()->setTabTextColor(2,Qt::darkGreen);
#endif
}

void CamacClientForm::on_clearLogButton_clicked()
{
    ui->output->clear();
}

void CamacClientForm::on_openViewerButton_clicked()
{
    if(!manager->getSettingsValue(metaObject()->className(), "dataViewerPath").isValid())
    {
        QString dataViewerPath = QFileDialog::getOpenFileName(this,
                                                              tr("Укажите путь до DataViewer"));

        manager->setSettingsValue(metaObject()->className(),
                                  "dataViewerPath", dataViewerPath);
    }

    QString path = manager->getSettingsValue(metaObject()->className(),
                                             "dataViewerPath").toString();

    QProcess::startDetached(path, QStringList(tr("--directory=temp/%1").arg(online->getCurrSubFolder())));
}
