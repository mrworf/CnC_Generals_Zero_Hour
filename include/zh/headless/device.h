#pragma once

#include <memory>
#include <string_view>

namespace zh::headless {

class NullDevice {
public:
    virtual ~NullDevice() = default;
    virtual std::string_view name() const noexcept = 0;
    virtual std::string_view capability() const noexcept = 0;
    virtual void initialize() = 0;
    virtual void shutdown() noexcept = 0;
};

std::unique_ptr<NullDevice> make_null_platform();
std::unique_ptr<NullDevice> make_null_renderer();
std::unique_ptr<NullDevice> make_null_audio();
std::unique_ptr<NullDevice> make_null_video();

} // namespace zh::headless
