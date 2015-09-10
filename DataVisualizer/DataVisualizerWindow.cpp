#include "DataVisualizerWindow.h"
#include "ui_DataVisualizerWindow.h"


DataVisualizerWindow::DataVisualizerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataVisualizerWindow)
{
    this->settings = new QSettings(tr("%1.ini").arg(metaObject()->className()),
                                   QSettings::IniFormat);

    settings->beginGroup("DataVisualizerForm");
        if(!settings->contains("hide_abs_time"))
            settings->setValue("hide_abs_time", true);
    settings->endGroup();

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
    settings->beginGroup("DataVisualizer");
    QString dir = settings->value("last_opened_dir", QVariant(QString())).toString();
    dir = QFileDialog::getExistingDirectory(this, tr("Открыть папку с данными"), dir);

    if(!dir.isEmpty())
        settings->setValue("last_opened_dir", dir);
    settings->endGroup();

    form->openDir(dir);
}
