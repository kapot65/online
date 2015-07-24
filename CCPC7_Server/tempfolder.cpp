#include "tempfolder.h"

TempFolder::TempFolder(QString folderName, int maxSizeMBytes, QObject *parent) : QObject(parent)
{
    if(folderName.isEmpty())
        folderName = "temp";

    this->maxSizeMBytes = maxSizeMBytes;
    this->folderName = folderName;

    //попытка создать путь два раза
    inited = QDir().mkpath(getFolderPath());
    if(!inited)
    inited = QDir().mkpath(getFolderPath());
    if(!inited)
        return;
}

TempFolder::~TempFolder()
{

}

quint64 TempFolder::dir_size(const QString & str)
{
    quint64 sizex = 0;
    QFileInfo str_info(str);
    if (str_info.isDir())
    {
        QDir dir(str);
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                    sizex += dir_size(fileInfo.absoluteFilePath());
            }
            else
                sizex += fileInfo.size();

        }
    }
    return sizex;
}

void TempFolder::clear()
{
    QString folderPath = getFolderPath();
    quint64 size = dir_size(folderPath);

    if(size < maxSizeMBytes * 1024 * 1024)
        return;

    QDir tempDir(folderPath);

    QFileInfoList files = tempDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Time | QDir::Reversed);

    for(int i = 0; i < files.size(); i++)
    {
        if(size < maxSizeMBytes * 1024 * 1024)
            return;

        quint64 curr_size;

        if(files[i].isDir())
        {
            curr_size = dir_size(files[i].absoluteFilePath());
            QDir().rmpath(files[i].absoluteFilePath());
        }
        else
            if(files[i].isFile())
            {
                curr_size = files[i].size();
                QFile::remove(files[i].absoluteFilePath());
            }
        size -= curr_size;
    }
}

