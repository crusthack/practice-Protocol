#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <rawsock/rawsocketcore.hpp>


unsigned short CalcChecksumCommon(const char* buffer, int len);
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
    Word HeaderChecksum;            // calc only IP Header, exclude payload

    DoubleWord SourceAddress;

    DoubleWord DestinationAddress;
    // options, padding... 

    static void CheckHeaderSize();

    static void BuildInetHeader(char* buffer, unsigned short totalLength, 
        Word id, Byte ipProto, const char* srcIp, const char* dstIp);

    static void CalcInetHeaderChecksum(const char* buffer, int len);

    bool IsValidInetHeader(int totalLen);
};