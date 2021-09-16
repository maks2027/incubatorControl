#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCloseEvent>
#include <QWidget>
#include <QTableWidgetItem>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QDebug>
#include "savesettings.h"



namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

    SaveSettings &AllSettings;

public:
    explicit Settings(SaveSettings &_AllSettings,QWidget *parent = nullptr);
    ~Settings();

public slots:
    void createTableBox();
    void createTableSensors();
    void createTableBoxSensors();    
    void createTableDevice();
    int createTableDeviceItem();


    void updateTableSensors(int i);
    void updateTableBox(int i);
    void updateTableBoxSensors(int i);

    void changetDevices();

    void removeTableDeviceItem(int index);

    void fillPortsInfo();    

    void deviseStatus(int index, QString msg);

private slots:

    void on_tableWidget_boxes_itemChanged(QTableWidgetItem *item);

    void on_pushButtonPassword_clicked();

    void on_pushButton_pathReport_clicked();


    void on_tableWidget_boxes_sensors_itemChanged(QTableWidgetItem *item);
    void on_checkBox_errorTempMax_clicked(bool checked);
    void on_checkBox_errorTempMin_clicked(bool checked);
    void on_checkBox_errorHumiMax_clicked(bool checked);
    void on_checkBox_errorHumiMin_clicked(bool checked);
    void on_checkBox_errorData_clicked(bool checked);

    void on_pushButton_addDevice_clicked();
    void on_pushButton_remove_clicked();

    void on_time_valueChanged(int arg1);

    void on_spinBox_poll_valueChanged(int arg1);

private:
    Ui::Settings *ui;

    QTimer *updateCom;

    QFileDialog* openReportDialog;

    int freeComIndex = 0;

    void sendValue(int row);

    int indexToRow(int index);

protected:
    void closeEvent(QCloseEvent *event);

signals:
 void changeCom(deviceData);
 void removeCom(int);

};

#endif // SETTINGS_H
