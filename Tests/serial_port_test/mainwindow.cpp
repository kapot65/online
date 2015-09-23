#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QSerialPortInfo> com_ports = QSerialPortInfo::availablePorts();

    for(int i = 0; i < com_ports.size(); i++)
    {
        qDebug() << "Name        : " << com_ports[i].portName();
        qDebug() << "Description : " << com_ports[i].description();
        qDebug() << "Manufacturer: " << com_ports[i].manufacturer();
        qDebug() << "Is NULL     : " << com_ports[i].isNull();

        ui->textBrowser->insertPlainText(QString(
                                         "Name        :%1\n"
                                         "Description :%2\n"
                                         "Manufacturer:%3\n"
                                         "Is NULL     :%4\n")
                                         .arg(com_ports[i].portName())
                                         .arg(com_ports[i].description())
                                         .arg(com_ports[i].manufacturer())
                                         .arg(com_ports[i].isNull()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
