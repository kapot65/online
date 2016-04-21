
#include <qstring.h>
#include "oemstrings.h"

QString OEMStrings::name()
{
  return tr("Manual CAMAC Controller");
}

QString OEMStrings::release_date()
{
  QString tmp;
  tmp = tr("Compiled ");
  tmp += QString(__DATE__ __TIME__);
  return tmp;
}

QString OEMStrings::copyright()
{
  return tr("&copy; 2009 Ilja Slepnev");
}

QString OEMStrings::address()
{
  return tr("Joint Institute for Nuclear Research")+"<br>"+tr("Dubna, RUSSIA");
}

QString OEMStrings::www()
{
  return "http://afi.jinr.ru";
}
