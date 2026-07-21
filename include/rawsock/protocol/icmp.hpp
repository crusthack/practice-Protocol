#pragma once 
#include <rawsock/protocol/inet.hpp>
// 4 byte
struct IcmpHeader
{
    Byte Type;
    Byte Code;
    Word Checksum;

    static void CheckHeaderSize()
    {
        static_assert(sizeof(IcmpHeader) == 4, "Icmp Header size is not 4");
    }

    static void BuildIcmpHeader(char* buffer, Byte type, Byte code)
    {
        auto p = (IcmpHeader*) buffer;
        p->Type = type;
        p->Code = code;
        p->Checksum = 0;
    }

    static void CalcIcmpHeaderChecksum(char* buffer, int len)
    {
        auto p = (IcmpHeader*)buffer;
        p->Checksum = htons(CalcChecksum(buffer, len));
    }
};
