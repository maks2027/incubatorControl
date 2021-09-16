#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "savesettings.h"

#include <QSqlQuery>
#include <QtSql>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

    int indexBox;
    SaveSettings &AllSettings;

    int flagStatus;

public:
    explicit Dialog(SaveSettings &AllSettings, QWidget *parent = nullptr);
    ~Dialog();

    void setIndex(int index);

private:
    Ui::Dialog *ui;

signals:
    void updateDataBase();
    void newReport();
    void errorMessege(QString msg);
private slots:
    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    void updateCombo();
};

#endif // DIALOG_H
