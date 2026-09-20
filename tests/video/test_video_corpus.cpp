#include "zh/video/decoder.h"
#include "zh/data/vfs.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Case {
    std::string variant;
    std::string logical;
    std::uint32_t width = 0, height = 0, audio_rate = 0;
    std::size_t minimum_frames = 0;
};

std::vector<Case> read_cases(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot read committed video variant manifest");
    std::vector<Case> cases;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.front() == '#') continue;
        std::istringstream stream(line);
        Case item; std::string width, height, rate, frames;
        if (!std::getline(stream, item.variant, '\t') || !std::getline(stream, item.logical, '\t')
            || !std::getline(stream, width, '\t') || !std::getline(stream, height, '\t')
            || !std::getline(stream, rate, '\t') || !std::getline(stream, frames, '\t'))
            throw std::runtime_error("invalid video variant manifest row");
        item.width = static_cast<std::uint32_t>(std::stoul(width));
        item.height = static_cast<std::uint32_t>(std::stoul(height));
        item.audio_rate = static_cast<std::uint32_t>(std::stoul(rate));
        item.minimum_frames = std::stoul(frames);
        cases.push_back(std::move(item));
    }
    return cases;
}
}

int main(int argc, char** argv)
{
    if (argc != 5) { std::cerr << "video corpus test requires two roots, locale, and manifest\n"; return 2; }
    try {
        const auto vfs = zh::data::VirtualFileSystem::mount({argv[1], argv[2], argv[3], {}});
        const auto cases = read_cases(argv[4]);
        if (cases.empty()) throw std::runtime_error("video variant manifest is empty");
        for (const auto& item : cases) {
            zh::video::FfmpegVideoDecoder decoder(vfs, item.logical);
            const auto& metadata = decoder.metadata();
            if (metadata.video_codec != "binkvideo" || metadata.width != item.width || metadata.height != item.height)
                throw std::runtime_error("variant " + item.variant + " has unexpected Bink video metadata");
            if ((item.audio_rate == 0 && metadata.has_audio)
                || (item.audio_rate != 0 && (!metadata.has_audio || metadata.audio_codec != "binkaudio_dct"
                    || metadata.audio_sample_rate != item.audio_rate || metadata.audio_channels != 2)))
                throw std::runtime_error("variant " + item.variant + " has unexpected Bink audio metadata");
            double previous = -1.0;
            for (std::size_t frame_index = 0; frame_index < item.minimum_frames; ++frame_index) {
                zh::video::VideoFrame frame;
                if (!decoder.read_frame(frame)) throw std::runtime_error("variant " + item.variant + " ended before bounded frame expectation");
                if (frame.width != item.width || frame.height != item.height || frame.rgba.empty()
                    || frame.timestamp_seconds <= previous)
                    throw std::runtime_error("variant " + item.variant + " produced invalid converted frame order");
                previous = frame.timestamp_seconds;
                (void)decoder.drain_audio();
            }
            decoder.close();
            std::cout << "video variant ok: " << item.variant << " " << item.width << "x" << item.height
                      << " audio=" << item.audio_rate << '\n';
        }
        std::cout << "video corpus verification ok: variants=" << cases.size() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "video corpus verification failed: " << error.what() << '\n';
        return 1;
    }
}
