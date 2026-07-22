cmake_minimum_required(VERSION 3.20)

project(rawsock
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rawsock
    src/rawsock/protocol/inet.cpp
    src/rawsock/protocol/icmp.cpp
    src/rawsock/protocol/udp.cpp
    src/rawsock/protocol/tcp.cpp

    src/rawsock/rawsocketcore.cpp
    src/rawsock/socket/icmpsocket.cpp
    # src/socket/udpsocket.cpp
    src/rawsock/socket/tcpsocket.cpp
)

target_include_directories(rawsock
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(rawsock_app
    app/main.cpp
)

target_link_libraries(rawsock_app
    PRIVATE
        rawsock
)

add_executable(ping
    examples/ping.cpp
)

target_link_libraries(ping
    PRIVATE
        rawsock
)