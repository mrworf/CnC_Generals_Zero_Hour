#pragma once

#include "zh/renderer/contract.h"

#include <limits>

namespace zh::renderer::detail {

// Shared by the real native owner and its CPU-only admission witness. Resource
// and byte charges include retained baseline/candidate/retirement ownership.
class BgfxTransactionState final {
public:
    static constexpr UInt32 maximum_commands = 4096;
    static constexpr UInt32 maximum_resources = 4096;
    static constexpr UInt64 maximum_bytes = 64U * 1024U * 1024U;

    static bool valid_idle(const DeviceTransactionDesc& desc, UInt64 resources,
                           UInt64 bytes) noexcept
    {
        return desc.mode == DeviceTransactionMode::idle_preparation && desc.generation
            && desc.commands && desc.commands <= maximum_commands
            && desc.resources && desc.resources <= maximum_resources
            && desc.bytes && desc.bytes <= maximum_bytes && !desc.views
            && resources <= desc.resources && bytes <= desc.bytes;
    }

    bool begin(const DeviceTransactionDesc& desc, UInt64 device, UInt64 resources,
               UInt64 bytes) noexcept
    {
        if (active_ || !device || sequence_ == std::numeric_limits<UInt64>::max()
            || !valid_idle(desc, resources, bytes)) return false;
        desc_ = desc;
        token_ = {device, ++sequence_, desc.generation, desc.mode};
        commands_ = 0;
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
    UInt32 commands_ = 0;
    bool active_ = false, failed_ = false;
};

} // namespace zh::renderer::detail
