#ifndef CAMACCLIENTFORM_H
#define CAMACCLIENTFORM_H

#include <QMainWindow>
#include <QSettings>

#include <easylogging++.h>
#include "tcpclient.h"
#include "datavisualizerform.h"
#include <event.h>
#include <inimanager.h>
#include "ccpc7handler.h"
#include "ccpc7handlerform.h"
#include "hvhandler.h"
#include "hvhandlerform.h"

#include "online.h"
#include "onlineform.h"

#ifdef TEST_MODE
#include <QDebug>
#endif

namespace Ui {
class CamacClientForm;
}
/*!
 * \brief Главное окно программы.
 */
class CamacClientForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit CamacClientForm(QWidget *parent = 0);
    ~CamacClientForm();

signals:
#ifdef TEST_MODE
    /*!
     * \brief sendTestCamacMessage
     * \warning тестовая функция
     * \param message
     */
    void sendTestCamacMessage(QByteArray message);
#endif

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:

#ifdef TEST_MODE
    /*!
     * \brief Показывает тестовый вывод в окошке с логом
     * \warning тестовая функция
     */
    void showTextOutput(QByteArray output);
#endif
//    void showGraphicOutput(QVector<int> hist);

    void on_testProgrammBox_clicked(bool checked);

    /*!
     * \brief camacMarkError
     * \details При вызове, слот окрасит язычок вкладки CCPC7 в красный цвет
    */
    void camacMarkError();
    /*!
     * \brief camacMarkWarning
     * \details При вызове, слот окрасит язычок вкладки CCPC7 в желтый цвет
     */
    void camacMarkWarning();
    /*!
     * \brief camacMarkReady
     * \details При вызове, слот окрасит язычок вкладки CCPC7 в зеленый цвет
     */
    void camacMarkReady();

    /*!
     * \brief HVMarkError
     * \details При вызове, слот окрасит язычок вкладки HV в красный цвет
     */
    void HVMarkError();
    /*!
     * \brief HVMarkWarning
     * \details При вызове, слот окрасит язычок вкладки CCPC7 в желтый цвет
     */
    void HVMarkWarning();
    /*!
     * \brief HVMarkReady
     * \details При вызове, слот окрасит язычок вкладки CCPC7 в зеленый цвет
     */
    void HVMarkReady();

    void on_clearLogButton_clicked();

private:
    /*!
     * \brief Восстанавливает настройки из ini файла
     */
    void restoreSettings();

    /*!
     * \brief Создает и натсраивает виджет с графиками
    */
    void setGraphWidget();
    /*!
     * \brief Устанавливает обработчик сервера СCPC7
     * \details Создает обработчика и устанавливает связи с главным окошком, а именно
     * CamacClientForm::camacMarkReady, CamacClientForm::camacMarkWarning,
     * CamacClientForm::camacMarkError
     * \warning Метод не подключает к серверу и не инициализирует CamacClientForm::ccpc7Handler
     */
    void setCCPC7Handler();
    /*!
     * \brief Настраивает виджет для обработки СCPC
     * \details Создает виджет и настраивает связи с CamacClientForm::ccpc7Handler
     * \warning Метод не подключает к серверу и не инициализирует CamacClientForm::ccpc7Handler
     */
    void setCCPC7HandlerForm();
    /*!
     * \brief Устанавливает обработчик сервера HV
     * \details Создает обработчика и устанавливает связи с главным окошком, а именно
     * CamacClientForm::HVMarkReady, CamacClientForm::HVMarkWarning,
     * CamacClientForm::HVMarkError
     * \warning Метод не подключает к серверу и не инициализирует CamacClientForm::hvHandler
     */
    void setHVHandler();
    /*!
     * \brief Настраивает виджет для обработки HV
     * \details Создает виджет и настраивает связи с CamacClientForm::hvHandler
     * \warning Метод не подключает к серверу и не инициализирует CamacClientForm::hvHandler
     */
    void setHVHandlerForm();

    /*!
     * \brief Объект для работы с настройками
     * \todo Заменить на обычный QSettings, т.к. IniManager использовать незачем
     */
    IniManager *manager;

    /*!
     * \brief обработчик сервера CCPC7
     */
    CCPC7Handler *ccpc7Handler;
    /*!
     * \brief виджет для обработчика сервера CCPC7
     */
    CCPC7HandlerForm *ccpc7HandlerForm;

    /*!
     * \brief обработчик сервера HV
    */
    HVHandler *hvHandler;
    /*!
     * \brief виджет для обработчика сервера HV
    */
    HVHandlerForm *hvHandlerForm;

//    /*!
//     * \brief виджет для графики
//    */
//    GraphViewer *graphViewer;

    DataVisualizerForm *dataVisualizerForm;

    Online *online;
    OnlineForm *onlineForm;

    Ui::CamacClientForm *ui;
};

#endif // CAMACCLIENTFORM_H
