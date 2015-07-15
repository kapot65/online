#ifndef HVSERVERDIALOG_H
#define HVSERVERDIALOG_H

#include <QObject>
#include <tcpserver.h>
#include <inimanager.h>
#include "hvserver.h"
#include <iostream>
#include <easylogging++.h>


class HVServerHandler : public QObject
{
    Q_OBJECT

public:
    explicit HVServerHandler(QObject *parent = 0);
    ~HVServerHandler();

private slots:
    void on_ServerReady(QString ip, int port);
    void on_NewConnection(QString ip, int port);

private:
    IniManager *manager;
    HVServer *hvServer;
};

#endif // HVSERVERDIALOG_H
