#include <cstring>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>
#include <rawsock/socket/udpsocket.hpp>

int main()
{
    UdpSocket sock{};

    auto message = "hello, world!!";
    sock.SendTo("13.238.15.82", 5000, (const unsigned char*)message, strlen(message));

    return 0;
}
