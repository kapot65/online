#include "camacserverhandler.h"
#include "camacserversettings.h"

CamacServerHandler::CamacServerHandler(TempFolder *tempFolder, QObject *parent) :
    QObject(parent)
{
    CamacServerSettings *settings = new CamacServerSettings(this);

    connect(settings, SIGNAL(error(QString)), this, SLOT(showMessage(QString)));

    if(settings->loadSettings(qApp->applicationDirPath() + "/CamacServerSettings.ini"))
    {
        emit showMessage(QString("Start canceled. Please fix problems and restart server"));
        return;
    }
    settings->setSettingaValue("tempFolder", "dirPath", tempFolder->getFolderPath());

    server = new CamacServer(settings->getSettingsValue("CamacServer","port").toInt(), settings);

    connect(server, SIGNAL(serverReady(QString,int)), this, SLOT(onServerReady(QString,int)));

    connect(server, SIGNAL(serverError(QString)), this, SLOT(showMessage(QString)));
    connect(server, SIGNAL(newConnection(QString,int)), this, SLOT(showNewConnection(QString,int)));
    }

void CamacServerHandler::onServerReady(QString ip, int port)
{
    std::printf(tr("The server is running on IP: %1 port: %2\n")
           .arg(ip).arg(port).toStdString().c_str());
}

CamacServerHandler::~CamacServerHandler()
{
    delete server;
}

QString CamacServerHandler::toDebug(const QByteArray & line)
{

    QString s;
    uchar c;

    for ( int i=0 ; i < line.size() ; i++ ){
        c = line[i];
        if ( c >= 0x20 and c <= 126 ) {
            s.append(c);
        } else {
            s.append(QString("<%1>").arg(c, 2, 16, QChar('0')));
        }
    }
    return s;
}

void CamacServerHandler::showMessage(QByteArray message)
{
    printf((toDebug(message) + "\n").toStdString().c_str());
}

void CamacServerHandler::showNewConnection(QString peerName, int peerPort)
{
    printf(QString("new connection: %1 %2\n").arg(peerName).arg(peerPort).toStdString().c_str());
}

void CamacServerHandler::showMessage(QString message)
{
    printf((message + "\n").toStdString().c_str());
}
