#ifndef CAMACSERVERSETTINGS_H
#define CAMACSERVERSETTINGS_H

#include <QObject>
#include <inimanager.h>
#include <easylogging++.h>

/*!
 * \brief Класс для работы с настройками сервера.
 * Обеспечивает предварительный парсинг настроек из ini файла
 * и быстрый доступ к ним.
 */
class CamacServerSettings : public IniManager
{
    Q_OBJECT
public:
    explicit CamacServerSettings(QString settingsFile, QObject *parent = 0);
    ~CamacServerSettings();

    //проверяет наличие необходимых настроек
    /*!
     * \brief Загружает настройки из ini файла и осуществляет предварительный парсинг.
     * Если каких-то настроек нету, то функция испустит сигнал CamacServerSettings::error c описанием
     * недостающих настроек.
     * \return true - все настройки найдены,  0 - некоторых настроек не хватает.
     */
    bool loadSettings();

    /*!
     * \brief Возвращает положение блока ADC_CRATE.
     * \return Положение блока ADC_CRATE.
     */
    int getADC_CRATE(){ return ADC_CRATE; }

    /*!
     * \brief Возвращает положение блока MADC.
     * \return Положение блока MADC.
     */
    int getMADC(){ return MADC; }

    /*!
     * \brief Возвращает положение блока TG1.
     * \return Положение блока TG1.
     */
    int getTG1(){ return TG1; }

    /*!
     * \brief Возвращает положение блока OV1.
     * \return Положение блока OV1.
     */
    int getOV1(){ return OV1; }

    /*!
     * \brief Возвращает положение блока TTL_NIM.
     * \return Положение блока TTL_NIM.
     */
    int getTTL_NIM(){ return TTL_NIM; }

    /*!
     * \brief Возвращает положение блока COUNTER1.
     * \return Положение блока COUNTER1.
     */
    int getCOUNTER1(){ return COUNTER1; }

    /*!
     * \brief Возвращает положение блока COUNTER2.
     * \return Положение блока COUNTER2.
     */
    int getCOUNTER2(){ return COUNTER2; }

    /*!
     * \brief Возвращает положение блока TERMINAL1.
     * \return Положение блока TERMINAL1.
     */
    int getTERMINAL1(){ return TERMINAL1; }

    /*!
     * \brief Возвращает положение блока TERMINAL2.
     * \return Положение блока TERMINAL2.
     */
    int getTERMINAL2(){ return TERMINAL2; }

#if __cplusplus == 201103L
    el::Level getLogLevel(){return logLevel;}
#else
    //easyloggingpp::Level getLogLevel(){return;}
#endif

signals:
    /*!
     * \brief Сигнал испускается при ошибке парсинга.
     * \param err Текстовое описание ошибки.
     */
    void error(QString err);

private:
    /*!
     * \brief Проверяет наличие поля в файле с настройкми, и если его нету -
     * дополняет сообщение об ошибке соотвествующей записью.
     * \param group Группа настроек, к которой содержится поле.
     * \param field Название проверяемого поля.
     * \param [in, out] err Сообщение об ошибке.
     */
    void checkAndWriteErr(QString group, QString field, QString &err);

    ///индекс ADC_CRATE
    int ADC_CRATE;

    ///индекс MADC
    int MADC;

    ///индекс TG1
    int TG1;

    ///индекс OV1
    int OV1;

    ///индекс TTL_NIM
    int TTL_NIM;

    ///индекс COUNTER1
    int COUNTER1;

    ///индекс COUNTER2
    int COUNTER2;

    ///индекс TERMINAL1
    int TERMINAL1;

    ///индекс TERMINAL2
    int TERMINAL2;

#if __cplusplus == 201103L
    el::Level logLevel;
#else
    //easyloggingpp::Level logLevel;
#endif

};

#endif // CAMACSERVERSETTINGS_H
