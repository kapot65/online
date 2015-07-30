#ifndef DIVIDERREADER_H
#define DIVIDERREADER_H

#include "comport.h"
#include <QObject>

/*!
 * \brief Интерфейс взаимодействия с вольтметром <a href="http://www.keysight.com/en/pd-1000001295:epsg:pro-pn-34401A/digital-multimeter-6-digit?&cc=RU&lc=rus">Aligent 34401A</a>
 * через COM порт.
 */
class DividerReader : public ComPort
{
    Q_OBJECT
public:
    /*!
     * \details При неправильном создании интерфейса в лог выведутся ошибки создания.
     * \param DividerName Имя вольтметра. Используется для загрузки настроек из ini файла настроек.
     * \param manager Менеджер настроек.
     */
    explicit DividerReader(QString DividerName, IniManager *manager, QObject *parent = 0);
    ~DividerReader();

    /*!
     * \brief Проверяет, занят ли интерфейс.
     * \return Флаг занятости.
     */
    bool checkBusy(){return busy;}

    /*!
     * \brief Проверяет, инициализирован ли вольтметр.
     * \return Флаг инициализации.
     */
    bool checkInited(){return inited;}

signals:
    /*!
     * \brief Испускается, когда вольтметр заканчивает писать ответ. Конец ответа
     * определяется наличием символов "\r\n" на конце.
     */
    void receiveFinished();

    /*!
     * \brief Вольтметр прошел инициализацию.
     */
    void initVoltmeterDone();

    /*!
     * \brief Текущее напряжение измерено.
     * \param voltage Текущее напряжение.
     */
    void getVoltageDone(double voltage);

public slots:
    /*!
     * \brief Проводит инициализацию вольтметра.
     * \todo Добавить дополнительные проверки.
     */
    void initVoltmeter();

    /*!
     * \brief Снимает текущее напряжение. По окончании испускает сигнал DividerReader::getVoltageDone
     * \warning Процесс снятия напряжения занимает 3 - 5 секунд.
     */
    void getVoltage();

protected slots:
    /*!
     * \brief Читает сообщение с порта. После каждого прочитанного сообщения
     * испускает сигнал DividerReader::receiveFinished
     */
    virtual void readMessage();

private:
    /*!
     * \brief Флаг текущей занятости интерфейса.
     */
    bool busy;

    /*!
     * \brief Флаг инициализации вольтметра.
     */
    bool inited;

    /*!
     * \brief Коэфициент делителя.
     */
    double dividerNormCoeff;

    /*!
     * \brief Текущее сообщение с порта.
     * \warning Является законченным, только после испускания сигнала DividerReader::receiveFinished.
     */
    QByteArray curr_data;
};

#endif // DIVIDERREADER_H
