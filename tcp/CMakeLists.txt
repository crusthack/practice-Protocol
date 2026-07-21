cmake_minimum_required(VERSION 3.20)

project(cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(tcp
    src/main.cpp
    src/rawsock/inet.cpp
    src/rawsock/tcp.cpp
)

target_include_directories(tcp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(tcp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_custom_command(
    TARGET tcp
    POST_BUILD
    COMMAND sudo setcap cap_net_raw+ep $<TARGET_FILE:tcp>
    COMMENT "Granting CAP_NET_RAW to tcp"
    VERBATIM
)