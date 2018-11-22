#include "mainwindow.h"
#include <QApplication>
#include <common.h>

// настройки логгера
#ifdef EL_CPP11
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
    a.setApplicationVersion(APP_REVISION);

    MainWindow w;
    w.show();

    return a.exec();
}
