#include <rawsock/protocol/tcp.hpp>

void TcpHeader::CheckHeaderSize()
{
    static_assert(sizeof(TcpHeader) == 20, "Tcp Header size is not 20");
}