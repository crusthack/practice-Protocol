#pragma once 

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
