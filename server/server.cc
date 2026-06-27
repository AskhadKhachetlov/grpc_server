#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "echo.grpc.pb.h"
#include "echo.pb.h"

using echo::EchoRequest;
using echo::EchoResponse;
using echo::EchoService;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class EchoServiceImpl final : public EchoService::Service {
public:
    Status Echo(ServerContext*, const EchoRequest*, EchoResponse *response) override 
    {
        response->set_message("OK");
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

    return 0;
}