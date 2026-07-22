#pragma once 

#include <rawsock/rawsocketcore.hpp>
#include <rawsock/protocol/inet.hpp>

// 4 byte
struct IcmpHeader
{
    Byte Type;
    Byte Code;
    Word Checksum;

    static void CheckHeaderSize();

    static void BuildIcmpHeader(char* buffer, Byte type, Byte code);

    static void CalcIcmpHeaderChecksum(char* buffer, int len);
};

struct IcmpEchoHeader
{
    IcmpHeader Header;
    
    Word Identifier;
    Word SequenceNum;

    static void CheckHeaderSize();
    static void BuildIcmpHeader(char* buffer, Byte type, Byte code, Word id, Word sequenceNum);
    static void CalcPingChecksum(char* buffer, int len);
};