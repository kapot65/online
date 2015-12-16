#ifndef HVHANDLERFORM_H
#define HVHANDLERFORM_H

#include <QWidget>
#include <hvhandler.h>
#include <datavisualizerform.h>
#ifdef TEST_MODE
#include <QDebug>
#endif

#include <easylogging++.h>

namespace Ui {
class HVHandlerForm;
}

/*!
 * \brief Виджет для HVHandler.
 * \details Класс дает возможность ручного управления сервером HV.
 * \warning Визуализация этого виджета конфликтует с OnlineForm.
 */
class HVHandlerForm : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \param Указатель на обработчик сервера HV.
     * \param Менеджер настроек.
     * \param Указатель на визуализатор
     */
    explicit HVHandlerForm(HVHandler *hvHandler, IniManager *settingsManager,
                           DataVisualizerForm *dataVisualizerForm, QWidget *parent = 0);
    ~HVHandlerForm();

    //bool haveError(){return haveError;}
    //bool haveWarning(){return haveWarning;}
signals:
    /*!
     * \brief Испускается при появлении предупреждения.
     * \param warning Текст предупреждения.
     */
    void sendWarning(QString warning);

    /*!
     * \brief Испускается при изменениях параметров на интерфейсе.
     */
    void serverSettingsChanged();

private slots:
    void on_ipEdit_editingFinished();

    void on_portEdit_editingFinished();

    void on_reconnectButton_clicked();

    void on_serverSettingsChanged();

    void on_setHVButton_clicked();

    void on_getHVButton_clicked();

    void on_voltageBox_editingFinished();

    void on_block1Button_clicked(bool checked);

    void on_block2Button_clicked(bool checked);

private:
    ///\brief Менеджер настроек.
    IniManager *settingsManager;

    ///\brief Указатель на класс для визуализации данных.
    DataVisualizerForm *dataVisualizerForm;

    ///\brief Флаг предупреждения.
    bool haveWarning;

    ///\brief Указатель на обработчик сервера HV.
    HVHandler *hvHandler;

    Ui::HVHandlerForm *ui;
};

#endif // HVHANDLERFORM_H
