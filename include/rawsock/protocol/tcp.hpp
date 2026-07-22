#pragma once 

// 20 byte
struct TcpHeader
{
    unsigned short SrcPort;
    unsigned short DstPort;

    unsigned int SeqNum;

    unsigned int AckNum;

    unsigned short OffsetFlag;
    unsigned short Window;

    unsigned short Checksum;
    unsigned short Pointer;
    // options, padding... 

    static void CheckHeaderSize();
};
