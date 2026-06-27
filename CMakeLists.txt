cmake_minimum_required(VERSION 3.20)

project(myserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(server)

if (MSVC)
    target_compile_options(server PRIVATE /W4 /WX)
    target_compile_options(client PRIVATE /W4 /WX)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

#message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
#message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")