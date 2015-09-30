#ifndef CCPCCOMMANDS_H
#define CCPCCOMMANDS_H

#include <QObject>
#include <QByteArray>
#include <QtEndian>
#include <QFile>
#include <QTime>
#include <qmath.h>

#ifdef TEST_MODE
    #include <QDebug>
#endif

#include <QTimer>
#include <QEventLoop>

#include <easylogging++.h>
#include <ccpc7.h>

#include <tcpprotocol.h>
#include <event.h>

///\brief Класс содержит алгоритмы для работы с камаком.
class CCPCCommands
{
public:
    /*!
     * \warning В Конструктор не создает CCPCCommands::camac
     * Его создание и удаление должно быть проведено классах-наследниках.
     */
    explicit CCPCCommands();

    ~CCPCCommands();

protected:
    ///Указатель на класс для взаимодействия с устройством.
    ccpc::CamacImplCCPC7 *camac;

    /*!
     * \brief Ожидание.
     * \param Время ожидания в милисекундах.
     */
    void waitMSec(int msec);

    /*!
     * \brief Проведения цикла C
     */
    void C();

    /*!
     * \brief Проведение цикла Z
     */
    void Z();

    /*!
     * \brief Проведение операции NAF с 16-битными словами
     * \param n N
     * \param a A
     * \param f F
     * \param [in, out]data Данные.
     * \return Результат операции.
     */
    ccpc::CamacOp NAF(int n, int a, int f, unsigned short &data);

    /*!
     * \brief Проведение операции с 24-битными словами
     * \param n N
     * \param a A
     * \param f F
     * \param [in, out]data Данные.
     * \param use24bit Использовать передачу 24 бит.
     * \return Результат операции.
     */
    ccpc::CamacOp NAF(int n, int a, int f, long &data, bool use24bit = true);

private:
#if QT_VERSION >= 0x040800
    /*!
     * \brief Таймер для установки задержек.
     */
    QTimer *timer;
    /*!
     * \brief QEventLoop для ожидания во время задержек.
     */
    QEventLoop *eventLoop;
#endif

    /*!
     * \brief Выдает значение бита в байте.
     * \param Рассматриваемый байт.
     * \param Позиция бита в байте.
     * \return Значение бита.
     */
    static bool checkBit(unsigned short var, int pos){return (var) & (1<<(pos));}

    /*!
     * \brief Изменяет значение бита в байте.
     * \param Рассматриваемый байт.
     * \param Позиция заменяемого бита в байте.
     * \param Значение заменяемого бита.
     */
    static void replaceBit(long &var, int pos, bool bit);

    /*!
     * \brief Изменяет значение бита в байте.
     * \param Рассматриваемый байт.
     * \param Позиция заменяемого бита в байте.
     * \param Значение заменяемого бита.
     */
    static void replaceBit(unsigned short &var, int pos, bool bit);
};

#endif // CCPCCOMMANDS_H
