#include "zh/audio/manager.h"

#include "zh/audio/vfs.h"
#include "zh/data/vfs.h"
#include "miniaudio.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <limits>
#include <memory>

namespace zh::audio {
namespace {

constexpr std::size_t command_capacity = 128;
constexpr std::size_t completion_capacity = 256;
constexpr std::size_t render_chunk_frames = 256;
constexpr std::size_t maximum_groups = 32;

template <typename T, std::size_t Capacity>
class SpscQueue {
public:
    bool push(const T& value) noexcept
    {
        const auto write = write_.load(std::memory_order_relaxed);
        const auto next = (write + 1) % Capacity;
        if (next == read_.load(std::memory_order_acquire)) return false;
        values_[write] = value;
        write_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& value) noexcept
    {
        const auto read = read_.load(std::memory_order_relaxed);
        if (read == write_.load(std::memory_order_acquire)) return false;
        value = values_[read];
        read_.store((read + 1) % Capacity, std::memory_order_release);
        return true;
    }

private:
    std::array<T, Capacity> values_{};
    std::atomic<std::size_t> read_{0};
    std::atomic<std::size_t> write_{0};
};

float clamp_unit(float value) noexcept { return std::clamp(value, 0.0F, 1.0F); }

struct Voice {
    AudioHandle handle = 0;
    PlayRequest request;
    VoiceControl control;
    ma_decoder decoder{};
    bool decoder_ready = false;
    std::uint64_t delay_frames = 0;
    float filtered_left = 0.0F;
    float filtered_right = 0.0F;
};

enum class CommandType { play, stop, control, group, listener, paused, focused, shutdown };

struct Command {
    CommandType type = CommandType::play;
    Voice* voice = nullptr;
    AudioHandle handle = 0;
    VoiceControl control{};
    std::uint32_t group = 0;
    float value = 0.0F;
    Vec3 position{};
    bool flag = false;
};

struct Retired {
    Voice* voice = nullptr;
    Completion completion{};
};

float spatial_gain(const Voice& voice, Vec3 listener) noexcept
{
    if (!voice.request.spatial) return 1.0F;
    const auto dx = voice.control.position.x - listener.x;
    const auto dy = voice.control.position.y - listener.y;
    const auto dz = voice.control.position.z - listener.z;
    const auto distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (distance <= voice.request.minimum_distance) return 1.0F;
    if (distance >= voice.request.maximum_distance) return 0.0F;
    const auto range = voice.request.maximum_distance - voice.request.minimum_distance;
    const auto normalized = (distance - voice.request.minimum_distance) / range;
    return std::pow(1.0F - normalized, voice.request.attenuation_rolloff);
}

} // namespace

struct AudioManager::Impl {
    explicit Impl(const data::VirtualFileSystem& source, std::size_t maximum)
        : vfs(source), slots(std::max<std::size_t>(1, maximum), nullptr)
    {
        group_volumes.fill(1.0F);
    }

    ~Impl()
    {
        for (auto* voice : slots) destroy(voice);
        Command command;
        while (commands.pop(command)) if (command.type == CommandType::play) destroy(command.voice);
        Retired item;
        while (retired.pop(item)) destroy(item.voice);
    }

    static void destroy(Voice* voice) noexcept
    {
        if (voice == nullptr) return;
        if (voice->decoder_ready) ma_decoder_uninit(&voice->decoder);
        delete voice;
    }

    void queue(const Command& command)
    {
        if (stopping.load(std::memory_order_acquire)) throw AudioError("audio manager is shut down");
        if (!commands.push(command)) throw AudioError("audio command queue is full");
    }

    bool complete(Voice*& slot, CompletionReason reason) noexcept
    {
        if (slot == nullptr) return true;
        if (!retired.push({slot, {slot->handle, reason}})) return false;
        slot = nullptr;
        active.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }

    void apply_commands() noexcept
    {
        Command command;
        while (commands.pop(command)) {
            switch (command.type) {
            case CommandType::play: {
                auto empty = slots.end();
                auto lowest = slots.end();
                for (auto it = slots.begin(); it != slots.end(); ++it) {
                    if (*it == nullptr) { empty = it; break; }
                    if (lowest == slots.end() || (*it)->request.priority < (*lowest)->request.priority) lowest = it;
                }
                if (empty == slots.end() && lowest != slots.end() && command.voice->request.priority > (*lowest)->request.priority) {
                    if (complete(*lowest, CompletionReason::evicted)) empty = lowest;
                }
                if (empty == slots.end()) {
                    retired.push({command.voice, {command.voice->handle, CompletionReason::rejected}});
                } else {
                    *empty = command.voice;
                    active.fetch_add(1, std::memory_order_relaxed);
                }
                break;
            }
            case CommandType::stop:
                for (auto& slot : slots) if (slot != nullptr && slot->handle == command.handle) complete(slot, CompletionReason::stopped);
                break;
            case CommandType::control:
                for (auto* slot : slots) if (slot != nullptr && slot->handle == command.handle) slot->control = command.control;
                break;
            case CommandType::group:
                if (command.group < group_volumes.size()) group_volumes[command.group] = clamp_unit(command.value);
                break;
            case CommandType::listener: listener = command.position; break;
            case CommandType::paused: paused = command.flag; break;
            case CommandType::focused: focused = command.flag; break;
            case CommandType::shutdown:
                for (auto& slot : slots) complete(slot, CompletionReason::shutdown);
                stopped.store(true, std::memory_order_release);
                break;
            }
        }
    }

    MiniaudioVfs vfs;
    std::vector<Voice*> slots;
    std::array<float, maximum_groups> group_volumes{};
    SpscQueue<Command, command_capacity> commands;
    SpscQueue<Retired, completion_capacity> retired;
    std::atomic<AudioHandle> next_handle{1};
    std::atomic<std::size_t> active{0};
    std::atomic<bool> stopping{false};
    std::atomic<bool> stopped{false};
    Vec3 listener{};
    bool paused = false;
    bool focused = true;
};

AudioManager::AudioManager(const data::VirtualFileSystem& vfs, std::size_t maximum_voices)
    : impl_(std::make_unique<Impl>(vfs, maximum_voices)) {}
AudioManager::~AudioManager() = default;

AudioHandle AudioManager::play(const PlayRequest& request)
{
    if (request.logical_path.empty()) throw AudioError("audio logical path is empty");
    if (!(request.volume >= 0.0F) || request.pan < -1.0F || request.pan > 1.0F ||
        request.pitch < 0.25F || request.pitch > 4.0F || request.minimum_distance < 0.0F ||
        request.maximum_distance <= request.minimum_distance || request.attenuation_rolloff <= 0.0F ||
        request.delay_seconds < 0.0F || request.occlusion < 0.0F || request.occlusion > 1.0F ||
        request.low_pass < 0.0F || request.low_pass > 1.0F || request.group >= maximum_groups) {
        throw AudioError("invalid audio playback parameters for logical asset '" + request.logical_path + "'");
    }
    if (impl_->vfs.native_handle() == nullptr) throw AudioError("audio VFS is unavailable");

    auto voice = std::make_unique<Voice>();
    voice->handle = impl_->next_handle.fetch_add(1, std::memory_order_relaxed);
    voice->request = request;
    voice->control = {request.volume, request.pan, request.position, request.occlusion, request.low_pass, false};
    voice->delay_frames = static_cast<std::uint64_t>(request.delay_seconds * 48000.0F);
    auto decoder_config = ma_decoder_config_init(ma_format_f32, 2, static_cast<ma_uint32>(48000.0F / request.pitch));
    const auto result = ma_decoder_init_vfs(static_cast<ma_vfs*>(impl_->vfs.native_handle()),
        request.logical_path.c_str(), &decoder_config, &voice->decoder);
    if (result != MA_SUCCESS) {
        throw AudioError("cannot decode audio logical asset '" + request.logical_path + "': miniaudio result " + std::to_string(result));
    }
    voice->decoder_ready = true;
    ma_data_source_set_looping(&voice->decoder, request.loop ? MA_TRUE : MA_FALSE);

    const auto handle = voice->handle;
    try {
        impl_->queue({CommandType::play, voice.get()});
    } catch (...) {
        Impl::destroy(voice.release());
        throw;
    }
    voice.release();
    return handle;
}

void AudioManager::stop(AudioHandle handle) { impl_->queue({CommandType::stop, nullptr, handle}); }

void AudioManager::set_voice(AudioHandle handle, const VoiceControl& control)
{
    if (control.volume < 0.0F || control.pan < -1.0F || control.pan > 1.0F || control.occlusion < 0.0F ||
        control.occlusion > 1.0F || control.low_pass < 0.0F || control.low_pass > 1.0F) {
        throw AudioError("invalid audio voice control");
    }
    Command command{CommandType::control}; command.handle = handle; command.control = control; impl_->queue(command);
}

void AudioManager::set_group_volume(std::uint32_t group, float volume)
{
    if (group >= maximum_groups || volume < 0.0F) throw AudioError("invalid audio group control");
    Command command{CommandType::group}; command.group = group; command.value = volume; impl_->queue(command);
}

void AudioManager::set_listener(Vec3 position)
{
    Command command{CommandType::listener}; command.position = position; impl_->queue(command);
}

void AudioManager::set_paused(bool paused)
{
    Command command{CommandType::paused}; command.flag = paused; impl_->queue(command);
}

void AudioManager::set_focused(bool focused)
{
    Command command{CommandType::focused}; command.flag = focused; impl_->queue(command);
}

void AudioManager::render(float* output, std::size_t frame_count) noexcept
{
    if (output == nullptr) return;
    std::fill(output, output + frame_count * 2, 0.0F);
    impl_->apply_commands();
    if (impl_->paused || !impl_->focused || impl_->stopped.load(std::memory_order_acquire)) return;

    std::array<float, render_chunk_frames * 2> scratch{};
    for (auto& slot : impl_->slots) {
        if (slot == nullptr || slot->control.paused) continue;
        std::size_t output_offset = 0;
        if (slot->delay_frames != 0) {
            const auto skipped = std::min<std::uint64_t>(slot->delay_frames, frame_count);
            slot->delay_frames -= skipped;
            output_offset = static_cast<std::size_t>(skipped);
        }
        while (slot != nullptr && output_offset < frame_count) {
            const auto requested = std::min(render_chunk_frames, frame_count - output_offset);
            ma_uint64 decoded = 0;
            const auto result = ma_decoder_read_pcm_frames(&slot->decoder, scratch.data(), requested, &decoded);
            if (decoded == 0) {
                if (!impl_->complete(slot, CompletionReason::finished)) return;
                break;
            }
            const auto group = impl_->group_volumes[slot->request.group];
            const auto distance = spatial_gain(*slot, impl_->listener);
            const auto occlusion = 1.0F - 0.75F * clamp_unit(slot->control.occlusion);
            const auto gain = slot->control.volume * group * distance * occlusion;
            const auto left_gain = gain * (slot->control.pan <= 0.0F ? 1.0F : 1.0F - slot->control.pan);
            const auto right_gain = gain * (slot->control.pan >= 0.0F ? 1.0F : 1.0F + slot->control.pan);
            const auto filter = std::max(0.01F, clamp_unit(slot->control.low_pass));
            for (std::size_t frame = 0; frame < decoded; ++frame) {
                slot->filtered_left += filter * (scratch[frame * 2] - slot->filtered_left);
                slot->filtered_right += filter * (scratch[frame * 2 + 1] - slot->filtered_right);
                output[(output_offset + frame) * 2] += slot->filtered_left * left_gain;
                output[(output_offset + frame) * 2 + 1] += slot->filtered_right * right_gain;
            }
            output_offset += static_cast<std::size_t>(decoded);
            if (result != MA_SUCCESS && result != MA_AT_END) {
                impl_->complete(slot, CompletionReason::finished);
                break;
            }
        }
    }
}

std::vector<Completion> AudioManager::drain_completions()
{
    std::vector<Completion> result;
    Retired item;
    while (impl_->retired.pop(item)) {
        result.push_back(item.completion);
        Impl::destroy(item.voice);
    }
    return result;
}

void AudioManager::shutdown() noexcept
{
    if (impl_->stopping.exchange(true, std::memory_order_acq_rel)) return;
    if (!impl_->commands.push({CommandType::shutdown})) impl_->stopped.store(true, std::memory_order_release);
}

std::size_t AudioManager::active_voice_count() const noexcept { return impl_->active.load(std::memory_order_relaxed); }

std::string_view completion_reason_name(CompletionReason reason) noexcept
{
    switch (reason) {
    case CompletionReason::finished: return "finished";
    case CompletionReason::stopped: return "stopped";
    case CompletionReason::evicted: return "evicted";
    case CompletionReason::rejected: return "rejected";
    case CompletionReason::shutdown: return "shutdown";
    }
    return "unknown";
}

} // namespace zh::audio
