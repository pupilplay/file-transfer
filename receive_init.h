#ifndef RECEIVE_INIT_H
#define RECEIVE_INIT_H

#include <QDialog>
#include<QValidator>
#include<QMessageBox>

namespace Ui {
class receive_init;
}

class receive_init : public QDialog
{
    Q_OBJECT

public:
    explicit receive_init(QWidget *parent = nullptr);
    ~receive_init();

signals:
    //emitted when listen button is clicked and pass the port to the server to be initialized
    void listen_query(QString port);
private slots:
    void on_listen_btn_clicked();

    void on_cancel_btn_clicked();

private:
    Ui::receive_init *ui;
};

#endif // RECEIVE_INIT_H
