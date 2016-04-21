#include <QRubberBand>
#include <customplotzoom.h>

CustomPlotZoom::CustomPlotZoom(QLabel* posLabel, QWidget * parent)
    : QCustomPlot(parent)
    , mZoomMode(false)
    , mRubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    this->posLabel = posLabel;
}

CustomPlotZoom::~CustomPlotZoom()
{
    delete mRubberBand;
}

void CustomPlotZoom::setZoomMode(bool mode)
{
    mZoomMode = mode;
}

void CustomPlotZoom::mousePressEvent(QMouseEvent * event)
{
    if (mZoomMode)
    {
        if (event->button() == Qt::RightButton)
        {
            mOrigin = event->pos();
            mRubberBand->setGeometry(QRect(mOrigin, QSize()));
            mRubberBand->show();
        }
    }
    QCustomPlot::mousePressEvent(event);
}

void CustomPlotZoom::mouseMoveEvent(QMouseEvent * event)
{
    if (mRubberBand->isVisible())
    {
        mRubberBand->setGeometry(QRect(mOrigin, event->pos()).normalized());
    }

    //запись текущих координат
    if(posLabel)
    {
        double x = xAxis->pixelToCoord(event->pos().x());
        double y = yAxis->pixelToCoord(event->pos().y());

        posLabel->setText(tr("Положение курсора: ( %1,\t%2 )").arg(x).arg(y));
    }

    QCustomPlot::mouseMoveEvent(event);
}

void CustomPlotZoom::mouseReleaseEvent(QMouseEvent * event)
{
    if (mRubberBand->isVisible())
    {
        const QRect & zoomRect = mRubberBand->geometry();

        if(zoomRect.width() + zoomRect.height() < 8)
        {
            rescaleAxes(true);
        }
        else
        {
            int xp1, yp1, xp2, yp2;
            zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
            double x1 = xAxis->pixelToCoord(xp1);
            double x2 = xAxis->pixelToCoord(xp2);
            double y1 = yAxis->pixelToCoord(yp1);
            double y2 = yAxis->pixelToCoord(yp2);

            xAxis->setRange(x1, x2);
            yAxis->setRange(y1, y2);
        }
        mRubberBand->hide();
        replot();
    }
    QCustomPlot::mouseReleaseEvent(event);
}
