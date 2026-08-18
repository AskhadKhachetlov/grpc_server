#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
#include <cstddef>
#include <exception>
#include <optional>
#include <cstdlib>
#include <thread>
#include <system_error>
#include <csignal>

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

    std::size_t ParseMaxConcurrentRequests(int argc, char **argv)
    {
        if (argc < 2)
        {
            return kDefaultMaxConcurrentRequests;
        }

        try
        {
            const long parsed = std::stol(argv[1]);
            if (parsed <= 0)
            {
                std::cerr << "max_concurrent_requests must be a positive integer, got: "
                          << argv[1] << std::endl;
                std::exit(1);
            }
            return static_cast<std::size_t>(parsed);
        }
        catch (const std::exception &)
        {
            std::cerr << "Invalid max_concurrent_requests value: " << argv[1] << std::endl;
            std::exit(1);
        }
    }

    class SessionManager : public std::enable_shared_from_this<SessionManager>
    {
    public:
        class Session
        {
        public:
            Session(Session &&other) noexcept : manager_(std::move(other.manager_))
            {
                other.manager_.reset();
            }

            Session &operator=(Session &&other) noexcept
            {
                if (this != &other)
                {
                    Release();
                    manager_ = std::move(other.manager_);
                    other.manager_.reset();
                }
                return *this;
            }

            ~Session()
            {
                Release();
            }

            Session(const Session &) = delete;
            Session &operator=(const Session &) = delete;

        private:
            friend class SessionManager;

            explicit Session(std::shared_ptr<SessionManager> manager)
                : manager_(std::move(manager)) {}

            void Release()
            {
                if (manager_ != nullptr)
                {
                    manager_->ReleaseSlot();
                    manager_.reset();
                }
            }

            std::shared_ptr<SessionManager> manager_;
        };

        explicit SessionManager(std::size_t max_sessions) : available_slots_(max_sessions) {}

        std::optional<Session> CreateNewSession()
        {
            std::size_t current = available_slots_.load(std::memory_order_acquire);

            while (current > 0)
            {
                if (available_slots_.compare_exchange_weak(
                        current, current - 1,
                        std::memory_order_acq_rel, std::memory_order_acquire))
                {
                    return Session(shared_from_this());
                }
            }

            return std::nullopt;
        }

        SessionManager(const SessionManager &) = delete;
        SessionManager &operator=(const SessionManager &) = delete;

    private:
        void ReleaseSlot()
        {
            available_slots_.fetch_add(1, std::memory_order_acq_rel);
        }

        std::atomic<std::size_t> available_slots_;
    };
} // namespace

class CaptionServiceImpl final : public CaptionService::CallbackService
{
public:
    explicit CaptionServiceImpl(std::size_t max_concurrent_requests)
        : session_manager_(std::make_shared<SessionManager>(max_concurrent_requests)) {}

    ServerUnaryReactor *AddCaption(
        CallbackServerContext *context,
        const AddCaptionRequest *request,
        AddCaptionResponse *response) override
    {
        ServerUnaryReactor *reactor = context->DefaultReactor();

        auto session = session_manager_->CreateNewSession();

        if (!session)
        {
            reactor->Finish(Status(StatusCode::RESOURCE_EXHAUSTED,
                                   "Server is busy, try again later"));
            return reactor;
        }

        std::string image_data = request->image();
        std::string caption = request->caption();

        auto worker = [reactor, response, session = std::move(*session),
             image_data = std::move(image_data),
             caption = std::move(caption)]()
        {
            try
            {
                const std::vector<std::uint8_t> input_bytes = ToBytes(image_data);

                if (!ip::IsValidImage(input_bytes))
                {
                    reactor->Finish(Status(StatusCode::INVALID_ARGUMENT, "Invalid image data"));
                    return;
                }

                const cv::Mat decoded = ip::Decompress(input_bytes);
                const cv::Mat captioned = ip::AddCaption(decoded, caption);
                const std::vector<std::uint8_t> encoded = ip::Compress(captioned);

                if (encoded.empty())
                {
                    reactor->Finish(Status(StatusCode::INTERNAL, "Failed to encode result image"));
                    return;
                }

                response->set_image(encoded.data(), encoded.size());
                reactor->Finish(Status::OK);
            }
            catch (const std::exception &e)
            {
                reactor->Finish(Status(StatusCode::INTERNAL,
                                       std::string("Image processing failed: ") + e.what()));
            }
            catch (...)
            {
                reactor->Finish(Status(StatusCode::INTERNAL, "Unknown image processing error"));
            }
        };

        try
        {
            std::thread(std::move(worker)).detach();
        }
        catch (const std::system_error &e)
        {
            reactor->Finish(Status(StatusCode::RESOURCE_EXHAUSTED,
                                   std::string("Failed to start worker thread: ") + e.what()));
        }

        return reactor;
    }

private:
    std::shared_ptr<SessionManager> session_manager_;
};

int main(int argc, char **argv)
{
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &signal_set, nullptr);

    const std::size_t max_concurrent_requests = ParseMaxConcurrentRequests(argc, argv);

    std::string address = "0.0.0.0:50051";
    CaptionServiceImpl service(max_concurrent_requests);
    ServerBuilder builder;
    builder.AddListeningPort(address, InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server = builder.BuildAndStart();

    std::thread shutdown_waiter(
        [&signal_set, &server]()
        {
            int recevied_signal = 0;
            sigwait(&signal_set, &recevied_signal);
            server->Shutdown();
        });

    std::cout << "Server ready: " << address
              << " (max concurrent requests: " << max_concurrent_requests << ")"
              << std::endl;

    server->Wait();
    shutdown_waiter.join();
}