#ifndef HVSERVERDIALOG_H
#define HVSERVERDIALOG_H

#include <QObject>
#include <tcpserver.h>
#include <inimanager.h>
#include "hvserver.h"
#include <iostream>
#include <easylogging++.h>


/*!
 * \brief Обработчик HVServer. Создает HVServer и транслирует в консоль его
 * сообщения.
 */
class HVServerHandler : public QObject
{
    Q_OBJECT

public:
    explicit HVServerHandler(QObject *parent = 0);
    ~HVServerHandler();

private slots:
    /*!
     * \brief Вызывается при готовности сервера. Пишет в лог на каком порте
     * и по какому адресу работает сервер.
     * \param ip Адрес сервера.
     * \param port Порт.
     */
    void on_ServerReady(QString ip, int port);

    /*!
     * \brief Вызывается при новом подключении. Пишет в лог сообщение о новом подключении.
     * \todo Разобраться с аргументами.
     */
    void on_NewConnection(QString ip, int port);

private:
    /*!
     * \brief Указатель на менеджер настроек.
     */
    IniManager *manager;

    /*!
     * \brief Указатель на интерфейс сервера.
     */
    HVServer *hvServer;
};

#endif // HVSERVERDIALOG_H
