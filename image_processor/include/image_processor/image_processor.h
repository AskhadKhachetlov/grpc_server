#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace image_processor
{

    class ImageProcessor
    {
    public:
        static bool IsImage(const std::vector<std::uint8_t> &data);
        static bool IsImage(const std::string &path);

        static std::vector<std::uint8_t> Compress(const cv::Mat &image,
                                                  int quality = 90);

        static cv::Mat Decompress(const std::vector<std::uint8_t> &compressed);

        static cv::Mat AddCaption(const cv::Mat &image,
                                  const std::string &text,
                                  cv::Point origin = cv::Point(20, 40),
                                  double font_scale = 1.0,
                                  int thickness = 2,
                                  cv::Scalar color = cv::Scalar(0, 0, 255));
    };

} // namespace image_processor