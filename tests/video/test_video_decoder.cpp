#include "zh/video/decoder.h"
#include "zh/data/vfs.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected video error"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "actionable video error");
    }
}
}

int main()
{
    using namespace zh::video;
    BoundedMediaQueue<int> queue(2);
    check(queue.push(1) && queue.push(2) && !queue.push(3), "bounded queue rejects overflow");
    int value = 0;
    check(queue.pop(value) && value == 1 && queue.pop(value) && value == 2 && !queue.pop(value), "bounded queue preserves FIFO");
    expect_error([] { BoundedMediaQueue<int> invalid(0); }, "capacity");

    VideoLimits limits;
    validate_video_dimensions(800, 600, limits);
    expect_error([&] { validate_video_dimensions(0, 600, limits); }, "positive");
    limits.maximum_width = 640;
    expect_error([&] { validate_video_dimensions(800, 600, limits); }, "dimensions");
    limits = {};
    limits.maximum_frame_bytes = 100;
    expect_error([&] { validate_video_dimensions(8, 8, limits); }, "RGBA");

    check(normalize_media_timestamp(0.25, -1.0, 0.1) == 0.25, "valid first timestamp retained");
    check(std::abs(normalize_media_timestamp(0.1, 0.25, 0.05) - 0.3) < 0.0001, "regressing timestamp repaired");
    check(normalize_media_timestamp(NAN, -1.0, 0.05) == 0.0, "missing first timestamp starts at zero");

    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / ("zh-video-decoder-" + std::to_string(::getpid()));
    std::error_code ignored; fs::remove_all(root, ignored);
    const auto zh_root = root / "zh"; const auto generals_root = root / "generals";
    fs::create_directories(zh_root / "Data/Movies"); fs::create_directories(generals_root);
    { std::ofstream output(zh_root / "Data/Movies/broken.bik", std::ios::binary); output << "BIK"; }
    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    expect_error([&] { FfmpegVideoDecoder decoder(vfs, "Data/Movies/missing.bik"); }, "missing.bik");
    expect_error([&] { FfmpegVideoDecoder decoder(vfs, "Data/Movies/broken.bik"); }, "cannot open Bink stream");
    expect_error([&] { FfmpegVideoDecoder decoder(vfs, "Data/Movies/broken.bik", {}, false); }, "decoder is unavailable");
    VideoLimits tiny; tiny.maximum_source_bytes = 2;
    expect_error([&] { FfmpegVideoDecoder decoder(vfs, "Data/Movies/broken.bik", tiny); }, "source-byte limit");
    fs::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
