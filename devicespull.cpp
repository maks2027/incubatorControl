#include "devicespull.h"

DevicesPull::DevicesPull(QObject *parent) : QObject(parent)
{

}

void DevicesPull::setDevise(deviceData data)
{
    if(data.comName == "none") return;

    if(devices.contains(data.index))
    {
        devices[data.index]->setNewSettings(data);
    }
    else
    {
        DeviceCom *dev = new DeviceCom(data.index);

        connect(dev, &DeviceCom::newData, this, &DevicesPull::newData);
        connect(dev, &DeviceCom::mesege, this, &DevicesPull::mesege);
        connect(this, &DevicesPull::setPoolTimer, dev, &DeviceCom::setPoolTimer);

        devices.insert(data.index, dev);
        devices[data.index]->setNewSettings(data);
    }
}

void DevicesPull::removeDevise(int index)
{
    if(devices.contains(index))
    {
        devices[index]->deleteLater();
        devices.remove(index);
    }
}
