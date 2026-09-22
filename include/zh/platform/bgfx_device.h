#pragma once

#include "zh/renderer/contract.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct SDL_Window;

namespace zh::renderer {

struct BgfxOptions {
    std::filesystem::path shader_root;
    bool debug = true;
    std::string driver = "vulkan";
    // Optional SDL3 pixel-extent query seam for deterministic suspension/loss
    // tests. Production leaves it empty and uses SDL_GetWindowSizeInPixels.
    std::function<bool(SDL_Window*, int*, int*)> pixel_extent_query;
};

// The public engine device contract stays backend-neutral. bgfx owns only the
// physical device edge; SDL3 continues to own the window, events and input.
class BgfxGpuDevice final : public GpuDevice {
public:
    explicit BgfxGpuDevice(BgfxOptions options);
    ~BgfxGpuDevice() override;
    BgfxGpuDevice(const BgfxGpuDevice&) = delete;
    BgfxGpuDevice& operator=(const BgfxGpuDevice&) = delete;

    bool supports_texture_format(TextureFormat, TextureDimension, bool sampled, bool render_target) const noexcept override;
    BufferHandle create_buffer(const BufferDesc&, std::string_view) override;
    TextureHandle create_texture(const TextureDesc&, std::string_view) override;
    std::optional<TextureFormat> describe_texture_format(TextureHandle) const noexcept override;
    SamplerHandle create_sampler(const SamplerDesc&, std::string_view) override;
    ShaderHandle create_shader(const ShaderDesc&, std::string_view) override;
    PipelineHandle create_pipeline(const PipelineKey&, std::string_view) override;
    ValidationResult upload(const UploadDesc&, const void*) override;
    ValidationResult upload_texture(const TextureUploadDesc&, const void*) override;
    ValidationResult begin_pass(const RenderPassDesc&, std::string_view) override;
    std::pair<UInt32,UInt32> active_pass_extent() const noexcept override;
    ValidationResult set_viewport(const ViewportDesc&) override;
    ValidationResult clear_viewport(const ViewportClearDesc&) override;
    ValidationResult draw(const DrawDesc&) override;
    ValidationResult end_pass() override;
    ValidationResult present(TextureHandle) override;
    void destroy(BufferHandle) override;
    void destroy(TextureHandle) override;
    void destroy(SamplerHandle) override;
    void destroy(ShaderHandle) override;
    void destroy(PipelineHandle) override;
    const std::string& last_error() const noexcept override;
    bool pass_active() const noexcept override;
    void record_marker(std::string_view) override;

    ValidationResult claim_window(SDL_Window*);
    ValidationResult wait_idle();
    std::vector<UInt8> readback_rgba(TextureHandle source);
    void release_window() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace zh::renderer
