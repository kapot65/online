#ifndef INIMANAGER_H
#define INIMANAGER_H

#include <QObject>
#include <QVariant>
#include <QSettings>

/*!
 * \brief Менеджер настроек.
 * Менеджер для работы с настройками, записанными в ini файл.
 */
class IniManager : public QObject
{
    Q_OBJECT
public:
    /*!
     * \param Путь к файлу с настройками.
     */
    explicit IniManager(QString settingsFile, QObject *parent = 0);
    ~IniManager();
    /*!
     * \brief Получить значение параметра.
     * \param group Имя группы параметров.
     * \param fieldName Имя параметра.
     * \return Значение параметра.
     */
    QVariant getSettingsValue(QString group, QString fieldName);

    /*!
     * \brief Установить значение параметра.
     * \param group Имя группы параметров.
     * \param fieldName Имя параметра.
     * \param value Значение параметра.
     */
    void setSettingsValue(QString group, QString fieldName, QVariant value);

    /*!
     * \brief Указатель на стандарнтый менеджер настроек Qt.
     */
    QSettings *settings;
};

#endif // INIMANAGER_H
