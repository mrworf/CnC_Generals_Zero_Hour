#include "zh/platform/sdl_platform.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

#include <iostream>
#include <string_view>

namespace {

int failures = 0;

void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try {
        function();
        check(false, "expected platform error");
    } catch (const zh::platform::PlatformError& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos,
            "platform error diagnostic");
    }
}

SDL_Event event_of(SDL_EventType type)
{
    SDL_Event event{};
    event.type = type;
    event.common.timestamp = 42;
    return event;
}

} // namespace

int main()
{
    using zh::platform::EventType;
    using zh::platform::PhysicalKey;
    zh::platform::SdlEventTranslator translator;

    auto key_down = event_of(SDL_EVENT_KEY_DOWN);
    key_down.key.scancode = SDL_SCANCODE_A;
    key_down.key.key = SDLK_Q; // Deliberately distinct: physical A on a non-QWERTY layout.
    key_down.key.repeat = true;
    const auto pressed = translator.translate(key_down);
    check(pressed && pressed->type == EventType::key_down, "key down translated");
    check(pressed->physical_key == PhysicalKey{SDL_SCANCODE_A}, "physical identity comes from scancode");
    check(pressed->shortcut_key == SDLK_Q, "layout shortcut retains keycode");
    check(pressed->repeat, "key repeat retained");
    check(pressed->text.empty(), "key event synthesizes no text");
    check(translator.snapshot().key_held({SDL_SCANCODE_A}), "held key tracked");

    auto key_up = key_down;
    key_up.type = SDL_EVENT_KEY_UP;
    check(translator.translate(key_up)->type == EventType::key_up, "key up translated");
    check(!translator.snapshot().key_held({SDL_SCANCODE_A}), "released key cleared");

    auto text = event_of(SDL_EVENT_TEXT_INPUT);
    text.text.text = "Gr\xC3\xBC\xC3\x9F\x65 \xF0\x9F\x8C\x8D";
    const auto entered = translator.translate(text);
    check(entered && entered->type == EventType::text_input && entered->text == text.text.text,
        "UTF-8 text input retained");

    auto editing = event_of(SDL_EVENT_TEXT_EDITING);
    editing.edit.text = "\xE6\xBC\xA2\xE5\xAD\x97";
    editing.edit.start = 1;
    editing.edit.length = 1;
    const auto composition = translator.translate(editing);
    check(composition && composition->type == EventType::text_editing, "IME composition translated");
    check(composition->editing_start == 1 && composition->editing_length == 1,
        "IME composition range retained in characters");
    editing.edit.start = -1;
    editing.edit.length = -1;
    check(translator.translate(editing)->editing_start == -1, "IME unset range accepted");

    auto motion = event_of(SDL_EVENT_MOUSE_MOTION);
    motion.motion.x = 12.5F;
    motion.motion.y = 24.0F;
    motion.motion.xrel = -2.0F;
    motion.motion.yrel = 3.0F;
    const auto moved = translator.translate(motion);
    check(moved && moved->type == EventType::mouse_motion && moved->delta_x == -2.0F,
        "mouse motion and relative delta translated");

    auto button = event_of(SDL_EVENT_MOUSE_BUTTON_DOWN);
    button.button.button = SDL_BUTTON_LEFT;
    button.button.clicks = 2;
    button.button.x = 12.5F;
    button.button.y = 24.0F;
    const auto clicked = translator.translate(button);
    check(clicked && clicked->click_count == 2, "double click retained");
    check(translator.snapshot().mouse_button_held(SDL_BUTTON_LEFT), "mouse button tracked");

    auto wheel = event_of(SDL_EVENT_MOUSE_WHEEL);
    wheel.wheel.x = 1.0F;
    wheel.wheel.y = -2.0F;
    wheel.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
    const auto scrolled = translator.translate(wheel);
    check(scrolled && scrolled->delta_x == -1.0F && scrolled->delta_y == 2.0F,
        "flipped wheel normalized");

    translator.set_relative_capture(true);
    key_down.key.repeat = false;
    translator.translate(key_down);
    auto focus_lost = event_of(SDL_EVENT_WINDOW_FOCUS_LOST);
    check(translator.translate(focus_lost)->type == EventType::focus_lost, "focus loss translated");
    check(!translator.snapshot().focused(), "focus state cleared");
    check(!translator.snapshot().relative_capture(), "relative capture cleared on focus loss");
    check(!translator.snapshot().key_held({SDL_SCANCODE_A}), "held keys cleared on focus loss");
    check(!translator.snapshot().mouse_button_held(SDL_BUTTON_LEFT), "held buttons cleared on focus loss");
    check(translator.translate(event_of(SDL_EVENT_WINDOW_FOCUS_GAINED))->type == EventType::focus_gained,
        "focus gain translated");

    auto resized = event_of(SDL_EVENT_WINDOW_RESIZED);
    resized.window.data1 = 1280;
    resized.window.data2 = 720;
    const auto size = translator.translate(resized);
    check(size && size->width == 1280 && translator.snapshot().window_height() == 720,
        "window resize updates dimensions");
    check(translator.translate(event_of(SDL_EVENT_WINDOW_ENTER_FULLSCREEN))->enabled,
        "fullscreen enter translated");
    check(!translator.translate(event_of(SDL_EVENT_WINDOW_LEAVE_FULLSCREEN))->enabled,
        "fullscreen leave translated");
    check(translator.translate(event_of(SDL_EVENT_WINDOW_CLOSE_REQUESTED))->type == EventType::quit,
        "window close translated");
    check(translator.translate(event_of(SDL_EVENT_QUIT))->type == EventType::quit, "process quit translated");
    check(!translator.translate(event_of(SDL_EVENT_AUDIO_DEVICE_ADDED)), "unrelated event ignored");

    auto unknown_key = key_down;
    unknown_key.key.scancode = SDL_SCANCODE_UNKNOWN;
    expect_error([&] { translator.translate(unknown_key); }, "scancode");

    auto invalid_button = button;
    invalid_button.button.button = 0;
    expect_error([&] { translator.translate(invalid_button); }, "button index");
    check(!translator.snapshot().mouse_button_held(0), "invalid button does not mutate state");

    auto null_text = text;
    null_text.text.text = nullptr;
    expect_error([&] { translator.translate(null_text); }, "null UTF-8");
    auto malformed_text = text;
    malformed_text.text.text = "\xF0\x28\x8C\x28";
    expect_error([&] { translator.translate(malformed_text); }, "malformed UTF-8");

    auto invalid_edit = editing;
    invalid_edit.edit.start = 3;
    invalid_edit.edit.length = 1;
    expect_error([&] { translator.translate(invalid_edit); }, "composition start/length");
    invalid_edit.edit.start = -1;
    invalid_edit.edit.length = 0;
    expect_error([&] { translator.translate(invalid_edit); }, "composition start/length");

    auto invalid_resize = resized;
    invalid_resize.window.data1 = 0;
    expect_error([&] { translator.translate(invalid_resize); }, "nonpositive width/height");
    check(translator.snapshot().window_width() == 1280, "invalid resize does not mutate dimensions");

    return failures == 0 ? 0 : 1;
}
