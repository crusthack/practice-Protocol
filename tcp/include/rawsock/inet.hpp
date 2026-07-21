#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#define Byte unsigned char
#define Word unsigned short
#define DoubleWord unsigned int

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

// struct for send ICMPv4 message
struct IcmpSocket
{
public:
    IcmpSocket();

    ~IcmpSocket() = default;

    IcmpSocket(const IcmpSocket&) = delete;
    IcmpSocket(IcmpSocket&&) = delete;
    IcmpSocket& operator=(const IcmpSocket&) noexcept = default;
    IcmpSocket& operator=(IcmpSocket&&) noexcept = default;

    void SendPingRequestMessage(const char* dstIp = "127.0.0.1");

    void RecvIcmpReplyMessage(const int count);
    void RecvIcmpMessage(const int count, const int type = -1);

    void BuildPingRequestMessage(const char* buffer, int len, const char* srcIp = nullptr, const char* dstIp = "127.0.0.1");

    int GetLocalhostIp(char* buffer, int len);
    
private:
    int _Socket;
};
