#include "socket_tab.h"

socket_tab::socket_tab(QWidget *parent)
    : QWidget{parent}
{}

QThread *socket_tab::socket_thread()
{
    return this->m_socket_thread;
}
