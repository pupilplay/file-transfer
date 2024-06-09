#ifndef SEND_BLOCKER_H
#define SEND_BLOCKER_H

#include <QDialog>

namespace Ui {
class send_blocker;
}

//a simple blocker before the socket in the send_tab finishes its initialization
class send_blocker : public QDialog
{
    Q_OBJECT

public:
    explicit send_blocker(QWidget *parent = nullptr);
    ~send_blocker();
signals:
    //emitted when cancel is clicked, stop and close the send_tab
    void disconnect_query();

private slots:
    void on_cancel_btn_clicked();

private:
    Ui::send_blocker *ui;
};

#endif // SEND_BLOCKER_H
