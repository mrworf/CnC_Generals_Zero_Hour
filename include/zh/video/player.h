#pragma once

#include "zh/video/decoder.h"
#include "zh/renderer/recording_device.h"

#include <cstddef>
#include <deque>
#include <memory>
#include <string>

namespace zh::audio { class AudioManager; }

namespace zh::video {

struct PresentationRect {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

PresentationRect fit_aspect(std::uint32_t source_width, std::uint32_t source_height,
    std::uint32_t target_width, std::uint32_t target_height);

class VideoAudioSink {
public:
    virtual ~VideoAudioSink() = default;
    virtual bool submit(VideoAudioChunk chunk) = 0;
    virtual void set_paused(bool paused) noexcept = 0;
    virtual void advance(double seconds) noexcept = 0;
    virtual double clock_seconds() const noexcept = 0;
    virtual void reset() noexcept = 0;
};

class NullVideoAudioSink final : public VideoAudioSink {
public:
    explicit NullVideoAudioSink(std::size_t capacity = 32);
    bool submit(VideoAudioChunk chunk) override;
    void set_paused(bool paused) noexcept override;
    void advance(double seconds) noexcept override;
    double clock_seconds() const noexcept override { return clock_; }
    void reset() noexcept override;
    std::size_t queued_chunks() const noexcept { return chunks_.size(); }

private:
    struct Pending { double seconds = 0.0; };
    std::size_t capacity_;
    std::deque<Pending> chunks_;
    double clock_ = 0.0;
    bool paused_ = false;
};

class AudioManagerVideoSink final : public VideoAudioSink {
public:
    explicit AudioManagerVideoSink(audio::AudioManager& manager) : manager_(manager) {}
    bool submit(VideoAudioChunk chunk) override;
    void set_paused(bool paused) noexcept override;
    void advance(double) noexcept override {}
    double clock_seconds() const noexcept override;
    void reset() noexcept override;
private:
    audio::AudioManager& manager_;
};

enum class PlaybackState { idle, playing, paused, completed, skipped, error, stopped };

class VideoPlayer {
public:
    VideoPlayer(std::unique_ptr<VideoDecoder> decoder, VideoAudioSink& audio,
        renderer::GpuDevice& recorder, std::uint32_t target_width, std::uint32_t target_height);
    ~VideoPlayer();
    VideoPlayer(const VideoPlayer&) = delete;
    VideoPlayer& operator=(const VideoPlayer&) = delete;

    PlaybackState state() const noexcept { return state_; }
    const std::string& last_error() const noexcept { return last_error_; }
    double media_clock_seconds() const noexcept { return media_clock_; }
    std::size_t presented_frames() const noexcept { return presented_frames_; }
    void advance(double seconds);
    void set_paused(bool paused) noexcept;
    void set_focused(bool focused) noexcept;
    void skip() noexcept;
    void shutdown() noexcept;

private:
    bool ensure_resources(const VideoFrame& frame);
    bool present(const VideoFrame& frame);
    bool fetch_pending();
    void fail(std::string message) noexcept;
    void finish(PlaybackState state, std::string_view marker) noexcept;
    void release_resources() noexcept;

    std::unique_ptr<VideoDecoder> decoder_;
    VideoAudioSink& audio_;
    renderer::GpuDevice& recorder_;
    std::uint32_t target_width_;
    std::uint32_t target_height_;
    PlaybackState state_ = PlaybackState::playing;
    bool user_paused_ = false;
    bool focused_ = true;
    bool have_pending_ = false;
    VideoFrame pending_;
    double media_clock_ = 0.0;
    std::size_t presented_frames_ = 0;
    std::string last_error_;
    renderer::TextureHandle movie_texture_;
    renderer::TextureHandle color_target_;
    renderer::TextureHandle depth_target_;
    renderer::SamplerHandle sampler_;
    renderer::BufferHandle vertices_;
    renderer::BufferHandle frame_uniform_;
    renderer::ShaderHandle vertex_shader_;
    renderer::ShaderHandle fragment_shader_;
    renderer::PipelineHandle pipeline_;
};

} // namespace zh::video
