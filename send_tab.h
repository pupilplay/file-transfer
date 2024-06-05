#ifndef SEND_TAB_H
#define SEND_TAB_H

#include <QWidget>
#include<QThread>
#include<QTcpSocket>
#include<QObject>
#include"socket_tab.h"
#include<QFileDialog>
#include<QFile>
#include<QFileInfo>
#include<QEventLoop>
#include<QMessageBox>
#include<QIntValidator>
#include"file_info.h"
#include"send_blocker.h"

namespace Ui {
class send_tab;
}

class cworker:public QObject
{
    Q_OBJECT
    friend class send_tab;
public:
    cworker();
    ~cworker();
public slots:
    void connect_to_host(QString ip,QString port);
    void init(QString path);
    void quit();
    void send();
    void set_interval(unsigned long interval);
signals:
    void send_started();
    void send_finished();
    void send_failed();
    void connected();
    void disconnected();
    void error_occurred(QTcpSocket::SocketError error);
private:
    QTcpSocket* socket;
    QFile file;
    file_info finfo;
    unsigned long interval=500;
    bool shutdown;
};

class send_tab : public socket_tab
{
    Q_OBJECT

public:
    explicit send_tab(QWidget *parent = nullptr);
    explicit send_tab(QWidget *parent,QString ip,QString port);
    ~send_tab();

signals:
    void send_start(QString path);
    void socket_init(QString ip,QString port);
    void interval_changed(unsigned long interval);
public slots:
    void abandon();
private slots:
    void on_select_btn_clicked();
    void on_send_btn_clicked();

    void on_interval_input_textEdited(const QString &arg1);

private:
    Ui::send_tab *ui;
    cworker* m_worker;
};


#endif // SEND_TAB_H
