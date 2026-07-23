#include <cstring>
#include <iostream>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>
#include <rawsock/socket/udpsocket.hpp>
#include <string>

int main()
{
    UdpSocket sock{};

    std::string mes{};
    std::cin >> mes;
    sock.SendTo("13.238.15.82", 5000, (const unsigned char*)mes.c_str(), mes.length());
    
    return 0;
}
