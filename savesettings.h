#ifndef SAVESETTINGS_H
#define SAVESETTINGS_H

#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QSqlQuery>
#include <QtSql>
#include <QSound>

#include "box.h"
#include "devicespull.h"

struct inValue
{
    inValue(){};
    inValue(int boxIndex, double temp, double humi, QString time)
        : boxIndex(boxIndex),
          temp(temp),
          humi(humi),
          time(time){}

    int boxIndex;
    double temp;
    double humi;
    QString time;
};

class SaveSettings : public QObject
{
    Q_OBJECT

    QTimer *dayTimer = nullptr;
    QTimer *hourTimer = nullptr;
    QTimer *writeTimer = nullptr;

    QSound *sound;
    QThread *thread;

    int cointErrors = 0;

    //отключение полностью звука по событию
    bool soundEnableMaxTemp = true;
    bool soundEnableMinTemp = true;
    bool soundEnableMaxHumi = true;
    bool soundEnableMinHumi = true;
    bool soundEnableErr = true;

    const QString bdName = "history.db";

    QVector<Box*> boxes;
    QVector<sensorData> sensors;
    QVector<inValue> valueBuffer;

    int timeError = 15;
    int timePool = 10;

    QString password;
    QString pathReport;

    void createDb();
    void createBoxes();
    void readSettingsBd();

public:
    explicit SaveSettings(QObject *parent = nullptr);
    ~SaveSettings();

    DevicesPull *readData;

    QString dbPath;

    void setTime(int value);//таймаут
    int getTime() const;

    //данные боксов
    int sizeBoxes();
    const Box *getBox(int index);
    QString getName(int index);

    //настройки бокса
    void setName(int index,QString name);
    void setTempAddress(int i,int address);
    void setHumiAddress(int i,int address);
    void setTempMin(int index,double value);
    void setTempMax(int index,double value);
    void setHumiMin(int index,double value);
    void setHumiMax(int index,double value);

    void setActive(int index,bool active);
    void setActiveAll(bool active);
    void setDateTime(int index, QDateTime val);


    int sizeSensors();
    const sensorData& getSensor(int index);

    //сокхранение в файл
    void save();
    void read();


    //звук
    bool getSoundEnableMaxTemp() const;
    void setSoundEnableMaxTemp(bool value);

    bool getSoundEnableMinTemp() const;
    void setSoundEnableMinTemp(bool value);

    bool getSoundEnableMaxHumi() const;
    void setSoundEnableMaxHumi(bool value);

    bool getSoundEnableMinHumi() const;
    void setSoundEnableMinHumi(bool value);

    bool getSoundEnableErr() const;
    void setSoundEnableErr(bool value);

    void pauseMaxSound(int time);
    void pauseMinSound(int time);
    void pauseErrSound(int time);

    QString getPassword() const;
    void setPassword(const QString &value);

    QString getPathReport() const;
    void setPathReport(const QString &value);

    int getTimePool() const;
    void setTimePool(int value);

public slots:
    void setValue(sensorData s);


private slots:
    void dayTimeout();
    void hourTimeout();


    void writeBuffer();
    void addBufferBox(int i);

    void boxError(int index);
    void noBoxError(int index);

private:
    //int getIndexSenderTime(QTimer *timer);
    void removeSensor(int address, typeSensor type);

signals:
    void settingsChanged(int index);
    void inEmptyAddress(int index);
    void sensorRemoved();

    void boxActive(bool active);
    void isDay();
    void isHour();

    void valueChanged(int index);
    void sensorChanged(int index);

    void comIsConnect();
    void comIsClose();
    void comIsError();


    void noDataBox(int indexBox);//не пришли данные

    void mesege(int ind, QString msg);

    void changeCom(deviceData);
    void removeCom(int);

    void setPoolTimer(int value);
};

#endif // SAVESETTINGS_H
