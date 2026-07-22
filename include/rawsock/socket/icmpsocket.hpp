#pragma once 

// struct for send ICMPv4 message
struct IcmpSocket
{
public:
    IcmpSocket();

    ~IcmpSocket();

    IcmpSocket(const IcmpSocket&) = delete;
    IcmpSocket(IcmpSocket&&) = delete;
    IcmpSocket& operator=(const IcmpSocket&) noexcept = default;
    IcmpSocket& operator=(IcmpSocket&&) noexcept = default;

    void SendPingMessage(const char* dstIp = "127.0.0.1", const short id = 5, const short sequenceNum = 1);

    // recv icmp echo reply include ipv4 header;
    int RecvIcmpEchoReplyMessage(const char* buffer, const int len, const short id, const short sequenceNum);
    // recv icmp packet include ipv4 header
    int RecvIcmpMessage(const char* buffer, const int len);

    void BuildPingRequestMessage(const char* buffer, int len, const char* srcIp = nullptr, const char* dstIp = "127.0.0.1",
        const short id = 1, const short sequenceNum = 1);

    
private:
    int _Socket;
};
