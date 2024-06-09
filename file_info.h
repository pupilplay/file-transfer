#ifndef FILE_INFO_H
#define FILE_INFO_H
#include<QByteArray>
#include<QFile>
#include<QFileInfo>

// file_info is a simple struct for file info exchange
struct file_info
{
public:
    file_info();

    //read the data from a QFile class
    file_info(const QFile& file);

    //read the data from an encoded QByteArray
    file_info(QByteArray& data);
    file_info(QByteArray&& data);

    //read the data from a QFile class
    void operator()(const QFile& file);

    //encode the data in the struct and return as a QByteArray
    QByteArray data();
    qint64 file_size;
    QByteArray file_name;
};

#endif // FILE_INFO_H
