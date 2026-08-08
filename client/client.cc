#include <iostream>
#include <fstream>
#include <iterator>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "protolib/caption_service.grpc.pb.h"
#include "protolib/caption_service.pb.h"

using caption_service::AddCaptionRequest;
using caption_service::AddCaptionResponse;
using caption_service::CaptionService;

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;

namespace
{
    std::string ReadFile(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    }
} // namespace

class CaptionClient
{
public:
    explicit CaptionClient(std::shared_ptr<Channel> channel)
        : stub_(CaptionService::NewStub(channel)) {}

    std::string AddCaption(const std::string &image_data, const std::string &caption)
    {
        AddCaptionRequest request;
        request.set_image(image_data);
        request.set_caption(caption);

        AddCaptionResponse response;
        ClientContext context;
        CompletionQueue cq;
        Status status;

        std::unique_ptr<ClientAsyncResponseReader<AddCaptionResponse>> rpc(
            stub_->AsyncAddCaption(&context, request, &cq));

        rpc->Finish(&response, &status, (void *)1);

        void *got_tag = nullptr;
        bool ok = false;
        cq.Next(&got_tag, &ok);

        if (ok && got_tag == (void *)1 && status.ok())
        {
            return response.image();
        }

        std::cerr << "Error: " << status.error_message() << std::endl;
        return "";
    }

private:
    std::unique_ptr<CaptionService::Stub> stub_;
};

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <image_path> <caption> [output_path]" << std::endl;
        return 1;
    }

    const std::string image_path = argv[1];
    const std::string caption = argv[2];
    const std::string output_path = (argc >= 4) ? argv[3] : "captioned_output.jpg";

    const std::string image_data = ReadFile(image_path);
    
    if (image_data.empty())
    {
        std::cerr << "Failed to read image: " << image_path << std::endl;
        return 1;
    }

    auto channel = CreateChannel("localhost:50051", InsecureChannelCredentials());
    CaptionClient client(channel);

    const std::string result = client.AddCaption(image_data, caption);
    if (result.empty())
    {
        std::cerr << "Failed to get captioned image" << std::endl;
        return 1;
    }

    std::ofstream out(output_path, std::ios::binary);
    out.write(result.data(), result.size());

    std::cout << "Saved captioned image to " << output_path << std::endl;
}