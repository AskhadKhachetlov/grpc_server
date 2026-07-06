#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "protolib/echo.grpc.pb.h"
#include "protolib/echo.pb.h"

using echo::EchoRequest;
using echo::EchoResponse;
using echo::EchoService;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class EchoServiceImpl final : public EchoService::Service {
public:
    Status Echo(ServerContext*, const EchoRequest* request, EchoResponse* response) override 
    {
        std::string msg = request->message();
        response->set_message(msg);
        return Status::OK;
    }
};

int main() {
    std::string address = "0.0.0.0:50051";
    EchoServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server = builder.BuildAndStart();
    std::cout << "Server ready: " << address << std::endl;

    server->Wait();
}