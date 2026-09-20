#include "zh/data/vfs.h"
#include "zh/original_resources.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void le16(std::vector<std::uint8_t>& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value)); bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}
void le32(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
    le16(bytes, static_cast<std::uint16_t>(value)); le16(bytes, static_cast<std::uint16_t>(value >> 16U));
}
std::vector<std::uint8_t> pcm_wav(std::size_t frames = 96)
{
    std::vector<std::uint8_t> bytes;
    const auto size = static_cast<std::uint32_t>(frames * 4);
    bytes.insert(bytes.end(), {'R','I','F','F'}); le32(bytes, 36 + size); bytes.insert(bytes.end(), {'W','A','V','E'});
    bytes.insert(bytes.end(), {'f','m','t',' '}); le32(bytes, 16); le16(bytes, 1); le16(bytes, 2);
    le32(bytes, 48000); le32(bytes, 192000); le16(bytes, 4); le16(bytes, 16);
    bytes.insert(bytes.end(), {'d','a','t','a'}); le32(bytes, size);
    for (std::size_t frame = 0; frame < frames; ++frame) { le16(bytes, 500); le16(bytes, 500); }
    return bytes;
}
void write_bytes(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary).write(reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
}
void write_text(const std::filesystem::path& path, std::string_view text)
{
    write_bytes(path, {text.begin(), text.end()});
}
bool zero(const zh::original_resources::OwnershipCounts& counts)
{
    return counts.audio_definitions == 0 && counts.workers == 0 && counts.device_acquisitions == 0;
}
}

int main()
{
    using namespace zh::original_resources;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-original-resources-audio-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh_root = root / "zh";
    const auto generals_root = root / "generals";
    std::filesystem::create_directories(generals_root);
    write_bytes(zh_root / "Audio/theme.wav", pcm_wav());
    write_bytes(zh_root / "Audio/click.wav", pcm_wav(32));
    write_bytes(zh_root / "Audio/line.wav", pcm_wav(48));
    write_text(zh_root / "Data/INI/Default/Audio.ini",
        "AudioSettings DefaultAudio\nMaxVoices = 4\nEND\n"
        "MusicTrack Theme\nFilename = Audio/theme.wav\nVolume = 50%\nLoopCount = 0\nPriority = HIGH\nAmbient = No\nEND\n"
        "AudioEvent Click\nFilename = Audio/click.wav\nVolume = 75%\nLoopCount = 0\nPriority = LOW\nEND\n"
        "DialogEvent Line\nFilename = Audio/line.wav\nVolume = 100%\nLoopCount = 0\nPriority = NORMAL\nEND\n");
    write_text(zh_root / "Data/INI/Audio.ini",
        "MusicTrack Theme\nFilename = Audio/theme.wav\nVolume = 80%\nLoopCount = 0\nPriority = CRITICAL\nAmbient = Yes\nEND\n");
    write_text(zh_root / "Data/INI/MissingMusic.ini",
        "MusicTrack Missing\nFilename = Audio/not-present.wav\nVolume = 100%\nLoopCount = 0\nPriority = LOW\nEND\n");
    write_text(zh_root / "Data/INI/MalformedAudio.ini",
        "MusicTrack Broken\nFilename = Audio/theme.wav\nVolume = 800%\nEND\n");
    write_text(zh_root / "Data/INI/Incompatible.ini",
        "AudioEvent Theme\nFilename = Audio/click.wav\nVolume = 100%\nPriority = LOW\nEND\n");
    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    const std::vector<std::string> layers{"Data/INI/Default/Audio.ini", "Data/INI/Audio.ini"};

    {
        AudioDefinitions audio(vfs);
        check(audio.load(layers), audio.last_error());
        check(audio.ready() && audio.null_output(), "audio registry did not select deterministic null output");
        check(audio.counts().audio_definitions == 3 && audio.counts().workers == 0 &&
              audio.counts().device_acquisitions == 0, "audio ownership/device counts wrong");
        const auto* theme = audio.find("Theme");
        check(theme && theme->kind == DefinitionKind::music && theme->volume > 0.79F && theme->volume < 0.81F &&
              theme->priority == 4, "layered music override did not win");
        check(audio.find("Click") && audio.find("Line"), "event/dialog definitions missing");
        check(!audio.play_music("Click") && audio.last_error().find("not music") != std::string_view::npos,
            "non-music definition played as music");
        check(!audio.play_music("Unknown") && audio.last_error().find("unknown original music") != std::string_view::npos,
            "unknown music fabricated success");
        check(audio.play_music("Theme"), audio.last_error());
        audio.render_null(128);
        check(audio.active_voice_count() == 0, "valid owned music did not complete through null sink");
        check(audio.cd_search_attempts() == 0 && audio.modal_waits() == 0,
            "valid music entered excluded CD/modal path");
        audio.shutdown(); audio.shutdown();
        check(zero(audio.counts()) && audio.active_voice_count() == 0, "audio repeated shutdown retained state");
    }

    for (std::size_t stage = 1; stage <= 4; ++stage) {
        AudioDefinitions audio(vfs);
        check(!audio.load(layers, stage), "injected audio failure was accepted");
        check(audio.last_error().find("injected original audio failure") != std::string_view::npos,
            "injected audio diagnostic missing");
        check(zero(audio.counts()) && audio.active_voice_count() == 0,
            "partial audio failure retained definitions/voices/workers");
    }

    {
        AudioDefinitions audio(vfs);
        const auto start = std::chrono::steady_clock::now();
        check(!audio.load({"Data/INI/MissingMusic.ini"}), "missing music resource accepted");
        const auto elapsed = std::chrono::steady_clock::now() - start;
        check(elapsed < std::chrono::seconds(1), "missing music did not terminate promptly");
        check(audio.last_error().find("Audio/not-present.wav") != std::string_view::npos &&
              audio.last_error().find("CD search and modal retry are disabled") != std::string_view::npos,
            "missing music diagnostic did not preserve path/policy");
        check(audio.cd_search_attempts() == 0 && audio.modal_waits() == 0 && !audio.ready(),
            "missing music entered CD/modal wait or fabricated load");
    }
    {
        AudioDefinitions audio(vfs);
        check(!audio.load({"Data/INI/MalformedAudio.ini"}), "malformed audio definition accepted");
        check(audio.last_error().find("volume") != std::string_view::npos, "malformed audio diagnostic missing");
    }
    {
        AudioDefinitions audio(vfs);
        check(!audio.load({"Data/INI/Default/Audio.ini", "Data/INI/Incompatible.ini"}),
            "incompatible duplicate audio definition accepted");
        check(audio.last_error().find("incompatible duplicate") != std::string_view::npos,
            "incompatible duplicate diagnostic missing");
    }
    check(std::string(provider_audio_identity()) == "OriginalAudioDefinitions.cpp", "audio provider witness");
    std::filesystem::remove_all(root, ignored);
    if (failures != 0) return 1;
    std::cout << "original-resources audio: ok providers=" << provider_audio_identity()
              << " definitions=3 workers=0 devices=0 cd-search=0 modal=0\n";
    return 0;
}
