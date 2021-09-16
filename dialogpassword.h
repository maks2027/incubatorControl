#ifndef DIALOGPASSWORD_H
#define DIALOGPASSWORD_H

#include <QDialog>
#include <QShowEvent>
#include "savesettings.h"

namespace Ui {
class DialogPassword;
}

class DialogPassword : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPassword(SaveSettings &AllSettings, QWidget *parent = nullptr);
    ~DialogPassword();

    bool isPassword();

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogPassword *ui;

    SaveSettings &AllSettings;

    bool password = false;

protected:
    void showEvent(QShowEvent *event);

};

#endif // DIALOGPASSWORD_H
