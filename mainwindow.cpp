#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QTabWidget>
#include<QLayout>
#include"welcome.h"
#include<QTimer>
#include<QTabBar>
#include<QThread>
#include<QPushButton>
#include<QTcpServer>
#include<QTcpSocket>
#include<QDebug>
#include"send_init.h"
#include"receive_init.h"
#include"send_tab.h"
#include"receive_tab.h"
#include"send_blocker.h"
//send后关闭send_tab报错
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setCentralWidget(new QWidget(this));
    ui->setupUi(this);
    this->tabs = new QTabWidget(this);
    tabs->setTabsClosable(true);
    welcome* welcome_page = new welcome(this);
    tabs->addTab(welcome_page,"Welcome");
    connect(tabs->tabBar(),&QTabBar::tabCloseRequested,tabs,[this](int index)->void{
        QWidget* abandoned= this->tabs->widget(index);
        this->tabs->removeTab(index);
        delete abandoned;
    });
    QHBoxLayout *layout = new QHBoxLayout(this->ui->centralwidget);
    layout->addWidget(this->tabs);
    connect(welcome_page,&welcome::send_query,this,&MainWindow::send_dialog);
    connect(welcome_page,&welcome::receive_query,this,&MainWindow::receive_dialog);
    connect(ui->action_send,&QAction::triggered,this,&MainWindow::send_dialog);
    connect(ui->action_receive,&QAction::triggered,this,&MainWindow::receive_dialog);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::send_dialog()
{
    /*send_init dia(nullptr);
    connect(&dia,&send_init::connect_query,this,[&dia,this](QString ip,QString port)->void{
        dia.close();
        send_blocker blocker(nullptr);
        QTcpSocket* socket = new QTcpSocket();
        connect(&blocker,&send_blocker::disconnect,this,[socket,&blocker]()->void{
            socket->disconnectFromHost();
            socket->close();
            blocker.close();
        });
        connect(socket,&QTcpSocket::errorOccurred,this,[this,socket,&blocker](QTcpSocket::SocketError error)->void{
            socket->close();
            blocker.close();
            QMessageBox::warning(this,"error","Failed to initialize a connection, error code:" +QString::number(error));
        });
        connect(socket,&QTcpSocket::connected,this,[this,socket,port,ip,&blocker]()->void{
            disconnect(socket,&QTcpSocket::errorOccurred,nullptr,nullptr);
            QString label=ip;
            label.append(':');
            label+=port;
            this->tabs->addTab(new send_tab(this->tabs,socket),label);
            blocker.close();
        });
        socket->connectToHost(QHostAddress(ip),port.toUShort());
        blocker.exec();
    });
    dia.exec();*/
    send_init dia(nullptr);
    connect(&dia,&send_init::connect_query,this,[&dia,this](QString ip,QString port)->void{
        QString label=ip;
        label.append(':');
        label+=port;
        dia.close();
        this->tabs->addTab(new send_tab(this->tabs,ip,port),label);
    });
    dia.exec();
}

void MainWindow::receive_dialog()
{
    receive_init dia(nullptr);
    connect(&dia,&receive_init::listen_query,this,[&dia,this](QString port)->void{
        my_server* server = new my_server();
        int ret=server->listen(QHostAddress::Any,port.toUShort());
        if(ret)
        {
            QString label="listening:";
            label+=port;
            dia.close();
            this->tabs->addTab(new receive_tab(this->tabs,server),label);
        }
        else
        {
            QMessageBox::warning(this,"error","Failed to initialize a listening socket");
            dia.close();
        }
    });
    dia.exec();
}
