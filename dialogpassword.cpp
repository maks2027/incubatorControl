#include "dialogpassword.h"
#include "ui_dialogpassword.h"

DialogPassword::DialogPassword(SaveSettings &AllSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPassword),
    AllSettings(AllSettings)

{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowTitle("Ввод пароля");
    setFixedSize(200,80);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

DialogPassword::~DialogPassword()
{
    delete ui;
}

bool DialogPassword::isPassword()
{
    return password;
}

void DialogPassword::on_pushButton_clicked()
{    
    if(ui->lineEdit->text() == AllSettings.getPassword())
        password = true;
    else
        password = false;

    close();
}

void DialogPassword::showEvent(QShowEvent *event)
{
    password = false;
    ui->lineEdit->clear();

    event->accept();
}
