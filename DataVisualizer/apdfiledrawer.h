#ifndef APDFILEDRAWER_H
#define APDFILEDRAWER_H

#include <QObject>
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

/*!
 * \brief Структура для хранения гистограмм APDFileDrawer
 */
struct APDHist
{
    APDHist():hist(0){}

    /// \brief Указатель на гистограмму.
    QCPBars *hist;

    /// \brief Данные гистограммы APDHist::hist
    /// Первый член - количество событий в бине,
    /// второй - координата середины бина.
    QPair<QVector<double>, QVector<double> > histValues;
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
    APDFileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0);
    ~APDFileDrawer();

public slots:
    virtual void setMetaDataToTable();
    virtual void setVisible(bool visible, GraphMode graphMode);
    virtual void setColor(QColor color);
    /// \todo добавить обработку обнуленных счетчиков
    virtual void update();

protected:
    /*!
     * \brief Получить индексы элементов, ограничивающих минимальное и максимальные значения.
     * \param vector Вектор элементов.
     * \param min Минимальное значение элемента.
     * \param max Максимальное значение элемента
     * \param [out] minInd Найденый элемент, соотвествующий \p min.
     * \param [out] maxInd Найденый элемент, соотвествующий \p max.
     */
    template<typename T>
    void getMinMaxInd(T &vector, double min, double max, quint64 &minInd, quint64 &maxInd);

    /*!
     * \brief Вычисляет количество событий в окне гистограммы и посылает сообщение
     * о них.
     * \param range Интервал окна.
     * \param apdHist Гистограмма.ы
     */
    void sendHistEventsInWindow(QCPRange range, APDHist &apdHist);

private slots:
    /*!
     * \brief Рисует видимую часть графика. Также подчитывает
     * текущее количество событий и выводит их в таблицу метаданных.
     * Присоединен к сигналу QCPAxis::rangeChanged
     */
    void drawPart(QCPRange range);

    /*!
     * \brief Сохранить гистограмму.
     * Гистограмма сохраняется рядом с исходным файлом.
     */
    void saveHist();

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
    APDHist amplHist;

    /// \brief Гистограмма по интервалу
    APDHist intervalHist;

    ///\brief Вектор времени
    std::vector<quint64> time;
    ///\brief Вектор амплитуды
    QVector<int> val;
    ///\brief Вектор интервалов между событиями
    QVector<int> interval;
    ///\brief Вектор ширины
    QVector<quint64> width;

    static APD_HIST_TYPE histType;
    static APD_GRAPH_TYPE graphType;

    QWidget *histSetWidget;
    QRadioButton *histSetButtons[2];

    QWidget *graphSetWidget;
    QRadioButton *graphSetButtons[2];
};
#endif //APDFILEDRAWER_H
