#ifndef RECEIVE_TAB_H
#define RECEIVE_TAB_H

#include <QWidget>
#include<QTcpSocket>
#include"socket_tab.h"
#include<QTcpServer>
#include<QThread>
#include<QEventLoop>
#include<QFileDialog>
#include<QTableWidget>
#include<QTableWidgetSelectionRange>
#include"file_info.h"
namespace Ui {
class receive_tab;
}


class connection:public QObject
{
    Q_OBJECT
    friend class sworker;
public:
    connection(QObject* parent,qintptr socketDescriptor);
    ~connection();
public slots:
    void init();
    void accept();
    void receive();
signals:
    void connection_ready(QString host,QString file_name,QString size);
    void connected(QString host);
    void disconnected(QString host);
    void quitted(QString host);
private:
    void prepare();
    QTcpSocket* socket;
    qintptr socketDescriptor;
    QString host;
    QString file_path;
    qint64 size;
};

class my_server:public QTcpServer
{
    Q_OBJECT
public:
    my_server(QObject* parent=nullptr);
    ~my_server();
protected:
    void incomingConnection(qintptr socketDescriptor) override;
signals:
    void need_connection(qintptr socketDescriptor);
};

class sworker:public QObject
{
    Q_OBJECT
    friend class receive_tab;
public:
    sworker(my_server* server);
    ~sworker();
signals:
    void client_query(QString host,QString file_name,QString size);
    void connection_accept();
    void connection_init();
    void connection_quitted(QString host);
    void quitted();
public slots:
    void accept(QString host,QString file_path,qint64 size);
    void quit();
private:
    my_server* socket;
    QHash<QString,QPair<connection*,QThread*>>connections;
};

class receive_tab : public socket_tab
{
    Q_OBJECT

public:
    explicit receive_tab(QWidget *parent = nullptr);
    explicit receive_tab(QWidget *parent,my_server* server);
    sworker* worker();
    ~receive_tab();
signals:
    void accept_query(QString host,QString file_path,qint64 size);
    void disconnect_query();
private slots:
    void on_receive_btn_clicked();

    void on_disconnect_btn_clicked();

private:
    Ui::receive_tab *ui;
    sworker* m_worker;
};

#endif // RECEIVE_TAB_H
