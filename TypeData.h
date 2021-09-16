#ifndef TYPEDATA_H
#define TYPEDATA_H

#include <QString>
#include <QVector>
#include <QDateTime>

struct adrInd
{
    adrInd(){}
    adrInd(int address,int index):address(address),index(index){}

    int address = -1;
    int index = -1;

    bool operator== (const adrInd &a)
    {
        return (a.index == this->index && a.address == this->address);
    }
};

enum typeSensor
{
    Temp,
    Humi,
    Volt
};

struct sensorData
{    
    int adr;
    typeSensor type;
    QDateTime date;

    double value = 0;
};

struct deviceData
{
    int index;
    QString comName;
    int speed;
    int dataBit;
    int stopBit;

    bool enable;
};

#endif // TYPEDATA_H
