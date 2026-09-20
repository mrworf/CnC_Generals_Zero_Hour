#include "zh/headless/device.h"

#include <memory>

namespace zh::headless {
namespace {

class NullPlatform final : public NullDevice {
public:
    std::string_view name() const noexcept override { return "platform"; }
    std::string_view capability() const noexcept override { return "display and input skipped (null platform)"; }
    void initialize() override { initialized_ = true; }
    void shutdown() noexcept override { initialized_ = false; }

private:
    bool initialized_ = false;
};

} // namespace

std::unique_ptr<NullDevice> make_null_platform()
{
    return std::make_unique<NullPlatform>();
}

} // namespace zh::headless
