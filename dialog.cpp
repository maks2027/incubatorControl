#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(SaveSettings &AllSettings, QWidget *parent):
    QDialog(parent),
    ui(new Ui::Dialog),
    AllSettings(AllSettings)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setFixedSize(250,120);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    updateCombo();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setIndex(int index)
{
    flagStatus = -1;
    indexBox = index;

    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    db.setDatabaseName(AllSettings.dbPath);
    if(!db.open())return;

    QSqlQuery query(db);

    QString strSQL = QString("SELECT count(*) FROM unf_stowage  WHERE id_box_1 = %0 AND date_2 IS NULL").arg(index);// заселёные но не переселёные
    if(query.exec(strSQL))// заселёные но не переселёные
    {
        query.next();

        if(query.value(0).toInt() == 1)
            flagStatus = 1;
        else
        {
            QString strSQL = QString("SELECT count(*) FROM unf_stowage  WHERE id_box_2 = %0 AND date_3 IS NULL").arg(index);// переселёные в выводной
            if(query.exec(strSQL))// переселёные в выводной
            {
                query.next();

                if(query.value(0).toInt() == 1)
                    flagStatus = 2;
                else
                    flagStatus = 0;
            }
        }
    }
    else return;

    if(flagStatus == 0)
    {
        if(indexBox < 9)  return;//запрет заселения в выводной

        ui->widget_1->show();
        ui->widget_2->hide();
        ui->widget_3->hide();

        setWindowTitle("Заселение");

        //resize(0,0);

        ui->dateTimeEdit_1->setDateTime(QDateTime::currentDateTime());
    }
    else if(flagStatus == 1)
    {
        ui->widget_1->hide();
        ui->widget_2->show();
        ui->widget_3->hide();

        setWindowTitle("Переселение");

        //resize(0,0);

        ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
    }
    else if(flagStatus == 2)
    {
        ui->widget_1->hide();
        ui->widget_2->hide();
        ui->widget_3->show();

        setWindowTitle("Выселение");

        //resize(0,0);

        ui->dateTimeEdit_3->setDateTime(QDateTime::currentDateTime());
    }
    else return;

    updateCombo();

    show();
}

void Dialog::on_pushButton_1_clicked()
{
    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO unf_stowage (id_box_1, eggs, date_1) VALUES (:id,:val,:date)"));
    query.bindValue(":id", indexBox);
    query.bindValue(":val", ui->spinBox->value());
    query.bindValue(":date", ui->dateTimeEdit_1->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if(!query.exec())
        emit errorMessege("Бд блокирована");

    emit updateDataBase();

    close();
}

void Dialog::on_pushButton_2_clicked()
{
    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    QSqlQuery query(db);
    query.prepare(QString("UPDATE unf_stowage SET id_box_2=:id_2,date_2=:date WHERE id_box_1=:id AND date_2 IS NULL"));
    query.bindValue(":id", indexBox);
    query.bindValue(":id_2", ui->comboBox->currentData().toInt());
    query.bindValue(":date", ui->dateTimeEdit_2->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if(!query.exec())
        emit errorMessege("Бд блокирована");

    emit updateDataBase();

    close();
}

void Dialog::on_pushButton_3_clicked()
{
    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    int value = ui->spinBox_2->value();
    QString date = ui->dateTimeEdit_3->dateTime().toString("yyyy-MM-dd hh:mm:ss");


    if(!db.transaction())//emit errorMessege("Бд блокирована");
        qDebug()<<"Bd blok";

    QSqlQuery query(db);

    query.prepare(QString("UPDATE unf_stowage SET chickens=:val,date_3=:date WHERE id_box_2=:id AND date_3 IS NULL"));
    query.bindValue(":id", indexBox);
    query.bindValue(":val", value);
    query.bindValue(":date", date);
    query.exec();

    query.prepare(QString("INSERT INTO stowage SELECT * FROM unf_stowage WHERE id_box_2=:id AND chickens=:val AND date_3=:date"));
    query.bindValue(":id", indexBox);
    query.bindValue(":val", value);
    query.bindValue(":date", date);
    query.exec();

    query.prepare(QString("DELETE FROM unf_stowage WHERE id_box_2=:id AND chickens=:val AND date_3=:date"));
    query.bindValue(":id", indexBox);
    query.bindValue(":val", value);
    query.bindValue(":date", date);
    query.exec();

    db.commit();

    emit updateDataBase();
    emit newReport();

    close();
}

void Dialog::updateCombo()
{
    ui->comboBox->clear();

    for(int i = 0; i < AllSettings.sizeBoxes() && i < 9;i++)
    {
        if(!AllSettings.getBox(i)->getEnabled())
            ui->comboBox->addItem(AllSettings.getBox(i)->getName(),i);
    }
}
