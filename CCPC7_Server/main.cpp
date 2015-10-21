#include "camacserver.h"
#include "camacserverhandler.h"
#include "tempfolder.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

// настройки логгера
#include <easylogging++.h>

// настройки логгера
#if __cplusplus == 201103L
    INITIALIZE_EASYLOGGINGPP
#elif __cplusplus == 199711L
    _INITIALIZE_EASYLOGGINGPP
#endif

#ifdef Q_OS_LINUX
    #define LOG_DIRECTORY "/home/Logs/CCPC7_Server/"
#endif

#ifdef Q_OS_WIN
    #define LOG_DIRECTORY "D:\\Logs\\CCPC7_Server\\"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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


#if __cplusplus == 201103L
    START_EASYLOGGINGPP(argc, argv);
#elif __cplusplus == 199711L
    _START_EASYLOGGINGPP(argc, argv);
#endif

    //создание временной папки
    TempFolder tempFolder("temp/CCPC7Server", 200);
    QDateTime curr_datetime = QDateTime::currentDateTime();
    //обновление каждые 2 минуты
    QTimer timer;
    timer.connect(&timer, SIGNAL(timeout()), &tempFolder, SLOT(clear()));
    timer.start(120000);

#if __cplusplus == 201103L
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (tempFolder.getFolderPath() +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#elif __cplusplus == 199711L
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (tempFolder.getFolderPath() +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif

    CamacServerHandler camacServerHandler(&tempFolder);

    return a.exec();
}
