#ifndef HVCONTROLER_H
#define HVCONTROLER_H

#include "comport.h"

/*!
 * \brief The HVControler class
 * Класс, реализующий общение с ICP CON модели I-7021P через Com порт.
 * Описание протокола общения: http://ftp.icpdas.com/pub/cd/8000cd/napdos/7000/manual/7021_22_24.pdf
 */
class HVControler : public ComPort
{
    Q_OBJECT
public:
    /*!
     * \brief Загружает настройки и открывает порт.
     * \param manager Менеджер настроек.
     * \param controllerName Имя контролера HV. Используется при загрузке настроек из ini файла.
     * \param voltage [out] Переменная, в которую будет записываться настоящее напряжение на блоке.
     * Напряжения вычисляются в HVServer::onDivider1GetVoltageDone и HVServer::onDivider2GetVoltageDone соответственно.
     * \param ok Успешность создания класса. Если происходят ошибки при создании - они записываются в лог.
     */
    explicit HVControler(IniManager *manager, QString controllerName, double* voltage, bool *ok = 0,  QObject *parent = 0);
    ~HVControler();

signals:
    /*!
     * \brief Установка напряжения закончена.
     * \todo Добавить проверку установленного напряжения.
     */
    void setVoltageDone();

    /*!
     * \brief Сигнал испускается функцией HVControler::setVoltageAndCheck.
     * \param status Ответ. Содержит тэги:
     * - "status" - "ok" - Если напряжение установлено,
     *              "timeout" - если напряжение не установилось.
     *              "bad_params" - в функцию поданы неверные аргументы.
     * - "error" - Разница между фактическим напряжением и установленным в вольтах. Этого тега нету в случае "bad_params".
     * - "description" - Описание ошибки. Поле присутствует только в случае "bad_params".
     */
    void voltageSetAndCheckDone(QVariantMap status);

    void startCorrect();

public slots:
    /*!
     * \brief Устанавливает напряжение. Если напряжение выходит за границы
     * диапазона - устанавливает граничное значение и записывает предупреждение в лог.
     * Диапазоны устанавливаются в ini файле в полях "minTreshold" и "maxTreshold".
     * \param voltage Напряжение в вольтах.
     * \todo Добавить вывод ошибок.
     */
    virtual void setVoltage(double voltage);

    /*!
     * \brief Выставить напряжение на блоке и проверить установку напряжения.
     * \param params Аргументы. Поле должног содержать тэги:
     * - "voltage" Выставляемое напряжение в вольтах.
     * - "max_error" Допустимое отклонение в вольтах.
     * - "timeout" Таймаут в секундах.
     * \return По окончании работы функция испускает сигнал HVControler::voltageSetAndCheckDone.
     * \todo Нужен флаг busy?
     */
    void setVoltageAndCheck(QVariantMap params);

protected:
    virtual void run();

    /*!
     * \brief Загружает настройки из ini файла.
     * \param controllerName Имя HVControler.
     * \param manager Указатель на менеджер настроек.
     * \return Успешность загрузки параметров.
     */
    bool loadSettings(QString controllerName, IniManager *manager);

    /*!
     * \brief Метод проверяет файл настроек на наличие поля setting во вкладке
     * controllerName. Если поля нет, то метод пытается остановить приложение.
     * \param setting Название параметра.
     * \param controllerName Имя HVControler.
     */
    void processSettingError(QString setting, QString controllerName);

    /*!
     * \brief Коэффициент преобразования при свободном члене.
     * Используется в функции HVControler::setVoltage
     */
    double c0;

    /*!
     * \brief Коэффициент преобразования при линейном члене.
     * Используется в функции HVControler::setVoltage
     */
    double c1;

    /*!
     * \brief Минимальное возможное установочное значение.
     * Используется в функции HVControler::setVoltage
     */
    double minTreshold;

    /*!
     * \brief Максимальное возможное установочное значение.
     * Используется в функции HVControler::setVoltage
     */
    double maxTreshold;

    /*!
     * \brief Имя COM порта, к которому подключен контроллер.
     */
    QString portName;

    /*!
     * \brief Указатель на переменную, содержащую настоящее напряжение на блоке. Т.е. снятое с соответсвующего делителя и
     * умноженное на коэффициент.
     */
    double* actualVoltage;

    /// \brief Установленное пользователем напряжение
    double settedVoltage;

    /// \brief Название контролера.
    QString controllerName;

    /// \brief Максимально возможная величина корректирующего смещения в вольтах.
    double maxCorrection;

    /*!
     * \brief коэффициент затухания поправки при коррекции напряжения. Должен быть в пределах [0, 1]
     */
    double correctionCoefficient;

    /*!
     * \brief Стоп-флаг для цикла коррекции напряжения
     */
    bool stopFlag;

protected:
    double lastCorrectionVoltage;

    // ComPort interface
protected slots:
    void readMessage();

    virtual void setVoltageShift(double voltage);

    /*!
     * \brief Коррекция напряжения
     */
    virtual void correctVoltage();
};

#endif // HVCONTROLER_H
