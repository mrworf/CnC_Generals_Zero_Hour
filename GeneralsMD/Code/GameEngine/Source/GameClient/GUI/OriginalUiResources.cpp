/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M28 extraction provenance:
// Image::parseImageCoords/parseImageStatus, FontLibrary::getFont,
// WindowLayout::load/destroyWindows and GameWindowManager script callback
// resolution remain authoritative. This unit binds those CPU semantics to the
// accepted VFS, retained FontFace and UiRecorder providers. Complete GameClient
// registration and interactive callbacks stay in M20/M23/M25.

#include "zh/original_resources.h"

#include "zh/data/vfs.h"
#include "zh/original_data.h"
#include "zh/ui/font.h"
#include "zh/ui/renderer.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <map>
#include <sstream>
#include <utility>

namespace zh::original_resources {
namespace {

std::vector<std::string> lines(const std::vector<std::uint8_t>& bytes)
{
    if (bytes.size() > 1024U * 1024U) throw Error("original UI resource exceeds 1 MiB limit");
    std::vector<std::string> result;
    std::istringstream stream(std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    for (std::string line; std::getline(stream, line);) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.size() > 4096) throw Error("original UI resource line exceeds 4096 bytes");
        const auto first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == ';' || line[first] == '#') continue;
        line.erase(0, first);
        result.push_back(std::move(line));
        if (result.size() > 4096) throw Error("original UI resource exceeds 4096 records");
    }
    return result;
}

int integer(std::string_view text, std::string_view field)
{
    int value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size())
        throw Error("invalid original UI integer for " + std::string(field));
    return value;
}

struct ImageDefinition {
    std::string name;
    std::string texture;
    int texture_width = 0;
    int texture_height = 0;
    int left = 0, top = 0, right = 0, bottom = 0;
    bool rotated = false;
};

std::map<std::string, ImageDefinition> parse_images(const std::vector<std::uint8_t>& bytes)
{
    std::map<std::string, ImageDefinition> images;
    for (const auto& line : lines(bytes)) {
        std::istringstream input(line);
        std::string keyword;
        input >> keyword;
        if (keyword != "IMAGE") throw Error("unsupported original image definition '" + keyword + "'");
        ImageDefinition image;
        std::string rotated;
        if (!(input >> image.name >> image.texture >> image.texture_width >> image.texture_height
              >> image.left >> image.top >> image.right >> image.bottom >> rotated))
            throw Error("malformed original image definition");
        std::string extra;
        if (input >> extra) throw Error("unexpected original image definition field");
        if (image.name.empty() || image.texture.empty() || image.texture_width <= 0 || image.texture_height <= 0 ||
            image.right <= image.left || image.bottom <= image.top || image.left < 0 || image.top < 0 ||
            image.right > image.texture_width || image.bottom > image.texture_height)
            throw Error("invalid original image coordinates for '" + image.name + "'");
        if (rotated == "NORMAL") image.rotated = false;
        else if (rotated == "ROTATED_90_CLOCKWISE") image.rotated = true;
        else throw Error("unsupported original image status for '" + image.name + "'");
        if (!images.emplace(image.name, image).second)
            throw Error("duplicate original image '" + image.name + "'");
    }
    if (images.empty()) throw Error("original image collection is empty");
    return images;
}

struct WindowDefinition {
    std::string name;
    std::string type;
    ui::Rect bounds;
    std::string image;
    std::string callback;
    std::string text;
};

struct LayoutDefinition {
    std::string init, update, shutdown;
    std::vector<WindowDefinition> windows;
};

LayoutDefinition parse_layout(const std::vector<std::uint8_t>& bytes)
{
    LayoutDefinition layout;
    for (const auto& line : lines(bytes)) {
        std::istringstream input(line);
        std::string keyword;
        input >> keyword;
        if (keyword == "LAYOUT") {
            std::string extra;
            if (!(input >> layout.init >> layout.update >> layout.shutdown) || input >> extra)
                throw Error("malformed original WND layout callbacks");
            continue;
        }
        if (keyword != "WINDOW") throw Error("unsupported original WND record '" + keyword + "'");
        WindowDefinition window;
        std::string x, y, width, height;
        if (!(input >> window.name >> window.type >> x >> y >> width >> height >> window.image >> window.callback))
            throw Error("malformed original WND window");
        std::getline(input, window.text);
        const auto first = window.text.find_first_not_of(" \t");
        window.text = first == std::string::npos ? std::string{} : window.text.substr(first);
        if (window.name.empty() || (window.type != "ROOT" && window.type != "BUTTON" && window.type != "STATICTEXT"))
            throw Error("unsupported original WND control '" + window.type + "'");
        window.bounds = {static_cast<float>(integer(x, "x")), static_cast<float>(integer(y, "y")),
            static_cast<float>(integer(width, "width")), static_cast<float>(integer(height, "height"))};
        if (window.bounds.width <= 0 || window.bounds.height <= 0)
            throw Error("invalid original WND bounds for '" + window.name + "'");
        if (std::any_of(layout.windows.begin(), layout.windows.end(), [&](const auto& item) { return item.name == window.name; }))
            throw Error("duplicate original WND window '" + window.name + "'");
        layout.windows.push_back(std::move(window));
    }
    if (layout.init.empty() || layout.update.empty() || layout.shutdown.empty() || layout.windows.empty())
        throw Error("original WND layout is incomplete");
    return layout;
}

} // namespace

struct UiResources::Impl {
    Impl(const data::VirtualFileSystem& mounted, renderer::GpuDevice& target)
        : vfs(mounted), files(mounted), device(target) {}

    const data::VirtualFileSystem& vfs;
    original_data::LogicalFiles files;
    renderer::GpuDevice& device;
    std::map<std::string, WindowCallback> callbacks;
    std::map<std::string, ImageDefinition> images;
    LayoutDefinition layout;
    ui::FontFace font;
    std::unique_ptr<ui::UiRecorder> recorder;
    OwnershipCounts ownership;
    std::string error;
    bool loaded = false;

    void clear_loaded() noexcept
    {
        recorder.reset();
        font = {};
        layout = {};
        images.clear();
        ownership.layouts = ownership.windows = ownership.fonts = ownership.assets = 0;
        loaded = false;
    }

    bool fail(std::string message, bool clear_callbacks = false)
    {
        error = std::move(message);
        clear_loaded();
        if (clear_callbacks) { callbacks.clear(); ownership.callbacks = 0; }
        return false;
    }
};

UiResources::UiResources(const data::VirtualFileSystem& vfs, renderer::GpuDevice& recorder)
    : impl_(std::make_unique<Impl>(vfs, recorder)) {}
UiResources::~UiResources() { shutdown(); }

bool UiResources::register_callback(std::string name, WindowCallback callback)
{
    if (impl_->loaded) { impl_->error = "callbacks cannot change while an original layout is active"; return false; }
    if (name.empty() || !callback) { impl_->error = "original WND callback name/function must be present"; return false; }
    if (!impl_->callbacks.emplace(std::move(name), std::move(callback)).second) {
        impl_->error = "original WND callback replacement is not permitted";
        return false;
    }
    impl_->ownership.callbacks = impl_->callbacks.size();
    return true;
}

bool UiResources::load(std::string_view image_definitions, std::string_view layout,
    std::string_view language, std::size_t fail_after_stage)
{
    if (impl_->loaded) return impl_->fail("original UI resources are already loaded");
    impl_->error.clear();
    try {
        std::size_t stage = 0;
        const auto checkpoint = [&] {
            ++stage;
            if (stage == fail_after_stage) throw Error("injected original UI failure at stage " + std::to_string(stage));
        };
        auto images = parse_images(impl_->files.read(image_definitions)); checkpoint();
        auto parsed_layout = parse_layout(impl_->files.read(layout)); checkpoint();
        const auto language_bytes = impl_->files.read(language);
        const auto selection = ui::parse_language_font_selection(
            std::string_view(reinterpret_cast<const char*>(language_bytes.data()), language_bytes.size()));
        auto font = ui::load_selected_font(selection, &impl_->vfs, 16); checkpoint();
        const ui::TextLayout text_layout(font);
        for (const auto& callback : {parsed_layout.init, parsed_layout.update, parsed_layout.shutdown})
            if (impl_->callbacks.find(callback) == impl_->callbacks.end())
                throw Error("missing original WND layout callback '" + callback + "'");
        for (const auto& window : parsed_layout.windows) {
            if (images.find(window.image) == images.end())
                throw Error("missing original WND image '" + window.image + "'");
            if (impl_->callbacks.find(window.callback) == impl_->callbacks.end())
                throw Error("missing original WND callback '" + window.callback + "'");
            if (!window.text.empty())
                (void)text_layout.layout(window.text, {static_cast<int>(window.bounds.width), 4, true, false, 256, 256});
        }
        checkpoint();
        impl_->images = std::move(images);
        impl_->layout = std::move(parsed_layout);
        impl_->font = std::move(font);
        impl_->recorder = std::make_unique<ui::UiRecorder>(impl_->device);
        impl_->ownership.assets = impl_->images.size();
        impl_->ownership.layouts = 1;
        impl_->ownership.windows = impl_->layout.windows.size();
        impl_->ownership.fonts = 1;
        impl_->loaded = true;
        return true;
    } catch (const std::exception& exception) {
        return impl_->fail(exception.what());
    }
}

bool UiResources::record()
{
    if (!impl_->loaded) { impl_->error = "original UI resources are not loaded"; return false; }
    ui::Scene scene;
    scene.width = 800; scene.height = 600;
    for (const auto& window : impl_->layout.windows) {
        const auto& image = impl_->images.at(window.image);
        const int image_width = image.rotated ? image.bottom - image.top : image.right - image.left;
        const int image_height = image.rotated ? image.right - image.left : image.bottom - image.top;
        impl_->device.record_marker("original image=" + image.name + " uv=" +
            std::to_string(image.left) + "," + std::to_string(image.top) + "," +
            std::to_string(image.right) + "," + std::to_string(image.bottom) +
            " size=" + std::to_string(image_width) + "x" + std::to_string(image_height) +
            " rotated=" + (image.rotated ? "true" : "false"));
        scene.elements.push_back({window.name, window.bounds, {0, 0, 800, 600}, 0xffffffffU,
            ui::BlendMode::alpha, window.type == "ROOT" ? ui::Layer::background : ui::Layer::content});
    }
    impl_->device.record_marker("original WND layout callbacks=" + impl_->layout.init + "," +
        impl_->layout.update + "," + impl_->layout.shutdown + " font=" + impl_->font.source());
    const auto result = impl_->recorder->record(scene);
    if (!result) { impl_->error = result.error; return false; }
    return true;
}

bool UiResources::invoke(std::string_view window_name)
{
    if (!impl_->loaded) { impl_->error = "original UI resources are not loaded"; return false; }
    const auto window = std::find_if(impl_->layout.windows.begin(), impl_->layout.windows.end(),
        [&](const auto& value) { return value.name == window_name; });
    if (window == impl_->layout.windows.end()) { impl_->error = "unknown original WND window '" + std::string(window_name) + "'"; return false; }
    impl_->callbacks.at(window->callback)(window->name);
    return true;
}

void UiResources::shutdown() noexcept
{
    impl_->clear_loaded();
    impl_->callbacks.clear();
    impl_->ownership = {};
}
bool UiResources::ready() const noexcept { return impl_->loaded; }
OwnershipCounts UiResources::counts() const noexcept { return impl_->ownership; }
std::string_view UiResources::last_error() const noexcept { return impl_->error; }
const char* provider_ui_identity() noexcept { return "OriginalUiResources.cpp"; }

} // namespace zh::original_resources
