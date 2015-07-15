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

class HVHandlerForm : public QWidget
{
    Q_OBJECT

public:
    explicit HVHandlerForm(HVHandler *hvHandler, IniManager *settingsManager,
                           DataVisualizerForm *dataVisualizerForm, QWidget *parent = 0);
    ~HVHandlerForm();

    //bool haveError(){return haveError;}
    //bool haveWarning(){return haveWarning;}
signals:

    void sendTextOutput(QByteArray text);
    void sendGraphicOutput(QVector<int> hist);

    void sendTestJsonMessage(QByteArray message);

    void sendPointToSave(QByteArray data);

    void sendReady();
    void sendError(QString error);
    void sendWarning(QString warning);

    void serverSettingsChanged();

    /*
    public slots:
    void receiveTestJson(QByteArray message);

    private slots:
    void processMessage(QByteArray message);
    */


private slots:
    void on_ipEdit_editingFinished();

    void on_portEdit_editingFinished();

    void on_reconnectButton_clicked();

    void on_serverSettingsChanged();

    void on_setHVButton_clicked();

    void on_getHVButton_clicked();

    void on_voltageBox_editingFinished();

    void on_monitorVoltmeterBox_clicked(bool checked);
    void on_monitorCheckVoltage();
    void on_monitorDrawVoltage(QVariantMap message);

private:
    IniManager *settingsManager;

    DataVisualizerForm *dataVisualizerForm;
    int currPointNum;

    bool haveWarning;

    HVHandler *hvHandler;

    Ui::HVHandlerForm *ui;
};

#endif // HVHANDLERFORM_H
