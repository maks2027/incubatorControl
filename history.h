#ifndef HISTORY_H
#define HISTORY_H

#include <QWidget>
#include <QSqlQuery>
#include <QtSql>
#include <QPageSize>
#include <QPrinter>
#include <QDebug>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

#include "savesettings.h"
#include "readbdthread.h"

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QLineSeries;
QT_CHARTS_END_NAMESPACE


QT_CHARTS_USE_NAMESPACE

namespace Ui {
class History;
}

class History : public QWidget
{
    Q_OBJECT

    SaveSettings &AllSettings;


    struct ErrorsT
    {
        ErrorsT(double value,QDateTime dateTime):value(value),dateTime(dateTime){}

        double value;
        QDateTime dateTime;
    };

public:
    explicit History(SaveSettings &_AllSettings,QWidget *parent = nullptr);
    ~History();

private slots:
    void on_pushButton_5_clicked();

    void on_hour1_clicked();

    void on_day_clicked();

    void on_hour3_clicked();

    void on_day7_clicked();

    void on_comboBox_currentIndexChanged(int index);    

    void on_day21_clicked();

    void on_day18_clicked();

    void on_stop_clicked();

    void on_maxTemp_valueChanged(double arg1);

    void on_minTemp_valueChanged(double arg1);

    void on_maxHumi_valueChanged(double arg1);

    void on_minHumi_valueChanged(double arg1);

    void on_checkBox_auto_clicked(bool checked);

private:
    Ui::History *ui;

    //QSqlQueryModel *model;

    QChartView *chartViewTemp;
    QChart *chartTemp;
    QLineSeries *seriesTemp;
    QDateTimeAxis *axisTempX;
    QValueAxis *axisTempY;

    QChartView *chartViewHumi;
    QChart *chartHumi;
    QLineSeries *seriesHumi;
    QDateTimeAxis *axisHumiX;
    QValueAxis *axisHumiY;

    double tempMin = 0;
    double tempMax = 0;
    double humiMin = 0;
    double humiMax = 0;

    void genAxisYRange(double min, double max, QValueAxis *axis);

    QThread *thread;
    readBdThread *worker;
    bool workerStarted = false;

    //double filter(double value, double &lastValue, double k);

public slots:
    void updateUi();

    void started();
    void stoped();
    void finish();
    void progress(int value);

    void openHistory(int indBox, QDateTime date1, QDateTime date2);

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // HISTORY_H
