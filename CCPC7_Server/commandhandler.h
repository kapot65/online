#ifndef COMMANDHANDLER
#define COMMANDHANDLER

#include <QThread>
#include <QDataStream>
#include <QVector>
#include <QEventLoop>
#include <stdlib.h>
#include <QCoreApplication>
#include <QJson/Parser>
#include <QJson/Serializer>
#include <QDate>
#include <QTime>

#include <ccpc7.h>

#include <tcpprotocol.h>
#include "camacalgoritm.h"
#include "camacserversettings.h"

class CommandHandler : public CamacAlgoritm
{
    Q_OBJECT
public:
    CommandHandler(CamacServerSettings *settings, QObject *parent = 0);
    bool checkBusy() {return busyFlag;}
    bool checkInit(); //если инициализация не пройдена, высылает ошибку

    static ccpc::CamacOp messageToCamacOp(QVariantMap message);
    static QVariantMap camacOpToMessage(ccpc::CamacOp op);

signals:
    void error(QVariantMap info);
    void breakAcquisition();
    void sendMessage(QVariantMap message, QByteArray binaryData);
    void sendRawMessage(QByteArray message);

public slots:
    void processInit(QVariantMap message);
    void processAcquirePoint(QVariantMap message);
    void processBreakAcquisition(QVariantMap message);
    void ProcessNAF(QVariantMap message);
    void processResetCounters(QVariantMap message);
    void processGetCountersValue(QVariantMap message);

private:
    QString tempFolder;
    bool busyFlag;
};


#endif // COMMANDHANDLER

