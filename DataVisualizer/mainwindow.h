#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <datavisualizerform.h>

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
    void on_openFolderButton_clicked();

private:
    QSettings *settings;
    DataVisualizerForm *form;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
