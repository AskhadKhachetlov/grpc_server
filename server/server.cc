#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
#include <cstddef>
#include <exception>

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
    constexpr std::size_t kDefaultMaxConcurrentRequests = 10;

    std::vector<std::uint8_t> ToBytes(const std::string &data)
    {
        return std::vector<std::uint8_t>(data.begin(), data.end());
    }

    class ActiveRequestGuard
    {
    public:
        ActiveRequestGuard(std::atomic<std::size_t> &counter, std::size_t max_count)
            : counter_(counter)
        {
            const std::size_t previous_count =
                counter_.fetch_add(1, std::memory_order_acq_rel);
            acquired_ = previous_count < max_count;

            if (!acquired_)
            {
                counter_.fetch_sub(1, std::memory_order_acq_rel);
            }
        }

        ~ActiveRequestGuard()
        {
            if (acquired_)
            {
                counter_.fetch_sub(1, std::memory_order_acq_rel);
            }
        }

        bool Acquired() const
        {
            return acquired_;
        }

        ActiveRequestGuard(const ActiveRequestGuard &) = delete;
        ActiveRequestGuard &operator=(const ActiveRequestGuard &) = delete;

    private:
        std::atomic<std::size_t> &counter_;
        bool acquired_;
    };
} // namespace

class CaptionServiceImpl final : public CaptionService::CallbackService
{
public:
    static std::size_t max_concurrent_requests;

    ServerUnaryReactor *AddCaption(
        CallbackServerContext *context,
        const AddCaptionRequest *request,
        AddCaptionResponse *response) override
    {
        ServerUnaryReactor *reactor = context->DefaultReactor();
        const ActiveRequestGuard guard(active_requests_, max_concurrent_requests);

        if (!guard.Acquired())
        {
            reactor->Finish(Status(StatusCode::RESOURCE_EXHAUSTED,
                                   "Server is busy, try again later"));
            return reactor;
        }

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

private:
    std::atomic<std::size_t> active_requests_{0};
};

std::size_t CaptionServiceImpl::max_concurrent_requests = kDefaultMaxConcurrentRequests;

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        try
        {
            const long parsed = std::stol(argv[1]);
            if (parsed <= 0)
            {
                std::cerr << "max_concurrent_requests must be a positive integer, got: "
                          << argv[1] << std::endl;
                return 1;
            }
            CaptionServiceImpl::max_concurrent_requests = static_cast<std::size_t>(parsed);
        }
        catch (const std::exception &)
        {
            std::cerr << "Invalid max_concurrent_requests value: " << argv[1] << std::endl;
            return 1;
        }
    }

    std::string address = "0.0.0.0:50051";
    CaptionServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(address, InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server = builder.BuildAndStart();
    std::cout << "Server ready: " << address << " (max concurrent requests: "
              << CaptionServiceImpl::max_concurrent_requests << ")" << std::endl;

    server->Wait();
}