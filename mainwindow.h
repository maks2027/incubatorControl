#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>
#include <QtNetwork>
#include <QFrame>
#include <QMessageBox>
#include <QMenu>
#include <QEvent>

//#include "dbwriter.h"
#include "settings.h"
#include "history.h"
#include "savesettings.h"
#include "dialog.h"

#include "report.h"
#include "dialogpassword.h"

#include "TypeData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QFont font;
    QFont font2;
    QFont font3;



    struct boxUi
    {
        boxUi(QFrame* box,QLabel* name,QLabel* temp,QLabel* humi):box(box),name(name),temp(temp),humi(humi){}
        QFrame* box;
        QLabel* name;
        QLabel* temp;
        QLabel* humi;
    };

    QVector<boxUi> boxes;

    SaveSettings &AllSettings;

    Dialog* dialog;

public:
    MainWindow(SaveSettings &_AllSettings,QWidget *parent = nullptr);
    ~MainWindow();

    QFrame* createBox(int index);

private:
    Ui::MainWindow *ui;

    History *h;
    Settings *s;
    Report *r;
    DialogPassword *pass;
    QMenu *menu = new QMenu(this);

    qint32 ArrayToInt(QByteArray source);

    QMessageBox* about;

private slots:    

    void on_action_triggered();

    void on_action_2_triggered();

    void on_action_3_triggered();

    void on_action_4_triggered();

    //void showSettings(QString str);

    //void hourHistory(int id);


public slots:
    //void setValue(int i, double value);//вставка плюс цвет по значению и сигналмзация
    //void setSensor(sensorData s);

    void updateBox(int index);

    void readBoxBd();

    void errorData();

    void updateToolTip();
protected:
    void closeEvent(QCloseEvent *event);

    bool eventFilter(QObject *target, QEvent *event);

    //void contextMenuEvent(QContextMenuEvent *event);
signals:
    void openHistory(int indBox, QDateTime date1, QDateTime date2);


};
#endif // MAINWINDOW_H
