#include <grpcpp/grpcpp.h>
#include "proto/echo.grpc.pb.h"
#include "proto/echo.pb.h"

using echo::EchoRequest;
using echo::EchoResponse;
using echo::EchoService;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class EchoClient
{
public:
    EchoClient(std::shared_ptr<Channel> channel) : stub_(EchoService::NewStub(channel)) {}

    std::string Echo(const std::string &msg)
    {
        EchoRequest request;
        request.set_message(msg);

        EchoResponse response;
        ClientContext context;
        Status status = stub_->Echo(&context, request, &response);

        if (status.ok())
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
    auto channel = grpc::CreateChannel(
        "localhost:50051",
        grpc::InsecureChannelCredentials());

    EchoClient client(channel);

    std::string msg = "Hello gRPC!";
    std::string reply = client.Echo(msg);

    std::cout << msg << std::endl;
    std::cout << reply << std::endl;

    return 0;
}