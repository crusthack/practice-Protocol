cmake_minimum_required(VERSION 3.20)

project(rawsock
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>
    )

    add_link_options(
        -stdlib=libc++
    )
endif()

add_library(rawsock
    src/rawsock/protocol/inet.cpp
    src/rawsock/protocol/icmp.cpp
    src/rawsock/protocol/udp.cpp
    src/rawsock/protocol/tcp.cpp

    src/rawsock/rawsocketcore.cpp
    src/rawsock/socket/icmpsocket.cpp
    src/rawsock/socket/udpsocket.cpp
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

add_executable(traceroute
    examples/traceroute.cpp
)

target_link_libraries(traceroute
    PRIVATE
        rawsock
)
