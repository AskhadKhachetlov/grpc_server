#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <system_error>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "image_processor/image_processor.h"

using std::vector;
using std::uint8_t;
using std::error_code;

namespace ip = image_processor;
namespace fs = std::filesystem;

namespace
{

    cv::Mat MakeTestImage()
    {
        return cv::Mat(64, 64, CV_8UC3, cv::Scalar(10, 20, 30));
    }

    fs::path MakeTempJpegPath()
    {
        return fs::temp_directory_path() / "image_processor_test.jpg";
    }

    class TempFile
    {
    public:
        explicit TempFile(fs::path path) : path_(std::move(path)) {}

        ~TempFile()
        {
            error_code ec;
            fs::remove(path_, ec);
        }

        TempFile(const TempFile &) = delete;
        TempFile &operator=(const TempFile &) = delete;
        TempFile(TempFile &&) = delete;
        TempFile &operator=(TempFile &&) = delete;

        const fs::path &path() const
        {
            return path_;
        }

    private:
        fs::path path_;
    };

    TEST(ImageProcessorTest, CompressAndDecompressJpeg)
    {
        const cv::Mat original = MakeTestImage();
        const auto compressed = ip::Compress(original, 90);

        ASSERT_FALSE(compressed.empty());

        const cv::Mat restored = ip::Decompress(compressed);

        ASSERT_FALSE(restored.empty());
        EXPECT_EQ(restored.rows, original.rows);
        EXPECT_EQ(restored.cols, original.cols);
        EXPECT_EQ(restored.type(), original.type());
    }

    TEST(ImageProcessorTest, IsImageReturnsTrueForJpegBuffer)
    {
        const cv::Mat original = MakeTestImage();
        const auto compressed = ip::Compress(original, 90);

        ASSERT_FALSE(compressed.empty());
        EXPECT_TRUE(ip::IsImage(compressed));
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForEmptyBuffer)
    {
        const vector<uint8_t> empty_buffer;
        EXPECT_FALSE(ip::IsImage(empty_buffer));
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForInvalidBytes)
    {
        const vector<uint8_t> invalid_buffer = {
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
            0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB};

        EXPECT_FALSE(ip::IsImage(invalid_buffer));
    }

    TEST(ImageProcessorTest, IsImageReturnsTrueForJpegFilePath)
    {
        const fs::path temp_path = MakeTempJpegPath();
        TempFile cleanup(temp_path);

        const cv::Mat original = MakeTestImage();
        ASSERT_TRUE(cv::imwrite(temp_path.string(), original));

        EXPECT_TRUE(ip::IsImage(temp_path));
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForMissingFilePath)
    {
        EXPECT_FALSE(ip::IsImage(fs::path("this_file_does_not_exist.jpg")));
    }

    TEST(ImageProcessorTest, DecompressReturnsEmptyMatForInvalidBytes)
    {
        const vector<uint8_t> invalid_buffer = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
        EXPECT_TRUE(ip::Decompress(invalid_buffer).empty());
    }

    int CountDifferentPixels(const cv::Mat &image, const cv::Scalar &background)
    {
        const cv::Mat background_mat(image.size(), image.type(), background);

        cv::Mat diff;
        cv::absdiff(image, background_mat, diff);

        std::vector<cv::Mat> channels;
        cv::split(diff, channels);

        cv::Mat combined = channels[0];
        for (std::size_t i = 1; i < channels.size(); ++i)
        {
            cv::bitwise_or(combined, channels[i], combined);
        }

        return cv::countNonZero(combined);
    }

    TEST(ImageProcessorTest, AddCaptionReturnsModifiedCopy)
    {
        const cv::Scalar background(10, 20, 30);
        const cv::Mat original = MakeTestImage();
        const cv::Mat captioned = ip::AddCaption(original, "test");

        ASSERT_FALSE(captioned.empty());
        EXPECT_EQ(captioned.rows, original.rows);
        EXPECT_EQ(captioned.cols, original.cols);
        EXPECT_EQ(captioned.type(), original.type());
        EXPECT_EQ(CountDifferentPixels(original, background), 0);
        EXPECT_GT(CountDifferentPixels(captioned, background), 0);
    }

    TEST(ImageProcessorTest, AddCaptionRespectsCustomParams)
    {
        const cv::Scalar background(10, 20, 30);
        const cv::Mat original = MakeTestImage();

        ip::CaptionParams params;
        params.origin = cv::Point(5, 30);
        params.font_scale = 0.5;
        params.thickness = 1;
        params.color = cv::Scalar(255, 0, 0);

        const cv::Mat captioned = ip::AddCaption(original, "x", params);

        ASSERT_FALSE(captioned.empty());
        EXPECT_GT(CountDifferentPixels(captioned, background), 0);
    }

} // namespace