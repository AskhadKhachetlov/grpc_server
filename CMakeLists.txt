cmake_minimum_required(VERSION 3.20)

project(echo_service LANGUAGES CXX)

include(cmake/project_configuration.cmake)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(server)

target_link_libraries(client PRIVATE ProjectConfiguration)
target_link_libraries(server PRIVATE ProjectConfiguration)