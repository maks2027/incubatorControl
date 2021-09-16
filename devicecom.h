#ifndef DEVICECOM_H
#define DEVICECOM_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include "TypeData.h"
#include <QVariant>
#include <QTimer>
#include <QDebug>

class DeviceCom : public QObject
{
    Q_OBJECT
public:
    explicit DeviceCom(int ind, QObject *parent = nullptr);


public slots:
    void setNewSettings(deviceData values);
    void setPoolTimer(int value);
    //void setConnect(bool val);

signals:
    void mesege(int ind, QString msg);
    void newData(sensorData data);

private slots:    
    void workBus();
    void reconnect();
    void onStateChanged(int state);
    void errorOccurred(QModbusDevice::Error error);
    void readReady();

    void startPool();
    void startScan();


private:
    QModbusRtuSerialMaster *modbusDevice;    
    QTimer *busTimer = new QTimer(this);
    QTimer *busPoolTimer = new QTimer(this);

    int index = -1;
    int currentAddress = 1;
    int currentIndex = 0;
    bool request = false;
    bool isScan = false;
    bool isPool = false;
    bool toScan = false;
    bool toPool = false;
    int poolTime = 10000;
    bool onConnect = false;

    QVector<int> addresses;

    void runScan();
    void stopScan();

    void runPool();
    void stopPool();

    void prepareRead(int adr);
};

#endif // DEVICECOM_H
