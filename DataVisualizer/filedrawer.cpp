#include "filedrawer.h"

QCPBars *FileDrawer::createHistFromData(QCustomPlot *plot, QVector<double> &binVal, QVector<double> &binCoord, double minVal, double maxVal)
{
    QCPBars *bars;
    bars = new QCPBars(plot->xAxis, plot->yAxis);
    double size = binVal.size();
    double width = (1. / size) * (double)(maxVal - minVal);
    bars->setWidth(width);
    bars->setData(binCoord, binVal);
    bars->setPen(Qt::NoPen);
    bars->setBrush(QBrush(color));

    return bars;
}

QCPGraph *FileDrawer::createGraphHistFromData(QCustomPlot *plot, QVector<double> &binVal, QVector<double> &binCoord, double minVal, double maxVal)
{
    QCPGraph *graph = new QCPGraph(plot->xAxis, plot->yAxis);
    graph->setPen(QPen(color));
    graph->setLineStyle(QCPGraph::lsStepCenter);
    graph->setData(binCoord, binVal);

    return graph;
}

FileDrawer::FileDrawer(QTableWidget *table, QCustomPlot *plot, QString filename, QObject *parent) : QObject(parent)
{
    this->table = table;
    this->plot = plot;

    isVisible = false;
    color = getRandomColor();

    file = new QFile(filename, this);
    file->open(QIODevice::ReadOnly);
    TcpProtocol::parceMessage(file->readAll(), meta, data, 1);
    file->close();
    file->open(QIODevice::ReadOnly);

    connect(this, SIGNAL(updated()), plot, SLOT(replot()));
}

FileDrawer::~FileDrawer()
{
    file->close();
}

void FileDrawer::setMetaTableText(int col, int row, QString text)
{
    if((table->rowCount() > row) && (table->columnCount() > col))
    {
        table->setItem(row,col, new QTableWidgetItem());
        table->item(row,col)->setText(text);
    }
}

QColor FileDrawer::getRandomColor()
{
    // this are the numbers of the QT default colors
    static int QtColours[]= { 3, 2, 7, 13, 8, 14, 9,
                              15, 10, 16, 11, 17, 12,
                              18, 5, 4, 6, 19, 0, 1 };


    QColor color(QtColours[qrand() % sizeof(QtColours)]);
    //color.setAlpha(150);

#ifdef TEST_MODE
    qDebug() << color;
#endif

    return color;
}

