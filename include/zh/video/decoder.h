#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::data { class VirtualFileSystem; }

namespace zh::video {

struct VideoLimits {
    std::uint64_t maximum_source_bytes = 512ULL * 1024ULL * 1024ULL;
    std::size_t maximum_packet_bytes = 32U * 1024U * 1024U;
    std::uint32_t maximum_width = 4096;
    std::uint32_t maximum_height = 4096;
    std::uint64_t maximum_pixels = 16ULL * 1024ULL * 1024ULL;
    std::size_t maximum_frame_bytes = 64U * 1024U * 1024U;
    std::size_t maximum_audio_samples_per_chunk = 262144;
    std::size_t maximum_audio_chunks = 32;
};

struct VideoMetadata {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    double frame_rate = 0.0;
    bool has_audio = false;
    std::uint32_t audio_sample_rate = 0;
    std::uint32_t audio_channels = 0;
    std::string video_codec;
    std::string audio_codec;
};

struct VideoFrame {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    double timestamp_seconds = 0.0;
    std::vector<std::uint8_t> rgba;
};

struct VideoAudioChunk {
    double timestamp_seconds = 0.0;
    std::uint32_t sample_rate = 48000;
    std::uint32_t channels = 2;
    std::vector<float> interleaved;
};

class VideoError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename T>
class BoundedMediaQueue {
public:
    explicit BoundedMediaQueue(std::size_t capacity) : capacity_(capacity)
    {
        if (capacity == 0) throw VideoError("video queue capacity must be positive");
    }

    bool push(T value)
    {
        if (values_.size() >= capacity_) return false;
        values_.push_back(std::move(value));
        return true;
    }

    bool pop(T& value)
    {
        if (values_.empty()) return false;
        value = std::move(values_.front());
        values_.pop_front();
        return true;
    }

    std::size_t size() const noexcept { return values_.size(); }
    bool empty() const noexcept { return values_.empty(); }
    void clear() noexcept { values_.clear(); }

private:
    std::size_t capacity_;
    std::deque<T> values_;
};

void validate_video_dimensions(std::uint32_t width, std::uint32_t height, const VideoLimits& limits);
double normalize_media_timestamp(double candidate, double previous, double step) noexcept;

class FfmpegVideoDecoder {
public:
    FfmpegVideoDecoder(const data::VirtualFileSystem& vfs, std::string logical_path,
        VideoLimits limits = {}, bool decoder_available = true);
    ~FfmpegVideoDecoder();
    FfmpegVideoDecoder(FfmpegVideoDecoder&&) noexcept;
    FfmpegVideoDecoder& operator=(FfmpegVideoDecoder&&) noexcept;
    FfmpegVideoDecoder(const FfmpegVideoDecoder&) = delete;
    FfmpegVideoDecoder& operator=(const FfmpegVideoDecoder&) = delete;

    const VideoMetadata& metadata() const noexcept;
    const std::string& logical_path() const noexcept;
    bool read_frame(VideoFrame& frame);
    std::vector<VideoAudioChunk> drain_audio();
    bool end_of_stream() const noexcept;
    void close() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::video
