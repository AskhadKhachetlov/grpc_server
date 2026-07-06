# gRPC Server — C++

C++ gRPC client-server project built with CMake and vcpkg.
The client sends a string to the server.
The server returns the same string.

## Requirements

- CMake 3.20+
- C++ compiler with C++20 support
- Protobuf
- gRPC

## Build with vcpkg

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

If you use another installation method for gRPC and Protobuf, adjust the CMake configuration accordingly.

## Run

```bash
# Terminal 1
server\debug\server.exe

# Terminal 2
client\debug\server.exe
```