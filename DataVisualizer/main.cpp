#include "DataVisualizerWindow.h"
#include <QApplication>
#include <QTextCodec>

#include <easylogging++.h>

#include <common.h>

// настройки логгера
#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

int main(int argc, char *argv[])
{
    setCodecs();
    initLogging(argc, argv);
    logModes();

    QApplication a(argc, argv);

    qApp->setStyleSheet(getStyleSheet());

    DataVisualizerWindow w;
    w.show();

    return a.exec();
}
