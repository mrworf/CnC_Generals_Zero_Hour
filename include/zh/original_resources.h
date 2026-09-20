#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::data { class VirtualFileSystem; }
namespace zh::original_data { class LogicalFiles; }
namespace zh::renderer { class GpuDevice; }

namespace zh::original_resources {

class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class ResourceKind { model, texture, animation };
enum class Capability { cpu_resources, recorded_drawing, physical_device, web_browser };

struct ResourceRequest {
    ResourceKind kind = ResourceKind::model;
    std::string logical_name;
};

struct OwnershipCounts {
    std::size_t loaders = 0;
    std::size_t assets = 0;
    std::size_t scenes = 0;
    std::size_t lights = 0;
    std::size_t callbacks = 0;
    std::size_t layouts = 0;
    std::size_t windows = 0;
    std::size_t fonts = 0;
    std::size_t audio_definitions = 0;
    std::size_t workers = 0;
    std::size_t device_acquisitions = 0;
};

class CpuPresentation {
public:
    CpuPresentation(const original_data::LogicalFiles& files, renderer::GpuDevice& recorder);
    ~CpuPresentation();
    CpuPresentation(const CpuPresentation&) = delete;
    CpuPresentation& operator=(const CpuPresentation&) = delete;

    bool initialize(const std::vector<ResourceRequest>& resources, std::size_t fail_after_stage = 0);
    bool record_scene();
    void shutdown() noexcept;
    bool ready() const noexcept;
    bool supports(Capability capability) const noexcept;
    OwnershipCounts counts() const noexcept;
    std::string_view last_error() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

using WindowCallback = std::function<void(std::string_view)>;

class UiResources {
public:
    UiResources(const data::VirtualFileSystem& vfs, renderer::GpuDevice& recorder);
    ~UiResources();
    UiResources(const UiResources&) = delete;
    UiResources& operator=(const UiResources&) = delete;

    bool register_callback(std::string name, WindowCallback callback);
    bool load(std::string_view image_definitions, std::string_view layout,
        std::string_view language, std::size_t fail_after_stage = 0);
    bool record();
    bool invoke(std::string_view window_name);
    void shutdown() noexcept;
    bool ready() const noexcept;
    OwnershipCounts counts() const noexcept;
    std::string_view last_error() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

enum class DefinitionKind { music, sound, dialog };

struct AudioDefinitionView {
    DefinitionKind kind = DefinitionKind::sound;
    std::string name;
    std::string filename;
    float volume = 1.0F;
    bool loop = false;
    int priority = 0;
};

class AudioDefinitions {
public:
    explicit AudioDefinitions(const data::VirtualFileSystem& vfs);
    ~AudioDefinitions();
    AudioDefinitions(const AudioDefinitions&) = delete;
    AudioDefinitions& operator=(const AudioDefinitions&) = delete;

    bool load(const std::vector<std::string>& definition_layers, std::size_t fail_after_stage = 0);
    bool play_music(std::string_view name);
    void render_null(std::size_t frames);
    void shutdown() noexcept;
    const AudioDefinitionView* find(std::string_view name) const noexcept;
    bool ready() const noexcept;
    bool null_output() const noexcept;
    OwnershipCounts counts() const noexcept;
    std::size_t active_voice_count() const noexcept;
    std::size_t cd_search_attempts() const noexcept;
    std::size_t modal_waits() const noexcept;
    std::string_view last_error() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

const char* provider_cpu_identity() noexcept;
const char* provider_ui_identity() noexcept;
const char* provider_audio_identity() noexcept;

} // namespace zh::original_resources
