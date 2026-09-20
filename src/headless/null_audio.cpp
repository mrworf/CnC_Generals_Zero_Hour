#include "zh/headless/device.h"

#include <memory>

namespace zh::headless {
namespace {

class NullAudio final : public NullDevice {
public:
    std::string_view name() const noexcept override { return "audio"; }
    std::string_view capability() const noexcept override { return "audio device skipped (null audio)"; }
    void initialize() override { initialized_ = true; }
    void shutdown() noexcept override { initialized_ = false; }

private:
    bool initialized_ = false;
};

} // namespace

std::unique_ptr<NullDevice> make_null_audio()
{
    return std::make_unique<NullAudio>();
}

} // namespace zh::headless
