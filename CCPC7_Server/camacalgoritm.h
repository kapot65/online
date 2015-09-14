#ifndef CAMACALGORITM_H
#define CAMACALGORITM_H

#include <QObject>
#include <QThread>
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

#include "camacserversettings.h"
#include <tcpprotocol.h>
#include <event.h>
#include <ccpccommands.h>

/*!
 * \brief Класс содержит алгоритмы для работы с камаком.
 */
class CamacAlgoritm : public CCPCCommands, public QObject
{
    Q_OBJECT
public:
    explicit CamacAlgoritm(QObject *parent = 0);

protected slots:
    /*!
     * \brief Слот, вызываемый при ручной остановке набора.
     * Выставляет параметр CamacAlgoritm::breakFlag.
     */
    void on_breakAcquisition();

    /*!
     * \brief Ручная запись в MADC и чтение из нее.
     * \warning Функция для тестирования.
     */
    void testMADC();

signals:
    /*!
     * \brief Испускается в процессе набора точки.
     * \param Текущее количество набраных событий.
     */
    void currentEventCount(int count);

#ifdef VIRTUAL_MODE
    /*!
     * \brief Испускается при ручной остановке сбора.
     * \note работает только в режиме VIRTUAL_MODE.
     */
    void breakAcquisition();
#endif

protected:
    ///Указатель на класс для взаимодействия с устройством.
    ccpc::CamacImplCCPC7 *camac;

    ///Указатель на сенеджер настроек сервера.
    CamacServerSettings *settings;

    /*!
     * \brief Проведение набора точки, с заданным временем набора.
     * \param measureTime Код времени набора.
     * \param [out] manuallyBreak Флаг прерывания программы пользователем.
     * \return Вектор набранных событий в формате Event.
     */
    QVector<Event> acquirePoint(unsigned short measureTime, bool *manuallyBreak = NULL);

    /*!
     * \brief Обнуление каунтеров.
     */
    void resetCounters();


    /*!
     * \brief Получение значения на каунтере.
     * \param counterNum Номер каунтера (1 или 2).
     * \param channelNum Номер канала на каунтере (0 - 3).
     * \param withReset Обнулить каунтер после считывания.
     * \return Значение на каунтере.
     */
    unsigned int getCounterValue(int counterNum, int channelNum, bool withReset);


private:
    /*!
     * \brief Флаг ручного прерывания алгоритма.
     */
    bool breakFlag;

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
     * \brief Отключение возможности измерений.
     * \details Выполняет на блоке MADC команду с параметрами A = 0, F = 12.
     */
    void disableMeasurement();

    /*!
     * \brief Включение возможности измерений.
     * \details Выполняет на блоке MADC команду с параметрами A = 0, F = 11.
     */
    void enableMeasurement();

    /*!
     * \brief Пишет событие в MADC.
     * \param [in] data Канал события.
     * \param [in] time Время события.
     * \warning Функция нигде не используется. Возможно она не работает.
     */
    void writeMADC(unsigned short &data, long &time);

    /*!
     * \brief Считывает время и канал событие с MADC.
     * \details Считывает 12 бит данных, 28 бит времени, 1 бит валидности и прибавляет инкремент к адресу.
     * \param [out] data Канал события.
     * \param [out] time Время события.
     * \param [out] valid Валидность события.
     */
    void readMADC(unsigned short &data, long &time, bool &valid);

    /*!
     * \brief Считывает канал события с MADC.
     * \details Считывает 12 бит данных,1 бит валидности и прибавляет инкремент к адресу.
     * \param [out] data Канал события.
     * \param [out] time Время события.
     * \param [out] valid Валидность события.
     * \warning Функция нигде не используется. Возможно она не работает.
     */
    void readMADCData(unsigned short &data, long &time, bool &valid);

    /*!
     * \brief Устанавливает адрес (18 бит) и время набора в MADC.
     * \param [in] addr Адрес.
     * \param [in] measureTime Код времени измерений. Берется из TcpProtocol::getAviableMeasuteTimes.
     */
    void setMADCAddr(long &addr, unsigned short &measureTime);

    /*!
     * \brief Считывает текущий адрес, флаг переполнения и флаг окончания измерений с MADC.
     * \param [out] addr Адрес.
     * \param [out] addrOverflow Флаг переполнения адреса.
     * \param [out] endOfMeasurement Флаг окончания измерений.
     */
    void getMADCAddr(long &addr, bool &addrOverflow, bool &endOfMeasurement);//GET ADDRESS, ADDRESS-OVERFLOW AND END-OF-MEASURE BITS

    //вспомогательные функции
    /*!
     * \brief Доступные времена сбора.
     * Создается с помощью TcpProtocol::getAviableMeasuteTimes
     */
    QMap<int, unsigned short> aviableMeasureTimes;
};

#endif // CAMACALGORITM_H
