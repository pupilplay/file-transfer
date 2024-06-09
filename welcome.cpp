#include "welcome.h"
#include "ui_welcome.h"

welcome::welcome(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::welcome)
{
    ui->setupUi(this);
}

welcome::~welcome()
{
    delete ui;
}

void welcome::on_receive_button_clicked()
{
    emit receive_query();
}

void welcome::on_send_button_clicked()
{
    emit send_query();
}
