#ifndef CAMACSERVERDIALOG_H
#define CAMACSERVERDIALOG_H

#include "camacserver.h"
#include <tempfolder.h>
#include <stdio.h>

/*!
 * \brief Обработчик CamacServer. Создает CamacServer и транслирует в консоль его
 * сообщения.
 */
class CamacServerHandler : public QObject
{
    Q_OBJECT

public:
    /*!
     * \param tempFolder Обработчик временной папки.
     */
    explicit CamacServerHandler(TempFolder *tempFolder, QObject *parent = 0);
    ~CamacServerHandler();

private slots:
    /*!
     * \brief Вывод сообщения в консоль.
     * \param message Сообщение.
     */
    void showMessage(QByteArray message);

    /*!
     * \brief Вывод сообщения в консоль.
     * \param message Текст сообщения.
     */
    void showMessage(QString message);

    /*!
     * \brief Вывод в консоль сообщения о новом подключении.
     * Связан с CamacServer::newConnection
     * \todo Разобраться с аргументами.
     */
    void showNewConnection(QString peerName, int peerPort);

    /*!
     * \brief Вывод в консоль сообщения о готовности сервера.
     * \param ip Адрес сервера.
     * \param port Порт.
     */
    void onServerReady(QString ip, int port);

private:
    ///Указатель на обрабатываемый сервер.
    CamacServer *server;
};

#endif // CAMACSERVERDIALOG_H
