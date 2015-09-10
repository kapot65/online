#ifndef DATAVISUALIZERFORM_H
#define DATAVISUALIZERFORM_H

#include <QWidget>
#include <QFileDialog>
#include <QSettings>
#include "customplotzoom.h"

#include "tcpprotocol.h"
#include "event.h"

#include <QDir>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QPair>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include <QColorDialog>
#include <QRadioButton>
#include <QStyledItemDelegate>
#include <vector>

#include <filedrawer.h>
#include <apdfiledrawer.h>
#include <infofiledrawer.h>
#include <pointfiledrawer.h>
#include <voltagefiledrawer.h>

namespace Ui {
class DataVisualizerForm;
}

/*!
 * \brief Делегат для таблицы с файлами.
 * Работает с объектами типа FileDrawer, ячейки, указывающие на них в
 * разные цвета. Белый - файл не открыт, Серый - Файл загружен и открыт.
 */
class CustomItemDelegate : public QStyledItemDelegate
{
public:
    /*!
     * \param model Указатель на модель таблицы.
     * \param opened_files Указатель на карту открытых файлов.
     */
    CustomItemDelegate(QFileSystemModel *model,
                       QMap<QString, FileDrawer*> *opened_files,
                       QObject *parent = 0);

    virtual void paint(QPainter * painter,
                       const QStyleOptionViewItem & option,
                       const QModelIndex & index) const;
private:
    ///\brief Указатель на модель таблицы.
    QFileSystemModel *model;

    ///\brief Указатель на карту открытых файлов.
    QMap<QString, FileDrawer*> *opened_files;
};

///
/// \brief Линейка для класса DataVisualizerForm
/// \todo Добавить описание.
class Ruler : public QObject
{
    Q_OBJECT
public:
    Ruler(QCustomPlot *plot, QObject *parent = 0);

private slots:

    void processMouseMove(QMouseEvent *ev);
    void clearRuler();
    void processMouseRelease(QMouseEvent *ev);

private:
    double y;

    double x1;
    double x2;

    QCPGraph *graph;

    QCPItemText *text;
    QCustomPlot *plot;

    bool mouseJustMovedFlag;
};


/*!
 * \brief Виджет для визуализации набранных данных.
 * Виджет позволяет открывать файлы с данными и визуализировать их
 * с помощью QCustomPlot.
 * \todo Добавить возможность визуализации файлов сценария.
 */
class DataVisualizerForm : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \param interactive Интерактивный график. Если это значение true, класс
     * будет автоматически обновлять открытые графики с интервалом 5 секунд.
     * \param settings Указатель на менеджер настроек.
     */
    explicit DataVisualizerForm(bool interactive = 0, QSettings *settings = 0,
                                QWidget *parent = 0);
    ~DataVisualizerForm();

private slots:
    /*!
     * \brief Обновить ось времени.
     * Слот пересчитывает абслоютное время из данных в абсолютное время в секундах, или
     * относительное время в секундах и обновляет ось времени.
     */
    void updateTimeAxis(QCPRange range);

    void on_colorEditButton_clicked();

    void change_mode();

    void on_fileBrowser_doubleClicked(const QModelIndex &index);

    void on_fileBrowser_clicked(const QModelIndex &index);


public slots:
    void openDir(QString dir);

    /*!
     * \brief Загрузка и визуализация файла.
     * Если файл уже был загружен, то меняет его видимость.
     * \param filepath Путь к файлу.
     */
    void visualizeFile(QString filepath);

    /*!
     * \brief Удаление всех построенных графиков.
     */
    void clear();

private:
    /*!
     * \brief Подгон размера дерева файлов под содержимое.
     * Взято из http://www.qtcentre.org/threads/2841-Resize-QTreeWidget-columns-to-contents.
     * \param treeWidget_ Дерево файлов.
     */
    void resizeColumnsToContents(QTreeWidget *treeWidget_ );

    ///\brief Флаг интерактивности.
    bool interactive;

    ///\brief Получить текущее представление графиков, указанное на форме.
    GraphMode getCurrentGraphMode();

    /// \brief Ось времени.
    QCPAxis *timeAxis;

    /// \brief Указатель на менеджер памяти.
    QSettings *settings;

    /// \brief Указатель на виджет для графики.
    QCustomPlot *plot;

    /// \brief Карта открытых файлов.
    QMap<QString, FileDrawer*> opened_files;

    /// \brief Модель таблицы файлов.
    QFileSystemModel *model;

    /// \brief Линейка.
    Ruler *ruler;

    Ui::DataVisualizerForm *ui;
};

#endif // DATAVISUALIZERFORM_H
