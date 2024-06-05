#include "send_tab.h"
#include "ui_send_tab.h"

send_tab::send_tab(QWidget *parent)
    : socket_tab(parent)
    , ui(new Ui::send_tab)
{
    ui->setupUi(this);
}

send_tab::send_tab(QWidget *parent, QString ip, QString port):send_tab(parent)
{
    m_worker=new cworker();
    m_socket_thread=new QThread();
    m_worker->moveToThread(m_socket_thread);
    this->ui->interval_input->setValidator(new QIntValidator(0,ULONG_MAX,this->ui->interval_input));
    connect(m_socket_thread,&QThread::finished,m_worker,&QObject::deleteLater);
    connect(this,&send_tab::send_start,m_worker,&cworker::init);
    connect(this->m_worker,&cworker::send_started,this,[this]()->void{
        this->ui->info->setText("transferring");
    });
    connect(this->m_worker,&cworker::send_finished,this,[this]()->void{
        this->ui->info->setText("transfer finished");
        this->ui->send_btn->setEnabled(true);
        this->ui->select_btn->setEnabled(true);
        this->ui->file_path->setEnabled(true);
    });
    connect(this,&send_tab::socket_init,m_worker,&cworker::connect_to_host);
    m_socket_thread->start();
    send_blocker blocker;
    connect(&blocker,&send_blocker::disconnect_query,this,[this,&blocker]()->void{
        blocker.close();
        this->deleteLater();
    });
    connect(m_worker,&cworker::connected,this,[this,&blocker]()->void{
        blocker.close();
        this->setEnabled(true);
    });
    connect(m_worker,&cworker::error_occurred,this,[this,&blocker](QTcpSocket::SocketError error)->void{
        blocker.close();
        this->ui->info->setText("error code:" +QString::number(errno));
        QMessageBox::warning(this,"error","Failed to initialize a connection, error code:" +QString::number(errno));
    });
    emit socket_init(ip,port);
    blocker.exec();
    disconnect(m_worker,&cworker::error_occurred,this,nullptr);
    connect(m_worker,&cworker::disconnected,this,&send_tab::abandon);
}

send_tab::~send_tab()
{
    m_socket_thread->quit();
    m_socket_thread->wait();
    delete m_socket_thread;
    delete ui;
}

void send_tab::abandon()
{
    this->ui->info->setText("disconnected");
    this->setEnabled(false);
}

cworker::cworker()
{
    ;
}

void cworker::connect_to_host(QString ip, QString port)
{
    socket=new QTcpSocket(this);
    connect(socket,&QTcpSocket::connected,this,&cworker::connected);
    connect(socket,&QTcpSocket::disconnected,this,&cworker::disconnected);
    connect(socket,&QTcpSocket::errorOccurred,this,&cworker::error_occurred);
    socket->connectToHost(QHostAddress(ip),port.toUShort());
}

void cworker::init(QString path)
{
    file.setFileName(path);
    file.open(QIODevice::ReadOnly);
    finfo(file);
    connect(socket,&QTcpSocket::readyRead,this,&cworker::send);
    this->socket->write(finfo.data());
    file.close();
}
void cworker::send()
{
    emit send_started();
    this->socket->disconnect(SIGNAL(readyRead()));
    file.open(QIODevice::ReadOnly);
    this->socket->readAll();
    char buf[1024];
    qint64 size = this->finfo.file_size;
    while(size>0)
    {
        int ret=file.read(buf,1024);
        if(ret==-1)
        {
            break;
        }
        int sent = this->socket->write(buf,ret);
        if(!socket->waitForReadyRead())
        {
            break;
        }
        QByteArray reply = socket->read(16);
        if(sent==-1)
        {
            file.close();
            return;
        }
        size-=sent;
        QThread::msleep(500);
    }
    file.close();
    emit send_finished();
}

void cworker::set_interval(unsigned long interval)
{
    this->interval=interval;
}

void cworker::quit()
{
    ;
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
    this->ui->send_btn->setEnabled(false);
    this->ui->select_btn->setEnabled(false);
    this->ui->file_path->setEnabled(false);
    emit send_start(this->ui->file_path->text());
    this->ui->info->setText("waiting for the reply from server");
}


void send_tab::on_interval_input_textEdited(const QString &arg1)
{
    this->m_worker->set_interval(arg1.toULong());
}

