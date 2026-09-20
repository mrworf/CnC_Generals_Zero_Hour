#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

const char* provider_cpu_identity() noexcept;

} // namespace zh::original_resources
