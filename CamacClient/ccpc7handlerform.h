#ifndef CCPC7HANDLERFORM_H
#define CCPC7HANDLERFORM_H

#include <QWidget>
#include <QMessageBox>
#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include <QFileDialog>
#include <QTextStream>
#include <easylogging++.h>
#include <inimanager.h>
#include <event.h>
#include <tcpclient.h>
#include <QTimer>
#include "ccpc7handler.h"
#include <datavisualizerform.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

namespace Ui {
class CCPC7HandlerForm;
}

/*!
 * \brief Виджет для CCPC7Handler.
 * \details Класс дает возможность ручного управления сервером CCPC7.
 */
class CCPC7HandlerForm : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief CCPC7HandlerForm
     * \param ccpc7Handler
     * \param settingsManager
     * \param graphViewer Указатель на визуализатор данных
     * \param parent
     */
    explicit CCPC7HandlerForm(CCPC7Handler *ccpc7Handler, DataVisualizerForm *dataVisualizerForm,
                              IniManager *settingsManager, QWidget *parent = 0);
    ~CCPC7HandlerForm();
    /*!
     * \brief метод для ручной проверки наличия предупреждения
     * \details Предупреждение может быть вызвано изменением
     * парметров без переподключения к камаку
     */
    bool ccpc7HaveWarning(){return haveWarning;}

signals:
    /*!
     * \brief Сигнал посылает предупреждение.
     * \details Сигнал также связан с формой.
     * При появлении сигнлала текст предупрежедния пишется на форму
     * \param warning текст предупреждения
     */
    void sendWarning(QString warning);

    /*!
     * \brief Сигнал вырабатывается при изменении параметров на форме.
     */
    void serverSettingsChanged();

private slots:
    void on_camacIpEdit_editingFinished();
    void on_camacPortEdit_editingFinished();
    void on_acquirePointButton_clicked();
    void on_reconnectButton_clicked();
    void on_serverSettingsChanged();
    void on_writeCountersButton_clicked(bool checked);
    void on_countersFileEdit_editingFinished();
    void on_reloadCountersBox_editingFinished();
    void on_countersFileSelect_clicked();
    void on_monitorReceiveCountersValue(QVariantMap message);

//    /*!
//     * \brief drawAcquiredPoint
//     * \param meta
//     * \param events
//     * \details Создает новый график для отрисованной точки. После создания графика
//     * отключается от сигнала CCPC7Handler::pointAcquired
//     */
//    void drawAcquiredPoint(MachineHeader machineHeader, QVariantMap meta, QVector<Event> events);

    /*!
     * \brief Слот вызывается при нажатии на кнопу сбора информации с каунтеров
     * \details Метод считывает информацию о каунтеров (номера каунтеров и каналы) и
     * вызывает сигнал CCPC7Handler::getCountersValue
     */
    void sendGetCountersValue();

private:
    /*!
     * \brief указатель на обработчик CCPC7
     * \details указывается в конструкторе
     */
    CCPC7Handler *ccpc7Handler;

    /*!
     * \brief флаг наличия ошибки
     */
    bool haveWarning;

    /*!
     * \brief countersFile
     * \warining тестовый объект
     * \todo убрать
     */
    QFile *countersFile;

    /*!
     * \brief менеджер настроек
     */
    IniManager *settingsManager;

    /*!
     * \brief Указатель на визуализатор данных
     */
    DataVisualizerForm *dataVisualizerForm;

//    /*!
//     * \brief указатели на текущие графики каунтеров
//     * \details пока не используются (возможно и не будут)
//     */
//    GraphUnit *currCountersGraphs[8];

    Ui::CCPC7HandlerForm *ui;
};

#endif // CCPC7HANDLERFORM_H
