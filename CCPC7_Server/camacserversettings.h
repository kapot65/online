#ifndef CAMACSERVERSETTINGS_H
#define CAMACSERVERSETTINGS_H

#include <QObject>
#include <QJson/Serializer>
#include <inimanager.h>
#include <easylogging++.h>

class CamacServerSettings : public QObject
{
    Q_OBJECT
public:
    explicit CamacServerSettings(QObject *parent = 0);
    ~CamacServerSettings();

    //проверяет наличие необходимых настроек
    bool loadSettings(QString fileName);

    //оберточные функции
    QVariant getSettingsValue(QString group, QString fieldName);
    void setSettingaValue(QString group, QString fieldName, QVariant value);

    int getADC_CRATE(){ return ADC_CRATE; }
    int getMADC(){ return MADC; }
    int getTG1(){ return TG1; }
    int getOV1(){ return OV1; }
    int getTTL_NIM(){ return TTL_NIM; }
    int getCOUNTER1(){ return COUNTER1; }
    int getCOUNTER2(){ return COUNTER2; }
    int getTERMINAL1(){ return TERMINAL1; }
    int getTERMINAL2(){ return TERMINAL2; }
    QJson::IndentMode getIdentMode(){ return identMode;}

#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050300
    el::Level getLogLevel(){return logLevel;}
#endif
#elif defined(Q_OS_LINUX)
    //easyloggingpp::Level getLogLevel(){return;}
#endif


signals:
    void error(QString err);

private:
    void checkAndWriteErr(QString group, QString field, QString &err);
    IniManager *iniManager;

    //индексы блоков (парсятся заранее для увеличения скорости)
    int ADC_CRATE;
    int MADC;
    int TG1;
    int OV1;
    int TTL_NIM;
    int COUNTER1;
    int COUNTER2;
    int TERMINAL1;
    int TERMINAL2;

    QJson::IndentMode identMode;
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x050300
    el::Level logLevel;
#endif
#elif defined(Q_OS_LINUX)
    //easyloggingpp::Level logLevel;
#endif

};

#endif // CAMACSERVERSETTINGS_H
