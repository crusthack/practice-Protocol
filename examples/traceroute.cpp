#include <iostream>
#include <rawsock/socket/icmpsocket.hpp>

int main()
{
    IcmpSocket sock {};

    char buffer[500] {};
    sock.BuildPingRequestMessage(buffer, 40);

    return 0;
}