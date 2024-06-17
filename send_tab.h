#ifndef SEND_TAB_H
#define SEND_TAB_H

#include <QFileDialog>
#include <QIntValidator>
#include <QMessageBox>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QWidget>
#include "file_info.h"
#include "send_blocker.h"
#include "socket_tab.h"

namespace Ui {
class send_tab;
}

class cworker : public QObject
{
    Q_OBJECT
    //for accessing the member variable shutdown and interval conveniently
    friend class send_tab;

public:
    cworker();
    ~cworker();
public slots:
    //initialize the QTcpSocket
    void connect_to_host(QString ip, QString port);

    //get the file info and send to the server
    void init(QString path);

    //transfer the data
    void send();
signals:

    //emitted in function send() to notify the send_tab to update the information
    void send_started();
    void send_finished();
    void send_failed();
    void send_progress(int percentage);

    //pass the corresponding QTcpSocket signals
    void connected();
    void disconnected();
    void error_occurred(QTcpSocket::SocketError error);

private:
    QTcpSocket *socket;
    QFile file;
    file_info finfo;
    unsigned long interval = 200;
    bool shutdown;
};

class send_tab : public socket_tab
{
    Q_OBJECT

public:
    explicit send_tab(QWidget *parent = nullptr);
    explicit send_tab(QWidget *parent, QString ip, QString port);
    ~send_tab();

signals:
    //used to make the cworker start transferring
    void send_start(QString path);

    //work as a temporary signal to initialize the socket in another thread
    void socket_init(QString ip, QString port);

public slots:

    //disable the send_tab when the socket is disconnected
    void abandon();

    //update the progress bar with the size of sent data
    void progress_update(int percentage);
private slots:
    void on_select_btn_clicked();
    void on_send_btn_clicked();

    //change the interval in cworker
    void on_interval_input_textEdited(const QString &arg1);

private:
    Ui::send_tab *ui;
    cworker *m_worker;
};

#endif // SEND_TAB_H
