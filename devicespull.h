#ifndef DEVICESPULL_H
#define DEVICESPULL_H

#include <QObject>
#include "TypeData.h"
#include "devicecom.h"

class DevicesPull : public QObject
{
    Q_OBJECT
public:
    explicit DevicesPull(QObject *parent = nullptr);

public slots:
    void setDevise(deviceData data);
    void removeDevise(int index);

signals:
    void mesege(int ind, QString msg);
    void newData(sensorData data);
    void setPoolTimer(int value);

private:
    QHash<int, DeviceCom*> devices;
};

#endif // DEVICESPULL_H
