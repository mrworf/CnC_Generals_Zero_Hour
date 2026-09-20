#include "zh/platform/sdl_platform.h"

#include "zh/foundation/unicode.h"

#include <SDL3/SDL.h>

#include <string>
#include <utility>

namespace zh::platform {
namespace {

[[noreturn]] void sdl_failure(const char* operation)
{
    const char* detail = SDL_GetError();
    throw PlatformError(std::string("SDL ") + operation + " failed: " +
        (detail != nullptr && *detail != '\0' ? detail : "unknown SDL error"));
}

void require_success(bool success, const char* operation)
{
    if (!success) sdl_failure(operation);
}

void validate_dimensions(int width, int height)
{
    if (width <= 0 || height <= 0) throw PlatformError("SDL window dimensions must be positive");
}

} // namespace

struct SdlWindow::Impl {
    SDL_Window* window = nullptr;
    bool video_initialized = false;
    bool text_input = false;

    ~Impl()
    {
        if (window != nullptr) {
            if (text_input) SDL_StopTextInput(window);
            (void)SDL_SetWindowRelativeMouseMode(window, false);
            (void)SDL_SetWindowMouseGrab(window, false);
            SDL_DestroyWindow(window);
        }
        if (video_initialized) SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
};

SdlWindow::SdlWindow(const WindowOptions& options) : impl_(std::make_unique<Impl>())
{
    validate_dimensions(options.width, options.height);
    if (options.title.empty()) throw PlatformError("SDL window title must not be empty");

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) sdl_failure("video initialization");
    impl_->video_initialized = true;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
    if (options.hidden) flags |= SDL_WINDOW_HIDDEN;
    impl_->window = SDL_CreateWindow(options.title.c_str(), options.width, options.height, flags);
    if (impl_->window == nullptr) sdl_failure("window creation");
}

SdlWindow::~SdlWindow() = default;

std::vector<PlatformEvent> SdlWindow::poll_events()
{
    std::vector<PlatformEvent> events;
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        auto translated = translate_event(event);
        if (translated) events.push_back(std::move(*translated));
    }
    return events;
}

std::optional<PlatformEvent> SdlWindow::translate_event(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST && SDL_GetWindowRelativeMouseMode(impl_->window)) {
        require_success(SDL_SetWindowRelativeMouseMode(impl_->window, false), "relative mouse release on focus loss");
    }
    return translator_.translate(event);
}

void SdlWindow::start_text_input()
{
    require_success(SDL_StartTextInput(impl_->window), "text input start");
    impl_->text_input = true;
}

void SdlWindow::stop_text_input()
{
    require_success(SDL_StopTextInput(impl_->window), "text input stop");
    impl_->text_input = false;
}

void SdlWindow::set_clipboard_text(std::string_view text)
{
    try {
        (void)foundation::utf8_to_utf16(text);
    } catch (const foundation::UnicodeError& error) {
        throw PlatformError("SDL clipboard text has malformed UTF-8: " + std::string(error.what()));
    }
    const std::string owned(text);
    require_success(SDL_SetClipboardText(owned.c_str()), "clipboard write");
}

std::string SdlWindow::clipboard_text() const
{
    char* text = SDL_GetClipboardText();
    if (text == nullptr) sdl_failure("clipboard read");
    std::string result(text);
    SDL_free(text);
    try {
        (void)foundation::utf8_to_utf16(result);
    } catch (const foundation::UnicodeError& error) {
        throw PlatformError("SDL clipboard returned malformed UTF-8: " + std::string(error.what()));
    }
    return result;
}

void SdlWindow::set_cursor_visible(bool visible)
{
    require_success(visible ? SDL_ShowCursor() : SDL_HideCursor(), visible ? "cursor show" : "cursor hide");
}

void SdlWindow::set_cursor_confined(bool confined)
{
    require_success(SDL_SetWindowMouseGrab(impl_->window, confined), "cursor confinement");
}

void SdlWindow::set_relative_capture(bool enabled)
{
    require_success(SDL_SetWindowRelativeMouseMode(impl_->window, enabled), "relative mouse mode");
    translator_.set_relative_capture(enabled);
}

void SdlWindow::resize(int width, int height)
{
    validate_dimensions(width, height);
    require_success(SDL_SetWindowSize(impl_->window, width, height), "window resize");
    require_success(SDL_SyncWindow(impl_->window), "window resize synchronization");
}

void SdlWindow::set_fullscreen(bool enabled)
{
    require_success(SDL_SetWindowFullscreen(impl_->window, enabled), "fullscreen change");
    require_success(SDL_SyncWindow(impl_->window), "fullscreen synchronization");
}

std::string SdlWindow::video_driver() const
{
    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver == nullptr || *driver == '\0') throw PlatformError("SDL video driver is unavailable after window creation");
    return driver;
}

bool SdlWindow::text_input_active() const noexcept
{
    return impl_->text_input;
}

bool SdlWindow::fullscreen() const noexcept
{
    return (SDL_GetWindowFlags(impl_->window) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool SdlWindow::cursor_confined() const noexcept
{
    return SDL_GetWindowMouseGrab(impl_->window);
}

bool SdlWindow::relative_capture() const noexcept
{
    return SDL_GetWindowRelativeMouseMode(impl_->window);
}

int SdlWindow::width() const noexcept
{
    int width = 0;
    (void)SDL_GetWindowSize(impl_->window, &width, nullptr);
    return width;
}

int SdlWindow::height() const noexcept
{
    int height = 0;
    (void)SDL_GetWindowSize(impl_->window, nullptr, &height);
    return height;
}

} // namespace zh::platform
