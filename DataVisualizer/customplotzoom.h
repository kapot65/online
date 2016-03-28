#ifndef CUSTOMPLOTZOOM_H
#define CUSTOMPLOTZOOM_H

#include <QPoint>
#include <QMouseEvent>
#include <QRubberBand>
#include <qcustomplot.h>
#include <QWidget>

/*!
 * \brief Виджет QCustomPlot с поддержкой увеличения по окну.
 * \details Для увеличения по окну в данном виджете необходимо зажав правую кнопку мыши,
 * выделить нужный участок и затем отпустить правую кнопку.
 *
 * Реализация взята с http://www.qcustomplot.com/index.php/support/forum/227
 */
class CustomPlotZoom : public QCustomPlot
{
    Q_OBJECT

public:
    /*!
     * \param posLabel Указатель на лейбл, куда будут записываться текущее положение курсора
     */
    CustomPlotZoom(QLabel *posLabel = 0, QWidget * parent = 0);
    virtual ~CustomPlotZoom();

    /*!
     * \brief Установить тип увеличения.
     * \param mode Тип увеличения. 1 - увеличение по окну, 0 - стандартный QCustomPlot.
     */
    void setZoomMode(bool mode);

private slots:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);

private:
    bool mZoomMode;
    QRubberBand * mRubberBand;
    QPoint mOrigin;

    QLabel *posLabel;
};

#endif // CUSTOMPLOTZOOM_H
