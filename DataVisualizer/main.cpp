#include "DataVisualizerWindow.h"
#include <QApplication>
#include <QTextCodec>

#include <easylogging++.h>

// настройки логгера
#if QT_VERSION >= 0x050300
INITIALIZE_EASYLOGGINGPP

#elif QT_VERSION >= 0x040800
_INITIALIZE_EASYLOGGINGPP
#endif

#ifdef Q_OS_WIN
#define LOG_DIRECTORY "D:\\Logs\\DataVisualizer\\"
#elif defined(Q_OS_LINUX)
#define LOG_DIRECTORY "/Logs/DataVisualizer/"
#endif

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

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

    QApplication a(argc, argv);

    DataVisualizerWindow w;
    w.show();

    return a.exec();
}
