/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M28 extraction provenance:
// AudioManager::init/isMusicAlreadyLoaded, INI::parseMusicTrackDefinition,
// INI::parseAudioEventDefinition/parseDialogDefinition, AudioEventInfo's field
// table and MusicTrack::m_musicTrackFieldParseTable remain authoritative. This
// unit preserves bounded definition and lookup semantics while replacing the
// excluded CD search/system-modal loop with one prompt noninteractive failure.

#include "zh/original_resources.h"

#include "zh/audio/manager.h"
#include "zh/data/vfs.h"
#include "zh/original_data.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <map>
#include <sstream>
#include <utility>

namespace zh::original_resources {
namespace {

std::string trim(std::string value)
{
    const auto begin = value.find_first_not_of(" \t\r");
    if (begin == std::string::npos) return {};
    const auto end = value.find_last_not_of(" \t\r");
    return value.substr(begin, end - begin + 1);
}

bool parse_bool(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (value == "yes" || value == "true" || value == "1") return true;
    if (value == "no" || value == "false" || value == "0") return false;
    throw Error("invalid original audio boolean '" + value + "'");
}

int parse_int(std::string_view value, std::string_view field)
{
    int result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        throw Error("invalid original audio integer for " + std::string(field));
    return result;
}

float parse_volume(std::string value)
{
    bool percent = !value.empty() && value.back() == '%';
    if (percent) value.pop_back();
    char* end = nullptr;
    const float parsed = std::strtof(value.c_str(), &end);
    if (end != value.c_str() + value.size() || !std::isfinite(parsed))
        throw Error("invalid original audio volume");
    const float result = percent ? parsed / 100.0F : parsed;
    if (result < 0.0F || result > 1.0F) throw Error("original audio volume must be within 0..100%");
    return result;
}

int parse_priority(std::string value)
{
    static const std::vector<std::string> priorities{"LOWEST", "LOW", "NORMAL", "HIGH", "CRITICAL"};
    const auto found = std::find(priorities.begin(), priorities.end(), value);
    if (found == priorities.end()) throw Error("unsupported original audio Priority '" + value + "'");
    return static_cast<int>(std::distance(priorities.begin(), found));
}

bool parse_control(std::string value)
{
    std::replace(value.begin(), value.end(), ',', ' ');
    std::istringstream input(value);
    bool loop = false;
    for (std::string token; input >> token;) {
        if (token == "LOOP") loop = true;
        else if (token != "RANDOM" && token != "ALL" && token != "POSTDELAY" && token != "INTERRUPT")
            throw Error("unsupported original audio Control '" + token + "'");
    }
    return loop;
}

struct ParsedAudio {
    std::map<std::string, AudioDefinitionView> definitions;
    std::size_t maximum_voices = 32;
};

void parse_layer(const std::vector<std::uint8_t>& bytes, ParsedAudio& result)
{
    if (bytes.size() > 4U * 1024U * 1024U) throw Error("original audio definition layer exceeds 4 MiB");
    std::istringstream input(std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    std::size_t records = 0;
    AudioDefinitionView current;
    bool active = false;
    for (std::string raw; std::getline(input, raw);) {
        if (++records > 100000 || raw.size() > 4096) throw Error("original audio definition limits exceeded");
        auto line = trim(raw);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        if (!active) {
            std::istringstream header(line);
            std::string type, name, extra;
            header >> type;
            if (type == "AudioSettings") {
                if (!(header >> name) || header >> extra) throw Error("malformed AudioSettings header");
                current = {DefinitionKind::sound, "@settings:" + name, {}, 1.0F, false, 0};
            } else {
                if (!(header >> name) || header >> extra || name.empty()) throw Error("malformed original audio definition header");
                const auto kind = type == "MusicTrack" ? DefinitionKind::music
                    : type == "AudioEvent" ? DefinitionKind::sound
                    : type == "DialogEvent" ? DefinitionKind::dialog
                    : throw Error("unsupported original audio definition '" + type + "'");
                current = {kind, name, {}, 1.0F, false, 0};
            }
            active = true;
            continue;
        }
        if (line == "END") {
            if (current.name.rfind("@settings:", 0) != 0) {
                if (current.filename.empty()) throw Error("original audio definition '" + current.name + "' has no Filename");
                const auto existing = result.definitions.find(current.name);
                if (existing != result.definitions.end() && existing->second.kind != current.kind)
                    throw Error("incompatible duplicate original audio definition '" + current.name + "'");
                result.definitions[current.name] = current;
            }
            active = false;
            continue;
        }
        const auto equals = line.find('=');
        if (equals == std::string::npos) throw Error("malformed original audio field");
        const auto field = trim(line.substr(0, equals));
        const auto value = trim(line.substr(equals + 1));
        if (field.empty() || value.empty()) throw Error("empty original audio field/value");
        if (current.name.rfind("@settings:", 0) == 0) {
            if (field != "MaxVoices") throw Error("unsupported original AudioSettings field '" + field + "'");
            const int voices = parse_int(value, field);
            if (voices < 1 || voices > 128) throw Error("original MaxVoices must be within 1..128");
            result.maximum_voices = static_cast<std::size_t>(voices);
        } else if (field == "Filename") current.filename = value;
        else if (field == "Volume") current.volume = parse_volume(value);
        else if (field == "LoopCount") {
            const int loops = parse_int(value, field);
            if (loops < 0 || loops > 10000) throw Error("original LoopCount must be within 0..10000");
            current.loop = loops != 0;
        }
        else if (field == "Priority") current.priority = parse_priority(value);
        else if (field == "Control") current.loop = current.loop || parse_control(value);
        else if (field == "Ambient" && current.kind == DefinitionKind::music) (void)parse_bool(value);
        else throw Error("unsupported original audio field '" + field + "'");
    }
    if (active) throw Error("unterminated original audio definition '" + current.name + "'");
}

} // namespace

struct AudioDefinitions::Impl {
    explicit Impl(const data::VirtualFileSystem& source) : vfs(source), files(source) {}
    const data::VirtualFileSystem& vfs;
    original_data::LogicalFiles files;
    std::map<std::string, AudioDefinitionView> definitions;
    std::unique_ptr<audio::AudioManager> manager;
    OwnershipCounts ownership;
    std::string error;
    bool loaded = false;
    std::size_t cd_attempts = 0;
    std::size_t modal_wait_count = 0;

    void clear() noexcept
    {
        if (manager) {
            manager->shutdown();
            float sample[2]{};
            manager->render(sample, 1);
            (void)manager->drain_completions();
            manager.reset();
        }
        definitions.clear();
        ownership = {};
        loaded = false;
    }
    bool fail(std::string message) { error = std::move(message); clear(); return false; }
};

AudioDefinitions::AudioDefinitions(const data::VirtualFileSystem& vfs) : impl_(std::make_unique<Impl>(vfs)) {}
AudioDefinitions::~AudioDefinitions() { shutdown(); }

bool AudioDefinitions::load(const std::vector<std::string>& definition_layers, std::size_t fail_after_stage)
{
    if (impl_->loaded) return impl_->fail("original audio definitions are already loaded");
    impl_->error.clear();
    impl_->cd_attempts = impl_->modal_wait_count = 0;
    if (definition_layers.empty() || definition_layers.size() > 64)
        return impl_->fail("original audio requires 1..64 definition layers");
    try {
        ParsedAudio parsed;
        std::size_t stage = 0;
        for (const auto& layer : definition_layers) {
            parse_layer(impl_->files.read(layer), parsed);
            if (++stage == fail_after_stage) throw Error("injected original audio failure at stage " + std::to_string(stage));
        }
        if (parsed.definitions.empty()) throw Error("original audio registry is empty");
        for (const auto& [name, definition] : parsed.definitions) {
            if (impl_->vfs.find(definition.filename) == nullptr)
                throw Error("missing original audio resource '" + definition.filename + "' for definition '" + name +
                    "'; CD search and modal retry are disabled on Linux");
        }
        if (++stage == fail_after_stage) throw Error("injected original audio failure at stage " + std::to_string(stage));
        auto manager = std::make_unique<audio::AudioManager>(impl_->vfs, parsed.maximum_voices);
        manager->configure_output(false);
        if (++stage == fail_after_stage) throw Error("injected original audio failure at stage " + std::to_string(stage));
        impl_->definitions = std::move(parsed.definitions);
        impl_->manager = std::move(manager);
        impl_->ownership.audio_definitions = impl_->definitions.size();
        impl_->ownership.workers = 0;
        impl_->loaded = true;
        return true;
    } catch (const std::exception& exception) {
        return impl_->fail(exception.what());
    }
}

bool AudioDefinitions::play_music(std::string_view name)
{
    if (!impl_->loaded) { impl_->error = "original audio definitions are not loaded"; return false; }
    const auto found = impl_->definitions.find(std::string(name));
    if (found == impl_->definitions.end()) { impl_->error = "unknown original music definition '" + std::string(name) + "'"; return false; }
    if (found->second.kind != DefinitionKind::music) { impl_->error = "original audio definition is not music: '" + std::string(name) + "'"; return false; }
    try {
        audio::PlayRequest request;
        request.logical_path = found->second.filename;
        request.kind = audio::AudioKind::music;
        request.volume = found->second.volume;
        request.loop = found->second.loop;
        request.priority = found->second.priority;
        request.streaming = true;
        (void)impl_->manager->play(request);
        return true;
    } catch (const std::exception& exception) {
        impl_->error = exception.what();
        return false;
    }
}

void AudioDefinitions::render_null(std::size_t frames)
{
    if (!impl_->manager || frames == 0 || frames > 48000) return;
    std::vector<float> output(frames * 2);
    impl_->manager->render(output.data(), frames);
    (void)impl_->manager->drain_completions();
}

void AudioDefinitions::shutdown() noexcept { impl_->clear(); }
const AudioDefinitionView* AudioDefinitions::find(std::string_view name) const noexcept
{
    const auto found = impl_->definitions.find(std::string(name));
    return found == impl_->definitions.end() ? nullptr : &found->second;
}
bool AudioDefinitions::ready() const noexcept { return impl_->loaded; }
bool AudioDefinitions::null_output() const noexcept
{
    return impl_->manager && impl_->manager->output_state() == audio::AudioOutputState::null_sink;
}
OwnershipCounts AudioDefinitions::counts() const noexcept { return impl_->ownership; }
std::size_t AudioDefinitions::active_voice_count() const noexcept
{
    return impl_->manager ? impl_->manager->active_voice_count() : 0;
}
std::size_t AudioDefinitions::cd_search_attempts() const noexcept { return impl_->cd_attempts; }
std::size_t AudioDefinitions::modal_waits() const noexcept { return impl_->modal_wait_count; }
std::string_view AudioDefinitions::last_error() const noexcept { return impl_->error; }
const char* provider_audio_identity() noexcept { return "OriginalAudioDefinitions.cpp"; }

} // namespace zh::original_resources
