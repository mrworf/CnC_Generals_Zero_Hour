#include "zh/headless/device.h"

#include <memory>

namespace zh::headless {
namespace {

class NullRenderer final : public NullDevice {
public:
    std::string_view name() const noexcept override { return "renderer"; }
    std::string_view capability() const noexcept override { return "GPU skipped (null renderer)"; }
    void initialize() override { initialized_ = true; }
    void shutdown() noexcept override { initialized_ = false; }

private:
    bool initialized_ = false;
};

} // namespace

std::unique_ptr<NullDevice> make_null_renderer()
{
    return std::make_unique<NullRenderer>();
}

} // namespace zh::headless
