#include <rawsock/rawsocketcore.hpp>

// 8 byte
struct UdpHeader
{
    Word SrcPort;
    Word DstPort;
    Word Length;
    Word Checksum;
};