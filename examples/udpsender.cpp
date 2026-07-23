#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>
#include <rawsock/socket/udpsocket.hpp>
#include <string>

int main()
{
    UdpSocket sock{};

    std::string mes{};
    std::getline(std::cin >> std::ws, mes);
    sock.SendTo("13.238.15.82", 5000, (const unsigned char*)mes.c_str(), mes.length());
    
    unsigned char buffer[1024];
    char from[INET_ADDRSTRLEN];
    unsigned short portnum {};
    auto r = sock.RecvFrom(buffer, 1024, from, portnum);
    printf("From %s:%d:\t", from, portnum);
    for(int i=0; i<r; ++i)
    {
        printf("%c", buffer[i]);
    }
    printf("\n");

    return 0;
}
