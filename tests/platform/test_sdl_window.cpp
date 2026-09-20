#include "zh/platform/sdl_platform.h"

#include <cstdlib>
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

} // namespace

int main()
{
    zh::platform::WindowOptions invalid;
    invalid.width = 0;
    expect_error([&] { zh::platform::SdlWindow window(invalid); }, "dimensions must be positive");

    invalid.width = 640;
    invalid.title.clear();
    expect_error([&] { zh::platform::SdlWindow window(invalid); }, "title must not be empty");

    setenv("SDL_VIDEODRIVER", "__zh_missing_display__", 1);
    zh::platform::WindowOptions options;
    options.hidden = true;
    expect_error([&] { zh::platform::SdlWindow window(options); }, "SDL video initialization failed:");
    unsetenv("SDL_VIDEODRIVER");

    return failures == 0 ? 0 : 1;
}
