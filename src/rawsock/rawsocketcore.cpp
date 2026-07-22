#include <rawsock/rawsocketcore.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

unsigned short CalcChecksumCommon(const char* buf, int len)
{
    auto buffer = (unsigned char*)buf;
    unsigned int sum = 0;
    while(len >= 2)
    {
        const unsigned short word = (buffer[0] << 8) | buffer[1];
        sum += word;
        len -= 2;
        buffer += 2;
    }
    // if len is odd
    if(len == 1)
    {
        sum += buffer[0] << 8;
    }
 
    while((sum >> 16) != 0)
    {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
 
    return ~sum;
}

// maybe, make rawsock class than move to static method. tcpsock icmpsock inherit rawsock
int GetLocalhostIp(char* buffer, int len)
{
    if(len < 16)
    {
        return -1;
    }
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(44);
    inet_pton(AF_INET, "8.8.8.8", (sockaddr_in*)&addr.sin_addr);
    if(connect(s, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        return -1;
    }

    sockaddr_in local {};
    socklen_t l = sizeof(local);
    getsockname(s, (sockaddr*)&local, &l);
    inet_ntop(AF_INET, (sockaddr*)&local.sin_addr, buffer, l);

    return 0;
}
