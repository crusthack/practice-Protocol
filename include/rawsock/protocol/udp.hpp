#pragma once
#include <rawsock/rawsocketcore.hpp>

// 8 byte
struct UdpHeader
{
    Word SrcPort;
    Word DstPort;
    Word Length;
    Word Checksum;

    static void CheckHeaderSize();
    static Word CalcUdpHeaderChecksum(char* buffer, int len, const char* srcIp, const char* dstIp);
    static void BuildUdpMessage(char* buffer,  const char* payload, int payloadLen, const unsigned short portNum, const char* srcIp, const char* dstIp);
};

struct PseudoUdpHeader
{
    DoubleWord SrcIp;
    DoubleWord DstIp;
    Byte Zero;
    Byte Protocol;  // IPPROTO_UDP
    Word Length;    // udp length?d 
};