#include "send_blocker.h"
#include "ui_send_blocker.h"

send_blocker::send_blocker(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::send_blocker)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setWindowFlags(Qt::CustomizeWindowHint);
}

send_blocker::~send_blocker()
{
    delete ui;
}


