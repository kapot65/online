#ifndef INIMANAGER_H
#define INIMANAGER_H

#include <QObject>
#include <QVariant>
#include <QSettings>


class IniManager : public QObject
{
    Q_OBJECT
public:
    explicit IniManager(QString settingsFile, QObject *parent = 0);
    ~IniManager();
    QVariant getSettingsValue(QString group, QString fieldName);
    void setSettingsValue(QString group, QString fieldName, QVariant value);

    QSettings *settings;
};

#endif // INIMANAGER_H
