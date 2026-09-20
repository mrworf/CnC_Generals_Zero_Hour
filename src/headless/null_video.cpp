#include "zh/headless/device.h"

#include <memory>

namespace zh::headless {
namespace {

class NullVideo final : public NullDevice {
public:
    std::string_view name() const noexcept override { return "video"; }
    std::string_view capability() const noexcept override { return "video presentation skipped (null video)"; }
    void initialize() override { initialized_ = true; }
    void shutdown() noexcept override { initialized_ = false; }

private:
    bool initialized_ = false;
};

} // namespace

std::unique_ptr<NullDevice> make_null_video()
{
    return std::make_unique<NullVideo>();
}

} // namespace zh::headless
