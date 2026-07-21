#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <rawsock/inet.hpp>
#include <rawsock/tcp.hpp>
#include <sys/socket.h>

int main()
{
    IcmpSocket sock {};
    sock.SendPingRequestMessage();

    return 0;
}