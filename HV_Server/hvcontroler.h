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
    /*!
     * \brief Загружает настройки и открывает порт.
     * \param manager Менеджер настроек.
     * \param controllerName Имя контролера HV. Используется при загрузке настроек из ini файла.
     * \param ok Успешность создания класса. Если происходят ошибки при создании - они записываются в лог.
     */
    explicit HVControler(IniManager *manager, QString controllerName, bool *ok = 0,  QObject *parent = 0);

signals:
    /*!
     * \brief Установка напряжения закончена.
     * \todo Добавить проверку установленного напряжения.
     */
    void setVoltageDone();

public slots:
    /*!
     * \brief Устанавливает напряжение. Если напряжение выходит за границы
     * диапазона - устанавливает граничное значение и записывает предупреждение в лог.
     * Диапазоны устанавливаются в ini файле в полях "minTreshold" и "maxTreshold".
     * \param voltage Напряжение в вольтах.
     */
    void setVoltage(double voltage);

protected:
    /*!
     * \brief Загружает настройки из ini файла.
     * \param controllerName Имя HVControler.
     * \param manager Указатель на менеджер настроек.
     * \return Успешность загрузки параметров.
     */
    bool loadSettings(QString controllerName, IniManager *manager);

    /*!
     * \brief Метод проверяет файл настроек на наличие поля setting во вкладке
     * controllerName. Если поля нет, то метод пытается остановить приложение.
     * \param setting Название параметра.
     * \param controllerName Имя HVControler.
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
     * \brief Имя COM порта, к которому подключен контроллер.
     */
    QString portName;


    /// \brief Название контролера.
    QString controllerName;
};

#endif // HVCONTROLER_H
