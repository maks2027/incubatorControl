#include "report.h"
#include "ui_report.h"

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

Report::Report(SaveSettings &AllSettings, QWidget *parent) :
    QWidget(parent),
    AllSettings(AllSettings),
    ui(new Ui::Report)
{
    ui->setupUi(this);

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

    setWindowFlags(Qt::Window);


    model = new QSqlQueryModel(this);
    proxyModel = new MySortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    ui->tableView->setModel(proxyModel);


    seriesTemp1 = new QLineSeries();
    seriesTemp2 = new QLineSeries();
    chartTemp = new QChart;

    seriesHumi1 = new QLineSeries();
    seriesHumi2 = new QLineSeries();
    chartHumi = new QChart;


    chartTemp->addSeries(seriesTemp1);
    chartTemp->addSeries(seriesTemp2);
    chartTemp->legend()->hide();

    axisTempX = new QDateTimeAxis;
    axisTempX->setTickCount(20);

    axisTempX->setLabelsAngle(-90);
    chartTemp->addAxis(axisTempX, Qt::AlignBottom);

    seriesTemp1->attachAxis(axisTempX);
    seriesTemp2->attachAxis(axisTempX);

    axisTempY = new QValueAxis;
    axisTempY->setLabelFormat("%.1f");
    axisTempY->setTitleText("Температура");
    chartTemp->addAxis(axisTempY, Qt::AlignLeft);

    seriesTemp1->attachAxis(axisTempY);
    seriesTemp2->attachAxis(axisTempY);

    chartTemp->setTheme(QChart::ChartThemeDark);


    chartHumi->addSeries(seriesHumi1);
    chartHumi->addSeries(seriesHumi2);
    chartHumi->legend()->hide();

    axisHumiX = new QDateTimeAxis;
    axisHumiX->setTickCount(20);

    axisHumiX->setLabelsAngle(-90);
    chartHumi->addAxis(axisHumiX, Qt::AlignBottom);

    seriesHumi1->attachAxis(axisHumiX);
    seriesHumi2->attachAxis(axisHumiX);

    axisHumiY = new QValueAxis;
    axisHumiY->setLabelFormat("%.1f");
    axisHumiY->setTitleText("Влажность");
    chartHumi->addAxis(axisHumiY, Qt::AlignLeft);

    seriesHumi1->attachAxis(axisHumiY);
    seriesHumi2->attachAxis(axisHumiY);

    chartHumi->setTheme(QChart::ChartThemeDark);


    chartViewTemp = new QChartView(chartTemp);
    chartViewTemp->setRenderHint(QPainter::Antialiasing);

    chartViewHumi = new QChartView(chartHumi);
    chartViewHumi->setRenderHint(QPainter::Antialiasing);

    seriesTemp1->setColor(QColor(255, 60, 0));
    seriesTemp2->setColor(QColor(60, 255, 0));

    seriesHumi1->setColor(QColor(255, 60, 0));
    seriesHumi2->setColor(QColor(60, 255, 0));

    QVBoxLayout *vbox = new QVBoxLayout;
    ui->widgetChart->setLayout(vbox);
    vbox->addWidget(chartViewTemp);
    vbox->addWidget(chartViewHumi);


    readTable();

    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    ui->doubleSpinBox_tempMin->setValue(settings.value("tempMin",ui->doubleSpinBox_tempMin->value()).toDouble());
    ui->doubleSpinBox_tempMax->setValue(settings.value("tempMax",ui->doubleSpinBox_tempMax->value()).toDouble());
    ui->doubleSpinBox_humiMin->setValue(settings.value("humiMin",ui->doubleSpinBox_humiMin->value()).toDouble());
    ui->doubleSpinBox_humiMax->setValue(settings.value("humiMax",ui->doubleSpinBox_humiMax->value()).toDouble());
    ui->checkBox_auto->setChecked(settings.value("auto",  ui->checkBox_auto->isChecked()).toBool());
    ui->splitter->restoreState(settings.value("splitter",ui->splitter->saveGeometry()).toByteArray());

    settings.endGroup();

    ui->dateEdit_1->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEdit_2->setDate(QDate::currentDate());
}

Report::~Report()
{
    delete ui;
}

void Report::on_pushButtonReport_clicked()
{
    if(idBox1 < 0 || idBox2 < 0) return;

    QString nameFile;
    nameFile.append(AllSettings.getPathReport());
    nameFile.append("/");
    nameFile.append(date3.toString("dd-MM-yyyy"));
    nameFile.append(" ");
    nameFile.append(AllSettings.getName(idBox1));
    nameFile.append("-");
    nameFile.append(AllSettings.getName(idBox2));
    nameFile.append(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFileName(nameFile);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A4);
    printer.setResolution(100);
    printer.setPageOrientation(QPageLayout::Landscape);
    QPainter painter(&printer);

    QRect size = printer.pageLayout().paintRectPixels(printer.resolution());

    int stepH = size.height()/10;

    QPen pen(Qt::black, 2, Qt::SolidLine);
    painter.setPen(pen);
    painter.drawLine(0, 0, 0, size.height());
    painter.drawLine(0, 0, size.width(), 0);
    painter.drawLine(size.width(), 0, size.width(), size.height());
    painter.drawLine(0, size.height(), size.width(), size.height());

    painter.drawLine(0, stepH*2, size.width(), stepH*2);

    QString v1 = date1.toString("dd-MM-yyyy hh:mm");
    QString v2 = date2.toString("dd-MM-yyyy hh:mm");
    QString v3 = date3.toString("dd-MM-yyyy hh:mm");


    QString title = "Отчёт о температуре за инкубационный ";
    title.append(AllSettings.getName(idBox1));
    title.append(" и выводной ");
    title.append(AllSettings.getName(idBox2));

    QString data1  = "Дата заселения: ";
    QString data2  = "Дата переселения: ";
    QString data3  = "Дата выведения: ";
    QString eggsS  = "Заселено яйца: ";
    QString chickS = "Полученно цыплят: ";
    QString perS   = "Процент вывода: ";

    data1.append(v1);
    data2.append(v2);
    data3.append(v3);
    eggsS.append(QString::number(eggs));
    chickS.append(QString::number(chick));
    perS.append(QString::number(percentChik,'f',2));

    QFont fontTitle("Courier", 50, QFont::DemiBold);
    QFont fontDate("Courier", 12, QFont::DemiBold);
    QFontMetrics fTitle(fontTitle);
    QFontMetrics fDate(fontDate);

    int dateTitleHeight = fTitle.height();
    int dateTextHeight = fDate.height();


    int dy = 0;
    dy += dateTitleHeight;

    painter.setFont(fontTitle);
    painter.drawText(10, dy, title);


    painter.setFont(fontDate);
    dy += dateTextHeight;
    painter.drawText(10, dy, data1);
    dy += dateTextHeight;
    painter.drawText(10, dy, data2);
    dy += dateTextHeight;
    painter.drawText(10, dy, data3);
    dy += dateTextHeight;
    painter.drawText(10, dy, eggsS);
    dy += dateTextHeight;
    painter.drawText(10, dy, chickS);
    dy += dateTextHeight;
    painter.drawText(10, dy, perS);


    QLineSeries *tempSeries1 = new QLineSeries();
    QLineSeries *tempSeries2 = new QLineSeries();
    QLineSeries *humiSeries1 = new QLineSeries();
    QLineSeries *humiSeries2 = new QLineSeries();

    tempSeries1->replace(worker->data[0].pointsBufferTemp);
    tempSeries2->replace(worker->data[1].pointsBufferTemp);
    humiSeries1->replace(worker->data[0].pointsBufferHumi);
    humiSeries2->replace(worker->data[1].pointsBufferHumi);


    QChart *tempChart = new QChart;
    QChart *humiChart = new QChart;


    tempChart->addSeries(tempSeries1);
    tempChart->addSeries(tempSeries2);
    tempChart->legend()->hide();


    humiChart->addSeries(humiSeries1);
    humiChart->addSeries(humiSeries2);
    humiChart->legend()->hide();


    QDateTimeAxis * tempAxisX = new QDateTimeAxis;
    QDateTimeAxis * humiAxisX = new QDateTimeAxis;


    tempAxisX->setTickCount(20);
    tempAxisX->setRange(date1, date3);
    tempAxisX->setFormat("dd-MM-yy");
    tempAxisX->setLabelsAngle(-90);

    humiAxisX->setTickCount(20);
    humiAxisX->setRange(date1, date3);
    humiAxisX->setFormat("dd-MM-yy");
    humiAxisX->setLabelsAngle(-90);


    tempChart->addAxis(tempAxisX, Qt::AlignBottom);
    humiChart->addAxis(humiAxisX, Qt::AlignBottom);

    tempSeries1->attachAxis(tempAxisX);
    tempSeries2->attachAxis(tempAxisX);

    humiSeries1->attachAxis(humiAxisX);
    humiSeries2->attachAxis(humiAxisX);

    QValueAxis *tempAxisY = new QValueAxis;
    tempAxisY->setLabelFormat("%.1f");
    tempAxisY->setTitleText("Температура");

    QValueAxis *humiAxisY = new QValueAxis;
    humiAxisY->setLabelFormat("%.1f");
    humiAxisY->setTitleText("Влажность");

    if(ui->checkBox_auto->isChecked())
    {
        genAxisYRange(tempMin, tempMax, tempAxisY);
        genAxisYRange(humiMin, humiMax, humiAxisY);
    }
    else
    {
        genAxisYRange(ui->doubleSpinBox_tempMin->value(), ui->doubleSpinBox_tempMax->value(), tempAxisY);
        genAxisYRange(ui->doubleSpinBox_humiMin->value(), ui->doubleSpinBox_humiMax->value(), humiAxisY);
    }


    tempChart->addAxis(tempAxisY, Qt::AlignLeft);
    tempSeries1->attachAxis(tempAxisY);
    tempSeries2->attachAxis(tempAxisY);

    humiChart->addAxis(humiAxisY, Qt::AlignLeft);
    humiSeries1->attachAxis(humiAxisY);
    humiSeries2->attachAxis(humiAxisY);


    tempChart->addAxis(tempAxisX, Qt::AlignBottom);
    tempChart->addAxis(tempAxisY, Qt::AlignLeft);
    tempChart->setAnimationOptions(QChart::NoAnimation);

    humiChart->addAxis(humiAxisX, Qt::AlignBottom);
    humiChart->addAxis(humiAxisY, Qt::AlignLeft);
    humiChart->setAnimationOptions(QChart::NoAnimation);



    QRectF chartRect1(0,stepH*2,size.width(),stepH*6);
    QRectF chartRect2(0,stepH*6,size.width(),stepH*10);


    QChartView* reportTempChartView = new QChartView(tempChart);
    QChartView* reportHumiChartView = new QChartView(humiChart);
    reportTempChartView->setGeometry(0,0,chartRect1.width(),stepH*4);
    reportHumiChartView->setGeometry(0,0,chartRect1.width(),stepH*4);

    reportTempChartView->show();
    reportHumiChartView->show();

    reportTempChartView->render(&painter,chartRect1);
    reportHumiChartView->render(&painter,chartRect2);


    painter.end();

    reportTempChartView->close();
    reportHumiChartView->close();

    reportTempChartView->deleteLater();
    reportHumiChartView->deleteLater();
}

void Report::readTable()
{
    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    QString str = "SELECT s.id, b1.name, s.eggs, s.date_1, b2.name, s.date_2, s.chickens, s.date_3 "
                  "FROM stowage AS s "
                  "JOIN `boxes` AS b1 "
                  "ON b1.id = s.id_box_1 "
                  "JOIN `boxes` AS b2 "
                  "ON b2.id = s.id_box_2 "
                  "WHERE s.date_3 IS NOT NULL";

    model->setQuery(str,db);

    model->setHeaderData(1, Qt::Horizontal, tr("Инкубационный"));
    model->setHeaderData(2, Qt::Horizontal, tr("Яйцо"));
    model->setHeaderData(3, Qt::Horizontal, tr("Заселения"));
    model->setHeaderData(4, Qt::Horizontal, tr("Выводной"));
    model->setHeaderData(5, Qt::Horizontal, tr("Переселение"));
    model->setHeaderData(6, Qt::Horizontal, tr("Цыплят"));
    model->setHeaderData(7, Qt::Horizontal, tr("Вывод"));

    ui->tableView->setColumnHidden(0,true);
    ui->tableView->resizeColumnsToContents();

    ui->tableView->setColumnWidth(1, ui->tableView->columnWidth(1) + 10);
    ui->tableView->setColumnWidth(2, ui->tableView->columnWidth(2) + 10);
    ui->tableView->setColumnWidth(3, ui->tableView->columnWidth(3) + 10);
    ui->tableView->setColumnWidth(4, ui->tableView->columnWidth(4) + 10);
    ui->tableView->setColumnWidth(5, ui->tableView->columnWidth(5) + 10);
    ui->tableView->setColumnWidth(6, ui->tableView->columnWidth(6) + 10);
    ui->tableView->setColumnWidth(7, ui->tableView->columnWidth(7) + 10);
}

void Report::started()
{
    ui->pushButton_stop->setEnabled(true);
    ui->progressBar->setEnabled(true);
}

void Report::stoped()
{
    ui->pushButtonReport->setEnabled(true);
    ui->pushButton_start->setEnabled(true);
    workerStarted = false;
    ui->pushButton_stop->setEnabled(false);
    ui->progressBar->setEnabled(false);
    ui->progressBar->setValue(0);
}

void Report::finish()
{
    ui->progressBar->setValue(198);
    seriesTemp1->replace(worker->data[0].pointsBufferTemp);
    seriesHumi1->replace(worker->data[0].pointsBufferHumi);

    ui->progressBar->setValue(199);
    seriesTemp2->replace(worker->data[1].pointsBufferTemp);
    seriesHumi2->replace(worker->data[1].pointsBufferHumi);

    if(worker->data[0].maxTemp > worker->data[1].maxTemp)
        tempMax = worker->data[0].maxTemp;
    else
        tempMax = worker->data[1].maxTemp;


    if(worker->data[0].minTemp < worker->data[1].minTemp)
        tempMin = worker->data[0].minTemp;
    else
        tempMin = worker->data[1].minTemp;

    if(worker->data[0].maxHumi > worker->data[1].maxHumi)
        humiMax = worker->data[0].maxHumi;
    else
        humiMax = worker->data[1].maxHumi;


    if(worker->data[0].minHumi < worker->data[1].minHumi)
        humiMin = worker->data[0].minHumi;
    else
        humiMin = worker->data[1].minHumi;

    if(ui->checkBox_auto->isChecked())
    {
        genAxisYRange(tempMin, tempMax, axisTempY);
        genAxisYRange(humiMin, humiMax, axisHumiY);
    }


    ui->percent->setText(QString::number(percentChik,'f',1));
    ui->tempMax->setText(QString::number(tempMax,'f',1));
    ui->tempMin->setText(QString::number(tempMin,'f',1));

    stoped();
}

void Report::progress(int value)
{
    ui->progressBar->setValue(value);
}

void Report::genAxisYRange(double min, double max, QValueAxis *axis)
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

void Report::closeEvent(QCloseEvent *event)
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("MainWindow");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("tempMin", ui->doubleSpinBox_tempMin->value());
    settings.setValue("tempMax", ui->doubleSpinBox_tempMax->value());
    settings.setValue("humiMin", ui->doubleSpinBox_humiMin->value());
    settings.setValue("humiMax", ui->doubleSpinBox_humiMax->value());
    settings.setValue("auto", ui->checkBox_auto->isChecked());

    settings.setValue("splitter", ui->splitter->saveState());

    settings.endGroup();

    event->accept();
}

void Report::on_tableView_clicked(const QModelIndex &index)
{
    //updateDataView();
}

void Report::on_lineEditIn_textChanged(const QString &arg1)
{
    proxyModel->setBox1Filter(arg1);
}

void Report::on_lineEditOut_textChanged(const QString &arg1)
{
    proxyModel->setBox2Filter(arg1);
}

void Report::on_dateEdit_1_userDateChanged(const QDate &date)
{
    proxyModel->setFilterMinimumDate(date);
}

void Report::on_dateEdit_2_userDateChanged(const QDate &date)
{
    proxyModel->setFilterMaximumDate(date);
}

void Report::on_checkBox_1_clicked(bool checked)
{
    proxyModel->setEnableMinDate(checked);
}

void Report::on_checkBox_2_clicked(bool checked)
{
    proxyModel->setEnableMaxDate(checked);
}

void Report::on_pushButton_start_clicked()
{
    if(workerStarted) return;

    workerStarted = true;
    ui->pushButton_start->setEnabled(false);
    ui->pushButtonReport->setEnabled(false);

    ui->percent->setText(QString::number(0,'f',1));
    ui->tempMax->setText(QString::number(0,'f',1));
    ui->tempMin->setText(QString::number(0,'f',1));


    if(ui->tableView->currentIndex().row() < 0)
    {
        stoped();
        return;
    }

    int id = model->data(model->index(proxyModel->mapToSource(ui->tableView->currentIndex()).row(), 0)).toInt();


    qDebug()<<id;

    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    percentChik = 0;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM stowage WHERE stowage.id = :id");
    query.bindValue(":id", id);
    query.exec();

    if(query.next())
    {
        idBox1 = query.value(1).toInt();//бокс 1
        idBox2 = query.value(4).toInt();//бокс 2

        date1 = query.value(3).toDateTime();
        date2 = query.value(5).toDateTime();
        date3 = query.value(7).toDateTime();

        eggs = query.value(2).toInt();//8000
        chick = query.value(6).toInt();//6000
        percentChik = (double)chick/(double)eggs * 100.0;

        QString t1 =  date1.toString("yyyy-MM-dd hh:mm:ss");
        QString t2 =  date2.toString("yyyy-MM-dd hh:mm:ss");
        QString t3 =  date3.toString("yyyy-MM-dd hh:mm:ss");

        axisTempX->setRange(date1, date3);
        axisHumiX->setRange(date1, date3);

        QMetaObject::invokeMethod(worker, "startRead", Qt::AutoConnection, Q_ARG(QString,  t1),Q_ARG(QString,  t2),
                                  Q_ARG(QString,  t3),Q_ARG(int,  idBox1),Q_ARG(int,  idBox2));

    }
    else
    {
        stoped();
    }
}

void Report::on_pushButton_stop_clicked()
{
    QMetaObject::invokeMethod(worker, "stopRead", Qt::AutoConnection);
}

void Report::on_doubleSpinBox_tempMax_valueChanged(double arg1)
{
    if(!ui->checkBox_auto->isChecked())
        genAxisYRange(ui->doubleSpinBox_tempMin->value(), ui->doubleSpinBox_tempMax->value(), axisTempY);
}

void Report::on_doubleSpinBox_tempMin_valueChanged(double arg1)
{
    if(!ui->checkBox_auto->isChecked())
        genAxisYRange(ui->doubleSpinBox_tempMin->value(), ui->doubleSpinBox_tempMax->value(), axisTempY);
}

void Report::on_doubleSpinBox_humiMin_valueChanged(double arg1)
{
    if(!ui->checkBox_auto->isChecked())
        genAxisYRange(ui->doubleSpinBox_humiMin->value(), ui->doubleSpinBox_humiMax->value(), axisHumiY);
}

void Report::on_doubleSpinBox_humiMax_valueChanged(double arg1)
{
    if(!ui->checkBox_auto->isChecked())
        genAxisYRange(ui->doubleSpinBox_humiMin->value(), ui->doubleSpinBox_humiMax->value(), axisHumiY);

}

void Report::on_pushButton_openIB_clicked()
{
    emit openHistory(idBox1, date1, date2);
}

void Report::on_pushButton_openVB_clicked()
{
    emit openHistory(idBox2, date2, date3);
}

void Report::on_checkBox_auto_clicked(bool checked)
{
    if(workerStarted) return;

    if(checked)
    {
        genAxisYRange(tempMin, tempMax, axisTempY);
        genAxisYRange(humiMin, humiMax, axisHumiY);
    }
    else
    {
        genAxisYRange(ui->doubleSpinBox_tempMin->value(), ui->doubleSpinBox_tempMax->value(), axisTempY);
        genAxisYRange(ui->doubleSpinBox_humiMin->value(), ui->doubleSpinBox_humiMax->value(), axisHumiY);
    }
}
