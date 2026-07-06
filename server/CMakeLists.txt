add_executable(server server.cc)
target_link_libraries(server PRIVATE protolib)
target_link_libraries(server PRIVATE ProjectConfiguration)