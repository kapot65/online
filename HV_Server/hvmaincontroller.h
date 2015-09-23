#ifndef HVMAINCONTROLLER_H
#define HVMAINCONTROLLER_H

#include "hvcontroler.h"
#include <ccpccommands.h>


/*!
 * \brief HVControler с функцией управления основным блоком через ccpc.
 */
class HvMainController : public HVControler, public CCPCCommands
{
public:
    HvMainController(IniManager *manager, QString controllerName, bool *ok = 0,  QObject *parent = 0);

public slots:
    virtual void setVoltage(double voltage);

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
};

#endif // HVMAINCONTROLLER_H
