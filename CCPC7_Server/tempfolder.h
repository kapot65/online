#ifndef TEMPFOLDER_H
#define TEMPFOLDER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
/*!
 * \brief The TempFolder class
 * \details Класс для обработки временной папки с данными
 */
class TempFolder : public QObject
{
    Q_OBJECT
public:
    explicit TempFolder(QString folderName = "temp", int maxSizeMBytes = 500, QObject *parent = 0);
    ~TempFolder();
    /*!
     * \brief getFolderPath
     * Возвращает путь в временной папке.
     * \return
     */
    QString getFolderPath(){return QDir::homePath() + "/" + folderName;}

signals:

public slots:
    /*!
     * \brief clear
     * \details очищает папку до максимума размера, удаляя самые старые файлы.
     */
    void clear();

private:
    /*!
     * \brief максимальный размер папки
     */
    int maxSizeMBytes;

    /*!
     * \brief относительный путь к временной папке
     */
    QString folderName;

    /*!
     * \brief флаг успешности иницализации временной папки
     */
    bool inited;

    /*!
     * \brief dir_size
     * Возвращает размер директории.
     * Взято из https://supportforums.blackberry.com/t5/Native-Development/How-can-I-get-a-Directory-Size-using-QFileInfo/m-p/2741851#M57506
     * \param str
     * \return
     */
    quint64 dir_size(const QString &str);
};

#endif // TEMPFOLDER_H
