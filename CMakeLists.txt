cmake_minimum_required(VERSION 3.20)

project(echo_service LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(project_configuration)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(image_processor)