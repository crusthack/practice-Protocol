#pragma once

#include <sys/socket.h>


// struct for send tcp message
class TcpSocket
{
public:
    TcpSocket();

private:
    int _Socket;

};