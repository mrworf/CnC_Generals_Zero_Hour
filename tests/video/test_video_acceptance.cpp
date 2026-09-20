#include "zh/video/resolver.h"
#include "zh/video/decoder.h"
#include "zh/data/vfs.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
template <typename Function>
void expect_error(Function&& function, std::string_view expected)
{
    try { function(); check(false, "expected video resolver error"); }
    catch (const std::exception& error) {
        check(std::string_view(error.what()).find(expected) != std::string_view::npos, "actionable resolver error");
    }
}
void write(const std::filesystem::path& path)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary); output << "fixture marker only";
}
}

int main()
{
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / ("zh-video-acceptance-" + std::to_string(::getpid()));
    std::error_code ignored; fs::remove_all(root, ignored);
    const auto zh_root = root / "zh"; const auto generals_root = root / "generals";
    write(zh_root / "Data/English/Movies/Localized.bik");
    write(zh_root / "Data/Movies/Fallback.bik");
    fs::create_directories(generals_root);
    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});

    check(zh::video::resolve_localized_movie(vfs, "Movies/Localized.bik", "English")
        == "Data/English/Movies/Localized.bik", "selected locale takes precedence");
    check(zh::video::resolve_localized_movie(vfs, "Movies/FALLBACK.BIK", "English")
        == "Data/Movies/Fallback.bik", "neutral fallback is case-insensitive through VFS");
    expect_error([&] { (void)zh::video::resolve_localized_movie(vfs, "Movies/Missing.bik", "English"); }, "fallback");
    expect_error([&] { (void)zh::video::localized_movie_candidates("../secret.bik", "English"); }, "safe");
    expect_error([&] { (void)zh::video::localized_movie_candidates("Movies/Test.bik", "../English"); }, "language");

    fs::remove_all(root, ignored);
    return failures == 0 ? 0 : 1;
}
