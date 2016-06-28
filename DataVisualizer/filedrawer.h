#ifndef FILEDRAWER_H
#define FILEDRAWER_H

#include <QObject>
#include <QTableWidget>
#include "customplotzoom.h"
#include "tcpprotocol.h"
#include <QTreeWidget>

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
    FileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent = 0, bool readMeta = true);
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

    /*!
     * \brief Посылает текстовую информацию на форму.
     * \param Текст информации.
     */
    void sendTextInfo(QString sender, QString info);

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

    /*!
     * \brief Функция должна выдавать количество событий в заданном диапазоне для объектов, к которым это применимо.
     * Также функция должна сама проверять видимость и открытость обЪекта.
     * \param range диапазон
     */
    virtual void sendHistEventsInWindow(QCPRange range) {}

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
    void getMinMaxInd(T &vector, double min, double max,
                                     quint64 &minInd, quint64 &maxInd)
    {
        minInd = 0;
        maxInd = vector.size() - 1;

        bool minFound = false;
        bool maxFound = false;
        for(quint64 i = 0; i < vector.size(); i++)
        {
            if(!minFound && vector[i] > min)
            {
                minFound = true;
                if(i == 0)
                    minInd = 0;
                else
                    minInd = i - 1;
            }

            if(!maxFound && vector[i] > max)
            {
                maxFound = true;
                maxInd = i;
                break;
            }
        }
    }

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
                        int *binMax = 0, bool abs = 1)
    {
            QVector<double> hist;
            QVector<double> bin;

            generateHistFromData<T>(data, hist, bin, minVal, maxVal, bins, binMax, abs);

            //создание графика
            return createHistFromData(plot, hist, bin, minVal, maxVal);
    }

    template<typename T>
    /*!
     * \brief Сгенерировать гистограмму из вектора данных.
     * \param [in] data Вектор входных данных
     * \param [out] binVal Количество событий в бине.
     * \param [out] binCoord Координата бина.
     * \param [in, out] minVal Минимальное значение данных.
     * \param [in, out] maxVal Максимальное значение данных.
     * \param bins Количество бинов.
     * \param [out] binMax Максимальное количество элементов в бине.
     * \param abs Брать модуль амплитуд событий. \note Работает только если \p minVal и \p maxVal неположительны.
     */
    void generateHistFromData(QVector<T> &data, QVector<double> &binVal, QVector<double> &binCoord,
                              double &minVal, double &maxVal, int bins = 128,
                              int *binMax = 0, bool abs = 1)
    {
        binVal = QVector<double>(bins);
        binCoord = QVector<double>(bins);


        if(abs && (minVal > 0 || maxVal > 0))
            abs = 0;

        if(abs)
        {
            double buf = minVal;
            minVal = qAbs(maxVal);
            maxVal = qAbs(buf);
        }

        //пересчет данных на бины
        float diff = ((float)bins / (double)(maxVal - minVal));

        for(int i = 0; i < data.size(); i++)
        {
            double currData;
            if(!abs)
                currData = data[i];
            else
                currData = qAbs(data[i]);
            binVal[qMin((int)((currData - minVal) * diff), bins - 1)]++;
        }

        //Определение самого большого бина
        if(binMax)
        {
            *binMax = (int)(std::max_element(binVal.begin(), binVal.end()));
    #ifdef TEST_MODE
            qDebug() << *binMax;
    #endif
        }

        for (int i = 0; i < bins; i++)
            binCoord[i] = (((double)i + 0.5) / (double)bins) * (double)(maxVal - minVal) + minVal;
    }


    /*!
     * \brief Создание графика гистограммы.
     * \param plot Виджет графиков, в котором будет согдана гистограмма.
     * \param [in] binVal Количество событий в бине. Получается с помощью метода FileDrawer::generateHistFromData.
     * \param [in] binCoord Координата бина. Получается с помощью метода FileDrawer::generateHistFromData.
     * \param minVal Минимальное значение данных.
     * \param maxVal Максимальное значение данных.
     * \return Указатель на график гистограммы.
     */
    QCPBars* createHistFromData(QCustomPlot *plot, QVector<double> &binVal, QVector<double> &binCoord, double minVal, double maxVal);


    /*!
     * \brief Создание гистограммы без закраски
     * \param plot Виджет графиков, в котором будет согдана гистограмма.
     * \param [in] binVal Количество событий в бине. Получается с помощью метода FileDrawer::generateHistFromData.
     * \param [in] binCoord Координата бина. Получается с помощью метода FileDrawer::generateHistFromData.
     * \param minVal Минимальное значение данных.
     * \param maxVal Максимальное значение данных.
     * \return Указатель на график гистограммы.
     */
    QCPGraph* createGraphHistFromData(QCustomPlot *plot, QVector<double> &binVal, QVector<double> &binCoord);


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

#endif // FILEDRAWER_H
