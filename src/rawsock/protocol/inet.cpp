#include "rawsock/protocol/inet.hpp"
#include "rawsock/rawsocketcore.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>

void InetHeader::CalcInetHeaderChecksum(const char* buffer, int len)
{
    auto p = (InetHeader*)buffer;
    p->HeaderChecksum = 0;
    p->HeaderChecksum = htons(CalcChecksumCommon(buffer, len));
}

void InetHeader::CheckHeaderSize()
{
    static_assert(sizeof(InetHeader) == 20, "IPv4 Header size is not 20");
}

void InetHeader::BuildInetHeader(char *buffer, unsigned short totalLength, unsigned short id, unsigned char ipProto, const char *srcIp, const char *dstIp)
{
    auto p = (InetHeader*) buffer;
    p->VersionAndHeaderLength = (4 << 4) | sizeof(InetHeader) / 4;
    p->TypeOfService = 0;
    p->TotalLength = htons(totalLength);

    p->Identification = htons(id);
    p->FlagsAndFragmentOffset = htons(0);

    p->TimeToLive = 64;
    p->Protocol = ipProto;
    p->HeaderChecksum = htons(0);
    
    inet_pton(AF_INET, srcIp, &p->SourceAddress);
    inet_pton(AF_INET, dstIp, &p->DestinationAddress);

    CalcInetHeaderChecksum(buffer, sizeof(InetHeader));
}

bool InetHeader::IsValidInetHeader(int totalLen)
{
    auto ipver = VersionAndHeaderLength >> 4;
    auto ihl = (VersionAndHeaderLength & 0x0f) * 4;
    auto total = ntohs(TotalLength);
    auto p = this;
    CalcInetHeaderChecksum((char*)p, sizeof(InetHeader));

    bool isValid =
        ipver == 4 &&
        ihl >= sizeof(InetHeader) &&
        total == totalLen &&
        // if checksum is valid, this will be 0
        CalcChecksumCommon((const char*)this, sizeof(InetHeader)) == 0 &&
        TimeToLive > 0;
    
    return isValid;
}; 