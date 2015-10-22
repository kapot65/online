#include "hvserverhandler.h"
#include <easylogging++.h>

#include <QCoreApplication>
#include <iostream>
#include <QDateTime>

#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

#ifdef Q_OS_LINUX
    #define LOG_DIRECTORY "/home/Logs/HV_Server/"
#endif

#ifdef Q_OS_WIN
    #define LOG_DIRECTORY "D:\\Logs\\HV_Server\\"
#endif


int main(int argc, char *argv[])
{

#if __cplusplus == 201103L
    START_EASYLOGGINGPP(argc, argv);
#else
    _START_EASYLOGGINGPP(argc, argv);
#endif

#if QT_VERSION <= 0x050000
     QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
     QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

#ifdef TEST_MODE
    LOG(INFO) << "Programm run in test mode";
#endif

#ifdef VIRTUAL_MODE
    LOG(INFO) << "Programm run in virtual mode";
#endif


    QCoreApplication a(argc, argv);

    QDateTime curr_datetime = QDateTime::currentDateTime();


#ifdef Q_OS_LINUX
    QDir().mkpath(QDir::homePath() + LOG_DIRECTORY);
#endif

#ifdef Q_OS_WIN
    QDir().mkpath(LOG_DIRECTORY);
#endif



#if __cplusplus == 201103L
        el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                             (LOG_DIRECTORY +
                                              curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
#else
        easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                         (QDir::homePath() + LOG_DIRECTORY +
                                          curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
#endif

    HVServerHandler hv_server;

    return a.exec();
}
