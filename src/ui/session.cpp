#include "zh/ui/session.h"

#include "zh/foundation/unicode.h"

#include <SDL3/SDL_scancode.h>

#include <algorithm>
#include <utility>

namespace zh::ui {
namespace {

constexpr std::size_t maximum_text_bytes = 256;
constexpr std::size_t menu_item_count = 5;

bool valid_utf8(std::string_view value)
{
    try {
        (void)foundation::utf8_to_utf16(value);
        return true;
    } catch (const foundation::UnicodeError&) {
        return false;
    }
}

std::size_t scalar_count(std::string_view value)
{
    std::size_t count = 0;
    for (const unsigned char byte : value)
        if ((byte & 0xc0U) != 0x80U) ++count;
    return count;
}

std::size_t previous_scalar(std::string_view value, std::size_t position)
{
    if (position == 0) return 0;
    --position;
    while (position > 0 && (static_cast<unsigned char>(value[position]) & 0xc0U) == 0x80U) --position;
    return position;
}

std::size_t next_scalar(std::string_view value, std::size_t position)
{
    if (position >= value.size()) return value.size();
    ++position;
    while (position < value.size() && (static_cast<unsigned char>(value[position]) & 0xc0U) == 0x80U) ++position;
    return position;
}

TextField flow_field(Flow flow)
{
    switch (flow) {
    case Flow::save_name: return TextField::save_name;
    case Flow::player_name: return TextField::player_name;
    case Flow::game_name: return TextField::game_name;
    case Flow::chat: return TextField::chat;
    default: return TextField::none;
    }
}

} // namespace

std::string_view flow_name(Flow flow) noexcept
{
    switch (flow) {
    case Flow::main_menu: return "menu";
    case Flow::loading: return "loading";
    case Flow::tooltip: return "tooltip";
    case Flow::subtitle: return "subtitle";
    case Flow::caption: return "caption";
    case Flow::save_name: return "save-name";
    case Flow::player_name: return "player-name";
    case Flow::game_name: return "game-name";
    case Flow::chat: return "chat";
    }
    return "unknown";
}

UiSession::UiSession(UiRecorder& recorder, TextLayout layout)
    : recorder_(recorder), layout_(std::move(layout)) {}

renderer::ValidationResult UiSession::fail(std::string message)
{
    last_error_ = "ui session: " + std::move(message);
    return {false, last_error_};
}

void UiSession::focus_text_field(TextField field)
{
    focused_field_ = field;
    text_.clear();
    cursor_byte_ = 0;
    composition_.clear();
    composition_start_ = composition_length_ = -1;
}

renderer::ValidationResult UiSession::handle(const platform::PlatformEvent& event)
{
    last_error_.clear();
    switch (event.type) {
    case platform::EventType::focus_lost:
        focused_ = false;
        composition_.clear();
        composition_start_ = composition_length_ = -1;
        last_action_ = "focus-lost: transient input cleared";
        return {};
    case platform::EventType::focus_gained:
        focused_ = true;
        last_action_ = "focus-gained";
        return {};
    case platform::EventType::window_resized:
        if (event.width <= 0 || event.height <= 0 || event.width > 16384 || event.height > 16384)
            return fail("resize width/height must be within 1..16384");
        width_ = event.width;
        height_ = event.height;
        last_action_ = "resize " + std::to_string(width_) + "x" + std::to_string(height_);
        return {};
    default:
        break;
    }
    if (!focused_) return {};

    if (event.type == platform::EventType::text_editing) {
        if (focused_field_ == TextField::none) return fail("composition requires a focused text field");
        if (!valid_utf8(event.text)) return fail("composition contains malformed UTF-8");
        const auto count = scalar_count(event.text);
        const bool clear = event.editing_start == -1 && event.editing_length == -1;
        if (!clear && (event.editing_start < 0 || event.editing_length < 0
                || static_cast<std::size_t>(event.editing_start) > count
                || static_cast<std::size_t>(event.editing_length) > count - static_cast<std::size_t>(event.editing_start)))
            return fail("composition start/length is outside pre-edit text");
        composition_ = clear ? std::string{} : event.text;
        composition_start_ = clear ? -1 : event.editing_start;
        composition_length_ = clear ? -1 : event.editing_length;
        last_action_ = clear ? "composition-cleared" : "composition-updated";
        return {};
    }
    if (event.type == platform::EventType::text_input) {
        if (focused_field_ == TextField::none) return fail("text input requires a focused text field");
        if (!valid_utf8(event.text)) return fail("text input contains malformed UTF-8");
        if (event.text.find('\0') != std::string::npos) return fail("text input contains NUL");
        if (text_.size() + event.text.size() > maximum_text_bytes) return fail("text field exceeds 256 UTF-8 bytes");
        text_.insert(cursor_byte_, event.text);
        cursor_byte_ += event.text.size();
        composition_.clear();
        composition_start_ = composition_length_ = -1;
        last_action_ = "text-committed";
        return {};
    }
    if (event.type == platform::EventType::mouse_button_down && event.mouse_button == 1) {
        last_action_ = "menu-activate " + std::to_string(selected_menu_item_);
        if (selected_menu_item_ == 3) last_action_ = recorder_.select_internet_action().explanation;
        return {};
    }
    if (event.type != platform::EventType::key_down || event.repeat) return {};
    const auto key = static_cast<SDL_Scancode>(event.physical_key.value);
    if (focused_field_ != TextField::none) {
        if (key == SDL_SCANCODE_LEFT) cursor_byte_ = previous_scalar(text_, cursor_byte_);
        else if (key == SDL_SCANCODE_RIGHT) cursor_byte_ = next_scalar(text_, cursor_byte_);
        else if (key == SDL_SCANCODE_BACKSPACE && cursor_byte_ != 0) {
            const auto previous = previous_scalar(text_, cursor_byte_);
            text_.erase(previous, cursor_byte_ - previous);
            cursor_byte_ = previous;
        } else if (key == SDL_SCANCODE_ESCAPE) {
            focused_field_ = TextField::none;
            composition_.clear();
        }
        last_action_ = "text-control " + std::to_string(event.physical_key.value);
        return {};
    }
    if (key == SDL_SCANCODE_UP) selected_menu_item_ = (selected_menu_item_ + menu_item_count - 1) % menu_item_count;
    else if (key == SDL_SCANCODE_DOWN) selected_menu_item_ = (selected_menu_item_ + 1) % menu_item_count;
    else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) {
        last_action_ = "menu-activate " + std::to_string(selected_menu_item_);
        if (selected_menu_item_ == 3) last_action_ = recorder_.select_internet_action().explanation;
        return {};
    }
    last_action_ = "menu-navigate " + std::to_string(selected_menu_item_);
    return {};
}

renderer::ValidationResult UiSession::record_flow(Flow flow, std::string_view text)
{
    last_error_.clear();
    LayoutOptions options;
    options.maximum_width = flow == Flow::tooltip ? 180 : (flow == Flow::subtitle || flow == Flow::caption ? 420 : 240);
    options.maximum_lines = flow == Flow::tooltip ? 3 : (flow == Flow::subtitle || flow == Flow::caption || flow == Flow::chat ? 2 : 1);
    options.wrap = flow == Flow::tooltip || flow == Flow::subtitle || flow == Flow::caption || flow == Flow::chat;
    options.truncate_with_ellipsis = !options.wrap;
    TextRun run;
    try {
        run = layout_.layout(text.empty() ? flow_name(flow) : text, options);
    } catch (const FontError& error) {
        return fail(std::string("layout for ") + std::string(flow_name(flow)) + " failed: " + error.what());
    }
    const auto field = flow_field(flow);
    const std::string details = " glyphs=" + std::to_string(run.glyphs.size()) + " lines=" + std::to_string(run.line_count)
        + " wrap=" + (run.wrapped ? "yes" : "no") + " truncate=" + (run.truncated ? "yes" : "no")
        + " fallback=" + (run.used_fallback ? "yes" : "no");
    Scene scene;
    scene.width = width_;
    scene.height = height_;
    scene.transition = flow == Flow::main_menu ? ScreenTransition::menu_enter
        : flow == Flow::loading ? ScreenTransition::loading_begin : ScreenTransition::none;
    const Rect viewport{0, 0, static_cast<float>(width_), static_cast<float>(height_)};
    scene.elements.push_back({"flow=" + std::string(flow_name(flow)) + details, {20, 20, 300, 40}, viewport,
        0xffffffffU, BlendMode::alpha, Layer::content});
    if (field != TextField::none)
        scene.elements.push_back({"text-field=" + std::string(flow_name(flow)), {20, 70, 320, 32}, viewport,
            0xffffffffU, BlendMode::alpha, Layer::overlay});
    scene.elements.push_back({focused_ ? "focus=focused" : "focus=unfocused", {0, 0, 1, 1}, viewport,
        0xffffffffU, BlendMode::opaque, Layer::background});
    scene.elements.push_back({"cursor", {100, 100, 16, 16}, viewport, 0xffffffffU, BlendMode::alpha, Layer::cursor});
    if (auto result = recorder_.record(scene); !result) return fail(result.error);
    return {};
}

} // namespace zh::ui
