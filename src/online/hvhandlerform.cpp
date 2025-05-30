#include "hvhandlerform.h"
#include "ui_hvhandlerform.h"

HVHandlerForm::HVHandlerForm(HVHandler *hvHandler, IniManager *settingsManager,
                             QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HVHandlerForm)
{
    ui->setupUi(this);

    this->hvHandler = hvHandler;

    haveWarning = 0;

    this->settingsManager = settingsManager;
    QString ip = settingsManager->getSettingsValue("HV_handler", "ip").toString();
    ui->ipEdit->setText(ip);
    int port = settingsManager->getSettingsValue("HV_handler", "port").toInt();
    ui->portEdit->setValue(port);

    ui->voltageBox->setValue(settingsManager->getSettingsValue("HV_handler", "last_voltage").toDouble());

    this->on_block1Button_clicked(ui->block1Button->isChecked());
    this->on_block2Button_clicked(ui->block2Button->isChecked());

    connect(this, SIGNAL(serverSettingsChanged()), this, SLOT(on_serverSettingsChanged()));
    connect(hvHandler, SIGNAL(getVoltageDone(QVariantMap)),
            this, SLOT(displayVoltage(QVariantMap)), Qt::QueuedConnection);
}

HVHandlerForm::~HVHandlerForm()
{
    delete ui;
}

void HVHandlerForm::on_ipEdit_editingFinished()
{
    if(ui->ipEdit->text() != settingsManager->getSettingsValue("HV_handler", "ip").toString()) {
        settingsManager->setSettingsValue("HV_handler", "ip", ui->ipEdit->text());
        emit serverSettingsChanged();
    }
}

void HVHandlerForm::on_portEdit_editingFinished()
{
    if(ui->portEdit->value() != settingsManager->getSettingsValue("HV_handler", "port").toInt()) {
        settingsManager->setSettingsValue("HV_handler", "port", ui->portEdit->value());
        emit serverSettingsChanged();
    }
}

void HVHandlerForm::on_reconnectButton_clicked()
{
    hvHandler->reconnect(ui->ipEdit->text(), ui->portEdit->value());
    haveWarning = 0;
}

void HVHandlerForm::on_serverSettingsChanged()
{
    ui->statusLabel->setText("Settings have been changed. Please reconnect device");
    LOG(WARNING) << "Settings have been changed.";
    haveWarning = 1;
    emit sendWarning("Settings have been changed.");
}

void HVHandlerForm::on_setHVButton_clicked()
{
    int block;
    if(ui->block1Button->isChecked())
        block = 1;
    else
        block = 2;

    hvHandler->setVoltage(block, ui->voltageBox->value());
}

void HVHandlerForm::on_voltageBox_editingFinished()
{
    settingsManager->setSettingsValue("HV_handler","last_voltage", ui->voltageBox->value());
}

void HVHandlerForm::on_block1Button_clicked(bool checked)
{
    if(checked)
    {
        ui->voltageBox->setMaximum(26000);
        ui->voltageBox->setMinimum(0);
    }
}


void HVHandlerForm::on_block2Button_clicked(bool checked)
{
    if(checked)
    {
        ui->voltageBox->setMaximum(900);
        ui->voltageBox->setMinimum(0);
    }
}

void HVHandlerForm::displayVoltage(QVariantMap meta)
{
    int block = meta["block"].toInt();
    double voltage = meta["voltage"].toDouble();

    switch (block) {
    case 1:
        ui->divider1VolatgeDisplay->display(voltage);
        break;
    case 2:
        ui->divider2VolatgeDisplay->display(voltage);
        break;
    default:
        break;
    }
}
