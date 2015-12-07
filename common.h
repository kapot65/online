#ifndef COMMON_H
#define COMMON_H

#include <easylogging++.h>
#include <QSettings>
#include <QDir>

void initLogging(int argc, char *argv[])
{
    // настройки логгера
    QDateTime curr_datetime = QDateTime::currentDateTime();

    QSettings settings(QObject::tr("%1Settings.ini").arg(BIN_NAME), QSettings::IniFormat);


    if(!settings.contains("log_dir"))
    {
#ifdef Q_OS_LINUX
        settings.setValue("log_dir", QObject::tr("/home/Logs/%1").arg(BIN_NAME));
#endif

#ifdef Q_OS_WIN
        settings.setValue("log_dir", QObject::tr("C:/Logs/%1").arg(BIN_NAME));
#endif
    }

    QString logDir = settings.value("log_dir").toString();

#ifdef Q_OS_LINUX
        QDir().mkpath(QDir::homePath() + logDir);
#endif

#ifdef Q_OS_WIN
        QDir().mkpath(LOG_DIRECTORY);
#endif

#if __cplusplus == 201103L
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (logDir +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#else
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (logDir +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif

#if __cplusplus == 201103L
        START_EASYLOGGINGPP(argc, argv);
#else
        _START_EASYLOGGINGPP(argc, argv);
#endif
}

void setCodecs()
{
    #if QT_VERSION <= 0x050000
         QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
         QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    #endif
}

void logModes()
{
    #ifdef TEST_MODE
        LOG(INFO) << "Programm run in test mode";
    #endif

    #ifdef VIRTUAL_MODE
        LOG(INFO) << "Programm run in virtual mode";
    #endif
}

#endif // COMMON_H

