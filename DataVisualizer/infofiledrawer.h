#ifndef INFOFILEDRAWER_H
#define INFOFILEDRAWER_H

#include "filedrawer.h"

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

#endif // INFOFILEDRAWER_H
