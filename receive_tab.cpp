#include "receive_tab.h"
#include "ui_receive_tab.h"
receive_tab::receive_tab(QWidget *parent)
    : socket_tab(parent)
    , ui(new Ui::receive_tab)
{
    ui->setupUi(this);
}

receive_tab::receive_tab(QWidget *parent, my_server *server):receive_tab(parent)
{
    this->m_worker=new sworker(server);
    m_socket_thread=new QThread();
    this->m_worker->moveToThread(m_socket_thread);
    connect(m_socket_thread,&QThread::finished,this->m_worker,&QObject::deleteLater);

    this->ui->clients_list->setRowCount(0);
    this->ui->clients_list->setHorizontalHeaderLabels(QStringList({"host","filename","size"}));
    this->ui->clients_list->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->ui->transfer_list->setRowCount(0);
    this->ui->transfer_list->setHorizontalHeaderLabels(QStringList({"host","filename","size"}));
    this->ui->transfer_list->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(this->m_worker,&sworker::client_query,this,[this](QString host,QString file_name,QString size)->void{
        int row=this->ui->clients_list->rowCount();
        this->ui->clients_list->setRowCount(row+1);
        this->ui->clients_list->setItem(row,0,new QTableWidgetItem(host));
        this->ui->clients_list->setItem(row,1,new QTableWidgetItem(file_name));
        this->ui->clients_list->setItem(row,2,new QTableWidgetItem(size));
    });
    connect(this->m_worker,&sworker::transfer_finished,this,[this](QString host)->void{
        for(int i=0;i<this->ui->transfer_list->rowCount();i++)
        {
            if(this->ui->transfer_list->item(i,0)->text()==host)
            {
                this->ui->transfer_list->removeRow(i);
                break;
            }
        }
    });

    connect(this,&receive_tab::accept_query,this->m_worker,&sworker::accept);
    m_socket_thread->start();
}


sworker::sworker(my_server *server):server(server)
{
    server->setParent(this);
    connect(server,&my_server::need_connection,this,&sworker::connection_create);
}


void sworker::accept(QString host,QString file_path,qint64 size)
{
    this->connections[host].first->file_path=file_path;
    this->connections[host].first->size=size;
    QMetaObject::Connection&& id =connect(this,&sworker::connection_accept,connections[host].first,&connection::accept);
    emit connection_accept();
    disconnect(id);
}

void sworker::connection_create(qintptr socketDescriptor)
{
    connection* client = new connection(nullptr,socketDescriptor);
    QThread *thread=new QThread();
    client->moveToThread(thread);
    connect(thread,&QThread::finished,client,&QObject::deleteLater);

    connect(client,&connection::connected,this,[this,client,thread](QString host)->void{
        this->connections[host]=QPair<connection*,QThread*>(client,thread);
    });
    connect(client,&connection::connection_ready,this,&sworker::client_query);
    connect(client,&connection::disconnected,this,[this](QString host)->void{
        auto connection_info=this->connections.take(host);
        connection_info.second->quit();
        connection_info.second->wait();
        delete connection_info.second;
    });
    connect(client,&connection::transfer_finished,this,&sworker::transfer_finished);

    QMetaObject::Connection&& id=connect(this,&sworker::connection_init,client,&connection::init);
    thread->start();
    emit connection_init();
    disconnect(id);
}

receive_tab::~receive_tab()
{
    this->m_socket_thread->quit();
    this->m_socket_thread->wait();
    delete m_socket_thread;
    delete ui;
}

sworker::~sworker()
{
    while(!connections.empty())
    {
        auto connection_info=connections.take(connections.begin().key());
        connection_info.first->shutdown=true;
        connection_info.second->quit();
        connection_info.second->wait();
        delete connection_info.second;
    }
}

connection::connection(QObject *parent,qintptr socketDescriptor):QObject(parent),socketDescriptor(socketDescriptor),shutdown(false)
{
    ;
}



void connection::init()
{
    this->socket=new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    host=socket->peerAddress().toString();
    host.append(':');
    host+=QString::number(socket->peerPort());
    connect(socket,&QTcpSocket::disconnected,this,[this]()->void{
        emit disconnected(host);
    });
    emit connected(host);
    this->prepare();
}
void connection::prepare()
{
    connect(this->socket,&QTcpSocket::readyRead,this,[this]()->void{
        file_info finfo(socket->readAll());
        QString file_name=QString::fromUtf8(finfo.file_name);
        QString file_size=QString::number(finfo.file_size);
        emit connection_ready(host,file_name,file_size);
        socket->disconnect(SIGNAL(readyRead()));
    });
}

void connection::accept()
{
    socket->write("accepted");
    this->receive();
}
void connection::receive()
{
    QFile file(this->file_path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    char buf[1024];
    while(!shutdown && this->size>0)
    {
        if(!socket->waitForReadyRead())
        {
            break;
        }
        int ret=this->socket->read(buf,1024);
        if(ret==-1)
        {
            file.close();
            return;
        }
        this->size-=ret;
        file.write(buf,ret);
        socket->write("recv");
    }
    file.close();
    emit transfer_finished(host);
    if(!shutdown)
    {
        prepare();
    }
}


connection::~connection()
{
    emit transfer_finished(host);
    socket->disconnect();
}



my_server::my_server(QObject *parent):QTcpServer(parent) {}

my_server::~my_server()
{
    ;
}

void my_server::incomingConnection(qintptr socketDescriptor)
{
    emit need_connection(socketDescriptor);
}

void receive_tab::on_receive_btn_clicked()
{
    auto &&list = this->ui->clients_list->selectedRanges();
    if(list.empty())
    {
        return;
    }
    QString host=this->ui->clients_list->item(list[0].bottomRow(),0)->text();
    QString file_path=QFileDialog::getSaveFileName(nullptr,"save file",this->ui->clients_list->item(list[0].bottomRow(),1)->text());
    qint64 size=this->ui->clients_list->item(list[0].bottomRow(),2)->text().toLongLong();
    emit accept_query(host,file_path,size);
    int row=this->ui->transfer_list->rowCount();
    this->ui->transfer_list->setRowCount(row+1);
    this->ui->transfer_list->setItem(row,0,new QTableWidgetItem(host));
    this->ui->transfer_list->setItem(row,1,new QTableWidgetItem(file_path));
    this->ui->transfer_list->setItem(row,2,new QTableWidgetItem(this->ui->clients_list->item(list[0].bottomRow(),2)->text()));
    this->ui->clients_list->removeRow(list[0].bottomRow());
}

void receive_tab::on_disconnect_btn_clicked()
{
    auto &&list = this->ui->transfer_list->selectedRanges();
    if(list.empty())
    {
        return;
    }
    QString host=this->ui->transfer_list->item(list[0].bottomRow(),0)->text();
    auto connection_info=this->m_worker->connections.take(host);
    connection_info.first->shutdown=true;
    connection_info.second->quit();
    connection_info.second->wait();
    delete connection_info.second;
}


void receive_tab::on_reject_btn_clicked()
{
    auto &&list = this->ui->clients_list->selectedRanges();
    if(list.empty())
    {
        return;
    }
    QString host=this->ui->clients_list->item(list[0].bottomRow(),0)->text();
    this->ui->clients_list->removeRow(list[0].bottomRow());
    auto connection_info=this->m_worker->connections.take(host);
    connection_info.first->shutdown=true;
    connection_info.second->quit();
    connection_info.second->wait();
    delete connection_info.second;
}

