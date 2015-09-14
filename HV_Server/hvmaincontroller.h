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
    void setVoltage(double voltage);

private:
    /// \brief Адрес блока управления вольтметром.
    int controllerId;

    /// \brief Флаг успешной инициализации
    bool initSuccesfullFlag;
};

#endif // HVMAINCONTROLLER_H
