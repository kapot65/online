#include "DataVisualizerWindow.h"
#include <QApplication>
#include <QTextCodec>

#include <easylogging++.h>

// настройки логгера
#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#elif __cplusplus == 199711L
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
#elif __cplusplus == 199711L
    _START_EASYLOGGINGPP(argc, argv);
#endif

    QDateTime curr_datetime = QDateTime::currentDateTime();

#if defined(Q_OS_LINUX)
    //убирание привилегий рута с папки с логами
    system((std::string("sudo chmod -R 777 ") + QFileInfo(LOG_DIRECTORY +
            curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).dir().path().toStdString()).c_str());
#endif

#if __cplusplus == 201103L
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (LOG_DIRECTORY +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#elif __cplusplus == 199711L
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (LOG_DIRECTORY +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif

    QApplication a(argc, argv);

    DataVisualizerWindow w;
    w.show();

    return a.exec();
}
