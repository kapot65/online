#ifndef CAMACSERVERDIALOG_H
#define CAMACSERVERDIALOG_H

#include "camacserver.h"
#include <tempfolder.h>
#include <stdio.h>

class CamacServerHandler : public QObject
{
    Q_OBJECT

public:
    explicit CamacServerHandler(TempFolder *tempFolder, QObject *parent = 0);
    ~CamacServerHandler();

private slots:
    void showMessage(QByteArray message);
    void showMessage(QString message);
    void showNewConnection(QString peerName, int peerPort);
    void onServerReady(QString ip, int port);

private:
    QString toDebug(const QByteArray & line);
    CamacServer *server;
};

#endif // CAMACSERVERDIALOG_H
