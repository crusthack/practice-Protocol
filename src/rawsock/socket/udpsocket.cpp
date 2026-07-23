#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include "rawsock/rawsocketcore.hpp"
#include "rawsock/socket/udpsocket.hpp"
#include "rawsock/protocol/udp.hpp"
#include "rawsock/protocol/inet.hpp"
#include <sys/socket.h>
#include <unistd.h>

UdpSocket::UdpSocket()
{
    InetHeader::CheckHeaderSize();
    UdpHeader::CheckHeaderSize();

    _Socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(_Socket == -1)
    {
        perror("UDP Socket Init error occurred\n");
    }

    int one {1};
    if(setsockopt(_Socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one))
        == -1)
    {
        perror("UDP Socket set opt error occurred\n");
    }
}

UdpSocket::~UdpSocket()
{
    close(_Socket);
}

void UdpSocket::SendTo(const char* dstIp, const Word portNum, const Byte* buffer, const int len)
{
    Byte* buf = new Byte[sizeof(InetHeader) + sizeof(UdpHeader) + len];
    Byte totalLen = sizeof(InetHeader) + sizeof(UdpHeader) + len;

    char srcIp[INET_ADDRSTRLEN];
    GetHostIp(srcIp, dstIp);
    InetHeader::BuildInetHeader((char*)buf, totalLen, getpid(), IPPROTO_UDP, srcIp, dstIp);

    auto ipHeaderP = (InetHeader*) buf;
    auto ipHeaderLen = ipHeaderP->GetIpHeaderLength();

    auto udpPointer = (UdpHeader*)(buf + ipHeaderLen);
    UdpHeader::BuildUdpMessage((char*)udpPointer, (char*)buffer, len, portNum, srcIp, dstIp);

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNum);
    inet_pton(AF_INET, dstIp, &addr.sin_addr);

    sendto(_Socket, buf, totalLen, 0, (sockaddr*)&addr, sizeof(addr));

    delete[] buf;
}
void UdpSocket::RecvFrom(Byte* const buffer, const int len, char* const srcIp, const Word& portNum)
{
    char buf[1024];
    sockaddr_in addr;
    socklen_t l = sizeof(addr);
    auto r = recvfrom(_Socket, buf, 1024, 0, (sockaddr*)&addr, &l);

    for(int i=0; i<r; ++i)
    {
        printf("0x%02x ", (unsigned char)buf[i]);
    }
    printf("\n");
}