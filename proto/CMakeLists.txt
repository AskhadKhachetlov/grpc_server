add_library(protolib ${CMAKE_CURRENT_SOURCE_DIR}/protolib/echo.proto)

target_link_libraries(protolib PUBLIC gRPC::grpc++ protobuf::libprotobuf)

target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(TARGET protolib LANGUAGE cpp)

protobuf_generate(TARGET protolib LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
)