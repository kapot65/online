#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QTemporaryDir>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void checkFolder();
    void onViewerClosed();

private:
    void convertFile(QString file);
    Ui::MainWindow *ui;
    QSettings* settings;
    QTemporaryDir* dir;
    QString folder;
    int checkIntervalSec;
    QString viewerExecutable;
    QProcess *thread;
};

#endif // MAINWINDOW_H
