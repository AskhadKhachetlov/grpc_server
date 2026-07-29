#include "image_processor/image_processor.h"

#include <algorithm>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace image_processor
{

    bool IsImage(const std::vector<std::uint8_t> &data)
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

    bool IsImage(const std::filesystem::path &path)
    {
        const cv::Mat image = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
        return !image.empty();
    }

    std::vector<std::uint8_t> Compress(const cv::Mat &image, int quality)
    {
        const int jpeg_quality = std::clamp(quality, 0, 100);
        const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpeg_quality};
        std::vector<std::uint8_t> encoded;

        if (!cv::imencode(".jpg", image, encoded, params))
        {
            return {};
        }

        return encoded;
    }

    cv::Mat Decompress(const std::vector<std::uint8_t> &compressed)
    {
        const cv::Mat buffer(1, static_cast<int>(compressed.size()), CV_8UC1,
                             const_cast<std::uint8_t *>(compressed.data()));

        return cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
    }

    cv::Mat AddCaption(const cv::Mat &image, const std::string &text,
                       const CaptionParams &params)
    {
        cv::Mat result = image.clone();

        cv::putText(result, text, params.origin, cv::FONT_HERSHEY_SIMPLEX,
                    params.font_scale, params.color, params.thickness, cv::LINE_AA);

        return result;
    }

} // namespace image_processor