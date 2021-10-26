#ifndef BOX_H
#define BOX_H

#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QSettings>
#include "TypeData.h"

class Box : public QObject
{
    Q_OBJECT

public:
    explicit Box(int boxIndex/* = -1*/,QObject *parent = nullptr);

    double getTemp() const;
    void setTemp(double value, QString time);

    double getHumi() const;
    void setHumi(double value, QString time);

    void setTimeout(int value);

    bool getIsTemp() const;

    bool getIsHumi() const;

    QString getName() const;
    void setName(const QString &value);

    int getAdrSensorTemp() const;

    int getAdrSensorHumi() const;

    void setTempAddress(int adr);    

    void setHumiAddress(int adr);    

    double getTempMax() const;
    void setTempMax(double value);

    double getTempMin() const;
    void setTempMin(double value);

    double getHumiMax() const;
    void setHumiMax(double value);

    double getHumiMin() const;
    void setHumiMin(double value);

    bool getEnabled() const;
    void setEnabled(bool value);

    QDateTime getEnableDate() const;

    void setEnableDate(const QDateTime &value);

    QString getValueTime() const;

    bool getEnabledErrorTempMax() const;
    void setEnabledErrorTempMax(bool value);

    bool getEnabledErrorTempMin() const;
    void setEnabledErrorTempMin(bool value);

    bool getEnabledErrorHumiMax() const;
    void setEnabledErrorHumiMax(bool value);

    bool getEnabledErrorHumiMin() const;
    void setEnabledErrorHumiMin(bool value);

    bool getEnabledErrorData() const;
    void setEnabledErrorData(bool value);

    double getTempCalib() const;
    void setTempCalib(double value);

    double getHumiCalib() const;
    void setHumiCalib(double value);

private:
    int boxIndex = -1;

    //приходили ли хоть раз данные
    bool isOneInDataTemp = false;
    bool isOneInDataHumi = false;

    //данные свежие
    bool isTemp = false;
    bool isHumi = false;

    //новые данные
    bool newTemp = false;
    bool newHumi = false;

    QString name;

    double tempMax = 37.9;
    double tempMin = 37.5;
    int adrSensorTemp;

    double humiMax = 60.0;
    double humiMin = 50.0;
    int adrSensorHumi;

    QString valueTime;

    QDateTime enableDate;
    bool enabled = false;

    bool enabledErrorTempMax = true;
    bool enabledErrorTempMin = true;
    bool enabledErrorHumiMax = true;
    bool enabledErrorHumiMin = true;
    bool enabledErrorData = true;

    bool tempErrorMax = false;
    bool tempErrorMin = false;
    bool humiErrorMax = false;
    bool humiErrorMin = false;

    bool error = false;

    //////////////////////////////////
    double temp = 0.0;
    double humi = 0.0;

    double tempCalib = 0.0;
    double humiCalib = 0.0;

    QTimer *tempTimer = new QTimer(this);
    QTimer *humiTimer = new QTimer(this);
    QTimer *newDataTimer = new QTimer(this);
    int timeoutNoData = 10000;
    int timeoutNewData = 3000;

    void processData();

    void defineError();
    void sendError(bool isErr);


    void load();
private slots:
    void tempTimeout();
    void humiTimeout();

    void sendNewData();

    void save();

signals:

    void noTemp(int index);
    void noHumi(int index);

    void errorBox(int index);
    void noErrorBox(int index);


    void allData(int index);

    void settingChanged(int index);

};

#endif // BOX_H
