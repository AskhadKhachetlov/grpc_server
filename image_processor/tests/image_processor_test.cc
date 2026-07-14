#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "image_processor/image_processor.h"

using std::vector;
using std::uint8_t;
using std::invalid_argument;
using std::runtime_error;
using std::error_code;
using image_processor::ImageProcessor;

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
        const auto compressed = ImageProcessor::Compress(original, 90);

        ASSERT_FALSE(compressed.empty());

        const cv::Mat restored = ImageProcessor::Decompress(compressed);

        ASSERT_FALSE(restored.empty());
        EXPECT_EQ(restored.rows, original.rows);
        EXPECT_EQ(restored.cols, original.cols);
        EXPECT_EQ(restored.type(), original.type());
    }

    TEST(ImageProcessorTest, CompressThrowsOnEmptyImage)
    {
        const cv::Mat empty_image;
        EXPECT_THROW(ImageProcessor::Compress(empty_image, 90), invalid_argument);
    }

    TEST(ImageProcessorTest, IsImageReturnsTrueForJpegBuffer)
    {
        const cv::Mat original = MakeTestImage();
        const auto compressed = ImageProcessor::Compress(original, 90);

        ASSERT_FALSE(compressed.empty());
        EXPECT_TRUE(ImageProcessor::IsImage(compressed));
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForEmptyBuffer)
    {
        const vector<uint8_t> empty_buffer;
        EXPECT_FALSE(ImageProcessor::IsImage(empty_buffer));
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForInvalidBytes)
    {
        const vector<uint8_t> invalid_buffer = {
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
            0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB};

        EXPECT_FALSE(ImageProcessor::IsImage(invalid_buffer));
    }

    TEST(ImageProcessorTest, IsImageReturnsTrueForJpegFilePath)
    {
        const fs::path temp_path = MakeTempJpegPath();
        TempFile cleanup(temp_path);

        const cv::Mat original = MakeTestImage();
        ASSERT_TRUE(cv::imwrite(temp_path.string(), original));

        const bool result = ImageProcessor::IsImage(temp_path.string());

        EXPECT_TRUE(result);
    }

    TEST(ImageProcessorTest, IsImageReturnsFalseForMissingFilePath)
    {
        EXPECT_FALSE(ImageProcessor::IsImage("this_file_does_not_exist.jpg"));
    }

    TEST(ImageProcessorTest, DecompressThrowsOnEmptyBuffer)
    {
        const vector<uint8_t> empty_buffer;
        EXPECT_THROW(ImageProcessor::Decompress(empty_buffer), invalid_argument);
    }

    TEST(ImageProcessorTest, DecompressThrowsOnInvalidBytes)
    {
        const vector<uint8_t> invalid_buffer = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
        EXPECT_THROW(ImageProcessor::Decompress(invalid_buffer), runtime_error);
    }

} // namespace