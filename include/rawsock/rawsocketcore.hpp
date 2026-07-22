#pragma once 

#define Byte unsigned char
#define Word unsigned short
#define DoubleWord unsigned int

unsigned short CalcChecksumCommon(const char* buf, int len);
int GetLocalhostIp(char* buffer, int len);