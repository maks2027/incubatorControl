#include "mainwindow.h"

#include <QApplication>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile fMessFile(qApp->applicationDirPath() + "/log.txt");
    if(!fMessFile.open(QIODevice::Append | QIODevice::Text | QIODevice::WriteOnly))
    {
        return;
    }
    QString sCurrDateTime = "[" +
            QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + "]";
    QTextStream tsTextStream(&fMessFile);
    switch(type){
    case QtInfoMsg:
        tsTextStream << QString("%1 Info - %2 in %3 %4").arg(sCurrDateTime).arg(msg).arg(context.file).arg(context.line)<<Qt::endl;
        break;
    case QtDebugMsg:
    {
        QTextStream out(stdout);
        out<<msg<<Qt::endl;
        break;
    }
    case QtWarningMsg:
        tsTextStream << QString("%1 Warning - %2 in %3 %4").arg(sCurrDateTime).arg(msg).arg(context.file).arg(context.line)<<Qt::endl;
        break;
    case QtCriticalMsg:
        tsTextStream << QString("%1 Critical - %2 in %3 %4").arg(sCurrDateTime).arg(msg).arg(context.file).arg(context.line)<<Qt::endl;
        break;
    case QtFatalMsg:
        tsTextStream << QString("%1 Fatal - %2 in %3 %4").arg(sCurrDateTime).arg(msg).arg(context.file).arg(context.line)<<Qt::endl;
        abort();
    }
    tsTextStream.flush();
    fMessFile.flush();
    fMessFile.close();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QApplication a(argc, argv);  

    qRegisterMetaType < sensorData > ( "sensorData" );
    qRegisterMetaType < deviceData > ( "deviceData" );


    SaveSettings allSettings;

    MainWindow w(allSettings);

    w.show();

    return a.exec();
}
