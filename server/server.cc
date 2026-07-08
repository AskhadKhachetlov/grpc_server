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
using grpc::ServerUnaryReactor;
using grpc::Status;
using grpc::CallbackServerContext;
using grpc::InsecureServerCredentials;

class EchoServiceImpl final : public EchoService::CallbackService
{
public:
    ServerUnaryReactor *Echo(
        CallbackServerContext *context,
        const EchoRequest *request,
        EchoResponse *response) override
    {
        response->set_message(request->message());
        ServerUnaryReactor *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }
};

int main()
{
    std::string address = "0.0.0.0:50051";
    EchoServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(address, InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server = builder.BuildAndStart();
    std::cout << "Server ready: " << address << std::endl;

    server->Wait();
}