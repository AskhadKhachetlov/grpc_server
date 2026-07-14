#include "image_processor/image_processor.h"

#include <algorithm>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace image_processor
{

    bool ImageProcessor::IsImage(const std::vector<std::uint8_t> &data)
    {
        if (data.empty())
        {
            return false;
        }

        const cv::Mat buffer(1, static_cast<int>(data.size()), CV_8UC1,
                             const_cast<std::uint8_t *>(data.data()));

        const cv::Mat decoded = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);

        return !decoded.empty();
    }

    bool ImageProcessor::IsImage(const std::string &path)
    {
        const cv::Mat image = cv::imread(path, cv::IMREAD_UNCHANGED);
        return !image.empty();
    }

    std::vector<std::uint8_t> ImageProcessor::Compress(const cv::Mat &image, int quality)
    {
        if (image.empty())
        {
            throw std::invalid_argument("Image is empty");
        }

        const int jpeg_quality = std::clamp(quality, 0, 100);
        const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpeg_quality};
        std::vector<std::uint8_t> encoded;

        if (!cv::imencode(".jpg", image, encoded, params))
        {
            throw std::runtime_error("Failed to compress image");
        }

        return encoded;
    }

    cv::Mat ImageProcessor::Decompress(const std::vector<std::uint8_t> &compressed)
    {
        if (compressed.empty())
        {
            throw std::invalid_argument("Compressed buffer is empty");
        }

        const cv::Mat buffer(1, static_cast<int>(compressed.size()), CV_8UC1,
                             const_cast<std::uint8_t *>(compressed.data()));

        const cv::Mat decoded = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);

        if (decoded.empty())
        {
            throw std::runtime_error("Failed to decompress image");
        }

        return decoded;
    }

    cv::Mat ImageProcessor::AddCaption(const cv::Mat &image, const std::string &text,
                                       cv::Point origin, double font_scale,
                                       int thickness, cv::Scalar color)
    {
        if (image.empty())
        {
            throw std::invalid_argument("Image is empty");
        }

        cv::Mat result = image.clone();

        cv::putText(result, text, origin, cv::FONT_HERSHEY_SIMPLEX, font_scale,
                    color, thickness, cv::LINE_AA);

        return result;
    }

} // namespace image_processor