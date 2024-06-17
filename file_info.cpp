#include "file_info.h"

file_info::file_info() {}

file_info::file_info(const QFile &file)
{
    QFileInfo finfo(file);
    this->file_name = finfo.fileName().toUtf8();
    this->file_size = finfo.size();
}

file_info::file_info(QByteArray &data)
{
    const char *dataptr = data.data();
    this->file_size = *(qint64 *) dataptr;
    dataptr += sizeof(qint64);
    this->file_name = QByteArray(dataptr);
};

file_info::file_info(QByteArray &&data)
{
    const char *dataptr = data.data();
    this->file_size = *(qint64 *) dataptr;
    dataptr += sizeof(qint64);
    this->file_name = QByteArray(dataptr);
}

void file_info::operator()(const QFile &file)
{
    QFileInfo finfo(file);
    this->file_name = finfo.fileName().toUtf8();
    this->file_size = finfo.size();
}

QByteArray file_info::data()
{
    return QByteArray((char *) &this->file_size, sizeof(qint64)) + this->file_name;
}
