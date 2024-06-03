#ifndef WELCOME_H
#define WELCOME_H

#include <QWidget>
#include"socket_tab.h"
namespace Ui {
class welcome;
}

class welcome : public socket_tab
{
    Q_OBJECT

public:
    explicit welcome(QWidget *parent = nullptr);
    ~welcome();

private slots:
    void on_receive_button_clicked();

    void on_send_button_clicked();
signals:
    void send_query();
    void receive_query();
private:
    Ui::welcome *ui;
};

#endif // WELCOME_H
