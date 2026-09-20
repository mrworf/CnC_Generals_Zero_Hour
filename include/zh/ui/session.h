#pragma once

#include "zh/platform/sdl_platform.h"
#include "zh/ui/font.h"
#include "zh/ui/renderer.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace zh::ui {

enum class TextField { none, save_name, player_name, game_name, chat };
enum class Flow { main_menu, loading, tooltip, subtitle, caption, save_name, player_name, game_name, chat };

class UiSession {
public:
    UiSession(UiRecorder& recorder, TextLayout layout);

    renderer::ValidationResult handle(const platform::PlatformEvent& event);
    renderer::ValidationResult record_flow(Flow flow, std::string_view text = {});
    void focus_text_field(TextField field);

    TextField focused_field() const noexcept { return focused_field_; }
    std::size_t selected_menu_item() const noexcept { return selected_menu_item_; }
    const std::string& text() const noexcept { return text_; }
    std::size_t cursor_byte() const noexcept { return cursor_byte_; }
    const std::string& composition() const noexcept { return composition_; }
    bool focused() const noexcept { return focused_; }
    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    const std::string& last_action() const noexcept { return last_action_; }
    const std::string& last_error() const noexcept { return last_error_; }

private:
    renderer::ValidationResult fail(std::string message);

    UiRecorder& recorder_;
    TextLayout layout_;
    TextField focused_field_ = TextField::none;
    std::size_t selected_menu_item_ = 0;
    std::string text_;
    std::size_t cursor_byte_ = 0;
    std::string composition_;
    int composition_start_ = -1;
    int composition_length_ = -1;
    bool focused_ = true;
    int width_ = 1280;
    int height_ = 720;
    std::string last_action_;
    std::string last_error_;
};

std::string_view flow_name(Flow flow) noexcept;

} // namespace zh::ui
