#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include "rawsock/rawsocketcore.hpp"
#include "rawsock/socket/udpsocket.hpp"
#include "rawsock/protocol/udp.hpp"
#include "rawsock/protocol/inet.hpp"
#include <ostream>
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
int UdpSocket::RecvFrom(Byte* const buffer, const int len, char* const srcIp, Word& portNum)
{
    char buf[1024] {};
    sockaddr_in addr;
    socklen_t l = sizeof(addr);
    auto r = recvfrom(_Socket, buf, 1024, 0, (sockaddr*)&addr, &l);
    if(len < r || r == -1)
    {
        return -1;
    }

    auto ipheader = (InetHeader*)buf;
    auto udpHeader = (UdpHeader*)((char*)buf + ipheader->GetIpHeaderLength());
    auto payload = (Byte*)((char*)udpHeader + sizeof(UdpHeader));
    auto payloadLen = ntohs(udpHeader->Length) - sizeof(UdpHeader);
    std::copy(payload, payload + payloadLen, buffer);

    portNum = udpHeader->SrcPort;
    inet_ntop(AF_INET, &ipheader->SourceAddress, srcIp, INET_ADDRSTRLEN);

    return payloadLen;
}