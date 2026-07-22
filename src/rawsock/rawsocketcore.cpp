#include <rawsock/rawsocketcore.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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
int GetHostIp(sockaddr_in* addr, const char* dstip)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(44);
    inet_pton(AF_INET, dstip, &addr->sin_addr);
    auto r = connect(s, (sockaddr*)addr, sizeof(sockaddr_in));

    socklen_t l = sizeof(sockaddr_in);
    getsockname(s, (sockaddr*)addr, &l);

    close(s);
    
    return 0;
}


int GetHostIp(char* const srcip, const char* const dstip)
{
    sockaddr_in addr {};
    GetHostIp(&addr, dstip);
    inet_ntop(AF_INET, &addr.sin_addr, srcip, INET_ADDRSTRLEN);
    
    return 0;
}