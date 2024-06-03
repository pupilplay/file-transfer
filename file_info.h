#ifndef FILE_INFO_H
#define FILE_INFO_H
#include<QByteArray>
#include<QFile>
#include<QFileInfo>
//a struct for file info exchange
//QByteArray file_name is converted from a QString using toUtf8()
struct file_info
{
public:
    file_info();
    file_info(const QFile& file);
    file_info(QByteArray& data);
    file_info(QByteArray&& data);
    void operator()(const QFile& file);
    QByteArray data();
    qint64 file_size;
    QByteArray file_name;

};

#endif // FILE_INFO_H
