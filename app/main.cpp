#include <cstdio>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>

int main()
{
    IcmpSocket sock {};
    sock.SendPingRequestMessage();

    return 0;
}
