#include "ccpccommands.h"

CCPCCommands::CCPCCommands()
{
#if QT_VERSION >= 0x040800
    timer = new QTimer;
    eventLoop = new QEventLoop;
    QObject::connect(timer, SIGNAL(timeout()), eventLoop, SLOT(quit()));
    QObject::connect(timer, SIGNAL(timeout()), timer, SLOT(stop()));
#endif
}

CCPCCommands::~CCPCCommands()
{
    timer->deleteLater();
    eventLoop->deleteLater();
}

void CCPCCommands::waitMSec(int msec)
{
    #if QT_VERSION >= 0x050300
            QThread::msleep(msec);
    #elif QT_VERSION >= 0x040800
        timer->start(msec);
        eventLoop->exec();
#endif
}

void CCPCCommands::C()
{
    LOG(INFO) << "Executing C cycle";

    ccpc::CamacOp op;
    op.mode = ccpc::Ccycle;
    camac->exec(op);
}

void CCPCCommands::Z()
{
    LOG(INFO) << "Executing Z cycle";

    ccpc::CamacOp op;
    op.mode = ccpc::Zcycle;
    camac->exec(op);
}

ccpc::CamacOp CCPCCommands::NAF(int n, int a, int f, unsigned short &data)
{
    ccpc::CamacOp op;
    op.n = n;
    op.a = a;
    op.f = f;

    op.mode = ccpc::Single;
    op.dir = (op.f>=16)?ccpc::DW16:ccpc::DR16;


    if(op.isWrite())
        op.data.push_back(data);


    camac->exec(op);

    if(op.data.size() != 0)
        data = op.data[0];
    return op;
}

void CCPCCommands::replaceBit(long &var, int pos, bool bit)
{
    //обнуление нужного бита
    var &= ~(1 << pos);
    //заполнение нужного бита
    if(bit)
        var |= (1 << pos);
}

void CCPCCommands::replaceBit(unsigned short &var, int pos, bool bit)
{
    //обнуление нужного бита
    var &= ~(1 << pos);
    //заполнение нужного бита
    if(bit)
        var |= (1 << pos);
}
