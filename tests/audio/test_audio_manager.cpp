#include "zh/audio/manager.h"
#include "zh/data/vfs.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

void le16(std::vector<unsigned char>& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<unsigned char>(value)); bytes.push_back(static_cast<unsigned char>(value >> 8U));
}
void le32(std::vector<unsigned char>& bytes, std::uint32_t value)
{
    le16(bytes, static_cast<std::uint16_t>(value)); le16(bytes, static_cast<std::uint16_t>(value >> 16U));
}
void be32(std::vector<unsigned char>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<unsigned char>(value >> 24U)); bytes.push_back(static_cast<unsigned char>(value >> 16U));
    bytes.push_back(static_cast<unsigned char>(value >> 8U)); bytes.push_back(static_cast<unsigned char>(value));
}

std::vector<unsigned char> pcm_wav(std::size_t frames = 96)
{
    std::vector<unsigned char> bytes;
    const auto data_size = static_cast<std::uint32_t>(frames * 4);
    bytes.insert(bytes.end(), {'R','I','F','F'}); le32(bytes, 36 + data_size); bytes.insert(bytes.end(), {'W','A','V','E'});
    bytes.insert(bytes.end(), {'f','m','t',' '}); le32(bytes, 16); le16(bytes, 1); le16(bytes, 2);
    le32(bytes, 48000); le32(bytes, 48000 * 4); le16(bytes, 4); le16(bytes, 16);
    bytes.insert(bytes.end(), {'d','a','t','a'}); le32(bytes, data_size);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const auto sample = static_cast<std::int16_t>((frame % 16) * 1500 - 11000);
        le16(bytes, static_cast<std::uint16_t>(sample)); le16(bytes, static_cast<std::uint16_t>(-sample));
    }
    return bytes;
}

void write_bytes(const std::filesystem::path& path, const std::vector<unsigned char>& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary); output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void write_big(const std::filesystem::path& path, std::string_view logical, const std::vector<unsigned char>& payload)
{
    std::vector<unsigned char> bytes{'B','I','G','F'};
    const auto table_end = static_cast<std::uint32_t>(24 + logical.size() + 1);
    le32(bytes, table_end + static_cast<std::uint32_t>(payload.size()));
    be32(bytes, 1); be32(bytes, table_end); be32(bytes, table_end); be32(bytes, static_cast<std::uint32_t>(payload.size()));
    bytes.insert(bytes.end(), logical.begin(), logical.end()); bytes.push_back(0);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    write_bytes(path, bytes);
}

template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected audio error"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "actionable audio error");
    }
}

bool audible(const std::vector<float>& samples)
{
    return std::any_of(samples.begin(), samples.end(), [](float value) { return std::abs(value) > 0.0001F; });
}
}

int main()
{
    namespace fs = std::filesystem;
    using namespace zh::audio;
    const auto root = fs::temp_directory_path() / ("zh-audio-manager-" + std::to_string(::getpid()));
    std::error_code ignored; fs::remove_all(root, ignored);
    const auto zh = root / "zh"; const auto generals = root / "generals";
    write_bytes(zh / "Audio/effect.wav", pcm_wav());
    write_bytes(zh / "Audio/speech.wav", pcm_wav(32));
    write_bytes(zh / "Audio/music.wav", pcm_wav(256));
    write_big(zh / "audio.big", "Audio/archive.wav", pcm_wav());
    fs::create_directories(generals);
    const auto vfs = zh::data::VirtualFileSystem::mount({zh, generals, "English", {}});

    {
        AudioManager manager(vfs, 4);
        PlayRequest request; request.logical_path = "audio/effect.wav"; request.kind = AudioKind::effect;
        request.pan = -1.0F; request.spatial = true; request.position = {1, 0, 0}; request.minimum_distance = 0.5F;
        request.maximum_distance = 10.0F; request.occlusion = 0.25F; request.low_pass = 0.5F;
        manager.set_listener({0, 0, 0}); manager.set_group_volume(0, 0.75F);
        const auto handle = manager.play(request);
        std::vector<float> output(128 * 2); manager.render(output.data(), 128);
        check(audible(output), "2D/3D effect renders");
        check(manager.active_voice_count() == 0, "completed voice leaves callback slots");
        const auto completed = manager.drain_completions();
        check(completed.size() == 1 && completed[0].handle == handle && completed[0].reason == CompletionReason::finished,
            "completion is queued for game thread");

        request.logical_path = "audio/archive.wav"; request.kind = AudioKind::speech; request.streaming = true;
        request.spatial = false; request.pan = 0; request.occlusion = 0; request.low_pass = 1;
        manager.play(request); std::fill(output.begin(), output.end(), 0); manager.render(output.data(), 96);
        check(audible(output), "archive-backed speech streams without extraction"); manager.drain_completions();

        request.logical_path = "audio/music.wav"; request.kind = AudioKind::music; request.loop = true; request.delay_seconds = 0.002F;
        const auto music = manager.play(request); std::vector<float> delayed(64 * 2, 1.0F); manager.render(delayed.data(), 64);
        check(!audible(delayed), "delayed voice remains silent");
        manager.set_paused(true); manager.render(output.data(), 128); check(!audible(output), "global pause is silent");
        manager.set_paused(false); manager.set_focused(false); manager.render(output.data(), 128); check(!audible(output), "focus loss pauses");
        manager.set_focused(true); manager.render(output.data(), 128); check(audible(output), "focus recovery resumes");
        VoiceControl control; control.volume = 0.5F; control.pan = 1.0F; control.position = {2,0,0};
        control.occlusion = 0.5F; control.low_pass = 0.2F; manager.set_voice(music, control);
        manager.stop(music); manager.render(output.data(), 1);
        const auto stopped = manager.drain_completions();
        check(!stopped.empty() && stopped.back().reason == CompletionReason::stopped, "loop stop queues completion");

        expect_error([&] { PlayRequest bad; bad.logical_path = "audio/missing.wav"; manager.play(bad); }, "audio/missing.wav");
        expect_error([&] { auto bad = request; bad.logical_path = "audio/effect.wav"; bad.pitch = 0; manager.play(bad); }, "invalid");
    }

    {
        AudioManager manager(vfs, 1); std::vector<float> output(2);
        PlayRequest low; low.logical_path = "audio/music.wav"; low.loop = true; low.priority = 1;
        const auto low_handle = manager.play(low); manager.render(output.data(), 1);
        auto high = low; high.priority = 10; const auto high_handle = manager.play(high); manager.render(output.data(), 1);
        auto events = manager.drain_completions();
        check(events.size() == 1 && events[0].handle == low_handle && events[0].reason == CompletionReason::evicted,
            "higher priority evicts lowest voice");
        auto rejected = low; rejected.priority = 0; const auto rejected_handle = manager.play(rejected); manager.render(output.data(), 1);
        events = manager.drain_completions();
        check(events.size() == 1 && events[0].handle == rejected_handle && events[0].reason == CompletionReason::rejected,
            "lower priority admission rejected");
        manager.stop(high_handle); manager.render(output.data(), 1); manager.drain_completions();
    }

    fs::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
