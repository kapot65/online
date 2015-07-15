#ifndef HVHANDLER_H
#define HVHANDLER_H

#include <QObject>
#include <QEventLoop>
#include "serverhandler.h"
#include <inimanager.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

class HVHandler : public ServerHandler
{
    Q_OBJECT
public:
    explicit HVHandler(QString ip = QString(), int port = -1, QObject *parent = 0);

signals:
    void getVoltageDone(QVariantMap meta);
    void setVoltageDone(QVariantMap meta);

#ifdef TEST_MODE
    void sendTestJsonMessage(QByteArray message);
#endif

public slots:
    virtual void initServer();
    void setVoltage(int block, double value);
    void getVoltage(int block);

protected slots:
    virtual void processMessage(MachineHeader machineHeader, QVariantMap metaData, QByteArray binaryData);
};

#endif // HVHANDLER_H
