#include "camacserversettings.h"

CamacServerSettings::CamacServerSettings(QString settingsFile, QObject *parent) : IniManager(settingsFile, parent)
{
}

inline void CamacServerSettings::checkAndWriteErr(QString group, QString field, QString &err)
{
    if(!getSettingsValue(group, field).isValid())
        err.push_back(QString("Setting error: [%1] %2 not set.\n").arg(group).arg(field));
}

//проверяет наличие необходимых настроек
bool CamacServerSettings::loadSettings()
{
    //проверка необходимых полей
    QString err;

    checkAndWriteErr("CamacServer", "port", err);
    checkAndWriteErr("CamacAlgoritm", "ADC_CRATE", err);
    checkAndWriteErr("CamacAlgoritm", "MADC", err);
    checkAndWriteErr("CamacAlgoritm", "TG1", err);
    checkAndWriteErr("CamacAlgoritm", "OV1", err);
    checkAndWriteErr("CamacAlgoritm", "TTL_NIM", err);
    checkAndWriteErr("CamacAlgoritm", "COUNTER1", err);
    checkAndWriteErr("CamacAlgoritm", "COUNTER2", err);
    checkAndWriteErr("CamacAlgoritm", "TERMINAL1", err);
    checkAndWriteErr("CamacAlgoritm", "TERMINAL2", err);

    if(!err.isEmpty())
    {
        err.push_front(QString("Errors in settings file (%1):\n").arg(settings->fileName()));

        emit error(err);
        return 1;
    }
    else
    {
        ADC_CRATE = getSettingsValue("CamacAlgoritm", "ADC_CRATE").toInt();
        MADC = getSettingsValue("CamacAlgoritm", "MADC").toInt();
        TG1 = getSettingsValue("CamacAlgoritm", "TG1").toInt();
        OV1 = getSettingsValue("CamacAlgoritm", "OV1").toInt();
        TTL_NIM = getSettingsValue("CamacAlgoritm", "TTL_NIM").toInt();
        COUNTER1 = getSettingsValue("CamacAlgoritm", "COUNTER1").toInt();
        COUNTER2 = getSettingsValue("CamacAlgoritm", "COUNTER2").toInt();
        TERMINAL1 = getSettingsValue("CamacAlgoritm", "TERMINAL1").toInt();
        TERMINAL2 = getSettingsValue("CamacAlgoritm", "TERMINAL2").toInt();

        //считывание уровня логирования
#if __cplusplus == 201103L
        logLevel = el::LevelHelper::convertFromString("warning");
#else
        //logLevel = el::LevelHelper::convertFromString("warning");
#endif

        if(getSettingsValue("General", "LogLevel").isValid())
        {
            QString logLevelStr = getSettingsValue("General", "LogLevel").toString();

#if __cplusplus == 201103L
            if(el::LevelHelper::convertFromString(logLevelStr.toStdString().c_str()) != el::Level::Unknown)
                logLevel = el::LevelHelper::convertFromString(logLevelStr.toStdString().c_str());
#else
            /*
            if(el::LevelHelper::convertFromString(logLevelStr.toStdString().c_str()) != el::Level::Unknown)
                logLevel = el::LevelHelper::convertFromString(logLevelStr.toStdString().c_str());
            */
#endif
        }

        return 0;
    }
}

CamacServerSettings::~CamacServerSettings()
{
}

