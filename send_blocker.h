#ifndef SEND_BLOCKER_H
#define SEND_BLOCKER_H

#include <QDialog>

namespace Ui {
class send_blocker;
}

class send_blocker : public QDialog
{
    Q_OBJECT

public:
    explicit send_blocker(QWidget *parent = nullptr);
    ~send_blocker();
signals:
    void disconnect();

private:
    Ui::send_blocker *ui;
};

#endif // SEND_BLOCKER_H
