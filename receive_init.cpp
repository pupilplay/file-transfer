#include "receive_init.h"
#include "ui_receive_init.h"

receive_init::receive_init(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::receive_init)
{
    ui->setupUi(this);
    ui->port_input->setValidator((new QIntValidator(0, 65535, ui->port_input)));
}

receive_init::~receive_init()
{
    delete ui;
}

void receive_init::on_listen_btn_clicked()
{
    if (this->ui->port_input->hasAcceptableInput())
        emit listen_query(this->ui->port_input->text());
    else
        QMessageBox::warning(this, "error", "the port mustn't be empty");
}

void receive_init::on_cancel_btn_clicked()
{
    this->close();
}
