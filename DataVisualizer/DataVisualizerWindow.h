#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <datavisualizerform.h>

namespace Ui {
class DataVisualizerWindow;
}

/*!
 * \brief Главное окно программы DataVisualizer.
 * Содержит виджет DataVisualizerForm и кнопки взаиомдействия с ним.
 */
class DataVisualizerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataVisualizerWindow(QString openDirectory = QString(), QWidget *parent = 0);
    ~DataVisualizerWindow();

private slots:
    void on_openFolderButton_clicked();

private:
    QSettings *settings;
    DataVisualizerForm *form;
    Ui::DataVisualizerWindow *ui;
};

#endif // MAINWINDOW_H
