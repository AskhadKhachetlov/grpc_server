add_executable(client client.cc)
target_link_libraries(client PRIVATE protolib)
target_link_libraries(client PRIVATE ProjectConfiguration)