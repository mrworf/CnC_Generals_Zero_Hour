#include "zh/platform/sdl_platform.h"

#include "zh/foundation/unicode.h"

#include <SDL3/SDL_events.h>

#include <cstddef>
#include <string_view>

namespace zh::platform {
namespace {

PhysicalKey physical_key(SDL_Scancode scancode)
{
    const auto value = static_cast<int>(scancode);
    if (value <= static_cast<int>(SDL_SCANCODE_UNKNOWN) || value >= static_cast<int>(SDL_SCANCODE_COUNT)) {
        throw PlatformError("SDL keyboard event has unknown/out-of-range scancode");
    }
    return {static_cast<std::uint16_t>(value)};
}

std::string validated_text(const char* text, std::string_view field)
{
    if (text == nullptr) throw PlatformError("SDL " + std::string(field) + " event has null UTF-8 text");
    const std::string value(text);
    try {
        (void)foundation::utf8_to_utf16(value);
    } catch (const foundation::UnicodeError& error) {
        throw PlatformError("SDL " + std::string(field) + " event has malformed UTF-8: " + error.what());
    }
    return value;
}

std::size_t unicode_scalar_count(std::string_view text)
{
    std::size_t count = 0;
    for (unsigned char byte : text) {
        if ((byte & 0xc0U) != 0x80U) ++count;
    }
    return count;
}

void validate_mouse_button(std::uint8_t button)
{
    if (button == 0 || button > 5) {
        throw PlatformError("SDL mouse button event has unsupported button index " + std::to_string(button));
    }
}

} // namespace

bool InputSnapshot::key_held(PhysicalKey key) const noexcept
{
    return key.value < held_keys_.size() && held_keys_[key.value];
}

bool InputSnapshot::mouse_button_held(std::uint8_t button) const noexcept
{
    return button < held_mouse_buttons_.size() && held_mouse_buttons_[button];
}

std::optional<PlatformEvent> SdlEventTranslator::translate(const SDL_Event& event)
{
    PlatformEvent translated;
    translated.timestamp_ns = event.common.timestamp;

    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        const auto key = physical_key(event.key.scancode);
        translated.type = event.type == SDL_EVENT_KEY_DOWN ? EventType::key_down : EventType::key_up;
        translated.physical_key = key;
        translated.shortcut_key = static_cast<std::uint32_t>(event.key.key);
        translated.repeat = event.key.repeat;
        snapshot_.held_keys_[key.value] = event.type == SDL_EVENT_KEY_DOWN;
        return translated;
    }
    case SDL_EVENT_TEXT_INPUT:
        translated.type = EventType::text_input;
        translated.text = validated_text(event.text.text, "text input");
        return translated;
    case SDL_EVENT_TEXT_EDITING: {
        translated.type = EventType::text_editing;
        translated.text = validated_text(event.edit.text, "text editing");
        const bool unset = event.edit.start == -1 && event.edit.length == -1;
        const auto scalar_count = unicode_scalar_count(translated.text);
        if (!unset && (event.edit.start < 0 || event.edit.length < 0 ||
                          static_cast<std::size_t>(event.edit.start) > scalar_count ||
                          static_cast<std::size_t>(event.edit.length) >
                              scalar_count - static_cast<std::size_t>(event.edit.start))) {
            throw PlatformError("SDL text editing event has invalid composition start/length");
        }
        translated.editing_start = event.edit.start;
        translated.editing_length = event.edit.length;
        return translated;
    }
    case SDL_EVENT_MOUSE_MOTION:
        translated.type = EventType::mouse_motion;
        translated.x = event.motion.x;
        translated.y = event.motion.y;
        translated.delta_x = event.motion.xrel;
        translated.delta_y = event.motion.yrel;
        snapshot_.mouse_x_ = event.motion.x;
        snapshot_.mouse_y_ = event.motion.y;
        return translated;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        validate_mouse_button(event.button.button);
        translated.type = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? EventType::mouse_button_down
                                                                    : EventType::mouse_button_up;
        translated.mouse_button = event.button.button;
        translated.click_count = event.button.clicks;
        translated.x = event.button.x;
        translated.y = event.button.y;
        snapshot_.held_mouse_buttons_[event.button.button] = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        return translated;
    case SDL_EVENT_MOUSE_WHEEL: {
        translated.type = EventType::mouse_wheel;
        const float direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0F : 1.0F;
        translated.delta_x = event.wheel.x * direction;
        translated.delta_y = event.wheel.y * direction;
        translated.x = event.wheel.mouse_x;
        translated.y = event.wheel.mouse_y;
        return translated;
    }
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        translated.type = EventType::focus_gained;
        snapshot_.focused_ = true;
        return translated;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        translated.type = EventType::focus_lost;
        snapshot_.focused_ = false;
        snapshot_.relative_capture_ = false;
        snapshot_.held_keys_.fill(false);
        snapshot_.held_mouse_buttons_.fill(false);
        return translated;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        if (event.window.data1 <= 0 || event.window.data2 <= 0) {
            throw PlatformError("SDL window resize event has nonpositive width/height");
        }
        translated.type = EventType::window_resized;
        translated.width = event.window.data1;
        translated.height = event.window.data2;
        snapshot_.window_width_ = event.window.data1;
        snapshot_.window_height_ = event.window.data2;
        return translated;
    case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
    case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        translated.type = EventType::fullscreen_changed;
        translated.enabled = event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN;
        return translated;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    case SDL_EVENT_QUIT:
        translated.type = EventType::quit;
        return translated;
    default:
        return std::nullopt;
    }
}

} // namespace zh::platform
