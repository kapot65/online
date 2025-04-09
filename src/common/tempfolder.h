#ifndef TEMPFOLDER_H
#define TEMPFOLDER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
/*!
 * \brief Класс для обработки временной папки с данными. Имеет методы для
 * очистки папки до определенного размера удалением самых старых файлов.
 */
class TempFolder : public QObject
{
    Q_OBJECT
public:
    explicit TempFolder(QString folderName = "temp", int maxSizeMBytes = 500, QObject *parent = 0);
    ~TempFolder();
    /*!
     * \brief Возвращает путь в временной папке.
     * \return Путь к временной папке.
     */
    QString getFolderPath();

signals:

public slots:
    /*!
     * \brief Очищает папку до максимума размера, удаляя самые старые файлы.
     */
    void clear();

private:
    /*!
     * \brief Максимальный размер папки.
     */
    int maxSizeMBytes;

    /*!
     * \brief Относительный путь к временной папке.
     */
    QString folderName;

    /*!
     * \brief Флаг успешности иницализации временной папки.
     */
    bool inited;

    /*!
     * \brief Возвращает размер директории.
     * Взято из https://supportforums.blackberry.com/t5/Native-Development/How-can-I-get-a-Directory-Size-using-QFileInfo/m-p/2741851#M57506
     * \param str Путь к директории.
     * \return Размер в байтах.
     */
    quint64 dir_size(const QString &str);
};

#endif // TEMPFOLDER_H
