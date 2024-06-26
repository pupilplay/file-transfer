#include "send_tab.h"
#include "ui_send_tab.h"

send_tab::send_tab(QWidget *parent)
    : socket_tab(parent)
    , ui(new Ui::send_tab)
{
    ui->setupUi(this);
}

send_tab::send_tab(QWidget *parent, QString ip, QString port)
    : send_tab(parent)
{
    m_worker = new cworker();
    m_socket_thread = new QThread();
    m_worker->moveToThread(m_socket_thread);

    this->ui->interval_input->setValidator(new QIntValidator(0, INT_MAX, this->ui->interval_input));
    this->ui->interval_input->setToolTip(
        "The interval between two data package sending\nIt is not recommended to set it too low as "
        "it may cause unexpected error during transmitting");
    this->ui->send_progress->hide();

    connect(m_socket_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &send_tab::send_start, m_worker, &cworker::init);

    connect(this->m_worker, &cworker::send_started, this, [this]() -> void {
        this->ui->info->setText("transferring");
        this->ui->info->hide();
        this->ui->send_progress->setValue(0);
        this->ui->send_progress->show();
    });
    connect(this->m_worker, &cworker::send_finished, this, [this]() -> void {
        this->ui->send_progress->hide();
        this->ui->info->setText("transfer finished");
        this->ui->info->show();
        this->ui->send_btn->setEnabled(true);
        this->ui->select_btn->setEnabled(true);
        this->ui->file_path->setEnabled(true);
    });
    connect(this->m_worker, &cworker::send_failed, this, [this]() -> void {
        this->ui->info->setText("transfer failed");
        this->ui->send_btn->setEnabled(true);
        this->ui->select_btn->setEnabled(true);
        this->ui->file_path->setEnabled(true);
    });
    connect(this->m_worker,&cworker::send_progress, this, &send_tab::progress_update);

    send_blocker blocker;
    connect(&blocker, &send_blocker::disconnect_query, this, [this, &blocker]() -> void {
        blocker.close();
        this->deleteLater();
    });
    connect(m_worker, &cworker::connected, this, [this, &blocker]() -> void {
        blocker.close();
        this->setEnabled(true);
    });
    connect(m_worker,
            &cworker::error_occurred,
            this,
            [this, &blocker](QTcpSocket::SocketError error) -> void {
                blocker.close();
                this->ui->info->setText("error code:" + QString::number(errno));
                QMessageBox::warning(this,
                                     "error",
                                     "Failed to initialize a connection, error code:"
                                         + QString::number(errno));
            });
    connect(this, &send_tab::socket_init, m_worker, &cworker::connect_to_host);
    m_socket_thread->start();
    emit socket_init(ip, port);
    blocker.exec();

    disconnect(m_worker, &cworker::error_occurred, this, nullptr);
    connect(m_worker, &cworker::disconnected, this, &send_tab::abandon);
}

send_tab::~send_tab()
{
    this->m_worker->shutdown = true;
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

void send_tab::progress_update(int percentage)
{
    this->ui->send_progress->setValue(percentage);
}

cworker::cworker()
    : shutdown(false)
{
    ;
}

void cworker::connect_to_host(QString ip, QString port)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &cworker::connected);
    connect(socket, &QTcpSocket::disconnected, this, &cworker::disconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &cworker::error_occurred);
    socket->connectToHost(QHostAddress(ip), port.toUShort());
}

void cworker::init(QString path)
{
    file.setFileName(path);
    file.open(QIODevice::ReadOnly);
    finfo(file);
    connect(socket, &QTcpSocket::readyRead, this, &cworker::send);
    this->socket->write(finfo.data());
    file.close();
}
void cworker::send()
{
    emit send_started();
    this->socket->disconnect(SIGNAL(readyRead()));
    file.open(QIODevice::ReadOnly);
    QByteArray reply = this->socket->readAll();
    if(reply != "accepted")
    {
        file.close();
        emit send_failed();
        return;
    }
    char buf[1024];
    qint64 size = this->finfo.file_size;
    while (!shutdown && size > 0) {
        int ret = file.read(buf, 1024);
        if (ret == -1)
        {
            break;
        }
        int sent = this->socket->write(buf, ret);
        if (!socket->waitForReadyRead())
        {
            break;
        }
        //the reply from the server is not used
        reply = socket->read(16);
        if (sent == -1)
        {
            file.close();
            return;
        }
        size -= sent;
        emit send_progress(100-size*100/this->finfo.file_size);
        QThread::msleep(this->interval);
    }
    file.close();
    if (size == 0)
    {
        emit send_finished();
    }
    else
    {
        emit send_failed();
    }
}

cworker::~cworker()
{
    ;
}

void send_tab::on_select_btn_clicked()
{
    QString file_path=QFileDialog::getOpenFileName(this, "select file");
    if(file_path.isEmpty())
    {
        return;
    }
    this->ui->file_path->setText(file_path);
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
    this->m_worker->interval = arg1.toULong();
}
