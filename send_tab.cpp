#include "send_tab.h"
#include "ui_send_tab.h"

send_tab::send_tab(QWidget *parent)
    : socket_tab(parent)
    , ui(new Ui::send_tab)
{
    ui->setupUi(this);
}

send_tab::send_tab(QWidget *parent, QTcpSocket *socket):send_tab(parent)
{
    m_worker=new cworker(socket);
    m_socket_thread=new QThread();
    m_worker->moveToThread(m_socket_thread);
    connect(this,&send_tab::client_quit,m_worker,&cworker::quit);
    connect(this,&send_tab::send_start,m_worker,&cworker::init);
    connect(socket,&QTcpSocket::disconnected,this,&send_tab::abandon);
    connect(this->m_worker,&cworker::send_started,this,[this]()->void{
        this->ui->info->setText("transferring");
    });
    connect(this->m_worker,&cworker::send_finished,this,[this]()->void{
        this->ui->info->setText("transfer finished");
    });
    m_socket_thread->start();
}

send_tab::~send_tab()
{
    emit client_quit();
    m_socket_thread->quit();
    m_socket_thread->wait();
    delete m_socket_thread;
    delete ui;
}

void send_tab::abandon()
{
    this->ui->info->setText("disconnected");
    emit client_quit();
    this->setEnabled(false);
}

cworker::cworker(QTcpSocket *socket):socket(socket)
{
    socket->setParent(this);
}

void cworker::init(QString path)
{
    file.setFileName(path);
    file.open(QIODevice::ReadOnly);
    finfo(file);
    this->socket->write(finfo.data());
    connect(socket,&QTcpSocket::readyRead,this,&cworker::send);
    file.close();
}
void cworker::send()
{
    file.open(QIODevice::ReadOnly);
    this->socket->readAll();
    char buf[1024];
    qint64 size = this->finfo.file_size;
    while(size>0)
    {
        QThread::sleep(1);
        int ret=file.read(buf,1024);
        if(ret==-1)
        {
            break;
        }
        int sent = this->socket->write(buf,ret);
        if(sent==-1)
        {
            file.close();
            //unconfirmed
            this->quit();
            return;
            //unconfirmed
        }
        if(sent!=ret)
        {
            QThread::sleep(1);
            while(sent<ret)
            {
                int ret2=this->socket->write(buf+sent,ret-sent);
                if(ret2==-1)
                {
                    file.close();
                    //unconfirmed
                    this->quit();
                    return;
                    //unconfirmed
                }
                sent+=ret2;
            }
        }
        size-=sent;
    }
    file.close();
}

void cworker::quit()
{
    this->socket->disconnectFromHost();
    this->socket->close();
    delete this;
}

cworker::~cworker()
{
    ;
}

void send_tab::on_select_btn_clicked()
{
    this->ui->file_path->setText(QFileDialog::getOpenFileName(this,"select file"));
}


void send_tab::on_send_btn_clicked()
{
    emit send_start(this->ui->file_path->text());
    this->ui->info->setText("waiting for the reply from server");
}

