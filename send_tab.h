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
#include"file_info.h"

namespace Ui {
class send_tab;
}

class cworker:public QObject
{
    Q_OBJECT
public:
    cworker(QTcpSocket* socket);
    ~cworker();
public slots:
    void init(QString path);
    void quit();
    void send();
signals:
    void send_started();
    void send_finished();
private:
    QTcpSocket* socket;
    QEventLoop loop;
    QFile file;
    file_info finfo;
};

class send_tab : public socket_tab
{
    Q_OBJECT

public:
    explicit send_tab(QWidget *parent = nullptr);
    explicit send_tab(QWidget *parent,QTcpSocket* socket);
    ~send_tab();

signals:
    void send_start(QString path);
    void client_quit();
public slots:
    void abandon();
private slots:
    void on_select_btn_clicked();
    void on_send_btn_clicked();

private:
    Ui::send_tab *ui;
    cworker* m_worker;
    QEventLoop loop;
};


#endif // SEND_TAB_H
