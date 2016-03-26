#include "hvserverhandler.h"
#include <easylogging++.h>

#include <QCoreApplication>
#include <iostream>
#include <QDateTime>

#include <common.h>

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

    TempFolder *tempFolder = initTempFolder(QObject::tr("temp/%1").arg(BIN_NAME), 50);

    logModes();

    HVServerHandler hv_server;

    return a.exec();
}
