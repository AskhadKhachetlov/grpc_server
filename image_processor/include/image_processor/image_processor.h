#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/core.hpp>

namespace image_processor
{

    struct CaptionParams
    {
        cv::Point origin = cv::Point(20, 40);
        double font_scale = 1.0;
        int thickness = 2;
        cv::Scalar color = cv::Scalar(0, 0, 255);
    };

    bool IsImage(const std::vector<std::uint8_t> &data);
    bool IsImage(const std::filesystem::path &path);

    // Сжимает изображение в JPEG. Требует непустого изображения на входе.
    // Возвращает пустой вектор, если кодирование не удалось.
    std::vector<std::uint8_t> Compress(const cv::Mat &image, int quality = 90);

    // Декодирует изображение из буфера. Требует непустого буфера на входе.
    // Возвращает пустой cv::Mat, если декодирование не удалось.
    cv::Mat Decompress(const std::vector<std::uint8_t> &compressed);

    // Возвращает копию изображения с наложенной подписью.
    // Требует непустого изображения на входе.
    cv::Mat AddCaption(const cv::Mat &image,
                       const std::string &text,
                       const CaptionParams &params = CaptionParams());

} // namespace image_processor