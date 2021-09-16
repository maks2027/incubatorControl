#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  

    qRegisterMetaType < sensorData > ( "sensorData" );
    qRegisterMetaType < deviceData > ( "deviceData" );


    SaveSettings allSettings;

    MainWindow w(allSettings);

    w.show();

    return a.exec();
}
