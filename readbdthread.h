#ifndef READBDTHREAD_H
#define READBDTHREAD_H

#include <QObject>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>

struct dataBd
{
    int boxIndex;

    double avgTemp = 0;
    double minTemp = 0;
    double maxTemp = 0;

    double avgHumi = 0;
    double minHumi = 0;
    double maxHumi = 0;

    uint64_t coint = 0;
    QVector<QPointF> pointsBufferTemp;
    QVector<QPointF> pointsBufferHumi;
};

class readBdThread : public QObject
{
    Q_OBJECT

    int percentNumber;

    QString dbPath;
    QString dbName;
    bool isFilter = false;
    double k = 0;

    bool isStop = false;
    bool works = false;

    double filter(double value, double &lastValue, double k);
    void readBD(int indexData,QString t1, QString t2, int boxId);

public:
    explicit readBdThread(QObject *parent = nullptr);


    QVector<dataBd> data;

//    int boxIndex;
//    int boxIndex2;

//    double avg = 0;
//    double min = 0;
//    double max = 0;

//    double avg2 = 0;
//    double min2 = 0;
//    double max2 = 0;

//    uint64_t coint = 0;
//    QVector<QPointF> pointsBuffer;

//    uint64_t coint2 = 0;
//    QVector<QPointF> pointsBuffer2;

public slots:
    void setBdPath(QString path);
    void setFilter(bool _enable, double _k);

    void startRead(QString t1, QString t2, QString t3, int boxId,int boxId2);
    void startRead(QString t1, QString t2, int boxId);
    void stopRead();   

signals:
    void progressRead(int value);

    void startedRead();
    void stopedRead();
    void errorRead();
    void finishRead();

};

#endif // READBDTHREAD_H
