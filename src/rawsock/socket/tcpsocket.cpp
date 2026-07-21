#include "rawsock/socket/tcpsocket.hpp"
#include "rawsock/protocol/inet.hpp"
#include <cassert>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>

TcpSocket::TcpSocket()
{
    InetHeader::CheckHeaderSize();
    TcpHeader::CheckHeaderSize();

    _Socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(_Socket == -1)
    {
        perror("TcpSocket constructor, socket init error");
    }

    // include ip header when send packet
    int one = 1;
    if(setsockopt(_Socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one))
        ==  -1)
    {
        perror("TcpSocket constructor, socket setopt error");
    }
}