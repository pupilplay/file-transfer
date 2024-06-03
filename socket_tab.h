#ifndef SOCKET_TAB_H
#define SOCKET_TAB_H

#include <QWidget>

class socket_tab : public QWidget
{
    Q_OBJECT
public:
    explicit socket_tab(QWidget *parent = nullptr);
    QThread* socket_thread();
protected:
    QThread* m_socket_thread;
};

#endif // SOCKET_TAB_H
