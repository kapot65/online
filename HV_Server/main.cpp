#include "hvserverhandler.h"
#include <easylogging++.h>

#include <QCoreApplication>
#include <iostream>
#include <QDateTime>

#include <common.h>

#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setCodecs();
    initLogging(argc, argv);
    logModes();

    HVServerHandler hv_server;

    return a.exec();
}
