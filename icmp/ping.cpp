#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
 
typedef struct _icmpHeader
{
    unsigned char type; // 0=Echo Reply, 8=Echo Request 11=TTL Exceeded
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short sequenceNum;
} IcmpHeader;
 
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
 
unsigned short calcIcmpChechsum(const unsigned char* buffer, int len)
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
 
void buildIcmpRequest(char* buffer, int payloadsize, unsigned short id, unsigned short seq)
{
    // header
    auto p = (IcmpHeader*)buffer;
    p->type = 8;
    p->code = 0;
    p->id = htons(id);
    p->sequenceNum = htons(seq);
    p->checksum = 0;
    
    // payload
    for(int i=0; i < payloadsize; ++i)
        buffer[sizeof(IcmpHeader) + i] = (char)('A' + i % 26);
    
    //calculate header
    p->checksum = htons(calcIcmpChechsum((unsigned char*)buffer, sizeof(IcmpHeader) + payloadsize));
}
 
void printIcmpHeader(const char* buffer, int len)
{
    if(len < sizeof(IcmpHeader) || buffer == nullptr)
    {
        return;
    }
 
    auto p = (IcmpHeader*)buffer;
    printf("ICMP Header Info\n");
    printf("icmp type: \t%02d\n", p->type);
    printf("icmp code: \t%02d\n", p->code);
    printf("icmp id: \t%d\n", ntohs(p->id));
    printf("icmp sqeuence: \t%d\n", ntohs(p->sequenceNum));
    printf("checksum: \t0x%X\n", ntohs(p->checksum));
 
    if(p->type==0 || p->type==8)
    {
        printf("ping message Payload...\n");
 
        for(int i=0; i<len - sizeof(IcmpHeader); ++i)
        {
            printf("%c ", buffer[sizeof(IcmpHeader) + i]);
        }
    }
    printf("\n");
}
 
 
int main(int argc, char** argv)
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
 
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", (sockaddr*)&addr.sin_addr);
 
    int payloadSize = 30;
    int bufferSize = payloadSize + sizeof(IcmpHeader);
    char* buffer = (char*)malloc(sizeof(char) * bufferSize);
 
    buildIcmpRequest(buffer, payloadSize, 3000, 3);

    // build ip header
    char* ipbuffer = (char*)malloc(sizeof(char) * (bufferSize + sizeof(IPv4Header)));
    std::copy(buffer, buffer + bufferSize, ipbuffer + sizeof(IPv4Header));
    IPv4Header* p = (IPv4Header*)ipbuffer;
    p->VerIhl = (4 << 4) | (20/4);
    // DSCP / ECN
    p->Type = 0;

    // 전체 길이
    p->TotalLength = htons(sizeof(IPv4Header) + bufferSize);

    // Identification
    static uint16_t id = 0;
    p->Id = htons(++id);

    // Flags + Fragment Offset
    // Don't Fragment = 0
    // More Fragment = 0
    // Offset = 0
    p->FlagFrag = htons(0);

    // TTL
    p->TTL = 64;

    // Protocol
    p->Proto = IPPROTO_ICMP;

    inet_pton(AF_INET,
          "172.17.2.68",
          &p->SourceAddress);

    // Destination IP
    inet_pton(AF_INET,
            "8.8.8.8",
            &p->DestinationAddress);

    p->Checksum = 0;
    p->Checksum = calcIcmpChechsum((unsigned char*)p, sizeof(IPv4Header));


    int r = sendto(sock, ipbuffer, bufferSize + sizeof(IPv4Header), 0, (sockaddr*)&addr, sizeof(addr));
 
    printf("ICMP Packet (%d bytes)\n", bufferSize);
 
    printIcmpHeader(buffer, bufferSize);
 
    char* recvBuffer = (char*)malloc(1500);
    int recvBufSize = 1500;
    sockaddr_in adr {};
    socklen_t len {};
    // all of icmp packet snipping
    while(1)
    {
        r = recvfrom(sock, recvBuffer, recvBufSize, 0, (sockaddr*)&adr, &len);
        if(r == -1)
        {
            printf("error\n");
            return 0;
        }
        if(r < sizeof(IPv4Header))
        {
            continue;
        }
 
        // print ip header
        auto p = (IPv4Header*)recvBuffer;
        char src[16], dst[16];
        inet_ntop(AF_INET, &p->SourceAddress, src, 16);
        inet_ntop(AF_INET, &p->DestinationAddress, dst, 16);
        int ipver = p->VerIhl >> 4;
        int ipHeaderLen = (p->VerIhl & 0b00011111) * 4;
        int totalLen = ntohs(p->TotalLength);
        int ttl = p->TTL;
        int proto = p->Proto;
 
        printf("IP  v=%d IHL=%d totalLen=%d TTL=%d proto=%d\n",
               ipver, ipHeaderLen, totalLen, ttl, proto);
        printf("Source IP: %s\n", src);
        printf("Destination IP: %s\n", dst);
        printf("\n");
    }
 
    return 0;
}