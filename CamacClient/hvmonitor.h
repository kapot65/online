#ifndef HVMONITOR_H
#define HVMONITOR_H

#include <tcpprotocol.h>
#include "hvhandler.h"


/*!
 * \brief Класс для считывания напряжения HV в отдельном потоке.
 * \warning В классе нету проверок вольтметров и таймаутов.
 * \todo Добавить таймауты и улучшить устойчивость к ошибкам.
 * \details Класс, в отдельном потоке отсылает через HVHandler запросы на считывание
 * напряжения и обрабатывает их. За каждую итерацию он отправляет 2 запроса на каждый
 * вольтметр и ждет пока с них обоих не придут ответы. После прихода ответов итерация
 * повторяется. Также класс реагирует на ошибки и ждет, пока они исправятся.
 */
class HVMonitor : public QThread
{
    Q_OBJECT
public:
    /*!
     * \brief Конструктор класса.
     * \param subFolder Подпапка куда будет писаться файл с напряжением.
     * Файл будет назваться "voltage"
     * \param hvHandler
     */
    HVMonitor(QString subFolder, HVHandler *hvHandler);
    void stopMonitoring(){emit stop();}

private slots:
    /*!
     * \brief Сохраняет в файл полученное напряжение.
     * \warning перед сохранением, необходимо подготовить файл
     * методом HVMonitor::prepareVoltageFile.
     * \param message Сообщение, присланное с HVHandler.
     */
    void saveCurrentVoltage(QVariantMap message);

    /*!
     * \brief Слот, вызывающийся перед закрытием потока.
     * Производит остановку основного цикла опроса.
     * \warning Этот слот только устанваливает флаг на остановку.
     * Остановку следует определять по сигналу QThread::finished.
     */
    void beforeClose();

signals:
    void stop();

protected:
    /*!
     * \brief Основной цикл опроса вольтметров.
     */
    virtual void run();

private:
    /*!
     * \brief Путь к подпапке, указанный при создании класса.
     */
    QString subFolder;

    /*!
     * \brief Указатель на HVHandler, указанный при создании класса.
     */
    HVHandler *hvHandler;

    /*!
     * \brief Создает и подготавливает файл для записи напряжения.
     * \param type Формат записи данных о напряжении.
     */
    void prepareVoltageFile(BINARYTYPE type = HV_TEXT_BINARY);

    /*!
     * \brief Закрывает файл с напряжением. Используется в слоте HVMonitor::beforeClose
     */
    void closeVoltageFile();

    /*!
     * \brief insertVoltageBinary
     * \details записывает напряжение в блоки в бинарном виде
     * \param message
     */
    void insertVoltageBinary(QVariantMap &message);

    /*!
     * \brief insertVoltageBinary
     * \details записывает напряжение в блоки в текстовом виде
     * \param message
     */
    void insertVoltageText(QVariantMap &message);

    /*!
     * \brief Машинный заголовок для файла с напряжением.
     */
    MachineHeader hvFileMachineHeader;

    /*!
     * \brief Тип данных, в котором будет записываться напряжение.
     */
    BINARYTYPE binaryType;

    /*!
     * \brief Текущий файл с напряжением.
     */
    QFile* voltageFile;
};

#endif // HVMONITOR_H
