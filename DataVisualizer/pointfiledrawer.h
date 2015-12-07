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

private:
    ///\brief Файл загружен.
    bool loaded;
    ///\brief Графики с абсолютным временем.
    QVector<QCPGraph*> graph_absolute;
    /// \brief Графики с относительным временен.
    QVector<QCPGraph*> graph_relative;
    /// \brief Графики гистограмм.
    QVector<QCPGraph*> bars;

    /*!
     * \brief Доступные времена набора.
     * Используются для пересчета времен.
     */
    static QMap<int, unsigned short> aviableMeasureTimes;
};

#endif // POINTFILEDRAWER_H
