#include "box.h"

Box::Box(int boxIndex, QObject *parent) : QObject(parent), boxIndex(boxIndex)
{
    load();

    connect(tempTimer,&QTimer::timeout,this,&Box::tempTimeout);
    connect(humiTimer,&QTimer::timeout,this,&Box::humiTimeout);
    connect(newDataTimer,&QTimer::timeout,this,&Box::sendNewData);

    connect(this,&Box::settingChanged,this,&Box::save);
}

double Box::getTemp() const
{
    return temp;
}

void Box::setTemp(double value, QString time)
{
    qDebug()<<"box temp: " << value;
    temp = value + tempCalib;
    isOneInDataTemp = true;
    isTemp = true;
    newTemp = true;
    valueTime = time;

    processData();

    tempTimer->start(timeoutNoData);
}

double Box::getHumi() const
{
    return humi;
}

void Box::setHumi(double value, QString time)
{
    qDebug()<<"box humi: " << value;
    humi = value + humiCalib;
    isOneInDataHumi = true;
    isHumi = true;
    newHumi = true;
    valueTime = time;

    processData();

    humiTimer->start(timeoutNoData);
}

void Box::setTimeout(int value)
{
    timeoutNoData = value * 1000;
}

bool Box::getIsTemp() const
{
    return isTemp;
}

bool Box::getIsHumi() const
{
    return isHumi;
}

QString Box::getName() const
{
    return name;
}

void Box::setName(const QString &value)
{
    name = value;

    emit settingChanged(boxIndex);
}

int Box::getAdrSensorTemp() const
{
    return adrSensorTemp;
}

int Box::getAdrSensorHumi() const
{
    return adrSensorHumi;
}

void Box::setTempAddress(int adr)
{
    adrSensorTemp = adr;

    emit settingChanged(boxIndex);
}

void Box::setHumiAddress(int adr)
{
    adrSensorHumi = adr;

    emit settingChanged(boxIndex);
}

double Box::getTempMax() const
{
    return tempMax;
}

void Box::setTempMax(double value)
{
    tempMax = value;

    defineError();

    emit settingChanged(boxIndex);
}

double Box::getTempMin() const
{
    return tempMin;
}

void Box::setTempMin(double value)
{
    tempMin = value;

    defineError();

    emit settingChanged(boxIndex);
}

double Box::getHumiMax() const
{
    return humiMax;
}

void Box::setHumiMax(double value)
{
    humiMax = value;

    defineError();

    emit settingChanged(boxIndex);
}

double Box::getHumiMin() const
{
    return humiMin;
}

void Box::setHumiMin(double value)
{
    humiMin = value;

    defineError();

    emit settingChanged(boxIndex);
}

bool Box::getEnabled() const
{
    return enabled;
}

void Box::setEnabled(bool value)
{
    if(enabled == value) return;

    enabled = value;

    if(enabled)
    {
        isTemp = true;
        isHumi = true;

        tempTimer->start(timeoutNoData);
        humiTimer->start(timeoutNoData);
    }
    else
    {
        tempTimer->stop();
        humiTimer->stop();

        defineError();
    }
}

QDateTime Box::getEnableDate() const
{
    return enableDate;
}

void Box::setEnableDate(const QDateTime &value)
{
    enableDate = value;
}

QString Box::getValueTime() const
{
    return valueTime;
}

bool Box::getEnabledErrorTempMax() const
{
    return enabledErrorTempMax;
}

void Box::setEnabledErrorTempMax(bool value)
{
    enabledErrorTempMax = value;

    defineError();
}

bool Box::getEnabledErrorTempMin() const
{
    return enabledErrorTempMin;
}

void Box::setEnabledErrorTempMin(bool value)
{
    enabledErrorTempMin = value;

    defineError();
}

bool Box::getEnabledErrorHumiMax() const
{
    return enabledErrorHumiMax;
}

void Box::setEnabledErrorHumiMax(bool value)
{
    enabledErrorHumiMax = value;

    defineError();
}

bool Box::getEnabledErrorHumiMin() const
{
    return enabledErrorHumiMin;
}

void Box::setEnabledErrorHumiMin(bool value)
{
    enabledErrorHumiMin = value;

    defineError();
}

bool Box::getEnabledErrorData() const
{
    return enabledErrorData;
}

void Box::setEnabledErrorData(bool value)
{
    enabledErrorData = value;

    defineError();
}

double Box::getTempCalib() const
{
    return tempCalib;
}

void Box::setTempCalib(double value)
{
    tempCalib = value;

    emit settingChanged(boxIndex);
}

double Box::getHumiCalib() const
{
    return humiCalib;
}

void Box::setHumiCalib(double value)
{
    humiCalib = value;

    emit settingChanged(boxIndex);
}

void Box::processData()
{
    if(newHumi && newTemp)
    {
        sendNewData();
        newDataTimer->stop();
        return;
    }

    if(!newDataTimer->isActive())
        newDataTimer->start(timeoutNewData);
}

void Box::defineError()
{
    if(!enabled)
    {
        sendError(false);

        return;
    }

    if(!isTemp || !isHumi)
    {
        if(enabledErrorData)
        {
            sendError(true);
        }
        else
        {
            sendError(false);
        }

        return;
    }


    if(isOneInDataTemp)
    {
        if(enabledErrorTempMax)
            if(temp > tempMax)
            {
                sendError(true);
                return;
            }


        if(enabledErrorTempMin)
            if(temp < tempMin)
            {
                sendError(true);
                return;
            }
    }

    if(isOneInDataHumi)
    {
        if(enabledErrorHumiMax)
            if(humi > humiMax)
                if(temp < tempMin)
                {
                    sendError(true);
                    return;
                }

        if(enabledErrorHumiMin)
            if(humi < humiMin)
            {
                sendError(true);
                return;
            }
    }

    sendError(false);
}

void Box::sendError(bool isErr)
{
    if(isErr)
    {
        if(!error)
        {
            error = true;
            emit errorBox(boxIndex);
        }
    }
    else
    {
        if(error)
        {
            error = false;
            emit noErrorBox(boxIndex);
        }
    }
}

void Box::save()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup(QString("Box%0").arg(boxIndex));

    settings.setValue("name", name);

    settings.setValue("tempAddress", adrSensorTemp);
    settings.setValue("humiAddress", adrSensorHumi);

    settings.setValue("tempMax", tempMax);
    settings.setValue("tempMin", tempMin);
    settings.setValue("humiMax", humiMax);
    settings.setValue("humiMin", humiMin);
    settings.setValue("tempCalib", tempCalib);
    settings.setValue("humiCalib", humiCalib);

    settings.endGroup();
}

void Box::load()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup(QString("Box%0").arg(boxIndex));

    name = settings.value("name","no name").toString();

    adrSensorTemp = settings.value("tempAddress", -1).toInt();
    adrSensorHumi = settings.value("humiAddress", -1).toInt();

    tempMax = settings.value("tempMax", 38.0).toDouble();
    tempMin = settings.value("tempMin", 37.4).toDouble();
    humiMax = settings.value("humiMax", 60.0).toDouble();
    humiMin = settings.value("humiMin", 50.0).toDouble();
    tempCalib = settings.value("tempCalib", 0.0).toDouble();
    humiCalib = settings.value("humiCalib", 0.0).toDouble();

    settings.endGroup();
}

void Box::tempTimeout()
{
    isTemp = false;

    defineError();

    emit noTemp(boxIndex);
}

void Box::humiTimeout()
{
    isHumi = false;

    defineError();

    emit noHumi(boxIndex);
}

void Box::sendNewData()
{
    emit allData(boxIndex);

    defineError();

    newTemp = false;
    newHumi = false;
}
