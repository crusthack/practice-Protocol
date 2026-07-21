#include <stdio.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ipv4Header
{
    unsigned char VerIhl;       // high 4 bit for ip version, low 4 bit for internet header length
    unsigned char Type;
    unsigned short TotalLength; // packet total length

    unsigned short Id;          // Identification
    unsigned short FlagFrag;    // high 3 bit for flag, low 13 bit for fragment offset

    unsigned char TTL;          // time to live
    unsigned char Proto;        // Protocol
    unsigned short Checksum;    // check sum

    unsigned int SourceAddress; // Source IP

    unsigned int DestinationAddress; // Destination IP
} IPv4Header;

typedef struct _udpHeader
{
    unsigned short SrcPort;
    unsigned short DstPort;
    unsigned short DataLen;     // UDP header + payload
    unsigned short Checksum;
} UDPHeader;

typedef struct _udpPseudoHeader
{
    unsigned int SourceAddress;
    unsigned int DestinationAddress;

    unsigned char Zero;
    unsigned char Protocol;
    unsigned short UdpLength;
} UDPPseudoHeader;

unsigned short calcChecksum(const unsigned char* buffer, int len)
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

unsigned short calcUdpChecksum(
    const char* udpBuffer,
    int udpLength,
    unsigned int sourceAddress,
    unsigned int destinationAddress)
{
    UDPPseudoHeader pseudoHeader {};

    pseudoHeader.SourceAddress = sourceAddress;
    pseudoHeader.DestinationAddress = destinationAddress;
    pseudoHeader.Zero = 0;
    pseudoHeader.Protocol = IPPROTO_UDP;
    pseudoHeader.UdpLength = htons(udpLength);

    int checksumBufferSize =
        sizeof(UDPPseudoHeader) + udpLength;

    // 체크섬 계산 길이가 홀수일 경우를 대비해 1바이트 추가
    int paddingSize = checksumBufferSize % 2;

    char* checksumBuffer =
        (char*)calloc(1, checksumBufferSize + paddingSize);

    memcpy(
        checksumBuffer,
        &pseudoHeader,
        sizeof(UDPPseudoHeader));

    memcpy(
        checksumBuffer + sizeof(UDPPseudoHeader),
        udpBuffer,
        udpLength);

    unsigned short checksum =
        calcChecksum(
            (unsigned char*)checksumBuffer,
            checksumBufferSize);

    free(checksumBuffer);

    /*
        IPv4 UDP에서 checksum 필드가 0이면
        "체크섬을 사용하지 않음"이라는 의미이다.

        계산 결과가 0이면 0xFFFF로 기록한다.
    */
    if(checksum == 0)
    {
        checksum = 0xFFFF;
    }

    return checksum;
}

void buildUdpDatagram(
    char* buffer,
    int payloadSize,
    unsigned short srcPort,
    unsigned short dstPort,
    unsigned int sourceAddress,
    unsigned int destinationAddress)
{
    // UDP header
    auto p = (UDPHeader*)buffer;

    int udpLength =
        sizeof(UDPHeader) + payloadSize;

    p->SrcPort = htons(srcPort);
    p->DstPort = htons(dstPort);
    p->DataLen = htons(udpLength);
    p->Checksum = 0;

    // payload
    for(int i = 0; i < payloadSize; ++i)
    {
        buffer[sizeof(UDPHeader) + i] =
            (char)('A' + i % 26);
    }

    // UDP pseudo header + UDP header + payload
    p->Checksum = htons(
        calcUdpChecksum(
            buffer,
            udpLength,
            sourceAddress,
            destinationAddress));
}

void printUdpHeader(const char* buffer, int len)
{
    if(len < sizeof(UDPHeader) || buffer == nullptr)
    {
        return;
    }

    auto p = (UDPHeader*)buffer;

    printf("UDP Header Info\n");
    printf("source port: \t%d\n", ntohs(p->SrcPort));
    printf("destination port: \t%d\n", ntohs(p->DstPort));
    printf("udp length: \t%d\n", ntohs(p->DataLen));
    printf("checksum: \t0x%X\n", ntohs(p->Checksum));

    int udpLength = ntohs(p->DataLen);

    if(udpLength > len)
    {
        udpLength = len;
    }

    printf("UDP Payload...\n");

    for(int i = 0; i < udpLength - sizeof(UDPHeader); ++i)
    {
        printf("%c ", buffer[sizeof(UDPHeader) + i]);
    }

    printf("\n");
}

int main(int argc, char** argv)
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

    if(sock == -1)
    {
        perror("socket");
        return 1;
    }

    int one = 1;

    if(setsockopt(
        sock,
        IPPROTO_IP,
        IP_HDRINCL,
        &one,
        sizeof(one)) == -1)
    {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &addr.sin_addr);

    unsigned int sourceAddress;
    unsigned int destinationAddress;

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &sourceAddress);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &destinationAddress);

    int payloadSize = 30;

    int bufferSize =
        payloadSize + sizeof(UDPHeader);

    char* buffer =
        (char*)malloc(sizeof(char) * bufferSize);

    buildUdpDatagram(
        buffer,
        payloadSize,
        3000,
        4000,
        sourceAddress,
        destinationAddress);

    // build IP header
    char* ipbuffer =
        (char*)malloc(
            sizeof(char) *
            (bufferSize + sizeof(IPv4Header)));

    std::copy(
        buffer,
        buffer + bufferSize,
        ipbuffer + sizeof(IPv4Header));

    IPv4Header* p =
        (IPv4Header*)ipbuffer;

    p->VerIhl = (4 << 4) | (20 / 4);

    // DSCP / ECN
    p->Type = 0;

    // IP header + UDP header + payload
    p->TotalLength =
        htons(sizeof(IPv4Header) + bufferSize);

    // Identification
    static unsigned short id = 0;
    p->Id = htons(++id);

    // Flags + Fragment Offset
    // Don't Fragment = 0
    // More Fragment = 0
    // Offset = 0
    p->FlagFrag = htons(0);

    // TTL
    p->TTL = 64;

    // Protocol
    p->Proto = IPPROTO_UDP;

    p->SourceAddress = sourceAddress;
    p->DestinationAddress = destinationAddress;

    // IPv4 header checksum
    p->Checksum = 0;
    p->Checksum = htons(
        calcChecksum(
            (unsigned char*)p,
            sizeof(IPv4Header)));

    int ipPacketSize =
        bufferSize + sizeof(IPv4Header);

    int r = sendto(
        sock,
        ipbuffer,
        ipPacketSize,
        0,
        (sockaddr*)&addr,
        sizeof(addr));

    if(r == -1)
    {
        perror("sendto");

        free(buffer);
        free(ipbuffer);
        close(sock);

        return 1;
    }

    printf("UDP Packet (%d bytes)\n", bufferSize);

    printUdpHeader(buffer, bufferSize);

    char* recvBuffer =
        (char*)malloc(1500);

    int recvBufSize = 1500;

    // all of UDP packet sniffing
    while(1)
    {
        sockaddr_in adr {};
        socklen_t len = sizeof(adr);

        r = recvfrom(
            sock,
            recvBuffer,
            recvBufSize,
            0,
            (sockaddr*)&adr,
            &len);

        if(r == -1)
        {
            perror("recvfrom");
            break;
        }

        if(r < sizeof(IPv4Header))
        {
            continue;
        }

        // print IP header
        auto p = (IPv4Header*)recvBuffer;

        char src[16], dst[16];

        inet_ntop(
            AF_INET,
            &p->SourceAddress,
            src,
            16);

        inet_ntop(
            AF_INET,
            &p->DestinationAddress,
            dst,
            16);

        int ipver =
            p->VerIhl >> 4;

        int ipHeaderLen =
            (p->VerIhl & 0x0F) * 4;

        int totalLen =
            ntohs(p->TotalLength);

        int ttl = p->TTL;
        int proto = p->Proto;

        if(ipver != 4)
        {
            continue;
        }

        if(proto != IPPROTO_UDP)
        {
            continue;
        }

        if(ipHeaderLen < sizeof(IPv4Header))
        {
            continue;
        }

        if(r < ipHeaderLen + sizeof(UDPHeader))
        {
            continue;
        }

        printf(
            "IP  v=%d IHL=%d totalLen=%d TTL=%d proto=%d\n",
            ipver,
            ipHeaderLen,
            totalLen,
            ttl,
            proto);

        printf("Source IP: %s\n", src);
        printf("Destination IP: %s\n", dst);

        // IPv4 header 뒤에 UDP header가 위치
        char* udpBuffer =
            recvBuffer + ipHeaderLen;

        auto udpHeader =
            (UDPHeader*)udpBuffer;

        int udpLength =
            ntohs(udpHeader->DataLen);

        if(udpLength < sizeof(UDPHeader))
        {
            printf("invalid UDP length\n\n");
            continue;
        }

        if(ipHeaderLen + udpLength > r)
        {
            printf("truncated UDP packet\n\n");
            continue;
        }

        printUdpHeader(
            udpBuffer,
            udpLength);

        printf("\n");
    }

    free(buffer);
    free(ipbuffer);
    free(recvBuffer);

    close(sock);

    return 0;
}