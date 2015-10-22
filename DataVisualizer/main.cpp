#include "DataVisualizerWindow.h"
#include <QApplication>
#include <QTextCodec>

#include <easylogging++.h>

// настройки логгера
#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

#ifdef Q_OS_LINUX
    #define LOG_DIRECTORY "/home/Logs/DataVisualizer/"
#endif

#ifdef Q_OS_WIN
    #define LOG_DIRECTORY "D:\\Logs\\DataVisualizer\\"
#endif

int main(int argc, char *argv[])
{
#if __cplusplus == 201103L
    START_EASYLOGGINGPP(argc, argv);
#else
    _START_EASYLOGGINGPP(argc, argv);
#endif

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
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#else
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (LOG_DIRECTORY +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif

    QApplication a(argc, argv);

    DataVisualizerWindow w;
    w.show();

    return a.exec();
}
