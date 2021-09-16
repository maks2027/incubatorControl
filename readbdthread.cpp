#include "readbdthread.h"
readBdThread::readBdThread(QObject *parent) : QObject(parent)
{
   data.resize(2);//2 записи всегда!!!
}

double readBdThread::filter(double value, double &lastValue, double k)
{
    return lastValue += (value - lastValue) * k;
}

void readBdThread::readBD(int indexData, QString t1, QString t2, int boxId)
{
    data[indexData].boxIndex = boxId;

    emit progressRead(percentNumber++);

    QSqlDatabase db = QSqlDatabase::database(dbName);
    QSqlQuery query(db);

    if(!db.transaction())
    {
        works = false;
        isStop = true;
        emit stopedRead();
        return;
    }

    QString sql;
    sql = QString("SELECT count(*) FROM `box_%0` WHERE (`date` BETWEEN \"%1\" AND \"%2\")").arg(boxId).arg(t1).arg(t2);

    query.exec(sql);

    qDebug()<<query.lastError();
    qDebug()<<query.lastQuery();

    if(query.next())
        data[indexData].coint = query.value(0).toInt();

    emit progressRead(percentNumber++);
    QCoreApplication::processEvents();

    qDebug()<<2;
    if((/*data[indexData].coint == 0 ||*/ data[indexData].coint > 3000000) && !isStop)
    {
        db.commit();
        works = false;
        isStop = true;
        emit stopedRead();
        return;
    }

    sql = QString("SELECT temp, humi , date FROM `box_%0` WHERE (`date` BETWEEN \"%1\" AND \"%2\")").arg(boxId).arg(t1).arg(t2);

    query.setForwardOnly(true);
    query.exec(sql);


    qDebug()<<"Size: " << data[indexData].coint;
    emit progressRead(percentNumber++);

    data[indexData].minTemp = 999;
    data[indexData].maxTemp = -999;
    data[indexData].minHumi = 999;
    data[indexData].maxHumi = -999;

    double sumValuesTemp = 0;
    double sumValuesHumi = 0;

    ///деление
    bool div = false;
    int divNum = 1;
    double tempValuePr = 0;
    double humiValuePr = 0;
    int cointDiv = 0;

    int percent = data[indexData].coint/95;
    if(percent <= 0) percent = 1;

    data[indexData].pointsBufferTemp.clear();///До Qt 5.6 это также освобождает память, используемую вектором
    data[indexData].pointsBufferHumi.clear();

    if(data[indexData].coint > 100000)
    {
        div = true;

        divNum = data[indexData].coint/50000;//делитель
        cointDiv = data[indexData].coint/divNum;

        data[indexData].pointsBufferTemp.reserve(cointDiv);
        data[indexData].pointsBufferHumi.reserve(cointDiv);
    }
    else
    {
        data[indexData].pointsBufferTemp.reserve(data[indexData].coint);
        data[indexData].pointsBufferHumi.reserve(data[indexData].coint);
    }

    int divI = 0;
    uint64_t i = 0;
    double lastValueTemp;
    double lastValueHumi;
    while (query.next())
    {
        if(i%percent == 0)
        {
            QCoreApplication::processEvents();//!!!!!!!!!!!!!!
            emit progressRead(percentNumber++);
        }

        if(isStop)
        {
            db.commit();
            emit stopedRead();
            works = false;
            return;
        }

        double tempValue = query.value(0).toDouble();
        double humiValue = query.value(1).toDouble();
        QDateTime date = query.value(2).toDateTime();

        if(i == 0)
        {
            lastValueTemp = tempValue;
            lastValueHumi = humiValue;
        }


        sumValuesTemp += tempValue;
        sumValuesHumi += humiValue;

        if(data[indexData].maxTemp < tempValue)
            data[indexData].maxTemp = tempValue;

        if(data[indexData].minTemp > tempValue)
            data[indexData].minTemp = tempValue;

        if(data[indexData].maxHumi < humiValue)
            data[indexData].maxHumi = humiValue;

        if(data[indexData].minHumi > humiValue)
            data[indexData].minHumi = humiValue;


        if(div)//делитель
        {
            tempValuePr += tempValue;
            humiValuePr += humiValue;
            divI++;

            if((i%divNum == 0 || i == (data[indexData].coint - 1)) && i != 0)
            {
                double sumValueTemp = tempValuePr / divI;
                double sumValueHumi = humiValuePr / divI;

                tempValuePr = 0;
                humiValuePr = 0;
                divI = 0;

                if(isFilter)//фильтр
                {
                    data[indexData].pointsBufferTemp.append(QPointF(date.toMSecsSinceEpoch(), filter(sumValueTemp, lastValueTemp, k)));
                    data[indexData].pointsBufferHumi.append(QPointF(date.toMSecsSinceEpoch(), filter(sumValueHumi, lastValueHumi, k)));
                }
                else
                {
                    data[indexData].pointsBufferTemp.append(QPointF(date.toMSecsSinceEpoch(), sumValueTemp));
                    data[indexData].pointsBufferHumi.append(QPointF(date.toMSecsSinceEpoch(), sumValueHumi));
                }
            }
        }
        else
        {
            if(isFilter)//фильтр
            {
                data[indexData].pointsBufferTemp.append(QPointF(date.toMSecsSinceEpoch(), filter(tempValue,lastValueTemp,k)));
                data[indexData].pointsBufferHumi.append(QPointF(date.toMSecsSinceEpoch(), filter(humiValue,lastValueHumi,k)));
            }
            else
            {
                data[indexData].pointsBufferTemp.append(QPointF(date.toMSecsSinceEpoch(), tempValue));
                data[indexData].pointsBufferHumi.append(QPointF(date.toMSecsSinceEpoch(), humiValue));
            }
        }

        i++;
    }

    data[indexData].avgTemp = sumValuesTemp/i;
    data[indexData].avgHumi = sumValuesHumi/i;

    db.commit();
}

void readBdThread::setBdPath(QString path)
{
    dbPath = path;
    dbName = "my_db_" + QString::number((quint64)QThread::currentThread(), 16);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dbName);
    db.setDatabaseName(dbPath);
    db.open();
}

void readBdThread::setFilter(bool _enable, double _k)
{
    isFilter = _enable;
    k = _k;
}

void readBdThread::startRead(QString t1, QString t2, QString t3, int boxId, int boxId2)
{
    if(dbName.isEmpty()){qDebug() << "dbName.isEmpty"; return;}
    if(works) return;

    emit startedRead();

    works = true;
    isStop = false;
    percentNumber = 1;

    readBD(0, t1, t2, boxId);

    if(isStop)
    {
       works = false;
       return;
    }

    readBD(1, t2, t3, boxId2);


    works = false;

    if(isStop)
       return;

    emit finishRead();
}

void readBdThread::startRead(QString t1, QString t2, int boxId)
{    
    if(dbName.isEmpty()){qDebug() << "dbName.isEmpty"; return;}
    if(works) return;

    emit startedRead();

    works = true;
    isStop = false;
    percentNumber = 1;

    readBD(0, t1, t2, boxId);

    works = false;

    if(isStop)
       return;

    emit finishRead();
}

void readBdThread::stopRead()
{
    isStop = true;
}

