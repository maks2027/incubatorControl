#ifndef REPORT_H
#define REPORT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPrinter>

#include <QtCharts/QChartGlobal>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

#include "savesettings.h"
#include "readbdthread.h"
#include "mysortfilterproxymodel.h"


QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QLineSeries;
QT_CHARTS_END_NAMESPACE


QT_CHARTS_USE_NAMESPACE

namespace Ui {
class Report;
}

class Report : public QWidget
{
    Q_OBJECT

    SaveSettings &AllSettings;

public:
    explicit Report(SaveSettings &AllSettings, QWidget *parent = nullptr);
    ~Report();

private slots:
    void on_pushButtonReport_clicked();

    void on_tableView_clicked(const QModelIndex &index);

    void on_lineEditIn_textChanged(const QString &arg1);

    void on_lineEditOut_textChanged(const QString &arg1);

    void on_dateEdit_1_userDateChanged(const QDate &date);

    void on_dateEdit_2_userDateChanged(const QDate &date);

    void on_checkBox_2_clicked(bool checked);

    void on_checkBox_1_clicked(bool checked);

    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_doubleSpinBox_tempMax_valueChanged(double arg1);

    void on_doubleSpinBox_humiMax_valueChanged(double arg1);

    void on_doubleSpinBox_tempMin_valueChanged(double arg1);

    void on_doubleSpinBox_humiMin_valueChanged(double arg1);

    void on_pushButton_openIB_clicked();

    void on_pushButton_openVB_clicked();

    void on_checkBox_auto_clicked(bool checked);

public slots:
    void readTable();    

    void started();
    void stoped();
    void finish();
    void progress(int value);
signals:
    void openHistory(int indBox, QDateTime date1, QDateTime date2);

private:
    Ui::Report *ui;

    int idBox1 = -1;
    int idBox2 = -1;
    QDateTime date1;
    QDateTime date2;
    QDateTime date3;
    int eggs;
    int chick;
    double percentChik;

    double tempMin = 0;
    double tempMax = 0;
    double humiMin = 0;
    double humiMax = 0;

    QChartView *chartViewTemp;
    QChart *chartTemp;
    QLineSeries *seriesTemp1;
    QLineSeries *seriesTemp2;

    QDateTimeAxis *axisTempX;
    QValueAxis *axisTempY;


    QChartView *chartViewHumi;
    QChart *chartHumi;
    QLineSeries *seriesHumi1;
    QLineSeries *seriesHumi2;

    QDateTimeAxis *axisHumiX;
    QValueAxis *axisHumiY;


    QSqlQueryModel *model;
    MySortFilterProxyModel *proxyModel;

    QThread *thread;
    readBdThread *worker;
    bool workerStarted = false;

    void genAxisYRange(double min, double max, QValueAxis *axis);

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // REPORT_H
