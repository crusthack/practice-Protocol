# Network Protocol Implementation practice 
- Implement ICMPv4, UDP, TCP, Ethernet Messages using Raw Socket in Linux Ubuntu
- using clang/llvm, cmake, ninja...
- project can be expand to IPv6, CAN, CAN FD...
- **RawSocket을** 사용해 IP Header부터 시작되는 패킷 작성을 하여 여러 프로토콜 메시지를 작성합니다.\
(Ethernet Frame 실습은 단발적으로 진행)

# Goal 
1. 작성한 커스텀 패킷 와어샤크에서의 관찰 
2. RawSocket 래핑 클래스를 만든 후 메시지 통신 구현 
3. 여러가지 환경에서의 소켓 통신 동작 확인 

# Project Environment Setup
## Virtual Environment 
- Windows Hyper-V. Ubuntu 24

## Install Project Tool
- Use VS Code for text edit
- [VS Code Download Link](https://code.visualstudio.com/)
```
sudo apt install clang llvm lld lldb clangd clang-format clang-tidy cmake ninja-build
```

## Write CMake 

## build 
```
cmake -S . -B build -G Ninja
cmake --build build
# or 
ninja -C build
```

# Implementation
## Protocol 
- IPv4 
- ICMP ping message 
- UDP 
- TCP 
- Ethernet Frame 

## Example Program 
- ping 
- traceroute
- udp p2p chatting 
- tcp chatting 

## expand
- iovec msghdr 사용해볼것. scatter i/o. recvmsg()


# Test 
- Wireshark packet capture 
- Console program functional test 
- Local Communication with Common Socket (Native Ubuntu env)
- Windows - Linux (Lan env, same wifi)

# raw socket 권한 부여
- `sudo setcap cap_net_raw=eip build/clang/ping`
- 아니면 매번 sudo 붙여서 프로그램 실행