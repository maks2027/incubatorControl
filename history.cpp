#include "history.h"
#include "ui_history.h"

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QAbstractBarSeries>
#include <QtCharts/QPercentBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLegend>

History::History(SaveSettings &_AllSettings, QWidget *parent) :
    QWidget(parent),AllSettings(_AllSettings),
    ui(new Ui::History)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);

    thread = new QThread;
    worker = new readBdThread;

    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(worker, SIGNAL(startedRead()), this, SLOT(started()));
    connect(worker, SIGNAL(stopedRead()), this, SLOT(stoped()));
    connect(worker, SIGNAL(finishRead()), this, SLOT(finish()));
    connect(worker, SIGNAL(progressRead(int)), this, SLOT(progress(int)));


    worker->moveToThread(thread);
    thread->start();

    QMetaObject::invokeMethod(worker, "setBdPath", Qt::AutoConnection, Q_ARG(QString,  AllSettings.dbPath));

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 70);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() <<"Температура"<<"Влажность"<<"Дата");


    seriesTemp = new QLineSeries();
    chartTemp = new QChart;

    seriesHumi= new QLineSeries();
    chartHumi = new QChart;


    chartTemp->addSeries(seriesTemp);
    chartTemp->legend()->hide();

    chartHumi->addSeries(seriesHumi);
    chartHumi->legend()->hide();

    axisTempX = new QDateTimeAxis;
    axisTempX->setTickCount(20);
    axisTempX->setRange(ui->dateTimeEdit->dateTime(), ui->dateTimeEdit_2->dateTime());
    //axisX->setFormat("dd-MM-yy");

    axisHumiX = new QDateTimeAxis;
    axisHumiX->setTickCount(20);
    axisHumiX->setRange(ui->dateTimeEdit->dateTime(), ui->dateTimeEdit_2->dateTime());
    //axisX->setFormat("dd-MM-yy");

    axisTempX->setLabelsAngle(-90);
    chartTemp->addAxis(axisTempX, Qt::AlignBottom);
    seriesTemp->attachAxis(axisTempX);

    axisHumiX->setLabelsAngle(-90);
    chartHumi->addAxis(axisHumiX, Qt::AlignBottom);
    seriesHumi->attachAxis(axisHumiX);

    axisTempY = new QValueAxis;
    axisTempY->setLabelFormat("%.1f");
    axisTempY->setTitleText("Температура");

    axisHumiY = new QValueAxis;
    axisHumiY->setLabelFormat("%.1f");
    axisHumiY->setTitleText("Влажность");

    genAxisYRange(ui->minTemp->value(),ui->maxTemp->value(), axisTempY);
    genAxisYRange(ui->minHumi->value(),ui->maxHumi->value(), axisHumiY);


    chartTemp->addAxis(axisTempY, Qt::AlignLeft);
    seriesTemp->attachAxis(axisTempY);

    chartHumi->addAxis(axisHumiY, Qt::AlignLeft);
    seriesHumi->attachAxis(axisHumiY);


    chartTemp->setTheme(QChart::ChartThemeDark);
    chartHumi->setTheme(QChart::ChartThemeDark);

    chartViewTemp = new QChartView(chartTemp);
    chartViewTemp->setRenderHint(QPainter::Antialiasing);

    chartViewHumi = new QChartView(chartHumi);
    chartViewHumi->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(chartViewTemp);
    vbox->addWidget(chartViewHumi);
    ui->widget->setLayout(vbox);

    seriesTemp->setColor(QColor(255, 60, 0));

    updateUi();

    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    ui->filterEnable->setChecked(settings.value("filterEnable", ui->filterEnable->isChecked()).toBool());
    ui->filterK->setValue(settings.value("filterK", ui->filterK->value()).toDouble());
    ui->maxTemp->setValue(settings.value("maxTemp",  ui->maxTemp->value()).toDouble());
    ui->minTemp->setValue(settings.value("minTemp",  ui->minTemp->value()).toDouble());
    ui->maxHumi->setValue(settings.value("maxHumi",  ui->maxHumi->value()).toDouble());
    ui->minHumi->setValue(settings.value("minHumi",  ui->minHumi->value()).toDouble());
    ui->checkBox_auto->setChecked(settings.value("auto",  ui->checkBox_auto->isChecked()).toBool());
    settings.endGroup();

    ui->stop->setEnabled(false);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setEnabled(false);
}

History::~History()
{
    delete ui;
}

void History::updateUi()
{
    for(int i = 0; i< AllSettings.sizeBoxes();i++)
    {
        ui->comboBox->addItem(AllSettings.getBox(i)->getName(),i);
    }
}

void History::started()
{
    qDebug()<<"started";

    ui->stop->setEnabled(true);
    ui->progressBar->setEnabled(true);
}

void History::stoped()
{
    qDebug()<<"stoped";

    ui->pushButton_5->setEnabled(true);
    workerStarted = false;
    ui->stop->setEnabled(false);
    ui->progressBar->setEnabled(false);
    ui->progressBar->setValue(0);
}

void History::finish()
{
    qDebug()<<"finish";

    seriesTemp->replace(worker->data[0].pointsBufferTemp);
    seriesHumi->replace(worker->data[0].pointsBufferHumi);
    ui->tableWidget->setRowCount(worker->data[0].pointsBufferTemp.size());

    ui->progressBar->setValue(99);

    for(uint64_t i = 0; i < worker->data[0].pointsBufferTemp.size(); i++)
    {
        if(i%1000 == 0)
        {
            QCoreApplication::processEvents();
        }

        double valueTemp = worker->data[0].pointsBufferTemp[i].y();
        double valueHumi = worker->data[0].pointsBufferHumi[i].y();

        QDateTime data;
        data.setMSecsSinceEpoch(worker->data[0].pointsBufferTemp[i].x());


        QTableWidgetItem *item1 = new QTableWidgetItem;
        QTableWidgetItem *item2 = new QTableWidgetItem;
        QTableWidgetItem *item3 = new QTableWidgetItem;

        if(valueTemp > AllSettings.getBox(worker->data[0].boxIndex)->getTempMax())
        {
            item1->setBackgroundColor(QColor(220,40,40));
        }
        else if(valueTemp < AllSettings.getBox(worker->data[0].boxIndex)->getTempMin())
        {
            item1->setBackgroundColor(QColor(66,150,200));
        }

        if(valueHumi > AllSettings.getBox(worker->data[0].boxIndex)->getHumiMax())
        {
            item2->setBackgroundColor(QColor(220,40,40));
        }
        else if(valueHumi < AllSettings.getBox(worker->data[0].boxIndex)->getHumiMin())
        {
            item2->setBackgroundColor(QColor(66,150,200));
        }

        item1->setData(Qt::DisplayRole, valueTemp);
        item2->setData(Qt::DisplayRole, valueHumi);
        item3->setData(Qt::DisplayRole, data);

        ui->tableWidget->setItem(i,0, item1);
        ui->tableWidget->setItem(i,1, item2);
        ui->tableWidget->setItem(i,2, item3);
    }

    tempMax = worker->data[0].maxTemp;
    tempMin = worker->data[0].minTemp;
    humiMax = worker->data[0].maxHumi;
    humiMin = worker->data[0].minHumi;


    ui->cointL->setText(QString("size: %0").arg(worker->data[0].coint));
    ui->minL->setText(QString("min: %0").arg(worker->data[0].minTemp,0,'f',1));
    ui->maxL->setText(QString("max: %0").arg(worker->data[0].maxTemp,0,'f',1));
    ui->avgL->setText(QString("avg: %0").arg(worker->data[0].avgTemp,0,'f',2));

    ui->progressBar->setValue(100);

    if(ui->checkBox_auto->isChecked())
    {
        genAxisYRange(tempMin, tempMax, axisTempY);
        genAxisYRange(humiMin, humiMax, axisHumiY);
    }
    else
    {
        genAxisYRange(ui->minTemp->value(),ui->maxTemp->value(), axisTempY);
        genAxisYRange(ui->minHumi->value(),ui->maxHumi->value(), axisHumiY);
    }

    stoped();
}

void History::progress(int value)
{
    qDebug()<<"progress: " << value;
    ui->progressBar->setValue(value);
}

void History::openHistory(int indBox, QDateTime date1, QDateTime date2)
{
    if(indBox < 0) return;

    ui->dateTimeEdit->setDateTime(date1);
    ui->dateTimeEdit_2->setDateTime(date2);

    ui->comboBox->setCurrentIndex(indBox);

    this->show();
    this->activateWindow();

    on_pushButton_5_clicked();
}

void History::closeEvent(QCloseEvent *event)
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("filterEnable", ui->filterEnable->isChecked());
    settings.setValue("filterK", ui->filterK->value());
    settings.setValue("maxTemp", ui->maxTemp->value());
    settings.setValue("minTemp", ui->minTemp->value());
    settings.setValue("maxHumi", ui->maxHumi->value());
    settings.setValue("minHumi", ui->minHumi->value());
    settings.setValue("auto", ui->checkBox_auto->isChecked());
    settings.endGroup();

    event->accept();
}

void History::on_pushButton_5_clicked()
{
    if(workerStarted) return;

    workerStarted = true;
    ui->pushButton_5->setEnabled(false);

    ui->cointL->setText(QString("size: %0").arg(0));
    ui->minL->setText(QString("min: %0").arg(0,'f',1));
    ui->maxL->setText(QString("max: %0").arg(0,'f',1));
    ui->avgL->setText(QString("avg: %0").arg(0,'f',2));

    seriesTemp->clear();
    seriesHumi->clear();
    ui->tableWidget->setRowCount(0);


    QDateTime v1 = ui->dateTimeEdit->dateTime();
    QDateTime v2 = ui->dateTimeEdit_2->dateTime();

    if(v1 >= v2)return;

    QString t1 =  v1.toString("yyyy-MM-dd hh:mm:ss");
    QString t2 =  v2.toString("yyyy-MM-dd hh:mm:ss");
    int boxIndex = ui->comboBox->currentData().toInt();

    axisTempX->setRange(v1, v2);
    axisHumiX->setRange(v1, v2);

    if(v2 > v1.addDays(1))//если вторая дата больше на день
    {
        axisTempX->setFormat("dd-MM-yy");
        axisTempX->setTitleText("Дата");

        axisHumiX->setFormat("dd-MM-yy");
        axisHumiX->setTitleText("Дата");
    }
    else
    {
        v1.addDays(-1);

        if(v2 > v1.addSecs(3600))
        {
            axisTempX->setFormat("hh:mm");
            axisTempX->setTitleText("Часы:Минуты");

            axisHumiX->setFormat("hh:mm");
            axisHumiX->setTitleText("Часы:Минуты");
        }
        else
        {
            axisTempX->setFormat("mm:ss");
            axisTempX->setTitleText("Минуты:секунды");

            axisHumiX->setFormat("mm:ss");
            axisHumiX->setTitleText("Минуты:секунды");
        }
    }

    QMetaObject::invokeMethod(worker, "setFilter", Qt::AutoConnection, Q_ARG(bool,  ui->filterEnable->isChecked()),Q_ARG(double,  ui->filterK->value()));
    QMetaObject::invokeMethod(worker, "startRead", Qt::AutoConnection, Q_ARG(QString,  t1),Q_ARG(QString,  t2),Q_ARG(int,  boxIndex));
}

void History::genAxisYRange(double min, double max, QValueAxis *axis)
{
    min = floor(min * 10)/10;
    max = ceil(max * 10)/10;

    double r = max - min;
    if(r <= 0) return;

    int coint;

    if(r <= 1.0)
        coint = round(r / 0.1) + 1;
    else
        coint = 11;

    axis->setTickCount(coint);
    axis->setRange(min, max);
}

//double History::filter(double value, double &lastValue, double k)
//{
//    return lastValue += (value - lastValue) * k;
//}

void History::on_hour1_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addSecs(-3600));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
}

void History::on_day_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
}

void History::on_hour3_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addSecs(-10800));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());    
}

void History::on_day7_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());   
}

void History::on_day21_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-21));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());    
}

void History::on_day18_clicked()
{
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-18));
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());  
}

void History::on_comboBox_currentIndexChanged(int index)
{
    //on_pushButton_5_clicked();
    //qDebug()<< index;
}

void History::on_stop_clicked()
{
    QMetaObject::invokeMethod(worker, "stopRead", Qt::AutoConnection);
}

void History::on_maxTemp_valueChanged(double arg1)
{
    genAxisYRange(ui->minTemp->value(),ui->maxTemp->value(), axisTempY);
}

void History::on_minTemp_valueChanged(double arg1)
{
    genAxisYRange(ui->minTemp->value(),ui->maxTemp->value(), axisTempY);
}

void History::on_maxHumi_valueChanged(double arg1)
{
    genAxisYRange(ui->minHumi->value(),ui->maxHumi->value(), axisHumiY);
}

void History::on_minHumi_valueChanged(double arg1)
{
    genAxisYRange(ui->minHumi->value(),ui->maxHumi->value(), axisHumiY);
}

void History::on_checkBox_auto_clicked(bool checked)
{
    if(checked)
    {
        genAxisYRange(tempMin, tempMax, axisTempY);
        genAxisYRange(humiMin, humiMax, axisHumiY);
    }
    else
    {
        genAxisYRange(ui->minTemp->value(),ui->maxTemp->value(), axisTempY);
        genAxisYRange(ui->minHumi->value(),ui->maxHumi->value(), axisHumiY);
    }
}
