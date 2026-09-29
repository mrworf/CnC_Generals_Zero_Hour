#pragma once

#include "zh/renderer/contract.h"

#include <limits>
#include <cmath>
#include <cstring>

namespace zh::renderer::detail {

// The logical descriptor, not bgfx's boolean full-chain allocation, owns the
// sampled view. Use subtraction so malformed first/count cannot overflow.
inline bool sampled_mip_range(UInt32 first, UInt32 count, UInt32 levels) noexcept
{ return levels && levels <= 255 && count && first < levels && count <= levels-first; }

// Exact native field representation used by capture and its generated CPU
// controls. Native-cap/target bounds are additional owner-specific checks.
inline bool bounded_view_rectangle(double x,double y,double width,double height) noexcept
{
    return x >= 0 && y >= 0 && x <= 32767 && y <= 32767
        && width > 0 && height > 0 && width <= 65535 && height <= 65535
        && std::trunc(x)==x && std::trunc(y)==y && std::trunc(width)==width && std::trunc(height)==height;
}
inline bool same_uniform_payload(UInt32 count,UInt32 bytes,const void* data,
                                 UInt32 other_count,UInt32 other_bytes,const void* other) noexcept
{ return count == other_count && bytes == other_bytes && data && other && !std::memcmp(data,other,bytes); }
struct BgfxCapturedSampler {
    UInt32 stage=0,uniform=0,texture=0,source_sampler=0,flags=0;
    UInt32 first_mip=0,mip_count=255;
};
enum class BgfxSamplerAlias { distinct, identical, conflict };
inline BgfxSamplerAlias sampler_alias(const BgfxCapturedSampler& prior,
                                     const BgfxCapturedSampler& candidate) noexcept
{
    if (prior.stage != candidate.stage)
        return prior.uniform == candidate.uniform ? BgfxSamplerAlias::conflict : BgfxSamplerAlias::distinct;
    return prior.uniform == candidate.uniform && prior.texture == candidate.texture
        && prior.source_sampler == candidate.source_sampler && prior.flags == candidate.flags
        && prior.first_mip == candidate.first_mip && prior.mip_count == candidate.mip_count
        ? BgfxSamplerAlias::identical : BgfxSamplerAlias::conflict;
}

// Shared by the real native owner and its CPU-only admission witness. Resource
// and byte charges include retained baseline/candidate/retirement ownership.
class BgfxTransactionState final {
public:
    static constexpr UInt32 maximum_commands = 4096;
    static constexpr UInt32 maximum_resources = 4096;
    static constexpr UInt64 maximum_bytes = 64U * 1024U * 1024U;
    static constexpr UInt32 maximum_views = 256;

    static bool valid_idle(const DeviceTransactionDesc& desc, UInt64 resources,
                           UInt64 bytes) noexcept
    {
        return desc.mode == DeviceTransactionMode::idle_preparation && desc.generation
            && desc.commands && desc.commands <= maximum_commands
            && desc.resources && desc.resources <= maximum_resources
            && desc.bytes && desc.bytes <= maximum_bytes && !desc.views
            && resources <= desc.resources && bytes <= desc.bytes;
    }

    static bool valid(const DeviceTransactionDesc& desc, UInt64 resources,
                      UInt64 bytes) noexcept
    {
        if (desc.mode == DeviceTransactionMode::idle_preparation)
            return valid_idle(desc, resources, bytes);
        auto idle = desc;
        idle.mode = DeviceTransactionMode::idle_preparation;
        idle.views = 0;
        return desc.mode == DeviceTransactionMode::frame_commands && desc.views
            && desc.views <= maximum_views && valid_idle(idle, resources, bytes);
    }

    bool begin(const DeviceTransactionDesc& desc, UInt64 device, UInt64 resources,
               UInt64 bytes) noexcept
    {
        if (active_ || !device || sequence_ == std::numeric_limits<UInt64>::max()
            || !valid(desc, resources, bytes)) return false;
        desc_ = desc;
        token_ = {device, ++sequence_, desc.generation, desc.mode};
        commands_ = 0;
        views_ = 0;
        resources_ = resources;
        bytes_ = bytes;
        failed_ = false;
        active_ = true;
        return true;
    }

    bool charge(UInt64 bytes = 0, UInt32 resources = 0) noexcept
    {
        if (!active_) return true;
        if (failed_ || commands_ >= desc_.commands || resources > desc_.resources - resources_
            || bytes > desc_.bytes - bytes_) {
            failed_ = true;
            return false;
        }
        ++commands_;
        resources_ += resources;
        bytes_ += bytes;
        return true;
    }

    bool retain_bytes(UInt64 bytes) noexcept
    {
        if (!active_) return true;
        if (failed_ || bytes > desc_.bytes - bytes_) {
            failed_ = true;
            return false;
        }
        bytes_ += bytes;
        return true;
    }
    bool retain_resources(UInt32 resources) noexcept
    {
        if (!active_) return true;
        if (failed_ || resources > desc_.resources - resources_) {
            failed_ = true;
            return false;
        }
        resources_ += resources;
        return true;
    }

    bool retain_views(UInt32 views) noexcept
    {
        if (!active_) return true;
        if (failed_ || views > desc_.views - views_) { failed_ = true; return false; }
        views_ += views;
        return true;
    }
    const DeviceTransactionDesc& descriptor() const noexcept { return desc_; }

    bool matches(const DeviceTransactionToken& token) const noexcept
    {
        return active_ && token.device == token_.device && token.sequence == token_.sequence
            && token.generation == token_.generation && token.mode == token_.mode;
    }
    bool finish(const DeviceTransactionToken& token, bool commit) noexcept
    {
        if (!matches(token) || (commit && failed_)) return false;
        active_ = false;
        return true;
    }
    void poison() noexcept { if (active_) failed_ = true; }
    bool active() const noexcept { return active_; }
    bool failed() const noexcept { return failed_; }
    const DeviceTransactionToken& token() const noexcept { return token_; }

private:
    DeviceTransactionDesc desc_{};
    DeviceTransactionToken token_{};
    UInt64 sequence_ = 0, resources_ = 0, bytes_ = 0;
    UInt32 commands_ = 0, views_ = 0;
    bool active_ = false, failed_ = false;
};

} // namespace zh::renderer::detail
