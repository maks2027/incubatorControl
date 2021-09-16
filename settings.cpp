#include "settings.h"
#include "ui_settings.h"

Settings::Settings(SaveSettings &_AllSettings, QWidget *parent) :
    QWidget(parent),AllSettings(_AllSettings),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);

    openReportDialog = new QFileDialog(this);

    openReportDialog->setAcceptMode(QFileDialog::AcceptOpen);
    openReportDialog->setFileMode(QFileDialog::Directory);
    openReportDialog->setOption(QFileDialog::ShowDirsOnly);

    connect(&AllSettings, &SaveSettings::mesege, this, &Settings::deviseStatus);
    connect(this, &Settings::changeCom, &AllSettings, &SaveSettings::changeCom);
    connect(this, &Settings::removeCom, &AllSettings, &SaveSettings::removeCom);


    ui->time->setValue(AllSettings.getTime());
    ui->spinBox_poll->setValue(AllSettings.getTimePool());


    ui->tableWidget_boxes->setColumnCount(7);
    ui->tableWidget_boxes->setShowGrid(true);
    ui->tableWidget_boxes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_boxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget_boxes->setHorizontalHeaderLabels(QStringList() << "Название" << "Температура" << "Влажность"<< "Т. Минимум" << "Т. Максимум" << "В. Минимум" << "В. Максимум");

    ui->tableWidget_sensors->setColumnCount(3);
    ui->tableWidget_sensors->setShowGrid(true);
    ui->tableWidget_sensors->setHorizontalHeaderLabels(QStringList() << "Адрес" << "Тип" << "Значние");

    ui->tableWidget_boxes_sensors->setColumnCount(3);
    ui->tableWidget_boxes_sensors->setShowGrid(true);
    ui->tableWidget_boxes_sensors->setHorizontalHeaderLabels(QStringList() << "Название" << "Адрес T" << "Адрес H");

    ui->tableWidget_device->setColumnCount(7);
    ui->tableWidget_device->setShowGrid(true);
    ui->tableWidget_device->setHorizontalHeaderLabels(QStringList() << "" << "Порт" << "Скорость" << "Биты данные" << "Стоп биты"<< "Вкл" << "Статус");
    ui->tableWidget_device->hideColumn(0);

    createTableDevice();

    connect(&AllSettings, &SaveSettings::sensorChanged, this, &Settings::updateTableSensors);
    connect(&AllSettings, &SaveSettings::sensorRemoved, this, &Settings::createTableSensors);
    connect(&AllSettings, &SaveSettings::valueChanged, this, &Settings::updateTableBox);


    ui->checkBox_errorTempMax->blockSignals(true);
    ui->checkBox_errorTempMax->setChecked(AllSettings.getSoundEnableMaxTemp());
    ui->checkBox_errorTempMax->blockSignals(false);

    ui->checkBox_errorTempMin->blockSignals(true);
    ui->checkBox_errorTempMin->setChecked(AllSettings.getSoundEnableMinTemp());
    ui->checkBox_errorTempMin->blockSignals(false);

    ui->checkBox_errorHumiMax->blockSignals(true);
    ui->checkBox_errorHumiMax->setChecked(AllSettings.getSoundEnableMaxHumi());
    ui->checkBox_errorHumiMax->blockSignals(false);

    ui->checkBox_errorHumiMin->blockSignals(true);
    ui->checkBox_errorHumiMin->setChecked(AllSettings.getSoundEnableMinHumi());
    ui->checkBox_errorHumiMin->blockSignals(false);

    ui->checkBox_errorData->blockSignals(true);
    ui->checkBox_errorData->setChecked(AllSettings.getSoundEnableErr());
    ui->checkBox_errorData->blockSignals(false);


    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("SettingsWindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    ui->splitter->restoreState(settings.value("splitter",ui->splitter->saveGeometry()).toByteArray());

    settings.endGroup();

    ui->lineEdit_pathReport->setText(AllSettings.getPathReport());

    createTableBox();
    createTableBoxSensors();


    fillPortsInfo();

    updateCom = new QTimer(this);
    connect(updateCom, SIGNAL(timeout()), this, SLOT(fillPortsInfo()));
    updateCom->start(1000);


    qDebug()<<"settings start";
}

Settings::~Settings()
{
    delete ui;
}

void Settings::createTableBox()
{    
    ui->tableWidget_boxes->blockSignals(true);
    ui->tableWidget_boxes->setRowCount(0);

    for(int i = 0;i< AllSettings.sizeBoxes();i++)
    {
        ui->tableWidget_boxes->insertRow(i);

        ui->tableWidget_boxes->setItem(i,0, new QTableWidgetItem(AllSettings.getBox(i)->getName()));

        QTableWidgetItem *item1 = new QTableWidgetItem;
        item1->setData(Qt::DisplayRole, AllSettings.getBox(i)->getTemp());
        ui->tableWidget_boxes->setItem(i,1, item1);
        item1->setFlags(item1->flags().setFlag(Qt::ItemIsEditable,false));

        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setData(Qt::DisplayRole, AllSettings.getBox(i)->getHumi());
        ui->tableWidget_boxes->setItem(i,2, item2);
        item2->setFlags(item2->flags().setFlag(Qt::ItemIsEditable,false));

        QTableWidgetItem *item3 = new QTableWidgetItem;
        item3->setData(Qt::DisplayRole, AllSettings.getBox(i)->getTempMin());
        ui->tableWidget_boxes->setItem(i,3, item3);

        QTableWidgetItem *item4 = new QTableWidgetItem;
        item4->setData(Qt::DisplayRole, AllSettings.getBox(i)->getTempMax());
        ui->tableWidget_boxes->setItem(i,4, item4);

        QTableWidgetItem *item5 = new QTableWidgetItem;
        item5->setData(Qt::DisplayRole, AllSettings.getBox(i)->getHumiMin());
        ui->tableWidget_boxes->setItem(i,5, item5);

        QTableWidgetItem *item6 = new QTableWidgetItem;
        item6->setData(Qt::DisplayRole, AllSettings.getBox(i)->getHumiMax());
        ui->tableWidget_boxes->setItem(i,6, item6);
    }

    ui->tableWidget_boxes->blockSignals(false);
}

void Settings::createTableSensors()
{    
    ui->tableWidget_sensors->setRowCount(0);

    for(int i = 0; i< AllSettings.sizeSensors();i++)
    {
        ui->tableWidget_sensors->insertRow(i);

        QTableWidgetItem *item1 = new QTableWidgetItem;
        item1->setData(Qt::DisplayRole, AllSettings.getSensor(i).adr);
        ui->tableWidget_sensors->setItem(i,0, item1);

        QString strType;
        if(AllSettings.getSensor(i).type == Temp)
            strType = "Temp";
        else if(AllSettings.getSensor(i).type == Humi)
            strType = "Humi";
        else if(AllSettings.getSensor(i).type == Volt)
            strType = "Volt";
        else
            strType = "unkn";

        ui->tableWidget_sensors->setItem(i,1, new QTableWidgetItem(strType));


        QTableWidgetItem *item4 = new QTableWidgetItem;
        item4->setData(Qt::DisplayRole, AllSettings.getSensor(i).value);
        ui->tableWidget_sensors->setItem(i,2, item4);
    }
}

void Settings::createTableBoxSensors()
{
    ui->tableWidget_boxes_sensors->blockSignals(true);
    ui->tableWidget_boxes_sensors->setRowCount(0);

    for(int i = 0;i< AllSettings.sizeBoxes();i++)
    {
        ui->tableWidget_boxes_sensors->insertRow(i);

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, AllSettings.getBox(i)->getName());
        ui->tableWidget_boxes_sensors->setItem(i,0, item);
        item->setFlags(item->flags().setFlag(Qt::ItemIsEditable,false));

        QTableWidgetItem *item1 = new QTableWidgetItem;
        item1->setData(Qt::DisplayRole, AllSettings.getBox(i)->getAdrSensorTemp());
        ui->tableWidget_boxes_sensors->setItem(i,1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setData(Qt::DisplayRole, AllSettings.getBox(i)->getAdrSensorHumi());
        ui->tableWidget_boxes_sensors->setItem(i,2, item2);
    }

    ui->tableWidget_boxes_sensors->blockSignals(false);
}

void Settings::createTableDevice()
{
    ui->tableWidget_device->blockSignals(true);
    ui->tableWidget_device->setRowCount(0);

    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("SettingsWindow");


    int size = settings.beginReadArray("ports");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        createTableDeviceItem();

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, i);
        ui->tableWidget_device->setItem(i,0, item);

        QComboBox* boxCom = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,1));
        if (!boxCom) return;
        boxCom->blockSignals(true);
        QString text = settings.value("com","none").toString();
        if(text != "none")
            boxCom->addItem(text);
        boxCom->setCurrentText(text);
        boxCom->blockSignals(false);

        QComboBox* speedBox = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,2));
        if (!speedBox) return;
        speedBox->blockSignals(true);
        speedBox->setCurrentText(settings.value("speed","9600").toString());
        speedBox->blockSignals(false);

        QComboBox* bitBox = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,3));
        if (!bitBox) return;
        bitBox->blockSignals(true);
        bitBox->setCurrentText(settings.value("bit","8").toString());
        bitBox->blockSignals(false);

        QComboBox* stopBitBox = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,4));
        if (!stopBitBox) return;
        stopBitBox->blockSignals(true);
        stopBitBox->setCurrentText(settings.value("stopBit","1").toString());
        stopBitBox->blockSignals(false);

        QCheckBox* enable = qobject_cast<QCheckBox*>(ui->tableWidget_device->cellWidget(i,5));
        if (!enable) return;
        enable->blockSignals(true);
        enable->setChecked(settings.value("enable",false).toBool());
        enable->blockSignals(false);

        sendValue(i);
    }

    ui->tableWidget_device->blockSignals(false);
}

int Settings::createTableDeviceItem()
{
    int i = ui->tableWidget_device->rowCount();
    ui->tableWidget_device->insertRow(i);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setData(Qt::DisplayRole, freeComIndex++);
    ui->tableWidget_device->setItem(i,0, item);

    QComboBox* comBox = new QComboBox();
    comBox->addItem("none");
    comBox->setCurrentText("none");
    connect(comBox, &QComboBox::currentTextChanged, this, &Settings::changetDevices);
    ui->tableWidget_device->setCellWidget(i,1, comBox);//

    QComboBox* speedBox = new QComboBox();
    speedBox->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    speedBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    speedBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    speedBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    speedBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    speedBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    speedBox->setCurrentText("9600");

    connect(speedBox, &QComboBox::currentTextChanged, this, &Settings::changetDevices);
    ui->tableWidget_device->setCellWidget(i,2, speedBox);


    QComboBox* bitBox = new QComboBox();
    bitBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    bitBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    bitBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    bitBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    bitBox->setCurrentText("8");

    connect(bitBox, &QComboBox::currentTextChanged, this, &Settings::changetDevices);
    ui->tableWidget_device->setCellWidget(i,3, bitBox);


    QComboBox* stopBitBox = new QComboBox();
    stopBitBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    stopBitBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
    stopBitBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    stopBitBox->setCurrentText("1");

    connect(stopBitBox, &QComboBox::currentTextChanged, this, &Settings::changetDevices);
    ui->tableWidget_device->setCellWidget(i,4, stopBitBox);

    QCheckBox* enable = new QCheckBox();
    enable->setChecked(false);

    connect(enable, &QCheckBox::stateChanged, this, &Settings::changetDevices);
    ui->tableWidget_device->setCellWidget(i,5, enable);

    QTableWidgetItem *status = new QTableWidgetItem;
    status->setData(Qt::DisplayRole, "Отключён");
    ui->tableWidget_device->setItem(i,6, status);

    return i;
}

void Settings::updateTableSensors(int i)
{
    if(AllSettings.sizeSensors() != ui->tableWidget_sensors->rowCount())//обновляем полностью
    {
        createTableSensors();
    }
    else
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, AllSettings.getSensor(i).value);
        ui->tableWidget_sensors->setItem(i,3, item);
    }
}

void Settings::updateTableBox(int i)
{
    if(AllSettings.sizeBoxes() != ui->tableWidget_boxes->rowCount())//не должен сюда зайти
    {
        createTableBox();
    }
    else
    {
        ui->tableWidget_boxes->blockSignals(true);

        ui->tableWidget_boxes->item(i,1)->setData(Qt::DisplayRole,AllSettings.getBox(i)->getTemp());
        ui->tableWidget_boxes->item(i,2)->setData(Qt::DisplayRole,AllSettings.getBox(i)->getHumi());

        ui->tableWidget_boxes->blockSignals(false);
    }
}

void Settings::updateTableBoxSensors(int i)
{
    ui->tableWidget_boxes_sensors->setItem(i,0, new QTableWidgetItem(AllSettings.getBox(i)->getName()));
}

void Settings::changetDevices()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (!widget)  return;

    QModelIndex index = ui->tableWidget_device->indexAt(widget->pos());

    int i = index.row();

    sendValue(i);
}

void Settings::sendValue(int row)
{
    if(row < 0 || row >= ui->tableWidget_device->rowCount()) return;

    qDebug()<<"send: " << row;

    deviceData device;
    device.index = ui->tableWidget_device->item(row,0)->text().toInt();

    QComboBox* boxCom = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(row,1));
    if (!boxCom) return;
    device.comName = boxCom->currentText();

    QComboBox* boxSpeed = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(row,2));
    if (!boxSpeed) return;
    device.speed = boxSpeed->currentText().toInt();

    QComboBox* boxDataBit = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(row,3));
    if (!boxDataBit) return;
    device.dataBit = boxDataBit->currentText().toInt();

    QComboBox* boxstopBit = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(row,3));
    if (!boxstopBit) return;
    device.dataBit = boxstopBit->currentText().toInt();

    QCheckBox* enable = qobject_cast<QCheckBox*>(ui->tableWidget_device->cellWidget(row,5));
    if (!enable) return;
    device.enable = enable->isChecked();

    emit changeCom(device);
}

int Settings::indexToRow(int index)
{
    for (int i = 0;i < ui->tableWidget_device->rowCount(); i++)
    {
        if(index == ui->tableWidget_device->item(i,0)->text().toInt())
            return i;
    }

    return -1;
}

void Settings::removeTableDeviceItem(int index)
{
    if(index < 0 || index > ui->tableWidget_device->rowCount()) return;

    int number = ui->tableWidget_device->item(index,0)->data(Qt::DisplayRole).toInt();

    emit removeCom(number);

    ui->tableWidget_device->removeRow(index);
}

void Settings::fillPortsInfo()
{
    QStringList list;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        list << info.portName();
    }

    for (int i = 0;i < ui->tableWidget_device->rowCount(); i++)
    {
        QComboBox* boxCom = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,1));
        if (!boxCom) return;

        boxCom->blockSignals(true);

        QString curentText = boxCom->currentText();

        boxCom->clear();

        if(!list.contains(curentText))
            boxCom->addItem(curentText);

        boxCom->addItems(list);
        boxCom->setCurrentText(curentText);

        boxCom->blockSignals(false);
    }
}

void Settings::deviseStatus(int index, QString msg)
{
    int row = indexToRow(index);
    if(row < 0) return;

    ui->tableWidget_device->item(row,6)->setData(Qt::DisplayRole,msg);
}

void Settings::closeEvent(QCloseEvent *event)
{
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("SettingsWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", ui->splitter->saveState());

    settings.beginWriteArray("ports");
    for (int i = 0; i < ui->tableWidget_device->rowCount(); i++)
    {
        settings.setArrayIndex(i);

        QComboBox* boxCom = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,1));
        if (!boxCom) return;
        settings.setValue("com", boxCom->currentText());

        QComboBox* boxSpeed = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,2));
        if (!boxSpeed) return;
        settings.setValue("speed", boxSpeed->currentText().toInt());

        QComboBox* boxDataBit = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,3));
        if (!boxDataBit) return;
        settings.setValue("bit", boxDataBit->currentText().toInt());

        QComboBox* boxstopBit = qobject_cast<QComboBox*>(ui->tableWidget_device->cellWidget(i,4));
        if (!boxstopBit) return;
        settings.setValue("stopBit", boxstopBit->currentText().toInt());

        QCheckBox* enable = qobject_cast<QCheckBox*>(ui->tableWidget_device->cellWidget(i,5));
        if (!enable) return;
        settings.setValue("enable", enable->isChecked());
    }
    settings.endArray();

    settings.endGroup();

    settings.sync();

    event->accept();
}

void Settings::on_tableWidget_boxes_itemChanged(QTableWidgetItem *item)
{
    int row = item->row();
    int column = item->column();

    switch (column)
    {
    case 0:
        AllSettings.setName(row,item->data(Qt::DisplayRole).toString());
        updateTableBoxSensors(row);
        break;
    case 3:
        AllSettings.setTempMin(row,item->data(Qt::DisplayRole).toDouble());
        break;
    case 4:
        AllSettings.setTempMax(row,item->data(Qt::DisplayRole).toDouble());
        break;
    case 5:
        AllSettings.setHumiMin(row,item->data(Qt::DisplayRole).toDouble());
        break;
    case 6:
        AllSettings.setHumiMax(row,item->data(Qt::DisplayRole).toDouble());
        break;
    }
}

void Settings::on_tableWidget_boxes_sensors_itemChanged(QTableWidgetItem *item)
{
    int row = item->row();
    int column = item->column();

    switch (column)
    {
    case 1:
        AllSettings.setTempAddress(row,item->data(Qt::DisplayRole).toInt());
        break;
    case 2:
        AllSettings.setHumiAddress(row,item->data(Qt::DisplayRole).toInt());
        break;
    }
}

void Settings::on_pushButtonPassword_clicked()
{
    AllSettings.setPassword(ui->lineEditPassword->text());
    ui->lineEditPassword->clear();
}

void Settings::on_pushButton_pathReport_clicked()
{
    QString str = QFileDialog::getExistingDirectory(0, "Выбор папки", "");

    if (!str.isEmpty())
    {
        ui->lineEdit_pathReport->setText(str);
        AllSettings.setPathReport(str);
    }
}

void Settings::on_checkBox_errorTempMax_clicked(bool checked)
{
    AllSettings.setSoundEnableMaxTemp(checked);
}

void Settings::on_checkBox_errorTempMin_clicked(bool checked)
{
    AllSettings.setSoundEnableMinTemp(checked);
}

void Settings::on_checkBox_errorHumiMax_clicked(bool checked)
{
    AllSettings.setSoundEnableMaxHumi(checked);
}

void Settings::on_checkBox_errorHumiMin_clicked(bool checked)
{
    AllSettings.setSoundEnableMinHumi(checked);
}

void Settings::on_checkBox_errorData_clicked(bool checked)
{
    AllSettings.setSoundEnableErr(checked);
}

void Settings::on_pushButton_addDevice_clicked()
{
    int row = createTableDeviceItem();

    fillPortsInfo();

    sendValue(row);
}

void Settings::on_pushButton_remove_clicked()
{
    int i = ui->tableWidget_device->currentRow();

    removeTableDeviceItem(i);
}


void Settings::on_time_valueChanged(int arg1)
{
    AllSettings.setTime(arg1);
}

void Settings::on_spinBox_poll_valueChanged(int arg1)
{
    AllSettings.setTimePool(arg1);
}
