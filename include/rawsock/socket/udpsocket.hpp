#pragma once

#include <rawsock/rawsocketcore.hpp>

class UdpSocket
{
public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    UdpSocket(UdpSocket&& other) noexcept = default;
    UdpSocket& operator=(UdpSocket&& other) noexcept = default;


    void SendTo(const char* dstIp, const Word portNum, const Byte* buffer, const int len);
    void RecvFrom(Byte* const buffer, const int len, char* const srcIp, const Word& portNum);

private:
    int _Socket {-1};
};