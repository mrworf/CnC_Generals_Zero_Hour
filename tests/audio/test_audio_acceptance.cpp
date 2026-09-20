#include "zh/audio/manager.h"
#include "zh/data/vfs.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <new>
#include <string_view>
#include <thread>
#include <vector>
#include <unistd.h>

namespace {
std::atomic<bool> audit_allocations{false};
std::atomic<std::size_t> callback_allocations{0};
}

void* operator new(std::size_t size)
{
    if (audit_allocations.load(std::memory_order_relaxed)) callback_allocations.fetch_add(1, std::memory_order_relaxed);
    if (void* memory = std::malloc(size)) return memory;
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

#ifndef ZH_SOURCE_DIR
#error "ZH_SOURCE_DIR is required"
#endif

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
template <typename Function>
void expect_error(Function&& function, std::string_view path)
{
    try { function(); check(false, "expected audio data error"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(path) != std::string_view::npos, "error names logical audio path");
    }
}
bool audible(const std::vector<float>& samples)
{
    return std::any_of(samples.begin(), samples.end(), [](float sample) { return std::abs(sample) > 0.00001F; });
}
}

int main()
{
    namespace fs = std::filesystem;
    using namespace zh::audio;
    const fs::path fixtures = fs::path(ZH_SOURCE_DIR) / "tests/audio/fixtures";
    const auto empty = fs::temp_directory_path() / ("zh-audio-empty-" + std::to_string(::getpid()));
    std::error_code ignored; fs::remove_all(empty, ignored); fs::create_directories(empty);
    const auto vfs = zh::data::VirtualFileSystem::mount({fixtures, empty, "English", {}});

    struct Expected { const char* path; const char* encoding; };
    constexpr Expected expected[]{{"pcm.wav", "pcm-wav"}, {"ms-adpcm.wav", "microsoft-adpcm-wav"},
        {"ima-adpcm.wav", "ima-adpcm-wav"}, {"sample.mp3", "mp3"}};
    AudioManager manager(vfs, 8); manager.configure_output(false);
    check(manager.output_state() == AudioOutputState::null_sink, "missing device selects null sink");
    check(manager.warning_count() == 1 && manager.last_warning().find("unavailable") != std::string_view::npos,
        "missing device warns once");
    manager.notify_device_failure();
    check(manager.warning_count() == 1, "repeated device failure does not warn again");

    for (const auto& item : expected) {
        const auto metadata = probe_audio(vfs, item.path);
        check(metadata.encoding == item.encoding, "encoding detected");
        check(metadata.channels == 2 && metadata.sample_rate == 48000 && metadata.frame_count > 0,
            "decoder reports converted stream metadata");
        PlayRequest request; request.logical_path = item.path;
        const auto handle = manager.play(request);
        std::vector<float> output(1024 * 2); bool rendered = false; bool completed = false;
        for (int pass = 0; pass < 128 && !completed; ++pass) {
            const auto before = callback_allocations.load(std::memory_order_relaxed);
            audit_allocations.store(true, std::memory_order_release);
            manager.render(output.data(), 1024); rendered = rendered || audible(output);
            audit_allocations.store(false, std::memory_order_release);
            check(callback_allocations.load(std::memory_order_relaxed) == before,
                "real-time callback performs no C++ allocation");
            for (const auto& completion : manager.drain_completions()) completed = completed || completion.handle == handle;
        }
        check(rendered, "required encoding renders through null sink");
        check(completed, "required encoding completes");
    }

    const auto corrupt_root = fs::temp_directory_path() / ("zh-audio-corrupt-" + std::to_string(::getpid()));
    fs::remove_all(corrupt_root, ignored); fs::create_directories(corrupt_root);
    std::ofstream(corrupt_root / "broken.wav", std::ios::binary) << "RIFFbad-WAVE";
    const auto corrupt_vfs = zh::data::VirtualFileSystem::mount({corrupt_root, corrupt_root, "English", {}});
    expect_error([&] { probe_audio(corrupt_vfs, "broken.wav"); }, "broken.wav");
    expect_error([&] { PlayRequest request; request.logical_path = "missing.wav"; AudioManager other(corrupt_vfs); other.play(request); },
        "missing.wav");

    {
        AudioManager concurrent(vfs, 2); concurrent.configure_output(true);
        PlayRequest request; request.logical_path = "pcm.wav"; request.loop = true;
        const auto handle = concurrent.play(request);
        std::thread callback([&] {
            std::vector<float> output(64 * 2);
            while (!concurrent.is_stopped()) { concurrent.render(output.data(), 64); std::this_thread::yield(); }
        });
        for (int attempt = 0; attempt < 1000 && concurrent.active_voice_count() == 0; ++attempt) std::this_thread::yield();
        concurrent.notify_device_failure();
        check(concurrent.output_state() == AudioOutputState::null_sink && concurrent.warning_count() == 1,
            "runtime device failure degrades to null sink once");
        concurrent.shutdown(); concurrent.shutdown(); callback.join();
        const auto completions = concurrent.drain_completions();
        check(concurrent.is_stopped() && concurrent.output_state() == AudioOutputState::stopped,
            "shutdown reaches stopped state");
        check(std::any_of(completions.begin(), completions.end(), [&](const auto& completion) {
            return completion.handle == handle && completion.reason == CompletionReason::shutdown;
        }), "shutdown queues voice completion");
        expect_error([&] { concurrent.play(request); }, "shut down");
    }

    fs::remove_all(empty, ignored); fs::remove_all(corrupt_root, ignored);
    return failures == 0 ? 0 : 1;
}
