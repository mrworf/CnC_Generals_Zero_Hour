#include "zh/platform/sdl_platform.h"

#include <SDL3/SDL.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

SDL_Event event_of(SDL_EventType type)
{
    SDL_Event event{};
    event.type = type;
    event.common.timestamp = SDL_GetTicksNS();
    return event;
}

} // namespace

int main(int argc, char** argv)
{
    const bool expect_no_display = argc == 2 && std::string(argv[1]) == "--expect-no-display";
    if (argc > 2 || (argc == 2 && !expect_no_display)) {
        std::cerr << "usage: zh_platform_smoke [--expect-no-display]\n";
        return 2;
    }

    try {
        zh::platform::WindowOptions options;
        options.title = "Zero Hour M6 platform smoke";
        options.width = 640;
        options.height = 480;
        zh::platform::SdlWindow window(options);
        if (expect_no_display) {
            std::cerr << "expected SDL video/window creation to fail without a display, but it succeeded\n";
            return 1;
        }

        std::cout << "platform-smoke: video-driver=" << window.video_driver() << '\n';
        (void)window.poll_events();

        window.start_text_input();
        require(window.text_input_active(), "text input did not start");
        std::cout << "platform-smoke: text-input=active\n";

        const std::string clipboard = "Zero Hour Gr\xC3\xBC\xC3\x9F\x65 \xF0\x9F\x8C\x8D";
        window.set_clipboard_text(clipboard);
        require(window.clipboard_text() == clipboard, "UTF-8 clipboard round trip differed");
        std::cout << "platform-smoke: clipboard=utf8-round-trip\n";

        window.set_cursor_visible(false);
        window.set_cursor_visible(true);
        window.set_cursor_confined(true);
        const bool compositor_confined = window.cursor_confined();
        window.set_cursor_confined(false);
        std::cout << "platform-smoke: cursor=hide-show confinement=requested compositor-state="
                  << (compositor_confined ? "engaged" : "deferred-until-focus") << '\n';

        window.set_relative_capture(true);
        require(window.relative_capture(), "relative mouse mode did not engage");

        auto key = event_of(SDL_EVENT_KEY_DOWN);
        key.key.scancode = SDL_SCANCODE_W;
        key.key.key = SDLK_Z;
        require(window.translate_event(key)->physical_key.value == SDL_SCANCODE_W,
            "keyboard scancode translation failed");
        auto button = event_of(SDL_EVENT_MOUSE_BUTTON_DOWN);
        button.button.button = SDL_BUTTON_LEFT;
        button.button.clicks = 2;
        require(window.translate_event(button)->click_count == 2, "mouse/double-click translation failed");
        auto motion = event_of(SDL_EVENT_MOUSE_MOTION);
        motion.motion.x = 200.0F;
        motion.motion.y = 120.0F;
        motion.motion.xrel = 4.0F;
        motion.motion.yrel = -3.0F;
        require(window.translate_event(motion)->delta_x == 4.0F, "mouse motion translation failed");
        auto wheel = event_of(SDL_EVENT_MOUSE_WHEEL);
        wheel.wheel.y = 1.0F;
        wheel.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
        require(window.translate_event(wheel)->delta_y == 1.0F, "mouse wheel translation failed");
        auto text = event_of(SDL_EVENT_TEXT_INPUT);
        text.text.text = "\xC3\xA9";
        require(window.translate_event(text)->text == text.text.text, "UTF-8 text translation failed");
        auto editing = event_of(SDL_EVENT_TEXT_EDITING);
        editing.edit.text = "\xE6\xBC\xA2\xE5\xAD\x97";
        editing.edit.start = 0;
        editing.edit.length = 2;
        require(window.translate_event(editing)->editing_length == 2, "IME composition translation failed");
        std::cout << "platform-smoke: keyboard=physical mouse=motion-button-wheel text=utf8 composition=ime\n";

        auto focus = event_of(SDL_EVENT_WINDOW_FOCUS_LOST);
        window.translate_event(focus);
        require(!window.input().key_held({SDL_SCANCODE_W}), "focus loss retained held key");
        require(!window.input().mouse_button_held(SDL_BUTTON_LEFT), "focus loss retained held button");
        require(!window.input().relative_capture() && !window.relative_capture(),
            "focus loss retained relative capture");
        std::cout << "platform-smoke: focus-loss=held-state-cleared\n";

        window.resize(800, 600);
        require(window.width() == 800 && window.height() == 600, "window resize did not apply");
        window.set_fullscreen(true);
        require(window.fullscreen(), "fullscreen did not engage");
        window.set_fullscreen(false);
        require(!window.fullscreen(), "fullscreen did not disengage");
        std::cout << "platform-smoke: resize=800x600 fullscreen=toggle\n";

        auto close = event_of(SDL_EVENT_WINDOW_CLOSE_REQUESTED);
        require(window.translate_event(close)->type == zh::platform::EventType::quit, "close did not request quit");
        window.stop_text_input();
        std::cout << "platform-smoke: shutdown=clean gpu-device=not-created\n";
        return 0;
    } catch (const zh::platform::PlatformError& error) {
        if (expect_no_display) {
            std::cout << "platform-smoke: expected-no-display-error: " << error.what() << '\n';
            return std::string_view(error.what()).find("SDL video") != std::string_view::npos ||
                    std::string_view(error.what()).find("SDL window") != std::string_view::npos
                ? 0
                : 1;
        }
        std::cerr << "platform-smoke: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "platform-smoke: " << error.what() << '\n';
        return 1;
    }
}
