#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "protolib/echo.grpc.pb.h"
#include "protolib/echo.pb.h"

using echo::EchoRequest;
using echo::EchoResponse;
using echo::EchoService;

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;


class EchoClient
{
public:
    explicit EchoClient(std::shared_ptr<Channel> channel)
        : stub_(EchoService::NewStub(channel)) {}

    std::string Echo(const std::string &msg)
    {
        EchoRequest request;
        request.set_message(msg);

        EchoResponse response;
        ClientContext context;
        CompletionQueue cq;
        Status status;

        std::unique_ptr<ClientAsyncResponseReader<EchoResponse>> rpc(
            stub_->AsyncEcho(&context, request, &cq));

        rpc->Finish(&response, &status, (void *)1);

        void *got_tag = nullptr;
        bool ok = false;
        cq.Next(&got_tag, &ok);

        if (ok && got_tag == (void *)1 && status.ok())
        {
            return response.message();
        }

        return "Error: " + status.error_message();
    }

private:
    std::unique_ptr<EchoService::Stub> stub_;
};

int main()
{
    auto channel = CreateChannel("localhost:50051", InsecureChannelCredentials());

    EchoClient client(channel);

    std::string msg = "Hello gRPC!";
    std::string reply = client.Echo(msg);

    std::cout << "Client: " << msg << std::endl;
    std::cout << "Server: " << reply << std::endl;
}