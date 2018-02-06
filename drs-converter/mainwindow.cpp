#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTemporaryDir>
#include <QTimer>
#include <QDir>
#include <QXmlStreamReader>
#include <tcpprotocol.h>
#include <QVector>
#include <QDateTime>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    folder = QFileDialog::getExistingDirectory(
                this, tr("Выберите папку с данными"),
                settings->value("last_opened", QString()).toString()
                );
    folder = settings->value("last_opened", QString()).toString();
    settings->setValue("last_opened", folder);
    checkIntervalSec = settings->value("check_sec", QVariant(5)).toInt();
    settings->setValue("check_sec", checkIntervalSec);
    if(!QFile::exists(settings->value("viewer_filepath").toString())) {
        QString viewerExecutable = QFileDialog::getOpenFileName(
            this, tr("Выберите бинарный файл вьювера"));
        settings->setValue("viewer_filepath", viewerExecutable);
    }
    viewerExecutable = settings->value("viewer_filepath").toString();

    dir = new QTemporaryDir();
    dir->setAutoRemove(true);

    thread = new QProcess(this);
    thread->start(tr("%1 -d %2").arg(viewerExecutable, dir->path()));
    connect(thread, SIGNAL(finished(int)), this, SLOT(onViewerClosed()));

    QTimer::singleShot(0, this, SLOT(checkFolder()));
}

MainWindow::~MainWindow()
{
    delete dir;
    delete ui;
}

void MainWindow::checkFolder() {
    foreach (QString file, QDir(folder).entryList(QStringList("*.xml"))) {
        if(!QFile::exists(dir->filePath(file))) {
            convertFile(file);
        }

    }
    QTimer::singleShot(
                checkIntervalSec * 1000, this,
                SLOT(checkFolder()));
}

void MainWindow::onViewerClosed() {
    qApp->exit();
}

void MainWindow::convertFile(QString file){

     QFile fileObj(QDir(folder).filePath(file));
     fileObj.open(QIODevice::ReadOnly);

     QXmlStreamReader xml(&fileObj);

     bool timeWritten = false;
     QDateTime started;

     QDateTime currEventTimestamp;
     QVector<Event> events;
     double ampl = -1000;

     while(!xml.atEnd()) {
         xml.readNext();
         if (xml.name() == "Event") {
             if (xml.isEndElement()) {
                 events.append(
                             Event(quint16(ampl * 10),
                                   started.msecsTo(currEventTimestamp),
                                   true));
                 ampl = -1000;
             }
         } else if (xml.name() == "Time") {
             if (!xml.isEndElement()) {
                 QDateTime timestamp = QDateTime::fromString(
                             xml.readElementText(), tr("yyyy/MM/dd HH:mm:ss.zzz"));
                 if(!timeWritten) {
                     started = timestamp;
                     timeWritten = true;
                 }
                 currEventTimestamp = timestamp;
             }
         } else if (xml.name() == "Data") {
             if (!xml.isEndElement()) {
                 double value = xml.readElementText().split(",")[1].toDouble();
                 if (ampl < value) {
                     ampl = value;
                 }
             }
         }
     }

     QVariantMap meta;
     meta["time_coeff"] = 1e6;
     meta["acquisition_time"] = int(started.msecsTo(currEventTimestamp) / 1000);
     meta["type"] = "reply";
     meta["reply_type"] = "aquired_point";
     meta["status"] = "ok";
     meta["start_time"] = started.toString(Qt::ISODate);
     meta["end_time"] = currEventTimestamp.toString(Qt::ISODate);
     meta["total_events"] = events.length();

     QFile outFile(dir->filePath(file));
     outFile.open(QIODevice::WriteOnly);
     outFile.write(
                 TcpProtocol::createMessageWithPoints(
                     meta, events, JSON_METATYPE,
                     POINT_DIRECT_BINARY));
     outFile.close();
     fileObj.close();
}
