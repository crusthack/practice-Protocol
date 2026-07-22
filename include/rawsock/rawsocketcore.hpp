#pragma once 

#include <netinet/in.h>

#define Byte unsigned char
#define Word unsigned short
#define DoubleWord unsigned int

unsigned short CalcChecksumCommon(const char* buf, int len);
int GetHostIp(sockaddr_in* addr, const char* dstip);
int GetHostIp(char* const srcip, const char* const dstip);