#ifndef RECEIVE_TAB_H
#define RECEIVE_TAB_H

#include <QWidget>
#include<QTcpSocket>
#include"socket_tab.h"
#include<QTcpServer>
#include<QThread>
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

    //for setting the member variable shutdown and file_info conveniently
    friend class sworker;
    friend class receive_tab;
public:
    connection(QObject* parent,qintptr socketDescriptor);
    ~connection();
public slots:
    //set the socketDescriptor, get the host and connect the signals
    void init();

    //respond the query and call receive
    void accept();

    //receive the data
    //when finished, call prepare()
    void receive();
signals:
    //emitted when connections is ready, tell the sworker to add the connection
    void connection_ready(QString host,QString file_name,QString size);

    //emitted when connected, pass the host for identification
    void connected(QString host);

    //emitted when disconnected, pass the host for identification
    void disconnected(QString host);

    //emitted when the connection is destructed, tell the sworker to remove the connection
    void transfer_finished(QString host);
private:

    //get ready for incoming transfer query
    void prepare();

    QTcpSocket* socket;
    qintptr socketDescriptor;
    QString host;
    QString file_path;
    qint64 size;
    bool shutdown;
};

//a class inheriting QTcpServer with an overrided incomingConnection()
//it passes the socketDescriptor in order to make it work in another thread
//note: Pending Connections mechanism is not working in this class
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

    //for accessing the connections conveniently
    friend class receive_tab;

public:
    sworker(my_server* server);

    //destroy all the subthreads and their connections before deleted
    ~sworker();

signals:
    //emitted when a new transfer is available, pass the file info and show them in the GUI
    void client_query(QString host,QString file_name,QString size);

    //work as a temporary signal to start a transfer in function accept()
    void connection_accept();

    //work as a temporary signal to initialize the connection in another thread
    void connection_init();

    //emitted when a transfer finishes or its connection quits, pass the host and remove the corresponding row in receive_tab GUI
    void transfer_finished(QString host);

public slots:
    //accept the file info from the receive_tab and set them to certain connection and start transferring
    void accept(QString host,QString file_path,qint64 size);

    //create a new connection
    void connection_create(qintptr socketDescriptor);
private:
    my_server* server;
    //the key is the host like(::ffff:127.0.0.1:1234)
    QHash<QString,QPair<connection*,QThread*>>connections;
};

class receive_tab : public socket_tab
{
    Q_OBJECT

public:
    explicit receive_tab(QWidget *parent = nullptr);
    explicit receive_tab(QWidget *parent,my_server* server);
    ~receive_tab();
signals:
    //emitted when receive button is clicked and pass the file info to m_worker
    void accept_query(QString host,QString file_path,qint64 size);
private slots:
    void on_receive_btn_clicked();

    void on_disconnect_btn_clicked();

    void on_reject_btn_clicked();

private:
    Ui::receive_tab *ui;
    sworker* m_worker;
};

#endif // RECEIVE_TAB_H
