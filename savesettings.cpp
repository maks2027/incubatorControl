#include "savesettings.h"

SaveSettings::SaveSettings(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    readData = new DevicesPull;

    connect(thread, SIGNAL(finished()), readData, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    readData->moveToThread(thread);
    thread->start();

    connect(readData, &DevicesPull::newData, this, &SaveSettings::setValue);
    connect(readData, &DevicesPull::mesege, this, &SaveSettings::mesege);
    connect(this, &SaveSettings::changeCom, readData, &DevicesPull::setDevise);
    connect(this, &SaveSettings::removeCom, readData, &DevicesPull::removeDevise);
    connect(this, &SaveSettings::setPoolTimer, readData, &DevicesPull::setPoolTimer);

    dayTimer = new QTimer(this);
    dayTimer->setSingleShot(true);
    hourTimer = new QTimer(this);
    hourTimer->setSingleShot(true);
    writeTimer = new QTimer(this);
    writeTimer->start(2000);

    dbPath = QDir::current().path() + "/" + bdName;

    //порядок кретичен
    createBoxes();
    read();
    createDb();

    for(int i = 0; i < boxes.size(); i++)//костыль
    {
        boxes[i]->setTimeout(timeError);
        boxes[i]->setEnabledErrorTempMax(soundEnableMaxTemp);
        boxes[i]->setEnabledErrorTempMin(soundEnableMinTemp);
        boxes[i]->setEnabledErrorHumiMax(soundEnableMaxHumi);
        boxes[i]->setEnabledErrorHumiMin(soundEnableMinHumi);
        boxes[i]->setEnabledErrorData(soundEnableErr);
    }

    connect(writeTimer, SIGNAL(timeout()), this, SLOT(writeBuffer()));

    sound = new QSound(QDir::current().path() + "/error.wav",this);
    sound->setLoops(QSound::Infinite);


    dayTimeout();
    hourTimeout();
}

int SaveSettings::getTime() const
{
    return timeError;
}

void SaveSettings::setTime(int value)
{
    timeError = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setTimeout(timeError);
}

void SaveSettings::dayTimeout()
{
    QTime t(0,0,0);
    int secsTo = 86400 - t.secsTo(QTime::currentTime());
    if(secsTo < 10) secsTo = 86400;

    dayTimer->start(secsTo * 1000);

    emit isDay();
}

void SaveSettings::hourTimeout()
{
    QTime t = QTime::currentTime();
    int secsTo = ((60 - t.minute()) * 60) + (60 - t.second());
    if(secsTo < 10) secsTo = 3600;

    hourTimer->start(secsTo * 1000);

    emit isHour();
}

void SaveSettings::writeBuffer()
{
    if(valueBuffer.isEmpty()) return;

    qDebug()<< "Write bd, size buffer: " << valueBuffer.size();

    QSqlDatabase db = QSqlDatabase::database(dbPath);

    if(!db.transaction()) return;

    QSqlQuery query(db);

    for(int i = 0;i < valueBuffer.size();i++)
    {
        query.prepare(QString("INSERT INTO box_%0 (temp, humi, date) VALUES (:val2,:val3,:val4)").arg(valueBuffer[i].boxIndex));
        query.bindValue(":val2", valueBuffer[i].temp);
        query.bindValue(":val3", valueBuffer[i].humi);
        query.bindValue(":val4", valueBuffer[i].time);
        query.exec();
    }

    db.commit();

    valueBuffer.clear();
}

void SaveSettings::boxError(int index)
{
    cointErrors++;

    qDebug()<<"error: " << index;
    qDebug()<<"cointErrors: " << cointErrors;

    if(cointErrors > 0)
        sound->play();
}

void SaveSettings::noBoxError(int index)
{
    cointErrors--;

    qDebug()<<"error: " << index;
    qDebug()<<"cointErrors: " << cointErrors;

    if(cointErrors == 0)
        sound->stop();
}

void SaveSettings::removeSensor(int address, typeSensor type)
{
    for(int i = 0; i < sensors.size(); i++)
    {
        if(sensors[i].adr == address && sensors[i].type == type)
        {
            sensors.remove(i);
            emit sensorRemoved();
            return;
        }
    }
}

void SaveSettings::addBufferBox(int i)
{
    valueBuffer.append(inValue(i,boxes[i]->getTemp(), boxes[i]->getHumi(), boxes[i]->getValueTime()));
}

SaveSettings::~SaveSettings()
{
    writeBuffer();
    save();
}

int SaveSettings::sizeBoxes()
{
    return boxes.size();
}

const Box *SaveSettings::getBox(int index)
{
    return boxes[index];
}

void SaveSettings::setName(int index, QString name)
{
    if(boxes.size()<=index) return;

    boxes[index]->setName(name);

    QSqlDatabase db = QSqlDatabase::database(dbPath);

    QSqlQuery query(db);
    query.prepare(QString("UPDATE boxes SET name=:name WHERE id=:id"));
    query.bindValue(":id", index);
    query.bindValue(":name", name);
    query.exec();
}

void SaveSettings::setTempAddress(int i, int address)
{
    if(boxes.size() <= i) return;

    boxes[i]->setTempAddress(address);

    removeSensor(boxes[i]->getAdrSensorTemp(),Temp);
}

void SaveSettings::setHumiAddress(int i, int address)
{
    if(boxes.size() <= i) return;

    boxes[i]->setHumiAddress(address);

    removeSensor(boxes[i]->getAdrSensorHumi(),Humi);
}

void SaveSettings::setTempMin(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setTempMin(value);
}

void SaveSettings::setTempMax(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setTempMax(value);
}

void SaveSettings::setHumiMin(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setHumiMin(value);
}

void SaveSettings::setHumiMax(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setHumiMax(value);
}

void SaveSettings::setTempCalib(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setTempCalib(value);
}

void SaveSettings::setHumiCalib(int index, double value)
{
    if(boxes.size()<=index) return;

    boxes[index]->setHumiCalib(value);
}

void SaveSettings::setActive(int index, bool active)
{
    if(boxes.size()<=index) return;
    if(boxes[index]->getEnabled() == active) return;

    boxes[index]->setEnabled(active);
}

void SaveSettings::setValue(sensorData s)
{
    bool isBox = false;
    for(int i = 0; i < boxes.size(); i++)
    {
        if(boxes[i]->getAdrSensorTemp() == s.adr && s.type == Temp)
        {
            boxes[i]->setTemp(s.value, s.date.toString("yyyy-MM-dd hh:mm:ss"));
            isBox = true;
        }

        if(boxes[i]->getAdrSensorHumi() == s.adr && s.type == Humi)
        {
            boxes[i]->setHumi(s.value, s.date.toString("yyyy-MM-dd hh:mm:ss"));
            isBox = true;
        }
    }
    if(isBox) return;

    for(int i = 0;i < sensors.size(); i++ )
    {
        if(s.adr == sensors[i].adr && s.type == sensors[i].type)
        {
            sensors[i].value = s.value;

            emit sensorChanged(i);
            return;
        }
    }

    sensors.append(s);
    emit sensorChanged(sensors.size()-1);
}

void SaveSettings::setDateTime(int index, QDateTime val)
{
    if(boxes.size()<=index) return;

    boxes[index]->setEnableDate(val);
}

void SaveSettings::setActiveAll(bool active)
{
    for(int i = 0;i < boxes.size();i++)
    {
        setActive(i,active);
    }
}

QString SaveSettings::getName(int index)
{
    if(boxes.size()<=index) return "";

    return boxes[index]->getName();
}

int SaveSettings::sizeSensors()
{
    return sensors.size();
}

const sensorData &SaveSettings::getSensor(int index)
{
    return sensors[index];
}

void SaveSettings::save()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("settings");

    settings.setValue("timeError", timeError);
    settings.setValue("timePool", timePool);

    settings.setValue("soundEnableMaxTemp", soundEnableMaxTemp);
    settings.setValue("soundEnableMinTemp", soundEnableMinTemp);
    settings.setValue("soundEnableMaxHumi", soundEnableMaxHumi);
    settings.setValue("soundEnableMinHumi", soundEnableMinHumi);
    settings.setValue("soundEnableErr", soundEnableErr);

    settings.setValue("password", password);
    settings.setValue("pathReport", pathReport);

    settings.endGroup();

    settings.sync();
}

void SaveSettings::read()
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("settings");

    timeError = settings.value("timeError", 15).toInt();
    timePool = settings.value("timePool", 10).toInt();

    soundEnableMaxTemp = settings.value("soundEnableMaxTemp",true).toBool();
    soundEnableMinTemp = settings.value("soundEnableMinTemp",true).toBool();
    soundEnableMaxHumi = settings.value("soundEnableMaxHumi",true).toBool();
    soundEnableMinHumi = settings.value("soundEnableMinHumi",true).toBool();
    soundEnableErr = settings.value("soundEnableErr",true).toBool();

    password = settings.value("password","").toString();
    pathReport = settings.value("pathReport","C:\\Отчёты по температуре").toString();

    settings.endGroup();
}

bool SaveSettings::getSoundEnableMaxTemp() const
{
    return soundEnableMaxTemp;
}

void SaveSettings::setSoundEnableMaxTemp(bool value)
{    
    soundEnableMaxTemp = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setEnabledErrorTempMax(value);
}

bool SaveSettings::getSoundEnableMinTemp() const
{
    return soundEnableMinTemp;
}

void SaveSettings::setSoundEnableMinTemp(bool value)
{
    soundEnableMinTemp = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setEnabledErrorTempMin(value);
}

bool SaveSettings::getSoundEnableMaxHumi() const
{
    return  soundEnableMaxHumi;
}

void SaveSettings::setSoundEnableMaxHumi(bool value)
{
    soundEnableMaxHumi = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setEnabledErrorHumiMax(value);
}

bool SaveSettings::getSoundEnableMinHumi() const
{
    return  soundEnableMinHumi;
}

void SaveSettings::setSoundEnableMinHumi(bool value)
{
    soundEnableMinHumi = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setEnabledErrorHumiMin(value);
}

bool SaveSettings::getSoundEnableErr() const
{
    return soundEnableErr;
}

void SaveSettings::setSoundEnableErr(bool value)
{
    soundEnableErr = value;

    for(int i = 0; i < boxes.size(); i++)
        boxes[i]->setEnabledErrorData(value);
}

QString SaveSettings::getPassword() const
{
    return password;
}

void SaveSettings::setPassword(const QString &value)
{
    password = value;
}

QString SaveSettings::getPathReport() const
{
    return pathReport;
}

void SaveSettings::setPathReport(const QString &value)
{
    pathReport = value;
}

int SaveSettings::getTimePool() const
{
    return timePool;
}

void SaveSettings::setTimePool(int value)
{
    timePool = value;

    emit setPoolTimer(value);
}

void SaveSettings::createDb()
{
    if(!QSqlDatabase::contains(dbPath))//проверка на существование соеденения
    {
        QFile file(dbPath);
        if(!file.exists())//проверка на присутсвие файла
        {
            //файла нет
        }

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dbPath);
        db.setDatabaseName(dbPath);
        db.open();
    }
    QSqlDatabase db = QSqlDatabase::database(dbPath);
    QSqlQuery query(db);

    query.exec("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='unf_stowage'");
    query.next();
    if(query.value(0).toInt() == 0)
    {
        QString   str  = "CREATE TABLE `unf_stowage` ("
                         "`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "`id_box_1` INTEGER NOT NULL,"
                         "`eggs` INTEGER NOT NULL,"
                         "`date_1` TEXT NOT NULL,"
                         "`id_box_2` INTEGER,"
                         "`date_2` TEXT,"
                         "`chickens` INTEGER,"
                         "`date_3` TEXT"
                         ")";
        query.exec(str);
    }

    query.exec("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='stowage'");
    query.next();
    if(query.value(0).toInt() == 0)
    {
        QString   str  = "CREATE TABLE `stowage` ("
                         "`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "`id_box_1` INTEGER NOT NULL,"
                         "`eggs` INTEGER NOT NULL,"
                         "`date_1` TEXT NOT NULL,"
                         "`id_box_2` INTEGER NOT NULL,"
                         "`date_2` TEXT NOT NULL,"
                         "`chickens` INTEGER NOT NULL,"
                         "`date_3` TEXT NOT NULL"
                         ")";
        query.exec(str);
    }

    query.exec("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='boxes'");
    query.next();
    if(query.value(0).toInt() == 0)
    {
        QString   str  = "CREATE TABLE `boxes` ("
                         "`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                         "`name` TEXT NOT NULL,"
                         "`tab_name` TEXT NOT NULL"
                         ")";
        query.exec(str);
    }

    for(int i = 0;i < boxes.size(); i++)//количество боксов
    {
        query.exec(QString("SELECT count(*) FROM boxes WHERE id='%0'").arg(i));
        query.next();
        if(query.value(0).toInt() == 0)
        {
            query.prepare(QString("INSERT INTO boxes (id, name,tab_name) VALUES (:id,:name,:tab)"));
            query.bindValue(":id", i);
            query.bindValue(":name", boxes[i]->getName());
            query.bindValue(":tab", QString("box_%0").arg(i));
            query.exec();
        }
    }

    for(int i = 0;i < boxes.size(); i++)//количество боксов
    {
        query.exec(QString("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='box_%0'").arg(i));
        query.next();
        if(query.value(0).toInt() == 0)
        {
            QString   str  =  "CREATE TABLE `box_%0` ("
                              "`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"
                              "`temp`,"
                              "`humi`,"
                              "`date` TEXT NOT NULL"
                              ")";


            query.exec(str.arg(i));
        }
    }
}

void SaveSettings::createBoxes()
{
    for(int i = 0; i < 45; i++)
    {
        Box *b = new Box(i);

        connect(b, &Box::settingChanged, this, &SaveSettings::settingsChanged);
        connect(b, &Box::allData, this, &SaveSettings::valueChanged);
        connect(b, &Box::allData, this, &SaveSettings::addBufferBox);


        connect(b, &Box::errorBox, this, &SaveSettings::boxError);
        connect(b, &Box::noErrorBox, this, &SaveSettings::noBoxError);

        connect(b, &Box::noTemp, this, &SaveSettings::noDataBox);
        connect(b, &Box::noHumi, this, &SaveSettings::noDataBox);

        boxes.append(b);
    }
}
