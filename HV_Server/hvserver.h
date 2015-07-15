#ifndef HVSERVER_H
#define HVSERVER_H
#include <tcpserver.h>
#include <inimanager.h>
#include <tcpprotocol.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <easylogging++.h>
#include "dividerreader.h"
#include "hvcontroler.h"

class HVServer : public TcpServer
{
    Q_OBJECT
public:
    HVServer(IniManager *manager, int port, QObject *parent = 0);
    ~HVServer();

signals:
    void initDivider1();
    void getDivider1Voltage();
    void setDivider1Voltage(double voltage);

    void initDivider2();
    void getDivider2Voltage();
    void setDivider2Voltage(double voltage);

private slots:
   void processMessage(MachineHeader header, QVariantMap meta, QByteArray data);

   void onDivider1GetVoltageDone(double voltage);
   void onInitDivider1Done();
   void onDivider1SetVoltageDone();

   void onDivider2GetVoltageDone(double voltage);
   void onInitDivider2Done();

private:
   void processCommand(QVariantMap message);
   void processReply(QVariantMap message);

   void dividerGetVoltageDone(QString dividerName, double voltage);
   void initDividerDone(QString dividerName);
   void dividerSetVoltageDone(QString dividerName);

   void sendBusyMessage(QString block);
   void sendInitError(QString block);
   void sendUnknownBlockError(QString block);
   void sendUnknownCommandError(QString commandType);

   IniManager *manager;
   DividerReader *divider1;
   DividerReader *divider2;
   HVControler *hvControlerDivider1;
};

#endif // HVSERVER_H
