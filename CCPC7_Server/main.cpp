#include "camacserver.h"
#include "camacserverhandler.h"
#include "tempfolder.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>

// настройки логгера
#if QT_VERSION >= 0x050300
INITIALIZE_EASYLOGGINGPP

#elif QT_VERSION >= 0x040800
_INITIALIZE_EASYLOGGINGPP
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef TEST_MODE
    LOG(INFO) << "Programm run in test mode";
#endif

#ifdef VIRTUAL_MODE
    LOG(INFO) << "Programm run in virtual mode";
#endif

    //создание временной папки
    TempFolder tempFolder("temp/CCPC7Server", 200);
    QDateTime curr_datetime = QDateTime::currentDateTime();
    //обновление каждые 2 минуты
    QTimer timer;
    timer.connect(&timer, SIGNAL(timeout()), &tempFolder, SLOT(clear()));
    timer.start(120000);

#if QT_VERSION >= 0x050300
START_EASYLOGGINGPP(argc, argv);
#elif QT_VERSION >= 0x040800
    _START_EASYLOGGINGPP(argc, argv);
#endif

#if defined(Q_OS_LINUX)
    //убирание привилегий рута с папки с логами
    system((std::string("sudo chmod -R 777 ") + QFileInfo(tempFolder.getFolderPath() +
            curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).dir().path().toStdString()).c_str());
#endif

#if QT_VERSION >= 0x050300
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (tempFolder.getFolderPath() +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#elif QT_VERSION >= 0x040800
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (tempFolder.getFolderPath() +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif
    CamacServerHandler camacServerHandler(&tempFolder);

    return a.exec();
}
