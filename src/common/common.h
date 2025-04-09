#ifndef COMMON_H
#define COMMON_H

#include <easylogging++.h>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <qdebug.h>
#include <qglobal.h>
#include <qresource.h>
#include <tempfolder.h>

/*!
 * \brief Инициализация логгирования
 * \param redirectLogs Перенаправление логов в созданную папку. Если для хранения логов будет
 * использоваться tempFolder, то этому параметру нужно присвоить значение false.
 */
void initLogging(int argc, char *argv[], bool redirectLogs = true)
{
    if(redirectLogs)
    {
        // настройки логгера
        QDateTime curr_datetime = QDateTime::currentDateTime();
        QSettings settings(QObject::tr("%1Settings.ini").arg(BIN_NAME), QSettings::IniFormat);

        if(!settings.contains("log_dir"))
        {
#ifdef Q_OS_LINUX
            settings.setValue("log_dir", QObject::tr("/home/logs/%1").arg(BIN_NAME));
#endif

#ifdef Q_OS_WIN
            settings.setValue("log_dir", QObject::tr("%1/logs/%2").arg(QDir::homePath()).arg(BIN_NAME));
#endif
        }

        QString logDir = settings.value("log_dir").toString();

#ifdef Q_OS_LINUX
        QDir().mkpath(logDir);
#endif

#ifdef Q_OS_WIN
        QDir().mkpath(logDir);
#endif

#ifdef EL_CPP11
        el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                             (logDir +
                                              curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#else
        easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                         (logDir +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
#endif
    }

#ifdef EL_CPP11
        START_EASYLOGGINGPP(argc, argv);
#else
        _START_EASYLOGGINGPP(argc, argv);
#endif
}

QString getStyleSheet()
{
    if(!QFile::exists("styleSheets.qss"))
        QFile::copy(":/common/resources/styleSheets.qss", "styleSheets.qss");

        
    QFile testFile2(":/common/resources/styleSheets.qss");
    qDebug() << QResource(":/common/resources/styleSheets.qss").data();

    QFile file("styleSheets.qss");
    file.open(QIODevice::ReadOnly);
    QString styleSheet = file.readAll();
    file.close();

    return styleSheet;
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

/*!
 * \brief инициализация директории с временными файлами.
 * Создает Объект временной папки и перенаправляет логи туда.
 * \param folderPath относительный путь к папке
 * \param maxSizeMbytes максимальный размер папки
 */
TempFolder* initTempFolder(QString folderPath, int maxSizeMbytes = 200)
{
    TempFolder *tempFolder = new TempFolder(folderPath, maxSizeMbytes);
    QDateTime curr_datetime = QDateTime::currentDateTime();

    //начальная очистка папки
    tempFolder->clear();

    //обновление каждые 2 минуты
    QTimer *timer = new QTimer(tempFolder);
    QObject::connect(timer, SIGNAL(timeout()), tempFolder, SLOT(clear()));
    timer->start(120000);

    #ifdef EL_CPP11
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (tempFolder->getFolderPath() +
                                          curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
    #else
    easyloggingpp::Loggers::reconfigureAllLoggers(easyloggingpp::ConfigurationType::Filename,
                                     (tempFolder->getFolderPath() +
                                      curr_datetime.toString("/log_yyyyMMdd-hhmmss.zzz")).toStdString());
    #endif

    return tempFolder;
}

#endif // COMMON_H

