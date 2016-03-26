#include "camacserver.h"
#include "camacserverhandler.h"
#include "tempfolder.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <common.h>

// настройки логгера
#include <easylogging++.h>

// настройки логгера
#ifdef EL_CPP11
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setCodecs();
    initLogging(argc, argv, false);

    TempFolder *tempFolder = initTempFolder(QObject::tr("temp/%1").arg(BIN_NAME));

    logModes();

    CamacServerHandler camacServerHandler(tempFolder);

    return a.exec();
}
