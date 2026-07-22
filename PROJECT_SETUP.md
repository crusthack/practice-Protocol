# Project Setup

## Overview

`rawsock` is a Linux C++26 project for experimenting with IPv4, ICMP, UDP, TCP,
and raw sockets. It builds a reusable `rawsock` static library and the following
executables:

- `rawsock_app` from `app/main.cpp`
- `ping` from `examples/ping.cpp`
- `traceroute` from `examples/traceroute.cpp`

## Toolchain

The configured development toolchain is:

- Ubuntu 24.04
- CMake 3.20 or newer
- Ninja
- Clang 18
- libc++ 18
- C++26 without compiler extensions
- clangd 18 for VS Code code completion and diagnostics

Clang and libc++ are used because the installed GCC 13 standard library does
not provide the `<print>` header. Clang 18 implements an experimental subset of
C++26; it does not implement every final C++26 feature.

Install the required Ubuntu packages:

```bash
sudo apt update
sudo apt install cmake ninja-build clang-18 clangd-18 libc++-18-dev libc++abi-18-dev
```

What each package provides:

- `cmake` generates the build system from `CMakeLists.txt`.
- `ninja-build` performs the actual compilation efficiently.
- `clang-18` provides the C and C++ compiler.
- `clangd-18` provides code completion and diagnostics in VS Code.
- `libc++-18-dev` provides LLVM's C++ standard library, including `<print>`.
- `libc++abi-18-dev` provides the ABI library used by libc++.

Confirm that the main tools are available:

```bash
cmake --version
ninja --version
clang++-18 --version
clangd-18 --version
```

## Creating the Project from Scratch

Create and enter a new project directory:

```bash
mkdir protocol
cd protocol
```

Create the source directory structure:

```bash
mkdir -p app examples include/rawsock/protocol include/rawsock/socket
mkdir -p src/rawsock/protocol src/rawsock/socket
```

The resulting organization separates the public headers from their
implementations:

```text
protocol/
├── app/
├── examples/
├── include/rawsock/
│   ├── protocol/
│   └── socket/
└── src/rawsock/
    ├── protocol/
    └── socket/
```

Add a root `CMakeLists.txt`. The important initial configuration is:

```cmake
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
    add_link_options(-stdlib=libc++)
endif()

add_library(rawsock
    src/rawsock/protocol/inet.cpp
)

target_include_directories(rawsock
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

For each application, create an executable and link it to the library. Linking
also propagates the library's public `include` path to the application:

```cmake
add_executable(ping examples/ping.cpp)

target_link_libraries(ping
    PRIVATE
        rawsock
)
```

The complete list of this repository's current sources and targets is in
`CMakeLists.txt`.

Next, add `CMakePresets.json` to select the correct compiler reproducibly:

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "clang-debug",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/clang",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_COMPILER": "/usr/bin/clang++-18",
        "CMAKE_EXPORT_COMPILE_COMMANDS": true
      }
    }
  ],
  "buildPresets": [
    {
      "name": "clang-debug",
      "configurePreset": "clang-debug"
    }
  ]
}
```

Finally, add `.clangd` so the editor reads the same compiler options as CMake:

```yaml
CompileFlags:
  CompilationDatabase: build/clang
```

## Configure and Build

The `clang-debug` CMake preset selects Clang 18, Ninja, libc++, and a Debug
build. From the project root, run:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```

The first command performs the configure step. It detects the compiler, reads
the targets in `CMakeLists.txt`, and generates Ninja files plus
`compile_commands.json`. The second command invokes Ninja to compile and link
the targets.

After editing only C++ source or header files, normally just rebuild:

```bash
cmake --build --preset clang-debug
```

Run the configure command again after changing CMake configuration or adding
source files.

Build output and the compilation database are generated in `build/clang`.

The executables can then be run with:

```bash
./build/clang/rawsock_app
./build/clang/ping
./build/clang/traceroute
```

Programs that create raw sockets normally require elevated network privileges.
For local development, run the relevant executable with `sudo`, or grant only
the required capability:

```bash
sudo setcap cap_net_raw+ep ./build/clang/ping
```

Capabilities must be granted again after the executable is rebuilt.

## C++26 Language Mode and `<print>`

The CMake configuration requests C++26, producing the `-std=c++26` compiler
option. When compiling with Clang, the project also adds `-stdlib=libc++` during
compilation and linking. Code can therefore use existing C++23 library
features such as `<print>` alongside the C++26 features implemented by this
toolchain:

```cpp
#include <print>

int main()
{
    std::println("C++{} <print> works", 23);
}
```

Use the `clang-debug` preset when `<print>` or C++26 mode is required.
Configuring this machine with its default GCC 13 compiler will not find
`<print>` and provides less C++26 support.

Because C++26 compiler and library support is still incomplete, check a feature
test macro before depending on a newer facility. For example, the installed
libc++ 18 supports the C++26 `std::span` initializer-list constructor:

```cpp
#include <span>
#include <version>

#if __cpp_lib_span_initializer_list >= 202311L
void consume(std::span<const int> values);

void example()
{
    consume({1, 2, 3});
}
#endif
```

## Header Layout

Public headers are under `include/rawsock`:

```text
include/rawsock/
├── protocol/
│   ├── icmp.hpp
│   ├── inet.hpp
│   ├── tcp.hpp
│   └── udp.hpp
├── socket/
│   ├── icmpsocket.hpp
│   ├── tcpsocket.hpp
│   └── udpsocket.hpp
└── rawsocketcore.hpp
```

CMake publishes the `include` directory through `target_include_directories`,
so consumers linked to `rawsock` use headers such as:

```cpp
#include <rawsock/protocol/inet.hpp>
#include <rawsock/socket/icmpsocket.hpp>
```

## VS Code and clangd

Install VS Code separately, then install the **clangd** extension published by
LLVM. If Microsoft's C/C++ extension is also installed, disable its IntelliSense
for this workspace to avoid duplicate or conflicting diagnostics; it can remain
installed for its other debugging features.

Open the project root—not the `examples` or `src` subdirectory—in VS Code:

```bash
code .
```

The repository's `.clangd` file points clangd to:

```text
build/clang/compile_commands.json
```

Generate that file with `cmake --preset clang-debug` before opening source files.
After configuring for the first time, reload VS Code or run **clangd: Restart
language server** from the command palette.

If an include is incorrectly reported as missing, confirm that the compilation
database contains the current project path and this include option:

```text
-I/path/to/protocol/include
```

Do not reuse a CMake build directory after moving or renaming the repository;
configure a fresh build directory so it does not retain absolute paths from the
old location.

## Relevant Configuration Files

- `CMakeLists.txt` defines the library, executables, include path, and C++ mode.
- `CMakePresets.json` defines the Clang 18 Debug configure and build presets.
- `.clangd` selects the compilation database used by VS Code clangd.
