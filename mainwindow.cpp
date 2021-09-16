#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>

MainWindow::MainWindow(SaveSettings &AllSettings,QWidget *parent)
    : QMainWindow(parent),AllSettings(AllSettings),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    h = new History(AllSettings,this);
    s = new Settings(AllSettings,this);
    r = new Report(AllSettings,this);
    pass = new DialogPassword(AllSettings,this);

    about = new QMessageBox(this);
    about->setWindowTitle("О программе");
    about->setText(QString("Инкубатор v2 0.9.4\nАвтор: Киян Максим Викторович\nQt %0").arg(qVersion()));


    connect(&AllSettings, SIGNAL(valueChanged(int)), this, SLOT(updateBox(int)));
    connect(&AllSettings, SIGNAL(settingsChanged(int)), this, SLOT(updateBox(int)));
    connect(&AllSettings, SIGNAL(noDataBox(int)), this, SLOT(updateBox(int)));
    connect(&AllSettings, SIGNAL(noDataBox(int)), this, SLOT(errorData()));

    connect(r, &Report::openHistory, h, &History::openHistory);
    connect(this, &MainWindow::openHistory, h, &History::openHistory);

    font.setPixelSize(26);
    font2.setPixelSize(14);
    font3.setPixelSize(10);

    int numberBox = 0;

    QHBoxLayout *hboxV = new QHBoxLayout();
    hboxV ->setSpacing(10);

    for(int i = 0; i < 9 ;i++)
    {
        QFrame *tempBoxes = new QFrame;
        tempBoxes->setObjectName(QString("f%0").arg(numberBox));
        tempBoxes->setStyleSheet(QString("QFrame#f%0 {border: 2px solid black}").arg(numberBox));

        QVBoxLayout *tempVbox = new QVBoxLayout();
        tempVbox->setSpacing(0);
        tempVbox->setMargin(0);
        tempBoxes->setLayout(tempVbox);

        QFrame* tempBox = createBox(numberBox++);

        tempVbox->addWidget(tempBox);

        hboxV->addWidget(tempBoxes);
    }

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(10);
    for(int i = 0;i < 10; i++)
    {
        QVBoxLayout *vbox = new QVBoxLayout();
        vbox->setSpacing(1);

        if(i == 1 || i == 4 || i == 5 || i == 8)
            hbox->addSpacing(80);
        else
        {
            for(int k = 0;k < 2; k++)//чтоб не копировать и не создавать функцию
            {
                QFrame *tempBoxes = new QFrame;
                tempBoxes->setObjectName(QString("f%0").arg(numberBox));
                tempBoxes->setStyleSheet(QString("QFrame#f%0 {border: 2px solid black}").arg(numberBox));

                QVBoxLayout *tempVbox = new QVBoxLayout();
                tempVbox->setSpacing(0);
                tempVbox->setMargin(0);
                tempBoxes->setLayout(tempVbox);

                for(int j = 0;j < 3 ; j++)//вертикальная линия шкафов
                {
                    QFrame* tempBox = createBox(numberBox++);
                    tempVbox->addWidget(tempBox);
                }

                if(k == 1) vbox->addSpacing(50);

                vbox->addWidget(tempBoxes);
            }

            hbox->addLayout(vbox);
        }

    }

    QVBoxLayout *vob = new QVBoxLayout();
    vob->setSpacing(1);

    vob->addLayout(hboxV,1);
    vob->addSpacing(40);
    vob->addLayout(hbox,6);


    QWidget *widget = new QWidget();
    widget->setLayout(vob);
    setCentralWidget(widget);

    widget->setObjectName("GW");
    widget->setStyleSheet("QWidget#GW {background-color: rgb(100, 100, 100)}");


    dialog = new Dialog(AllSettings,this);
    dialog->setModal(true);

    connect(dialog, SIGNAL(updateDataBase()), this, SLOT(readBoxBd()));
    connect(&AllSettings, SIGNAL(settingsChanged(int)), dialog, SLOT(updateCombo()));
    connect(dialog, SIGNAL(newReport()), r, SLOT(readTable()));

    readBoxBd();

    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state",saveState()).toByteArray());
    settings.endGroup();
}

MainWindow::~MainWindow()
{
    delete h;
    delete s;
    delete about;

    delete ui;
}

void MainWindow::readBoxBd()
{
    QSqlDatabase db = QSqlDatabase::database(AllSettings.dbPath);
    if(!db.open())return;

    QVector<bool> boxes(AllSettings.sizeBoxes(),false);
    QVector<QDateTime> date(AllSettings.sizeBoxes(),QDateTime());

    QSqlQuery query(db);

    QString strSQL = QString("SELECT * FROM unf_stowage  WHERE date_2 IS NULL");// заселёные но не переселёные
    if(query.exec(strSQL))// заселёные но не переселёные
    {
        while(query.next())
            if(query.value(1).toInt() < boxes.size())
            {
                boxes[query.value(1).toInt()] = true;
                date[query.value(1).toInt()] = query.value(3).toDateTime();
            }

    }

    QString strSQL2 = QString("SELECT * FROM unf_stowage  WHERE date_3 IS NULL AND date_2 IS NOT NULL");// переселёные
    if(query.exec(strSQL2))// заселёные но не переселёные
    {
        while(query.next())
            if(query.value(4).toInt() < boxes.size())
            {
                boxes[query.value(4).toInt()] = true;
                date[query.value(4).toInt()] = query.value(5).toDateTime();
            }
    }

    for(int i = 0; i < boxes.size();i++)
    {
        AllSettings.setActive(i,boxes[i]);
        AllSettings.setDateTime(i,date[i]);
    }

    for(int i = 0;i<AllSettings.sizeBoxes();i++)
    {
        updateBox(i);
    }

    updateToolTip();
}

void MainWindow::errorData()//нет данных
{
    //    QMessageBox messageBox;
    //    messageBox.critical(0,"Error","An error has occured !");
    //    messageBox.setFixedSize(500,200);
}

void MainWindow::updateToolTip()
{
    for(int i = 0; i < boxes.size(); i++)
    {
        QString str;
        str.append(QString("%0").arg(boxes[i].name->text()));

        if(AllSettings.getBox(i)->getEnabled())
        {
            QDateTime date = AllSettings.getBox(i)->getEnableDate();
            str.append(QString("\nЗаселён: %1").arg(date.toString("hh:mm dd-MM-yyyy")));

            if(i >= 9)
            {
                if(date.date().addDays(3) >= QDateTime::currentDateTime().date())
                {
                    if(date.date().addDays(3) == QDateTime::currentDateTime().date())
                        str.append(QString("\nПриоткрыть заслонку: сегодня").arg(date.toString("hh:mm dd-MM-yyyy")));
                    else
                        str.append(QString("\nПриоткрыть заслонку: %1").arg(date.date().addDays(3).toString("dd-MM-yyyy")));

                    if(date.date().addDays(21) == QDateTime::currentDateTime().date())
                        str.append(QString("\nПереселение: сегодня").arg(date.toString("hh:mm dd-MM-yyyy")));
                    else
                        str.append(QString("\nПереселение: %1").arg(date.date().addDays(21).toString("dd-MM-yyyy")));
                }
            }
            else
            {
                if(date.date().addDays(3) == QDateTime::currentDateTime().date())
                    str.append(QString("\nВыведение: сегодня").arg(date.toString("hh:mm dd-MM-yyyy")));
                else
                    str.append(QString("\nВыведение: %1").arg(date.date().addDays(3).toString("dd-MM-yyyy")));
            }
        }

        boxes[i].box->setToolTip(str);

    }
}

void MainWindow::updateBox(int index)
{
    if(index >= boxes.size())return;

    boxes[index].name->setText(AllSettings.getBox(index)->getName());

    if(AllSettings.getBox(index)->getEnabled())
    {
        if(AllSettings.getBox(index)->getIsTemp())
        {
            double tempValue = AllSettings.getBox(index)->getTemp();


            QString strTemp = QString::number(tempValue, 'f', 1);
            strTemp.append(" °C");


            boxes[index].temp->setText(strTemp);


            double min = AllSettings.getBox(index)->getTempMin();
            double max = AllSettings.getBox(index)->getTempMax();

            if(tempValue > max)
                boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(200, 70, 70)}").arg(index));
            else if(tempValue < min)
                boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(70, 70, 200)}").arg(index));
            else
                boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(70, 200, 70)}").arg(index));

            boxes[index].temp->setStyleSheet("color: rgb(0, 0, 0)");
        }
        else
        {
            boxes[index].temp->setText("err");

            boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(130, 130, 130)}").arg(index));
            boxes[index].temp->setStyleSheet("color: rgb(70, 70, 70)");
        }

        if(AllSettings.getBox(index)->getIsHumi())
        {

            double humiValue = AllSettings.getBox(index)->getHumi();

            QString strHumi = QString::number(humiValue, 'f', 1);
            strHumi.append(" %");

            boxes[index].humi->setText(strHumi);

            boxes[index].humi->setStyleSheet("color: rgb(0, 0, 0)");
        }
        else
        {
            boxes[index].humi->setText("err");

            //boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(130, 130, 130)}").arg(index));
            boxes[index].humi->setStyleSheet("color: rgb(70, 70, 70)");
        }

    }
    else
    {
        boxes[index].temp->setText("-");
        boxes[index].humi->setText("-");

        boxes[index].box->setStyleSheet(QString("QFrame#grop%0 {border: 2px solid black;background-color: rgb(130, 130, 130)}").arg(index));
        boxes[index].temp->setStyleSheet("color: rgb(70, 70, 70)");
        boxes[index].humi->setStyleSheet("color: rgb(70, 70, 70)");
    }
}


QFrame* MainWindow::createBox(int index)
{
    QFrame *frame = new QFrame();

    QLabel *nameLabel = new QLabel();
    nameLabel->setFont(font2);
    nameLabel->setText(AllSettings.getBox(index)->getName());
    nameLabel->setAlignment(Qt::AlignCenter);

    QLabel *tempLabel = new QLabel();
    tempLabel->setFont(font);
    tempLabel->setText("37.7");
    tempLabel->setAlignment(Qt::AlignCenter);

    QLabel *humiLabel = new QLabel();
    humiLabel->setFont(font);
    humiLabel->setText("55.5");
    humiLabel->setAlignment(Qt::AlignCenter);

    boxes.append(boxUi(frame,nameLabel,tempLabel,humiLabel));

    frame->installEventFilter(this);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(nameLabel,1);
    vbox->addWidget(tempLabel,6);
    vbox->addWidget(humiLabel,6);

    frame->setLayout(vbox);


    frame->setObjectName(QString("grop%0").arg(index));
    frame->setStyleSheet(QString("QFrame#grop%0 {border: 5px solid black; background-color: rgb(200, 70, 70)}").arg(index));

    return frame;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool bOk = false;
    QString str = QInputDialog::getText( 0,
                                         "Выйти?",
                                         "Пароль:",
                                         QLineEdit::Password,
                                         "",
                                         &bOk
                                         );
    if(!bOk || str != AllSettings.getPassword())
    {
        event->ignore();
    }
    else
    {
        QSettings settings("settings.ini",QSettings::IniFormat);
        settings.setIniCodec("UTF-8");

        settings.beginGroup("MainWindow");

        settings.setValue("geometry", saveGeometry());
        settings.setValue("state", saveState());
        settings.endGroup();

        settings.sync();

        event->accept();
    }
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        for(int i = 0;i<boxes.size();i++)
        {
            if(boxes[i].box == target)
            {
                pass->exec();

                if(pass->isPassword())
                {
                    dialog->mapToGlobal(pos());
                    dialog->setIndex(i);
                }
            }
        }
    }
    else if(event->type() == QEvent::ContextMenu)
    {
        for(int i = 0;i<boxes.size();i++)
        {
            if(boxes[i].box == target)
            {
                menu->clear();

                QAction *dayData = menu->addAction("Данные за сутки");


                QAction *selectedAction = menu->exec(QCursor::pos());


                if(selectedAction  == dayData)
                {
                    emit openHistory(i, QDateTime::currentDateTime().addDays(-1), QDateTime::currentDateTime());
                }

            }
        }
    }


    return false;
}

void MainWindow::on_action_triggered()
{
    h->show();
    h->activateWindow();
}

void MainWindow::on_action_2_triggered()
{
    s->show();
    s->activateWindow();
}

void MainWindow::on_action_3_triggered()
{
    r->show();
    r->activateWindow();
}

void MainWindow::on_action_4_triggered()
{
    about->open();
}
