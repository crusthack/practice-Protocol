#include <cstdio>
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>
#include <string>
#include <unistd.h>


int main(int argc, const char** argv)
{
    const char* dst_ip = "127.0.0.1";
    int n = 1;
    switch(argc)
    {
        // ./ping => ping localhost, 1 time
        case 1:
            break;
        // ./ping <dst_ip> => ping dst_ip, 1 time
        case 2:
            dst_ip = *(argv + 1);
            break;
        // ./ping <dst_ip> <count>  => ping dst_ip, n time
        case 3: 
            dst_ip = *(argv + 1);
            n = std::stoi(*(argv + 2));
            break;
        // print help
        default: 
            printf("usage: ./ping: ping localhost 1 time\n");
            printf("./ping <dst_ip>: ping dst_ip 1 time\n");
            printf("./ping <dst_ip> <count>: ping dst_ip n time\n");
            break;
    }
    auto pid = getpid();
    IcmpSocket sock {};
    printf("send ping message %s. %d times\n", dst_ip, n);
    for(int i=0; i < n; ++i)
    {
        sock.SendPingMessage(dst_ip, pid, i + 1);
    }
    
    return 0;
}
