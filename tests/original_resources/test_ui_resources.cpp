#include "zh/data/vfs.h"
#include "zh/original_resources.h"
#include "zh/renderer/recording_device.h"
#include "zh/ui/font.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {
int failures = 0;
void check(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

void write_text(const std::filesystem::path& path, std::string_view text)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary).write(text.data(), static_cast<std::streamsize>(text.size()));
}

void install_callbacks(zh::original_resources::UiResources& ui, int& calls)
{
    for (const auto* name : {"InitBlank", "UpdateBlank", "ShutdownBlank", "DrawBlank", "PushButtonInput", "DrawButton", "DrawText"})
        check(ui.register_callback(name, [&](std::string_view) { ++calls; }), "callback registration failed");
}

bool only_callbacks(const zh::original_resources::OwnershipCounts& counts)
{
    return counts.callbacks == 7 && counts.assets == 0 && counts.layouts == 0 && counts.windows == 0 &&
        counts.fonts == 0 && counts.device_acquisitions == 0;
}
}

int main()
{
    using namespace zh::original_resources;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-original-resources-ui-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh_root = root / "zh";
    const auto generals_root = root / "generals";
    std::filesystem::create_directories(generals_root);
    write_text(zh_root / "Data/INI/MappedImages.ini",
        "IMAGE BlankImage Art/blank.tga 64 64 0 0 64 64 NORMAL\n"
        "IMAGE ButtonImage Art/buttons.tga 128 64 0 0 64 32 ROTATED_90_CLOCKWISE\n");
    write_text(zh_root / "Menus/BlankWindow.wnd",
        "LAYOUT InitBlank UpdateBlank ShutdownBlank\n"
        "WINDOW BlankWindow ROOT 0 0 800 600 BlankImage DrawBlank\n"
        "WINDOW ContinueButton BUTTON 20 20 160 40 ButtonImage DrawButton Continue\n"
        "WINDOW Caption STATICTEXT 20 80 300 40 BlankImage DrawText Ready Commander\n");
    write_text(zh_root / "Menus/MissingCallback.wnd",
        "LAYOUT InitBlank UpdateBlank ShutdownBlank\n"
        "WINDOW Broken ROOT 0 0 800 600 BlankImage UnknownCallback\n");
    write_text(zh_root / "Menus/BadControl.wnd",
        "LAYOUT InitBlank UpdateBlank ShutdownBlank\n"
        "WINDOW Broken BROWSER 0 0 800 600 BlankImage DrawBlank\n");
    write_text(zh_root / "Data/English/Language.ini",
        "UnicodeFontName = sans-serif\nLocalFontFile = Data/English/Local.ttf\n");
    const auto system_font = zh::ui::FontFace::resolve_system_font("sans-serif");
    std::filesystem::create_directories(zh_root / "Data/English");
    std::filesystem::copy_file(system_font, zh_root / "Data/English/Local.ttf");

    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    {
        zh::renderer::RecordingGpuDevice device;
        UiResources ui(vfs, device);
        int calls = 0;
        install_callbacks(ui, calls);
        check(!ui.register_callback("DrawBlank", [&](std::string_view) { ++calls; }),
            "callback replacement accepted");
        check(ui.last_error().find("replacement") != std::string_view::npos, "replacement diagnostic missing");
        check(ui.load("Data/INI/MappedImages.ini", "Menus/BlankWindow.wnd", "Data/English/Language.ini"), ui.last_error());
        const auto live = ui.counts();
        check(live.assets == 2 && live.layouts == 1 && live.windows == 3 && live.fonts == 1 && live.callbacks == 7,
            "original UI ownership counts wrong");
        check(live.device_acquisitions == 0, "UI consumer acquired a physical device");
        check(ui.record(), ui.last_error());
        check(ui.invoke("ContinueButton") && calls == 1, "WND callback did not execute");
        check(!ui.invoke("Missing") && ui.last_error().find("unknown original WND window") != std::string_view::npos,
            "unknown window callback was accepted");
        const auto snapshot = device.snapshot();
        check(snapshot.find("original WND layout callbacks=InitBlank,UpdateBlank,ShutdownBlank") != std::string::npos,
            "BlankWindow layout callback producer absent");
        check(snapshot.find("original image=ButtonImage") != std::string::npos &&
              snapshot.find("size=32x64 rotated=true") != std::string::npos,
            "rotated original image dimensions were not observed");
        check(snapshot.find("ui-element label=ContinueButton") != std::string::npos,
            "representative child/control did not reach recorder");
        ui.shutdown();
        ui.shutdown();
        check(ui.counts().callbacks == 0 && ui.counts().windows == 0 && ui.counts().fonts == 0,
            "repeated UI shutdown retained ownership");
        check(device.resource_counts().total() == 0, "UI shutdown retained recording resources");
    }

    for (std::size_t stage = 1; stage <= 4; ++stage) {
        zh::renderer::RecordingGpuDevice device;
        UiResources ui(vfs, device);
        int calls = 0; install_callbacks(ui, calls);
        check(!ui.load("Data/INI/MappedImages.ini", "Menus/BlankWindow.wnd", "Data/English/Language.ini", stage),
            "injected UI failure was accepted");
        check(ui.last_error().find("injected original UI failure") != std::string_view::npos,
            "injected UI diagnostic missing");
        check(only_callbacks(ui.counts()), "partial UI failure changed callback baseline or retained resources");
        check(device.resource_counts().total() == 0, "partial UI failure retained recording resources");
        ui.shutdown();
        check(ui.counts().callbacks == 0, "failed UI shutdown retained callbacks");
    }

    for (const auto& item : {std::pair{"Menus/MissingCallback.wnd", "missing original WND callback"},
                             std::pair{"Menus/BadControl.wnd", "unsupported original WND control"},
                             std::pair{"Menus/Missing.wnd", "missing original logical file"}}) {
        zh::renderer::RecordingGpuDevice device;
        UiResources ui(vfs, device);
        int calls = 0; install_callbacks(ui, calls);
        check(!ui.load("Data/INI/MappedImages.ini", item.first, "Data/English/Language.ini"),
            "invalid WND resource accepted");
        check(ui.last_error().find(item.second) != std::string_view::npos, "invalid WND diagnostic missing");
        check(only_callbacks(ui.counts()) && device.resource_counts().total() == 0,
            "invalid WND retained resources");
    }
    check(std::string(provider_ui_identity()) == "OriginalUiResources.cpp", "UI provider witness");
    std::filesystem::remove_all(root, ignored);
    if (failures != 0) return 1;
    std::cout << "original-resources UI: ok providers=" << provider_ui_identity()
              << " layouts=1 windows=3 callbacks=7 devices=0\n";
    return 0;
}
