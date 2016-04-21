#include "DataVisualizerWindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QCommandLineParser>

#include <easylogging++.h>

#include <common.h>

// настройки логгера
#ifdef EL_CPP11
    INITIALIZE_EASYLOGGINGPP
#else
    _INITIALIZE_EASYLOGGINGPP
#endif

int main(int argc, char *argv[])
{
    setCodecs();
    initLogging(argc, argv);
    logModes();

    QApplication a(argc, argv);
    a.setApplicationVersion(APP_REVISION);

    QCommandLineParser parser;
    parser.setApplicationDescription("This appilcation draws Troitsk nu-mass data files");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption directoryOption(QStringList() << "d" << "directory",
          QCoreApplication::translate("main", "Data directory that will opens on start"),
          QCoreApplication::translate("main", "directory"));
    parser.addOption(directoryOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    QString directory = parser.value(directoryOption);

    qApp->setStyleSheet(getStyleSheet());

    DataVisualizerWindow w(directory);

    w.show();

    return a.exec();
}
