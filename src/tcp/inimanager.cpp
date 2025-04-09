#include "inimanager.h"

IniManager::IniManager(QString settingsFile, QObject *parent) : QObject(parent)
{
    settings = new QSettings(settingsFile, QSettings::IniFormat);
}

QVariant IniManager::getSettingsValue(QString group, QString fieldName)
{
    if(group != "General")
        settings->beginGroup(group);
    QVariant result = settings->value(fieldName);
    if(group != "General")
        settings->endGroup();
    return result;
}

void IniManager::setSettingsValue(QString group, QString fieldName, QVariant value)
{
    if(group != "General")
        settings->beginGroup(group);
    settings->setValue(fieldName, value);
    if(group != "General")
        settings->endGroup();
}

IniManager::~IniManager()
{
    delete settings;
}

