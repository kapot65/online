#include <QTcpSocket>
#include <QNetworkSession>
#include <QJson/Serializer>
#include "tcpserver.h"
#include "ccpc7base.h"
#include "tcpprotocol.h"

#ifndef CCPC7
#define CCPC7
namespace ccpc {

class CamacImplCCPC7 : public QObject
{
    Q_OBJECT
    public:
    explicit CamacImplCCPC7(QString ip, int host, QObject *parent = 0);
    ~CamacImplCCPC7();

    void init();

    //virtual void reset(){}
    virtual void exec(CamacOp &op);
    bool have_errors() {return errorCounter;}
    //virtual bool inf1() const;
    //virtual bool inf2() const;
    //virtual bool lam(int n) const;
    //virtual void readStatus(){}
    //virtual void setOut(bool out1, bool out2){}

  signals:
    void have_error(QString error);
    void connected();

  private slots:
    void sessionOpened();
    void processError();
    void onConnected();

  private:
    QTcpSocket *tcpSocket;
    QNetworkSession *networkSession;

    int errorCounter;

    QString error;
};
}
#endif //CCPC7

