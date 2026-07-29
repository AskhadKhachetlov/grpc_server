#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "protolib/caption_service.grpc.pb.h"
#include "protolib/caption_service.pb.h"

#include "image_processor/image_processor.h"

using caption_service::AddCaptionRequest;
using caption_service::AddCaptionResponse;
using caption_service::CaptionService;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerUnaryReactor;
using grpc::Status;
using grpc::StatusCode;
using grpc::CallbackServerContext;
using grpc::InsecureServerCredentials;

namespace ip = image_processor;

namespace
{
    std::vector<std::uint8_t> ToBytes(const std::string &data)
    {
        return std::vector<std::uint8_t>(data.begin(), data.end());
    }
} // namespace

class CaptionServiceImpl final : public CaptionService::CallbackService
{
public:
    ServerUnaryReactor *AddCaption(
        CallbackServerContext *context,
        const AddCaptionRequest *request,
        AddCaptionResponse *response) override
    {
        ServerUnaryReactor *reactor = context->DefaultReactor();
        
        const std::vector<std::uint8_t> input_bytes = ToBytes(request->image());

        if (!ip::IsValidImage(input_bytes))
        {
            reactor->Finish(Status(StatusCode::INVALID_ARGUMENT, "Invalid image data"));
            return reactor;
        }

        const cv::Mat decoded = ip::Decompress(input_bytes);
        const cv::Mat captioned = ip::AddCaption(decoded, request->caption());
        const std::vector<std::uint8_t> encoded = ip::Compress(captioned);

        if (encoded.empty())
        {
            reactor->Finish(Status(StatusCode::INTERNAL, "Failed to encode result image"));
            return reactor;
        }
        response->set_image(encoded.data(), encoded.size());
        reactor->Finish(Status::OK);
        return reactor;
    }
};

int main()
{
    std::string address = "0.0.0.0:50051";
    CaptionServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(address, InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server = builder.BuildAndStart();
    std::cout << "Server ready: " << address << std::endl;

    server->Wait();
}