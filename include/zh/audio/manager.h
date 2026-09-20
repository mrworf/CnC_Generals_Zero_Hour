#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::data { class VirtualFileSystem; }

namespace zh::audio {

using AudioHandle = std::uint32_t;

struct Vec3 {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

enum class AudioKind { effect, speech, music };
enum class CompletionReason { finished, stopped, evicted, rejected, shutdown };

struct PlayRequest {
    std::string logical_path;
    AudioKind kind = AudioKind::effect;
    std::uint32_t group = 0;
    std::int32_t priority = 0;
    bool spatial = false;
    bool streaming = false;
    bool loop = false;
    float volume = 1.0F;
    float pan = 0.0F;
    float pitch = 1.0F;
    Vec3 position{};
    float minimum_distance = 1.0F;
    float maximum_distance = 100.0F;
    float attenuation_rolloff = 1.0F;
    float delay_seconds = 0.0F;
    float occlusion = 0.0F;
    float low_pass = 1.0F;
};

struct VoiceControl {
    float volume = 1.0F;
    float pan = 0.0F;
    Vec3 position{};
    float occlusion = 0.0F;
    float low_pass = 1.0F;
    bool paused = false;
};

struct Completion {
    AudioHandle handle = 0;
    CompletionReason reason = CompletionReason::finished;
};

class AudioError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class AudioManager {
public:
    explicit AudioManager(const data::VirtualFileSystem& vfs, std::size_t maximum_voices = 32);
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    AudioHandle play(const PlayRequest& request);
    void stop(AudioHandle handle);
    void set_voice(AudioHandle handle, const VoiceControl& control);
    void set_group_volume(std::uint32_t group, float volume);
    void set_listener(Vec3 position);
    void set_paused(bool paused);
    void set_focused(bool focused);

    // Device/null-sink callback entry point. It performs no engine allocation,
    // simulation locking, or game-object dispatch.
    void render(float* interleaved_stereo, std::size_t frame_count) noexcept;

    // Game-thread boundary. Decoder destruction and completion dispatch occur here.
    std::vector<Completion> drain_completions();
    void shutdown() noexcept;

    std::size_t active_voice_count() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::string_view completion_reason_name(CompletionReason reason) noexcept;

} // namespace zh::audio
