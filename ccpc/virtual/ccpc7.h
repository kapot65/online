#include <QTcpSocket>
#include <QNetworkSession>
#include <tcpprotocol.h>
#include "tcpserver.h"
#include "ccpc7base.h"
#include <tcpprotocol.h>

#ifndef CCPC7
#define CCPC7
namespace ccpc {

/*!
 * \brief Виртуальный экземпляр устройства CCPC.
 * \details Класс создает соединение с программой CCPC7_Server, находящейся на камаке
 * и с передачи команд NAF, удаленно выполняет алгоритм.
 * Класс нужен для тестирования алгоритма.
 * \warning Класс сильно устарел.
 * \warning Класс имеет отличие с оригинальным API при инициализации: для иниализации виртуального устройства
 * необходимо дополнительно вызывать функцию CamacImplCCPC7::init()
 * \warning Класс не имеет никаких проверок соединения и обработчиков ошибок.
 * \todo Обновить класс. Поменять базу на TcpClient.
 */
class CamacImplCCPC7 : public QObject
{
    Q_OBJECT
    public:
    /*!
     * \brief Создает соединение с сервером.
     * \param ip Адрес машины, на которой выполняется CCPC7_Server.
     * \param host Порт, который прослушивает CCPC7_Server.
     */
    explicit CamacImplCCPC7(QString ip, int host, QObject *parent = 0);
    ~CamacImplCCPC7();

    /*!
     * \brief Инициализация устройства.
     */
    void init();

    //virtual void reset(){}
    /*!
     * \brief Выполнить операцию камак.
     * \param op
     */
    virtual void exec(CamacOp &op);

    /*!
     * \brief Выдает флаг о наличии ошибок.
     * \return Флаг наличия ошибок.
     */
    bool have_errors() {return errorCounter;}
    //virtual bool inf1() const;
    //virtual bool inf2() const;
    //virtual bool lam(int n) const;
    //virtual void readStatus(){}
    //virtual void setOut(bool out1, bool out2){}

  signals:
    /*!
     * \brief Испускается при появлении ошибки в соединении.
     * \param error Текстовое описание ошибки.
     */
    void have_error(QString error);

    /*!
     * \brief Испусикается при успешном подключении.
     */
    void connected();

  private slots:
    void sessionOpened();

    /*!
     * \brief Присоединен к сигналу QTcpSocket::error
     */
    void processError();

    /*!
     * \brief Испускает сигнал CamacImplCCPC7::connected после подключения.
     */
    void onConnected();

  private:
    ///\brief Класс сокета.
    QTcpSocket *connection;

    ///\brief Класс сеанса.
    QNetworkSession *networkSession;

    ///\brief Текущее количество ошибок.
    int errorCounter;

    ///\brief Текст последней ошибки.
    QString error;
};
}
#endif //CCPC7

