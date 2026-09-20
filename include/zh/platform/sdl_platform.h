#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace zh::platform {

class PlatformError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct PhysicalKey {
    std::uint16_t value = 0;

    friend bool operator==(PhysicalKey left, PhysicalKey right) noexcept { return left.value == right.value; }
};

enum class EventType {
    key_down,
    key_up,
    text_input,
    text_editing,
    mouse_motion,
    mouse_button_down,
    mouse_button_up,
    mouse_wheel,
    focus_gained,
    focus_lost,
    window_resized,
    fullscreen_changed,
    quit,
};

struct PlatformEvent {
    EventType type = EventType::quit;
    std::uint64_t timestamp_ns = 0;
    PhysicalKey physical_key{};
    std::uint32_t shortcut_key = 0;
    bool repeat = false;
    std::string text;
    std::int32_t editing_start = 0;
    std::int32_t editing_length = 0;
    float x = 0.0F;
    float y = 0.0F;
    float delta_x = 0.0F;
    float delta_y = 0.0F;
    std::uint8_t mouse_button = 0;
    std::uint8_t click_count = 0;
    int width = 0;
    int height = 0;
    bool enabled = false;
};

class InputSnapshot {
public:
    bool key_held(PhysicalKey key) const noexcept;
    bool mouse_button_held(std::uint8_t button) const noexcept;
    bool focused() const noexcept { return focused_; }
    bool relative_capture() const noexcept { return relative_capture_; }
    float mouse_x() const noexcept { return mouse_x_; }
    float mouse_y() const noexcept { return mouse_y_; }
    int window_width() const noexcept { return window_width_; }
    int window_height() const noexcept { return window_height_; }

private:
    friend class SdlEventTranslator;

    std::array<bool, SDL_SCANCODE_COUNT> held_keys_{};
    std::array<bool, 6> held_mouse_buttons_{};
    bool focused_ = true;
    bool relative_capture_ = false;
    float mouse_x_ = 0.0F;
    float mouse_y_ = 0.0F;
    int window_width_ = 0;
    int window_height_ = 0;
};

class SdlEventTranslator {
public:
    std::optional<PlatformEvent> translate(const SDL_Event& event);
    const InputSnapshot& snapshot() const noexcept { return snapshot_; }
    void set_relative_capture(bool enabled) noexcept { snapshot_.relative_capture_ = enabled; }

private:
    InputSnapshot snapshot_;
};

} // namespace zh::platform
