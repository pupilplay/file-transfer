#include "send_init.h"
#include "ui_send_init.h"

send_init::send_init(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::send_init)
{
    ui->setupUi(this);
    ui->ip_input->setValidator(new QRegularExpressionValidator(
        QRegularExpression(
            "(([0-1]?\\d{1,2}|2[0-4]\\d|25[0-5])\\.){3}[0-1]?\\d{1,2}|2[0-4]\\d|25[0-5]"),
        ui->ip_input));
    ui->port_input->setValidator((new QIntValidator(0, 65535, ui->port_input)));
}

send_init::~send_init()
{
    delete ui;
}

void send_init::on_cancel_btn_clicked()
{
    this->close();
}

void send_init::on_connect_btn_clicked()
{
    if (this->ui->ip_input->hasAcceptableInput() && this->ui->port_input->hasAcceptableInput())
        emit connect_query(this->ui->ip_input->text(), this->ui->port_input->text());
    else
        QMessageBox::warning(this, "error", "the ip or the port is invalid");
}
