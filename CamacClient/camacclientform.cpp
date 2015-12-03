#include "camacclientform.h"
#include "ui_camacclientform.h"

void CamacClientForm::restoreSettings()
{
    manager = new IniManager("CamacClientSettings.ini", this);

    if(!manager->settings->value("advanced_mode").isValid())
        manager->settings->setValue("advanced_mode", false);
}

void CamacClientForm::setGraphWidget()
{
    dataVisualizerForm = new DataVisualizerForm(1, manager->settings, this);
    ui->graphFrame->layout()->addWidget(dataVisualizerForm);
}

void CamacClientForm::setCCPC7Handler()
{
    ccpc7Handler = new CCPC7Handler(QString(), -1, this);
    ccpc7Handler->start();

    connect(ccpc7Handler, SIGNAL(ready()), this, SLOT(camacMarkReady()));
    connect(ccpc7Handler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(camacMarkError()));
#ifdef TEST_MODE
    connect(ccpc7Handler, SIGNAL(testReseivedMessage(QByteArray)), this, SLOT(showTextOutput(QByteArray)));
#endif
}

void CamacClientForm::setCCPC7HandlerForm()
{
    ccpc7HandlerForm =  new CCPC7HandlerForm(ccpc7Handler, dataVisualizerForm, manager, this);
    ui->tabWidget->widget(1)->layout()->addWidget(ccpc7HandlerForm);

    connect(ccpc7HandlerForm, SIGNAL(sendWarning(QString)), this, SLOT(camacMarkWarning()));
}

void CamacClientForm::setHVHandler()
{
    hvHandler = new HVHandler(QString(), -1, this);
    hvHandler->start();

    connect(hvHandler, SIGNAL(ready()), this, SLOT(HVMarkReady()), Qt::DirectConnection);
    connect(hvHandler, SIGNAL(unhandledError(QVariantMap)), this, SLOT(HVMarkError()), Qt::DirectConnection);
#ifdef TEST_MODE
    connect(hvHandler, SIGNAL(testReseivedMessage(QByteArray)), this, SLOT(showTextOutput(QByteArray)));
#endif
}

void CamacClientForm::setHVHandlerForm()
{
    hvHandlerForm = new HVHandlerForm(hvHandler, manager, dataVisualizerForm, this);

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
    sizes.push_back(size.width() * 0.15);
    sizes.push_back(size.width() * 0.85);
    ui->splitter_2->setSizes(sizes);

    sizes.clear();
    sizes.push_back(size.height());
    sizes.push_back(0);
    ui->splitter->setSizes(sizes);

    //считывание настроек из ini файла
    restoreSettings();

    setGraphWidget();

    //настройка обработчика Камак
    setCCPC7Handler();
    setCCPC7HandlerForm();

    //настройка обработчика HV
    setHVHandler();
    setHVHandlerForm();

    if(!manager->settings->value("advanced_mode").toBool())
    {
        ccpc7HandlerForm->setEnabled(false);
        hvHandlerForm->setEnabled(false);
    }

    //настройка окна онлайн
    online = new Online(manager, ccpc7Handler, hvHandler, this);
    onlineForm = new OnlineForm(ccpc7Handler, hvHandler, dataVisualizerForm, online, manager, this);
    ui->tabWidget->widget(0)->layout()->addWidget(onlineForm);

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
    button = QMessageBox::question(this, tr("Подтверждение"),
                                  tr("Вы точно хотите выйти из программы?"));

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
