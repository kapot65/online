#ifndef HVSERVER_H
#define HVSERVER_H
#include <tcpserver.h>
#include <inimanager.h>
#include <tcpprotocol.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <easylogging++.h>
#include "dividerreader.h"
#include "hvcontroler.h"

/*!
 * \brief Сервер для компьютера HV. Класс представляет собой оболочку для общения с вольтметрами
 * и контролерами напряжения на уровне функций и сигналов. Класс принимает сообщения и исполняет
 * запросы из них.
 * \todo Объединить нумерованные переменные в массивы.
 * \todo Добавление проверки вольтметров при инициализации.
 * \todo Добавить обработку ошибок портов.
 */
class HVServer : public TcpServer
{
    Q_OBJECT
public:
    /*!
     * \brief Создает Все интерфейсы устройств.
     * \warning Конструткор не проводит инициализацию устройств.
     * Для работы с устройтвом необходимо его инициализировать.
     * \param manager Менеджер настроек.
     * \param port Порт, который будет прослушивать сервер.
     */
    HVServer(IniManager *manager, int port, QObject *parent = 0);
    ~HVServer();

signals:
    /*!
     * \brief Присоединен к divider1->initVoltmeter.
     */
    void initDivider1();

    /*!
     * \brief Присоединен к divider1->getVoltage.
     */
    void getDivider1Voltage();

    /*!
     * \brief Присоединен к hvControlerDivider1->setVoltage.
     */
    void setDivider1Voltage(double voltage);

    /*!
     * \brief Присоединен к divider2->initVoltmeter.
     */
    void initDivider2();

    /*!
     * \brief Присоединен к divider2->getVoltage.
     */
    void getDivider2Voltage();

    /*!
     * \brief Присоединен к hvControlerDivider2->setVoltage.
     */
    void setDivider2Voltage(double voltage);

private slots:
    /*!
    * \brief Обработка сообщения. В этом методе происходит деление сообщений
    * на команды и ответы. Если сообщение не подходит ни под один из критериев, отправителю посылается
    * сообщение с ошибкой анализа сообщения. Далее, команды обрабатываются методом HVServer::processCommand, а
    * ответы - методом HVServer::processReply.
    * \param header Машинный заголовок сообщения.
    * \param meta Метаданные сообщения.
    * \param data Бинарные данные сообщения.
    */
   void processMessage(MachineHeader header, QVariantMap meta, QByteArray data);

   void onDivider1GetVoltageDone(double voltage);
   void onInitDivider1Done();
   void onDivider1SetVoltageDone();
   void onDivider2SetVoltageDone();

   void onDivider2GetVoltageDone(double voltage);
   void onInitDivider2Done();

private:
   void processCommand(QVariantMap message);
   void processReply(QVariantMap message);

   void dividerGetVoltageDone(QString dividerName, double voltage);
   void initDividerDone(QString dividerName);
   void dividerSetVoltageDone(QString dividerName);

   void sendBusyMessage(QString block);
   void sendInitError(QString block);
   void sendUnknownBlockError(QString block);
   void sendUnknownCommandError(QString commandType);

   IniManager *manager;
   DividerReader *divider1;
   DividerReader *divider2;
   HVControler *hvControlerDivider1;
   HVControler *hvControlerDivider2;
};

#endif // HVSERVER_H
