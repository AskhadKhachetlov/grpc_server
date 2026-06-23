cmake_minimum_required(VERSION 3.20)

project(myserver LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_library(protolib proto/echo.proto)
target_link_libraries(protolib gRPC::grpc++)
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET protolib LANGUAGE cpp)
protobuf_generate(TARGET protolib LANGUAGE grpc
GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_executable(server src/server.cc)
target_link_libraries(server protolib)
target_include_directories(server PRIVATE ${CMAKE_BINARY_DIR})

add_executable(client src/client.cc)
target_link_libraries(client protolib)
target_include_directories(client PRIVATE ${CMAKE_BINARY_DIR})

if (MSVC)
    target_compile_options(server PRIVATE /W4 /WX)
    target_compile_options(client PRIVATE /W4 /WX)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")