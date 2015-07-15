#include <QApplication>
#include <QDir>
#include <QDateTime>

#include <datavisualizerform.h>

#include "camacclientform.h"

// настройки логгера
INITIALIZE_EASYLOGGINGPP
#define LOG_DIRECTORY "D:\\Logs\\CamacClient\\"

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    QDateTime curr_datetime = QDateTime::currentDateTime();

    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (LOG_DIRECTORY +
                                          curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
    LOG(INFO) << "Programm started";

    QApplication a(argc, argv);
    /*
    MainWindow w;
    w.show();
    /*/

    CamacClientForm cF;
    cF.showMaximized();
    //SeverTester st;
    //st.show();
    //AlgoritmForm af;
    //af.show();
    //*/

    
    return a.exec();
}
