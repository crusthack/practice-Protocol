#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
    auto sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    while(1)
    {
        char buffer[500];
        
        sockaddr_in addr {};
        socklen_t len = sizeof(addr);
        recvfrom(sock, buffer, 500, 0, (sockaddr*)&addr, &len);
        
        printf("%s\n", buffer);
    }

    return 0;
}