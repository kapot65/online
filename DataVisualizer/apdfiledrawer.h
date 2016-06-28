#ifndef APDFILEDRAWER_H
#define APDFILEDRAWER_H

#include <QObject>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>
#include "filedrawer.h"

/// \brief Тип выводимой гистограммы для APDFileDrawer
enum APD_HIST_TYPE
{
    AMPLITUDE = 0,
    INTERVAL = 1
};

/// \brief Тип выводимого графика для APDFileDrawer
enum APD_GRAPH_TYPE
{
    COUNT = 0,
    PLOT = 1
};

/// \brief Класс для визуализации APD файлов.
/// \details Оптимизирован под вывод больших данных.
/// \todo Добавить описание.
/// \todo Ускорить поиск границ.
/// \todo Добавить возможность сохранения.
/// \bug Вылетает при изменении типа гистограммы при более чем одном открытом файле.
class APDFileDrawer : public FileDrawer
{
    Q_OBJECT
public:
    APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QSettings *settings, QObject *parent = 0);
    ~APDFileDrawer();
    QVector<quint64> getTime() const;

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color);
    /// \todo добавить обработку обнуленных счетчиков
    virtual void update();

protected:


private slots:
    /*!
     * \brief Рисует видимую часть графика. Также подчитывает
     * текущее количество событий и выводит их в таблицу метаданных.
     * Присоединен к сигналу QCPAxis::rangeChanged
     */
    void drawPart(QCPRange range);

    void changeHistType();

    void changeGraphType();

    void setHistVisible(bool visible);

    /*!
     * \brief Создание кнопок выбора типа гистограммы.
     * \param col Колонка в таблице метаданных.
     * \param row Строка в таблице метаданных.
     */
    void createHistModeChangeButtons(int col, int row);

    /*!
     * \brief Создание кнопок выбора типа графика.
     * \param col Колонка в таблице метаданных.
     * \param row Строка в таблице метаданных.
     * \todo поменять col -> row
     */
    void createGraphModeChangeButtons(int col, int row);

    /*!
     * \brief Загрузка метаданных из файла.
     */
    void loadMetaData();

    /// \brief Обнуляет адресс APDFileDrawer::histSetWidget при его удалении.
    /// Присоединен к сигналу APDFileDrawer::histSetWidget::destroyed
    void fullDeleteHistTab();

    /// \brief Обнуляет адресс APDFileDrawer::graphSetWidget при его удалении.
    /// Присоединен к сигналу APDFileDrawer::graphSetWidget::destroyed
    void fullDeleteGraphTab();

    void setGraphVisible(bool visible);

private:
    bool getEventData(quint64 number, quint64 &time, short &ampl, uint &inter, short &w);

    QSettings *settings;

    /// \brief Флаг загрузки файла.
    bool loaded;

    /// \brief Указатель на график.
    QCPGraph *graph;

    /// \brief Указатель на двухплотный график.
    QCPGraph *graph2;

    /// \brief Вспомогательный график для APDFileDrawer::graph.
    /// Содержит границы данных и имеет прозрачный цвет. Используется для
    /// автомасштабирования.
    QCPGraph *graphBorder;

    /// \brief Гистограмма по амплитуде.
    QCPGraph *amplHist;

    /// \brief Гистограмма по интервалу
    QCPGraph *intervalHist;

    ///\brief Вектор времени начала пачек
    QVector<quint64> timeMap;

    static APD_HIST_TYPE histType;
    static APD_GRAPH_TYPE graphType;

    QWidget *histSetWidget;
    QRadioButton *histSetButtons[2];

    QWidget *graphSetWidget;
    QRadioButton *graphSetButtons[2];

    QFile *binFile;

    uint BATCH;
    uint MAX_EVENTS;
    uint HIST_FLUSH_STEP;
    uint HIST_MIN_VAL;
    uint HIST_MAX_VAL;
    uint HIST_MIN_INTER_VAL;
    uint HIST_MAX_INTER_VAL;

    quint64 redrawLastTime;

    // FileDrawer interface
public slots:
    void sendHistEventsInWindow(QCPRange range){}
};
#endif //APDFILEDRAWER_H
