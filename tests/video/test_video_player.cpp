#include "zh/video/player.h"
#include "zh/audio/manager.h"
#include "zh/data/vfs.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

class FakeDecoder final : public zh::video::VideoDecoder {
public:
    FakeDecoder(bool audio, std::vector<zh::video::VideoFrame> frames, bool throw_on_read = false)
        : frames_(std::move(frames)), throw_on_read_(throw_on_read)
    {
        metadata_.width = frames_.empty() ? 320 : frames_.front().width;
        metadata_.height = frames_.empty() ? 240 : frames_.front().height;
        metadata_.frame_rate = 10; metadata_.has_audio = audio;
        if (audio) { metadata_.audio_sample_rate = 48000; metadata_.audio_channels = 2; metadata_.audio_codec = "binkaudio_dct"; }
        metadata_.video_codec = "binkvideo";
    }
    const zh::video::VideoMetadata& metadata() const noexcept override { return metadata_; }
    const std::string& logical_path() const noexcept override { return path_; }
    bool read_frame(zh::video::VideoFrame& frame) override
    {
        if (throw_on_read_) throw zh::video::VideoError("video 'test.bik': corrupt packet");
        if (index_ >= frames_.size()) { eos_ = true; return false; }
        frame = frames_[index_++];
        if (metadata_.has_audio) {
            zh::video::VideoAudioChunk chunk; chunk.timestamp_seconds = frame.timestamp_seconds;
            chunk.interleaved.assign(4800 * 2, 0.1F); audio_.push_back(std::move(chunk));
        }
        return true;
    }
    std::vector<zh::video::VideoAudioChunk> drain_audio() override
    { auto result = std::move(audio_); audio_.clear(); return result; }
    bool end_of_stream() const noexcept override { return eos_; }
    void close() noexcept override { closed_ = true; }
private:
    zh::video::VideoMetadata metadata_;
    std::string path_ = "test.bik";
    std::vector<zh::video::VideoFrame> frames_;
    std::vector<zh::video::VideoAudioChunk> audio_;
    std::size_t index_ = 0;
    bool throw_on_read_ = false, eos_ = false, closed_ = false;
};

zh::video::VideoFrame frame(std::uint32_t width, std::uint32_t height, double pts)
{
    return {width, height, pts, std::vector<std::uint8_t>(static_cast<std::size_t>(width) * height * 4U, 127)};
}
}

int main()
{
    using namespace zh::video;
    const auto pillar = fit_aspect(96, 120, 800, 600);
    check(std::abs(pillar.width - 480.0F) < 0.01F && std::abs(pillar.x - 160.0F) < 0.01F, "pillarbox aspect preserved");
    const auto letter = fit_aspect(720, 486, 800, 600);
    check(std::abs(letter.height - 540.0F) < 0.01F && std::abs(letter.y - 30.0F) < 0.01F, "letterbox aspect preserved");
    try { (void)fit_aspect(0, 1, 1, 1); check(false, "zero aspect rejected"); } catch (const VideoError&) {}

    {
        zh::renderer::RecordingGpuDevice recorder;
        NullVideoAudioSink audio;
        auto decoder = std::make_unique<FakeDecoder>(false,
            std::vector<VideoFrame>{frame(320, 240, 0.0), frame(320, 240, 0.1)});
        VideoPlayer player(std::move(decoder), audio, recorder, 800, 600);
        player.advance(0.0); check(player.presented_frames() == 1, "first silent frame presented at zero");
        player.set_paused(true); player.advance(1.0); check(player.presented_frames() == 1, "pause freezes presentation");
        player.set_paused(false); player.set_focused(false); player.advance(1.0);
        check(player.presented_frames() == 1, "focus loss freezes presentation");
        player.set_focused(true); player.advance(0.1); player.advance(0.0);
        check(player.state() == PlaybackState::completed && player.presented_frames() == 2, "EOS completes and returns to game");
        check(recorder.resource_counts().total() == 0, "EOS releases recorder resources");
        const auto commands = recorder.snapshot();
        check(commands.find("video-texture-upload") != std::string::npos, "texture upload recorded");
        check(commands.find("draw pipeline=") != std::string::npos, "video draw recorded");
        check(commands.find("return-game-render") != std::string::npos, "game render return recorded");
    }

    {
        zh::renderer::RecordingGpuDevice recorder;
        NullVideoAudioSink audio(4);
        auto decoder = std::make_unique<FakeDecoder>(true,
            std::vector<VideoFrame>{frame(400, 324, 0.0), frame(400, 324, 0.1), frame(400, 324, 0.2)});
        VideoPlayer player(std::move(decoder), audio, recorder, 640, 480);
        player.advance(0.05); check(player.presented_frames() == 1, "audio clock presents due frame");
        player.advance(0.1); check(player.presented_frames() == 2, "audio clock advances frame order");
        player.skip(); check(player.state() == PlaybackState::skipped && recorder.resource_counts().total() == 0, "skip tears down safely");
    }

    {
        zh::renderer::RecordingGpuDevice recorder;
        NullVideoAudioSink audio;
        auto decoder = std::make_unique<FakeDecoder>(false, std::vector<VideoFrame>{}, true);
        VideoPlayer player(std::move(decoder), audio, recorder, 640, 480);
        player.advance(0.1);
        check(player.state() == PlaybackState::error && player.last_error().find("corrupt packet") != std::string::npos,
            "decoder corruption transitions to actionable error");
        player.shutdown(); player.shutdown();
        check(player.state() == PlaybackState::stopped && recorder.resource_counts().total() == 0, "shutdown is idempotent");
    }

    {
        NullVideoAudioSink sink(1);
        VideoAudioChunk chunk; chunk.interleaved.assign(4800 * 2, 0.0F);
        check(sink.submit(chunk) && !sink.submit(chunk), "audio sink rejects bounded overflow");
        sink.advance(0.1); check(sink.queued_chunks() == 0 && std::abs(sink.clock_seconds() - 0.1) < 0.0001, "null sink clock consumes PCM");
    }
    {
        namespace fs = std::filesystem;
        const auto root = fs::temp_directory_path() / ("zh-video-audio-bridge-" + std::to_string(::getpid()));
        std::error_code ignored; fs::remove_all(root, ignored);
        const auto zh_root = root / "zh"; const auto generals_root = root / "generals";
        fs::create_directories(zh_root); fs::create_directories(generals_root);
        const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
        zh::audio::AudioManager manager(vfs);
        AudioManagerVideoSink sink(manager);
        VideoAudioChunk chunk; chunk.interleaved.assign(480 * 2, 0.25F);
        check(sink.submit(std::move(chunk)), "decoded PCM enters existing audio adapter");
        std::vector<float> output(480 * 2); manager.render(output.data(), 480);
        check(std::abs(sink.clock_seconds() - 0.01) < 0.0001 && output.front() == 0.25F,
            "existing audio adapter mixes PCM and owns master clock");
        sink.reset(); manager.render(output.data(), 1);
        check(sink.clock_seconds() == 0.0, "video audio adapter reset clears clock");
        fs::remove_all(root, ignored);
    }
    return failures == 0 ? 0 : 1;
}
