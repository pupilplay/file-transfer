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
    connect(m_socket_thread,&QThread::finished,this->m_worker,&sworker::quit);
    this->ui->clients_list->setRowCount(0);
    this->ui->clients_list->setHorizontalHeaderLabels(QStringList({"host","filename","size"}));
    this->ui->clients_list->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(this->m_worker,&sworker::client_query,this,[this](QString host,QString file_name,QString size)->void{
        int row=this->ui->clients_list->rowCount();
        this->ui->clients_list->setRowCount(row+1);
        this->ui->clients_list->setItem(row,0,new QTableWidgetItem(host));
        this->ui->clients_list->setItem(row,1,new QTableWidgetItem(file_name));
        this->ui->clients_list->setItem(row,2,new QTableWidgetItem(size));
    });
    connect(this,&receive_tab::accept_query,this->m_worker,&sworker::accept);
    m_socket_thread->start();
}

sworker *receive_tab::worker()
{
    return this->m_worker;
}


sworker::sworker(my_server *server):socket(server)
{
    socket->setParent(this);
    connect(server,&my_server::need_connection,this,[this](qintptr socketDescriptor)->void{
        connection* client = new connection(nullptr,socketDescriptor);
        QThread *thread=new QThread();
        client->moveToThread(thread);
        connect(this,&sworker::connection_init,client,&connection::init);
        connect(thread,&QThread::finished,client,&QObject::deleteLater);
        connect(client,&connection::connected,this,[this,client,thread](QString host)->void{
            this->connections[host]=QPair<connection*,QThread*>(client,thread);
        });
        connect(client,&connection::connection_ready,this,[this](QString host,QString file_name,QString size)->void{
            emit client_query(host,file_name,size);
        });
        connect(client,&connection::disconnected,this,[this](QString host)->void{
            auto connection_info=this->connections.take(host);
            connection_info.second->quit();
            connection_info.second->wait();
            delete connection_info.second;
        });
        thread->start();
        emit connection_init();
        disconnect(this,&sworker::connection_init,client,&connection::init);
    });
}


void sworker::accept(QString host,QString file_path,qint64 size)
{
    this->connections[host].first->file_path=file_path;
    this->connections[host].first->size=size;
    connect(this,&sworker::connection_accept,connections[host].first,&connection::accept);
    emit connection_accept();
}

receive_tab::~receive_tab()
{
    this->m_socket_thread->quit();
    this->m_socket_thread->wait();
    delete m_socket_thread;
    delete ui;
}

void sworker::quit()
{
    while(!connections.empty())
    {
        auto connection_info=connections.take(connections.begin().key());
        connection_info.second->quit();
        connection_info.second->wait();
        delete connection_info.second;
    }
    delete this;
}

sworker::~sworker()
{
    ;
}

connection::connection(QObject *parent,qintptr socketDescriptor):QObject(parent),socketDescriptor(socketDescriptor) {};



void connection::init()
{
    this->socket=new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket,&QTcpSocket::disconnected,this,[this]()->void{
        emit disconnected(host);
    });
    host=socket->peerAddress().toString();
    host.append(':');
    host+=QString::number(socket->peerPort());
    emit connected(host);
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
    connect(this->socket,&QTcpSocket::readyRead,this,&connection::receive);
    socket->write("accepted");
}
void connection::receive()
{
    socket->disconnect(SIGNAL(readyRead()));
    QFile file(this->file_path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    char buf[1024];
    while(this->size>0)
    {
        int ret=this->socket->read(buf,1024);
        qDebug()<<"receiver:"<<ret;
        if(ret==-1)
        {
            file.close();
            return;
        }
        this->size-=ret;
        file.write(buf,ret);
    }
    file.close();
}


connection::~connection()
{
    socket->disconnect();
    //
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
    auto &&list = this->ui->clients_list->selectedItems();
    if(list.empty())
    {
        return;
    }
    QString host=list[0]->text();
    QString file_path=QFileDialog::getSaveFileName(nullptr,"save file",list[1]->text());
    qint64 size=list[2]->text().toLongLong();
    emit accept_query(host,file_path,size);
}

void receive_tab::on_disconnect_btn_clicked()
{
    auto &&list = this->ui->clients_list->selectedRanges();
    if(list.empty())
    {
        return;
    }
    QString host=this->ui->clients_list->item(list[0].bottomRow(),list[0].leftColumn())->text();
    auto connection_info=this->m_worker->connections.take(host);
    connection_info.second->quit();
    connection_info.second->wait();
    delete connection_info.second;
    this->ui->clients_list->removeRow(list[0].bottomRow());
}

