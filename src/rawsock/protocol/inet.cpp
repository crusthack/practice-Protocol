#include "rawsock/protocol/inet.hpp"
#include "rawsock/protocol/icmp.hpp"
#include "rawsock/socket/icmpsocket.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <algorithm>

void InetHeader::CalcInetHeaderChecksum(const char* buffer, int len)
{
    auto p = (InetHeader*)buffer;
    p->HeaderChecksum = 0;
    p->HeaderChecksum = htons(CalcChecksum(buffer, len));
}

IcmpSocket::IcmpSocket()
{
    IcmpHeader::CheckHeaderSize();
    InetHeader::CheckHeaderSize();

    _Socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(_Socket == -1)
    {
        perror("Icmp Socket constructor. init socket failed\n");
    }

    int one = 1;
    if(setsockopt(_Socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one))
        == -1)
    {
        perror("Icmp Socket constructor. set socket opt failed\n");
    }
}

void IcmpSocket::SendPingRequestMessage(const char* dstIp)
{
    int payloadSize = 20;
    int bufferSize = payloadSize + sizeof(InetHeader) + sizeof(IcmpHeader);
    char* buffer = new char[bufferSize];
    BuildPingRequestMessage(buffer, bufferSize, "127.0.0.1", dstIp);


    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, dstIp, &addr.sin_addr);
    
    sendto(_Socket, buffer, bufferSize, 0, (sockaddr*)&addr, sizeof(addr));

    return;
}

void IcmpSocket::BuildPingRequestMessage(const char* buffer, int len, const char* srcIp, const char* dstIp)
{
    // ip header -> icmp header -> icmp message -> icmp checksum
    
    if(len < (int)(sizeof(InetHeader) + sizeof(IcmpHeader) + 1))
    {
        return;
    }

    // prepare src ip
    char _srcIp[16];
    if(srcIp == nullptr)
    {
        GetLocalhostIp(_srcIp, 16);
    }
    else
    {
        std::copy(srcIp, srcIp + 16, _srcIp);
    }

    auto ipHeaderp = (char*)buffer;
    auto icmpHeaderP = (char*)buffer + sizeof(InetHeader);
    auto payloadP = (char*) (buffer + sizeof(InetHeader) + sizeof(IcmpHeader));
    int payloadLen = len - sizeof(InetHeader) - sizeof(IcmpHeader);

    InetHeader::BuildInetHeader(ipHeaderp, len, 1, IPPROTO_ICMP, _srcIp, dstIp);
    IcmpHeader::BuildIcmpHeader(icmpHeaderP, 8, 0);
    // make payload. ascii stream
    for(int i=0; i<payloadLen; ++i)
    {
        payloadP[i] = ('A' + i) % 128;
    }
    IcmpHeader::CalcIcmpHeaderChecksum(icmpHeaderP, sizeof(IcmpHeader) + payloadLen);
}

// maybe, make rawsock class than move to static method. tcpsock icmpsock inherit rawsock
int IcmpSocket::GetLocalhostIp(char* buffer, int len)
{
    if(len < 16)
    {
        return -1;
    }
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(44);
    inet_pton(AF_INET, "8.8.8.8", (sockaddr_in*)&addr.sin_addr);
    if(connect(s, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        return -1;
    }

    sockaddr_in local {};
    socklen_t l = sizeof(local);
    getsockname(s, (sockaddr*)&local, &l);
    inet_ntop(AF_INET, (sockaddr*)&local.sin_addr, buffer, l);

    return 0;
}

unsigned short CalcChecksum(const unsigned char* buffer, int len)
{
    unsigned int sum = 0;
    while(len >= 2)
    {
        const unsigned short word = (buffer[0] << 8) | buffer[1];
        sum += word;
        len -= 2;
        buffer += 2;
    }
    // if len is odd
    if(len == 1)
    {
        sum += buffer[0] << 8;
    }
 
 
    while((sum >> 16) != 0)
    {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
 
    return ~sum;
}

unsigned short CalcChecksum(const char* buf, int len)
{
    auto buffer = (unsigned char*)buf;
    unsigned int sum = 0;
    while(len >= 2)
    {
        const unsigned short word = (buffer[0] << 8) | buffer[1];
        sum += word;
        len -= 2;
        buffer += 2;
    }
    // if len is odd
    if(len == 1)
    {
        sum += buffer[0] << 8;
    }
 
    while((sum >> 16) != 0)
    {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
 
    return ~sum;
}
