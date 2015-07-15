#ifndef OEMSTRING_H
#define OEMSTRING_H

#include <qstring.h>
#include <qobject.h>

class OEMStrings: public QObject {
public:
  QString name();
  QString release_date();
  QString copyright();
  QString address();
  QString www();
};

#endif /* OEMSTRING_H */
