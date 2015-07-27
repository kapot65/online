#ifndef HVCONTROLER_H
#define HVCONTROLER_H

#include "comport.h"

/*!
 * \brief The HVControler class
 * Класс, реализующий общение с ICP CON модели I-7021P через Com порт.
 * Описание протокола общения: http://ftp.icpdas.com/pub/cd/8000cd/napdos/7000/manual/7021_22_24.pdf
 */
class HVControler : public ComPort
{
    Q_OBJECT
public:
    explicit HVControler(IniManager *manager, QString controllerName,  QObject *parent = 0);
    bool checkBusy(){return busyFlag;}


    bool loadSettings(QString controllerName, IniManager *manager);
signals:
    void setVoltageDone();

public slots:
    void setVoltage(double voltage);

protected slots:
    virtual void readMessage();

protected:
    /*!
     * \brief processSettingError
     * Метод проверяет файл настроек на наличие поля setting во вкладке
     * controllerName. Если поля нет, то метод пытается остановить приложение.
     * \param setting
     * \param controllerName
     */
    void processSettingError(QString setting, QString controllerName);

    /*!
     * \brief Коэффициент преобразования при свободном члене.
     * Используется в функции HVControler::setVoltage
     */
    double c0;

    /*!
     * \brief Коэффициент преобразования при линейном члене.
     * Используется в функции HVControler::setVoltage
     */
    double c1;

    /*!
     * \brief Минимальное возможное установочное значение.
     * Используется в функции HVControler::setVoltage
     */
    double minTreshold;

    /*!
     * \brief Максимальное возможное установочное значение.
     * Используется в функции HVControler::setVoltage
     */
    double maxTreshold;

    /*!
     * \brief portName
     * Имя COM порта, к которому подключен контроллер.
     */
    QString portName;

    /*!
     * \brief busyFlag
     * Флаг занятости контроллера.
     */
    bool busyFlag;
};

#endif // HVCONTROLER_H
