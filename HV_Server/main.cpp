#include "hvserverhandler.h"
#include <easylogging++.h>

#include <QCoreApplication>
#include <iostream>
#include <QDateTime>

#if QT_VERSION >= 0x050300
INITIALIZE_EASYLOGGINGPP

#elif QT_VERSION >= 0x040800
_INITIALIZE_EASYLOGGINGPP
#endif

#ifdef Q_OS_WIN
#define LOG_DIRECTORY "E:\\Logs\\HV_Server\\"
#elif defined(Q_OS_LINUX)
#define LOG_DIRECTORY "/Logs/HV_server/"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDateTime curr_datetime = QDateTime::currentDateTime();

    #if QT_VERSION >= 0x050300
    START_EASYLOGGINGPP(argc, argv);
    #elif QT_VERSION >= 0x040800
    _START_EASYLOGGINGPP(argc, argv);
    #endif

    #if defined(Q_OS_LINUX)
    //убирание привилегий рута с папки с логами
    system((std::string("sudo chmod -R 777 ") + QFileInfo(QDir::homePath() + LOG_DIRECTORY +
            curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).dir().path().toStdString()).c_str());

    #endif

    #if QT_VERSION >= 0x050300
        el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                             (LOG_DIRECTORY +
                                              curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
    #elif QT_VERSION >= 0x040800

    #ifdef Q_OS_WIN
        easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                         (LOG_DIRECTORY +
                                          curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
    #elif defined(Q_OS_LINUX)
        easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                         (QDir::homePath() + LOG_DIRECTORY +
                                          curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
    #endif

    #endif

    HVServerHandler hv_server;

    return a.exec();
}
