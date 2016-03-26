#ifndef POINTFILEDRAWER_H
#define POINTFILEDRAWER_H

#include "filedrawer.h"

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

public slots:
    virtual void sendHistEventsInWindow(QCPRange range);

private:
    ///\brief Файл загружен.
    bool loaded;
    ///\brief Графики с абсолютным временем.
    QVector<QCPGraph*> graph_absolute;
    /// \brief Графики с относительным временен.
    QVector<QCPGraph*> graph_relative;
    /// \brief Графики гистограмм.
    QVector<QCPGraph*> bars;

    QVector<double> binVal;
    QVector<double> binCoord;


    /*!
     * \brief Доступные времена набора.
     * Используются для пересчета времен.
     */
    static QMap<int, unsigned short> aviableMeasureTimes;

    /*!
     * \brief Вытаскивает события из данных в формате Point
     */
    void extractEventsFromPointData(QVector<double> &time_abs, QVector<double> &time,
                                    QVector<double> &event_data, QVariantMap meta,
                                    QVector<Event> events);
    /*!
     * \brief Вытаскивает события из данных в формате Miltipoint (точка, разделенная на блоки по 5с)
     */
    void extractEventsFromMultiPointData(QVector<double> &time_abs, QVector<double> &time,
                                         QVector<double> &event_data, QVariantMap meta,
                                         QVector<Event> events);
};

#endif // POINTFILEDRAWER_H
