#include "DataVisualizerWindow.h"
#include "ui_DataVisualizerWindow.h"


DataVisualizerWindow::DataVisualizerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataVisualizerWindow)
{
    ui->setupUi(this);
    form = new DataVisualizerForm(1, settings, this);
    ui->dataVisualizerFrame->layout()->addWidget(form);

    connect(ui->clearButton, SIGNAL(clicked()), form, SLOT(clear()));
}

DataVisualizerWindow::~DataVisualizerWindow()
{
    delete ui;
}

void DataVisualizerWindow::on_openFolderButton_clicked()
{
    this->settings = new QSettings("DataVisualizerSettings.ini",
                                   QSettings::IniFormat);

    settings->beginGroup("DataVisualizer");
    QString dir = settings->value("last_opened_dir", QVariant(QString())).toString();
    dir = QFileDialog::getExistingDirectory(this, tr("Открыть папку с данными"), dir);

    if(!dir.isEmpty())
        settings->setValue("last_opened_dir", dir);
    settings->endGroup();

    form->openDir(dir);
}
