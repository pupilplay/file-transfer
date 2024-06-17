#ifndef SOCKET_TAB_H
#define SOCKET_TAB_H

#include <QWidget>
//This class was supposed to be a virtual class but has been given up as it does no help
class socket_tab : public QWidget
{
    Q_OBJECT
public:
    explicit socket_tab(QWidget *parent = nullptr);
    QThread *socket_thread();

protected:
    QThread *m_socket_thread;
};

#endif // SOCKET_TAB_H
