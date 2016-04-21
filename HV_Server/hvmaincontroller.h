#ifndef HVMAINCONTROLLER_H
#define HVMAINCONTROLLER_H

#include "hvcontroler.h"
#include <ccpccommands.h>


/*!
 * \brief HVControler с функцией управления основным блоком через ccpc.
 */
class HvMainController : public HVControler, public CCPCCommands
{
    Q_OBJECT
public:
    HvMainController(IniManager *manager, QString controllerName, double *voltage, bool *ok = 0,  QObject *parent = 0);

    ~HvMainController();

public slots:
    virtual void setVoltage(double voltage);

protected slots:
    /*!
     * \brief Установка смещения через com port.
     * \param voltage Напряжение в вольтах.
     */
    virtual void setVoltageShift(double voltage);

    void setVoltageBase(double voltage, bool &ok);

    /*!
     * \brief Здесь происходит коррекция напряжения в цикле
     */
    virtual void correctVoltage();

private:
    void setVoltage(double voltage, bool &ok);

    /// \brief Адрес блока управления вольтметром.
    int controllerId;

    /// \brief Флаг успешной инициализации
    bool initSuccesfullFlag;

    ///
    /// \brief Перевод напряжения в формат, распознаваемый вольтметром.
    /// \param voltage Исходное напряжение.
    /// \return Код напряжения.
    /// \note Напряжение перед этой функцией нужно перевести в единицы измерения вольтметра.
    /// \todo Добавить граничные значения в конфигурационный файл.
    long encodeVoltage(double voltage);

    /*!
     * \brief Коэффициент преобразования при свободном члене для управления напряжением
     * через ccpc.
     */
    double a0;

    /*!
     * \brief Коэффициент преобразования при линейном члене для управления напряжением
     * через ccpc.
     */
    double a1;

    /// \brief Часть от общего напряжения, которая будет задаваться через com порт, а не через ЦАП.
    /// Эта величина нужна для возможности проводить отрицательную коррекцию напряжения.
    double initialShift;

};

#endif // HVMAINCONTROLLER_H
