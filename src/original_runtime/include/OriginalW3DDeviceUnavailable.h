#pragma once

#include <stdexcept>

// The original producer has selected a physical GPU operation whose Linux
// translation is not installed yet. Never treat this as an empty draw.
class OriginalW3DDeviceUnavailable final : public std::runtime_error {
public:
    explicit OriginalW3DDeviceUnavailable(const char *operation)
        : std::runtime_error(operation) {}
};
