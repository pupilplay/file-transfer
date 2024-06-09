#ifndef SEND_INIT_H
#define SEND_INIT_H

#include <QDialog>
#include<QValidator>
#include<QHostAddress>
#include<QMessageBox>

namespace Ui {
class send_init;
}



class send_init : public QDialog
{
    Q_OBJECT

public:
    explicit send_init(QWidget *parent = nullptr);
    ~send_init();

private slots:
    void on_cancel_btn_clicked();

    void on_connect_btn_clicked();
signals:
    //emitted when connect button is clicked and pass the ip and the port to the socket to be initialized
    void connect_query(QString ip,QString port);
private:
    Ui::send_init *ui;
};

#endif // SEND_INIT_H
