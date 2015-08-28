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

#include <vector>

namespace Ui {
class DataVisualizerForm;
}


/*!
 * \brief Тип графика.
 */
enum GraphMode
{
    ABSOLUTE_TIME = 0, ///< График абсолютного времени.
    RELATIVE_TIME = 1, ///< График относительного времени.
    HISTOGRAMM = 2 ///< Гистограмма.
};

/*!
 * \brief Базовый класс графика для DataVisualizerForm.
 * Одному визуализированному файлу соответствует один экземпляр класса.
 * \todo Извлечь побольше методов в этот класс, например добавить реализацию
 * FileDrawer::setMetaDataToTable
 */
class FileDrawer : public QObject
{
    Q_OBJECT
public:
    /*!
     * \param table Указатель к виджету таблицы метаданных.
     * \param plot Указатель к виджету графика.
     * \param filename Путь к файлу.
     */
    FileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~FileDrawer();

    /*!
     * \brief Проверка видимости графика.
     * \return Флаг визимости графика.
     */
    bool visible(){return isVisible;}

    /*!
     * \brief Получение текущего цвета графика.
     * \return Текущий цвет графика.
     */
    QColor getColor(){return color;}

signals:
    /*!
     * \brief Испускается при успешном обновлении графика методом FileDrawer::update.
     */
    void updated();

public slots:
    ///\brief Вывксти метаданные в таблицу.
    virtual void setMetaDataToTable() = 0;
    /*!
     * \brief Показать/Спрятать график.
     * \param visible Флаг видимости.
     * \param graphMode Тип представления графика.
     */
    virtual void setVisible(bool visible, GraphMode graphMode) = 0;

    /*!
     * \brief Установить цвет графика.
     * \param color Цвет графика.
     */
    virtual void setColor(QColor color) = 0;

    ///\brief Обновить данные графика.
    virtual void update() = 0;

protected:

    /*!
     * \brief Создать график гистограммы по данным.
     * \param data Вектор данных.
     * \param plot Указатель на график, где будет нарисована гистограмма.
     * \param minVal Минимальное значение данных.
     * \param maxVal Максимальное значение данных.
     * \param bins Количество бинов.
     * \param binMax Максимальное количество элементов в бине.
     * \return График гистограммы.
     * \todo Добавить проверку значений.
     */
    template<typename T>
    QCPBars* createHist(QVector<T> data, QCustomPlot *plot,double minVal = 0, double maxVal = 4096, int bins = 128,
                        int *binMax = 0);

    /*!
     * \brief Получить произвольный цвет.
     * \return Цвет.
     */
    static QColor getRandomColor();

    ///\brief Буфер для данных файла.
    QByteArray fileBuffer;

    ///\brief Последнее время изменения файла.
    QDateTime fileLastModified;

    ///\brief Метаданные файла.
    QVariantMap meta;

    ///\brief Бинарные данные файла.
    QByteArray data;

    ///\brief Флаг видимости графика.
    bool isVisible;

    ///\brief Текущий цвет графика.
    QColor color;

    ///\brief Указатель на виджет таблицы метаданных.
    QTableWidget *table;

    ///\brief Указатель на виджет графика.
    QCustomPlot *plot;

    ///\brief Указатель на файл.
    QFile *file;
protected slots:

    /*!
     * \brief Устанавливает значение ячейки в таблице.
     * \param col Номер колонки.
     * \param row Номер строки.
     * \param text Текст ячейки.
     * \warning метод не проверяет границы таблицы.
     */
    void setMetaTableText(int col, int row, QString text);
};


///
/// \brief Тип выводимой гистограммы для APDFileDrawer
///
enum APD_HIST_TYPE
{
    AMPLITUDE = 0,
    INTERVAL = 1
};

/// \brief Класс для визуализации APD файлов.
/// \details Оптимизирован под вывод больших данных.
/// \todo Добавить обработку метаданных.
/// \todo Сделать корректное время.
/// \todo Добавить описание.
/// \todo Ускорить поиск границ.
class APDFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~APDFileDrawer();

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color);
    virtual void update();

private slots:
    /*!
     * \brief Рисует видимую часть графика. Также подчитывает
     * текущее количество событий и выводит их в таблицу метаданных.
     * Присоединен к сигналу QCPAxis::rangeChanged
     */
    void drawPart(QCPRange range);
    void changeHistType();

private:
    /// \brief Флаг загрузки файла.
    bool loaded = 0;

    /// \brief Указатель на график.
    QCPGraph *graph;

    /// \brief Вспомогательный график для APDFileDrawer::graph.
    /// Содержит границы данных и имеет прозрачный цвет. Используется для
    /// автомасштабирования.
    QCPGraph *graphBorder;

    /// \brief Указатель на гистограмму по амплитуде.
    QCPBars *amplHist;

    /// \brief Указатель на гистограмму по интервалу.
    QCPBars *intervalHist;


    ///\brief Вектор времени
    std::vector<int> time;
    ///\brief Вектор времени
    QVector<int> val;
    ///\brief Вектор интервалов между событиями
    QVector<int> interval;
    ///\brief Вектор ширины
    QVector<int> width;

    APD_HIST_TYPE histType;
    QWidget *histSetWidget;
    QRadioButton *histSetButtons[2];
};

/*!
 * \brief Класс для визуализации файла метаданных.
 */
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
    /*!
     * \brief Указатель на графичесикие элементы.
     */
    QVector<QCPAbstractItem*> items;
};

/*!
 * \brief Класс для визуализации файлов с точками.
 */
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
    ///\todo Добавить пересчет через коэфициент пересчета из метаданных.
    virtual void update();

private:
    ///\brief Файл загружен.
    bool loaded;
    ///\brief Графики с абсолютным временем.
    QVector<QCPGraph*> graph_absolute;
    /// \brief Графики с относительным временен.
    QVector<QCPGraph*> graph_relative;
    /// \brief Графики гистограмм.
    QVector<QCPBars*> bars;

    /*!
     * \brief Доступные времена набора.
     * Используются для пересчета времен.
     */
    static QMap<int, unsigned short> aviableMeasureTimes;
};

/*!
 * \brief Класс для визуализации файла с напряжением.
 */
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
    ///\brief Метаданные файла.
    QVariantMap meta;
    ///\brief Манинный заголовок файла.
    MachineHeader header;
    ///\brief Сырой машинный заголовок.
    QByteArray rawMachineHeader;

    /*!
     * \brief График напряжения первого блока (Пунктирный).
     */
    QCPGraph* graph_block_1;

    /*!
     * \brief График напряжения втогого блока (Сплошной).
     */
    QCPGraph* graph_block_2;
};

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

    Ui::DataVisualizerForm *ui;
};

#endif // DATAVISUALIZERFORM_H
