#include "rawsock/rawsocketcore.hpp"
#include <netinet/in.h>
#include <rawsock/protocol/icmp.hpp>

void IcmpHeader::CheckHeaderSize()
{
    static_assert(sizeof(IcmpHeader) == 4, "Icmp Header size is not 4");
}

void IcmpHeader::BuildIcmpHeader(char *buffer, unsigned char type, unsigned char code)
{
    auto p = (IcmpHeader*) buffer;
    p->Type = type;
    p->Code = code;
    p->Checksum = 0;
}

void IcmpHeader::CalcIcmpHeaderChecksum(char *buffer, int len)
{
    auto p = (IcmpHeader*)buffer;
    p->Checksum = htons(CalcChecksumCommon(buffer, len));
}

void IcmpEchoHeader::CheckHeaderSize()
{
    static_assert(sizeof(IcmpEchoHeader) == 8, "Icmp Echo Header size is not 4");
}

void IcmpEchoHeader::BuildIcmpHeader(
    char *buffer, unsigned char type, unsigned char code, 
    Word id, Word sequenceNum)
{
    IcmpHeader::BuildIcmpHeader(buffer, type, code);

    auto p = (IcmpEchoHeader*)buffer;
    p->Identifier = htons(id);
    p->SequenceNum = htons(sequenceNum);
}

void IcmpEchoHeader::CalcPingChecksum(char* buffer, int len)
{
    auto p = (IcmpHeader*)buffer;
    p->Checksum = htons(CalcChecksumCommon(buffer, len));
}