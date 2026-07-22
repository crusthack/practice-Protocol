#include "rawsock/rawsocketcore.hpp"
#include <arpa/inet.h>
#include <rawsock/protocol/udp.hpp>
#include <sys/socket.h>
#include <algorithm>
#include <unistd.h>

void UdpHeader::CheckHeaderSize()
{
    static_assert(sizeof(UdpHeader) == 8, "UDP Header size is not 8");
}

Word UdpHeader::CalcUdpHeaderChecksum(
    char* buffer,
    int len,
    const char* srcIp,
    const char* dstIp)
{
    PseudoUdpHeader pseudoHeader{};
    pseudoHeader.Length = htons(len);
    pseudoHeader.Protocol = IPPROTO_UDP;

    inet_pton(AF_INET, srcIp, &pseudoHeader.SrcIp);
    inet_pton(AF_INET, dstIp, &pseudoHeader.DstIp);

    // CalcChecksumCommon()은 이미 1의 보수를 적용한 값을 반환하므로
    // 다시 반전하여 원래의 16비트 합을 얻는다.
    const unsigned int udpSum =
        static_cast<unsigned short>(
            ~CalcChecksumCommon(buffer, len)
        );

    const unsigned int pseudoHeaderSum =
        static_cast<unsigned short>(
            ~CalcChecksumCommon(
                reinterpret_cast<const char*>(&pseudoHeader),
                sizeof(pseudoHeader)
            )
        );

    unsigned int checksum = udpSum + pseudoHeaderSum;

    while(checksum >> 16)
    {
        checksum =
            (checksum & 0xffffu) +
            (checksum >> 16);
    }

    Word result = static_cast<Word>(~checksum);

    // IPv4 UDP에서 0x0000은 체크섬 미사용을 의미하므로
    // 실제 계산 결과가 0이면 0xffff로 전송한다.
    if(result == 0)
    {
        result = 0xffff;
    }

    return result;
}

void UdpHeader::BuildUdpMessage(char* buffer, const char* payload, int payloadLen, const unsigned short portNum, const char* srcIp, const char* dstIp)
{
    auto udpP = (UdpHeader*)buffer;
    auto payloadP = buffer + sizeof(UdpHeader);
    std::copy(payload, payload + payloadLen, payloadP);

    udpP->Checksum = 0;
    udpP->SrcPort = htons(getpid() & 0xffff);
    udpP->DstPort = htons(portNum);
    udpP->Length = htons(payloadLen + sizeof(UdpHeader));
    udpP->Checksum = htons(UdpHeader::CalcUdpHeaderChecksum(buffer, payloadLen + sizeof(UdpHeader), srcIp, dstIp));
}