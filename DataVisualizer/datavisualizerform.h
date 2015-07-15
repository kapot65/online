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

#include <QDebug>
#include <QDataStream>
#include <QJsonDocument>
#include <QColorDialog>

namespace Ui {
class DataVisualizerForm;
}

enum GraphMode
{
    ABSOLUTE_TIME = 0,
    RELATIVE_TIME = 1,
    HISTOGRAMM = 2
};

class FileDrawer : public QObject
{
    Q_OBJECT
public:
    FileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~FileDrawer();
    bool visible(){return isVisible;}
    QColor getColor(){return color;}

signals:
    void updated();

public slots:
    virtual void setMetaDataToTable() = 0;
    virtual void setVisible(bool visible, GraphMode graphMode) = 0;
    virtual void setColor(QColor) = 0;
    virtual void update() = 0;

protected:
    static QColor getRandomColor();

    QByteArray fileBuffer;
    QDateTime fileLastModified;
    QVariantMap meta;
    QByteArray data;

    bool isVisible;
    QColor color;

    QTableWidget *table;
    QCustomPlot *plot;
    QFile *file;
protected slots:
    void setMetaTableText(int col, int row, QString text);
};

class InfoFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    InfoFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~InfoFileDrawer();

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color){this->color = color;}
    virtual void update();

private:
    QVector<QCPAbstractItem*> items;
};

class PointFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    PointFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~PointFileDrawer();

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color);
    virtual void update();

private:
    bool loaded;
    QVector<QCPGraph*> graph_absolute;
    QVector<QCPGraph*> graph_relative;
    QVector<QCPBars*> bars;

    static QMap<int, unsigned short> aviableMeasureTimes;
};

class VoltageFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    VoltageFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~VoltageFileDrawer();

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color);
    virtual void update();

private:
    //члены для метода update
    QVariantMap meta;
    MachineHeader header;
    QByteArray rawMachineHeader;

    QCPGraph* graph_block_1;
    QCPGraph* graph_block_2;
};

class CustomItemDelegate : public QStyledItemDelegate
{
public:
    CustomItemDelegate(QFileSystemModel *model,
                       QMap<QString, FileDrawer*> *opened_files,
                       QObject *parent = 0);

    virtual void paint(QPainter * painter,
                       const QStyleOptionViewItem & option,
                       const QModelIndex & index) const;
private:
    QFileSystemModel *model;
    QMap<QString, FileDrawer*> *opened_files;
};

class DataVisualizerForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataVisualizerForm(bool interactive = 0, QSettings *settings = 0,
                                QWidget *parent = 0);
    ~DataVisualizerForm();

private slots:
    void updateTimeAxis(QCPRange range);
    void on_colorEditButton_clicked();
    void change_mode();

    void on_fileBrowser_doubleClicked(const QModelIndex &index);

    void on_fileBrowser_clicked(const QModelIndex &index);

public slots:
    void openDir(QString dir);
    /*!
     * \brief visualizeFile
     * \details Загружает файл и показывает его.
     * Если файл уже был загружен, то меняет его видимость.
     * \param filepath
     */
    void visualizeFile(QString filepath);

    /*!
     * \brief удаляет все графики и закрывает файлы.
     */
    void clear();

private:
    bool interactive;

    GraphMode getCurrentGraphMode();
    QCPAxis *timeAxis;

    Ui::DataVisualizerForm *ui;
    QSettings *settings;

    QCustomPlot *plot;

    QMap<QString, FileDrawer*> opened_files;
    QFileSystemModel *model;
};

#endif // DATAVISUALIZERFORM_H
