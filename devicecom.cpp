
#include "devicecom.h"

DeviceCom::DeviceCom(int ind, QObject *parent) : QObject(parent)
{
    index = ind;
    modbusDevice = new QModbusRtuSerialMaster(this);
    modbusDevice->setTimeout(100);

    connect(modbusDevice, &QModbusClient::errorOccurred, this, &DeviceCom::errorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &DeviceCom::onStateChanged);


    connect(busTimer,&QTimer::timeout,this,&DeviceCom::workBus);
    connect(busPoolTimer,&QTimer::timeout,this,&DeviceCom::startPool);

    busTimer->setSingleShot(false);
    busTimer->setInterval(100);
    busTimer->start();

    busPoolTimer->setSingleShot(true);
    busPoolTimer->setInterval(poolTime);
}

void DeviceCom::setNewSettings(deviceData values)
{
    if(!modbusDevice) return;

    if(modbusDevice->state() == QModbusDevice::ConnectedState)
        modbusDevice->disconnectDevice();

    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, values.comName);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, values.dataBit);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, values.speed);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, values.stopBit);

    onConnect = values.enable;

    reconnect();
}

void DeviceCom::onStateChanged(int state)
{
     if(state  == QModbusDevice::ConnectedState)
         emit mesege(index,"Подключён");
     else if(state  == QModbusDevice::UnconnectedState)
         emit mesege(index,"Отключён");
     if(state  == QModbusDevice::ConnectingState)
         emit mesege(index,"Подключение..");
     else if(state  == QModbusDevice::ClosingState)
         emit mesege(index,"Отключение..");
}

void DeviceCom::errorOccurred(QModbusDevice::Error error)
{
     if(error == QModbusDevice::ConnectionError || error == QModbusDevice::UnknownError)
     {
         emit mesege(index,"Ошибка");
         modbusDevice->disconnectDevice();
         void stopScan();
         void stopPool();
     }

    request = false;
}

void DeviceCom::readReady()//MODBUS SHT30/31
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    request = false;

    if (reply->error() == QModbusDevice::NoError)
    {
        if(isScan)
            if(!addresses.contains(reply->serverAddress()))
                addresses.append(reply->serverAddress());

        qDebug()<<"rep: "<< reply->serverAddress();

        const QModbusDataUnit unit = reply->result();

        for (uint i = 0; i < unit.valueCount(); i++)
        {
            sensorData data;

            if(i == 0)
                data.type = Humi;
            else if(i == 1)
                data.type = Temp;

            data.adr = reply->serverAddress();
            data.date = QDateTime::currentDateTime();
            data.value = (double)unit.value(i)/10.0;

            qDebug()<<"data: "<< data.value;


            emit newData(data);
        }
    }
    else if (reply->error() == QModbusDevice::ProtocolError)
    {

    }
    else
    {
        // Выводим сообщение об остальных типах ошибки используя объект reply->errorString();
        // В принципе, можно убрать столь подробное деление обработчика ошибок, оставив только лишь этот блок.
    }

    reply->deleteLater();
}

void DeviceCom::startPool()
{
    toPool = true;
}

void DeviceCom::startScan()
{    
    toScan = true;
}

void DeviceCom::workBus()
{    
    if(modbusDevice->state() != QModbusDevice::ConnectedState)//переподключение
    {
        reconnect();//запускает иил остонавливает таймер
        return;
    }   

    if(request)//ожидание ответа
        return;

    if(!isScan && !isPool)
    {
        if(toScan)
            runScan();
        else if(toPool)
            runPool();

        return;
    }

    if(isScan)//сканирование
    {
        prepareRead(currentAddress);
        currentAddress++;

        if(currentAddress == 100)//0-247
            stopScan();

        return;
    }    

    if(isPool)//опрос
    {
        if(currentIndex >= addresses.size())
        {
            stopPool();
            return;
        }

        prepareRead(addresses[currentIndex]);
        currentIndex++;

        return;
    }
}

void DeviceCom::reconnect()
{
    if(onConnect)
    {
        if(modbusDevice->state() != QModbusDevice::ConnectedState)
        {
            if(!modbusDevice->connectDevice())
            {

            }
            else
            {               
                startScan();
            }
        }

        busPoolTimer->start();
    }
    else
    {
        if(modbusDevice->state() == QModbusDevice::ConnectedState)
        {
            modbusDevice->disconnectDevice();           
        }

        busPoolTimer->stop();

        isPool = false;
        isScan = false;
    }
}

void DeviceCom::setPoolTimer(int value)
{
    poolTime = value * 1000;
    busPoolTimer->setInterval(poolTime);
}

void DeviceCom::runScan()
{
    emit mesege(index, "Сканирование..");

    addresses.clear();
    currentAddress = 1;
    isScan = true;
    toScan = false;
}

void DeviceCom::stopScan()
{
    if(modbusDevice->state() == QModbusDevice::ConnectedState)
        emit mesege(index, "Подключён");
    else
        emit mesege(index, "Отключён");

    isScan = false;    
}

void DeviceCom::runPool()
{
    //qDebug()<<"start pool";

    emit mesege(index, "Опрос..");

    busPoolTimer->start();
    isPool = true;
    toPool = false;
    currentIndex = 0;
}

void DeviceCom::stopPool()
{
    if(modbusDevice->state() == QModbusDevice::ConnectedState)
        emit mesege(index, "Подключён");
    else
        emit mesege(index, "Отключён");

    isPool = false;    
}

void DeviceCom::prepareRead(int adr)//MODBUS SHT30/31
{
    if(!modbusDevice) return;

    //qDebug()<<"send to " << adr;

    if(auto *lastRequest = modbusDevice->sendReadRequest(QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 2), adr))
    {
        if(!lastRequest->isFinished())
        {
            connect(lastRequest, &QModbusReply::finished, this, &DeviceCom::readReady);
            request = true;
        }
        else
        {
            delete lastRequest;
        }
    }
    else
    {

    }
}
