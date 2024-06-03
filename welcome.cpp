#include "welcome.h"
#include "ui_welcome.h"

welcome::welcome(QWidget *parent)
    : socket_tab(parent)
    , ui(new Ui::welcome)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    this->m_socket_thread=nullptr;
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

