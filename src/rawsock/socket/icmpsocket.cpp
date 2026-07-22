#include "rawsock/rawsocketcore.hpp"
#include <rawsock/socket/icmpsocket.hpp>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/protocol/icmp.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <algorithm>
#include <time.h>

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

void IcmpSocket::SendPingMessage(const char* dstIp, const short id, const short sequenceNum)
{
    const int payloadSize = 20;
    const int bufferSize = payloadSize + sizeof(InetHeader) + sizeof(IcmpEchoHeader);
    char buffer[bufferSize];

    char _srcIp[16];
    GetHostIp(_srcIp, dstIp);

    BuildPingRequestMessage(buffer, bufferSize, _srcIp, dstIp, id, sequenceNum);

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, dstIp, &addr.sin_addr);
    
    timespec sendTime{};
    clock_gettime(CLOCK_MONOTONIC, &sendTime);

    sendto(_Socket, buffer, bufferSize, 0, (sockaddr*)&addr, sizeof(addr));

    char recvBuffer[bufferSize * 2];
    auto retcode = RecvIcmpEchoReplyMessage(recvBuffer, bufferSize * 2, id, sequenceNum);

    timespec receiveTime{};
    clock_gettime(CLOCK_MONOTONIC, &receiveTime);
    const int64_t elapsedNs =
        static_cast<int64_t>(receiveTime.tv_sec - sendTime.tv_sec)
            * 1'000'000'000LL
        + static_cast<int64_t>(
            receiveTime.tv_nsec - sendTime.tv_nsec
        );
        
    if(retcode > 0)
    {
        auto ipHeader = (const InetHeader*)recvBuffer;
        auto headerLen = (ipHeader->VersionAndHeaderLength & 0x0f) * 4;

        auto icmpHeader = (const IcmpEchoHeader*)(recvBuffer + headerLen);
        char srcIp[16];
        inet_ntop(AF_INET, &ipHeader->SourceAddress, srcIp, sizeof(srcIp));
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
            retcode, srcIp, ntohs(icmpHeader->SequenceNum), ipHeader->TimeToLive,
            elapsedNs / 1'000'000.0);
    }
    return;
}

void IcmpSocket::BuildPingRequestMessage(const char* buffer, int len, const char* srcIp, const char* dstIp,
    const short id, const short sequenceNum)
{
    // ip header -> icmp header -> icmp message -> icmp checksum
    
    if(len < (int)(sizeof(InetHeader) + sizeof(IcmpEchoHeader) + 1))
    {
        return;
    }

    // prepare src ip
    char _srcIp[16];
    if(srcIp == nullptr)
    {
        sockaddr_in addr {};
        GetHostIp(&addr, dstIp);
        inet_ntop(AF_INET, &addr.sin_addr, _srcIp, sizeof(sockaddr_in));
    }
    else
    {
        std::copy(srcIp, srcIp + 16, _srcIp);
    }

    auto ipHeaderp = (char*)buffer;
    auto icmpEchoHeaderP = (char*)buffer + sizeof(InetHeader);
    auto payloadP = (char*) (buffer + sizeof(InetHeader) + sizeof(IcmpEchoHeader));
    int payloadLen = len - sizeof(InetHeader) - sizeof(IcmpEchoHeader);

    InetHeader::BuildInetHeader(ipHeaderp, len, 1, IPPROTO_ICMP, _srcIp, dstIp);
    IcmpEchoHeader::BuildIcmpHeader(icmpEchoHeaderP, 8, 0, id, sequenceNum);
    // make payload. ascii stream
    for(int i=0; i<payloadLen; ++i)
    {
        payloadP[i] = ('A' + i) % 128;
    }
    IcmpEchoHeader::CalcPingChecksum(icmpEchoHeaderP, sizeof(IcmpEchoHeader) + payloadLen);
}

int IcmpSocket::RecvIcmpMessage(const char* buffer, const int len)
{
    sockaddr_in addr {};
    socklen_t addr_len = sizeof(addr);

    auto r = recvfrom(
        _Socket, (void*)buffer, len, 0, (sockaddr*)&addr, &addr_len);

    if(r==-1)
    {
        return -1;
    }
    // check ip header
    auto ipheaderP = (InetHeader*)buffer;
    ipheaderP->IsValidInetHeader(r);

    // check icmp header
    if(ipheaderP->Protocol != IPPROTO_ICMP)
    {
        printf("No icmp packet has captured\n");
        return 0;
    }

    return r;
}

int IcmpSocket::RecvIcmpEchoReplyMessage(
    const char* buffer, const int len, const short id, const short sequenceNum)
{
    int count = 30;
    while(count--)
    {
        auto r = RecvIcmpMessage(buffer, len);
        if(r == -1)
        {
            perror("IcmpSocket::RecvIcmpEchoReplyMessage recvfrom error");
            break;
        }
        else if(r==0 ||
            r < sizeof(InetHeader) + sizeof(IcmpEchoHeader))
        {
            continue;
        }
        
        // check icmp echo header
        auto ipHeader = (const InetHeader*)buffer;
        auto ipHeaderLength = (ipHeader->VersionAndHeaderLength & 0x0f) * 4;

        auto icmpHeader = (const IcmpEchoHeader*)(buffer + ipHeaderLength);
        // not echo reply message
        if(icmpHeader->Header.Type != 0)
        {
            continue;
        }

        // check id, sequence equal
        if(icmpHeader->Identifier == htons(id) && icmpHeader->SequenceNum == htons(sequenceNum))
        {
            return r;
        }
    }
    return 0;
}