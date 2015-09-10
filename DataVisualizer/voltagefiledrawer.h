#ifndef VOLTAGEFILEDRAWER_H
#define VOLTAGEFILEDRAWER_H

#include "filedrawer.h"

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


#endif // VOLTAGEFILEDRAWER_H
