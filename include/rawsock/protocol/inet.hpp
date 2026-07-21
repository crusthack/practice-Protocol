#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <rawsock/rawsocketcore.hpp>


unsigned short CalcChecksum(const char* buffer, int len);
// IPv4 Header. should be 20 byte
struct InetHeader
{
    Byte VersionAndHeaderLength;    // high 4 bit for ip version, low 4 bit for header length
    Byte TypeOfService;             // qos setting. commonly 0
    Word TotalLength;               // total packet length

    Word Identification;
    Word FlagsAndFragmentOffset;    // high 3 bit for flag, low bit for fragment offset

    Byte TimeToLive;                // default 64
    Byte Protocol;
    Word HeaderChecksum;

    DoubleWord SourceAddress;

    DoubleWord DestinationAddress;
    // options, padding... 

    static void CheckHeaderSize()
    {
        static_assert(sizeof(InetHeader) == 20, "IPv4 Header size is not 20");
    }

    static void BuildInetHeader(char* buffer, unsigned short totalLength, Word id, Byte ipProto,
        const char* srcIp, const char* dstIp)
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

    static void CalcInetHeaderChecksum(const char* buffer, int len);
};