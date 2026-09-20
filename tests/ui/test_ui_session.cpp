#include "zh/ui/session.h"

#include <SDL3/SDL_scancode.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

zh::ui::FontFace test_face()
{
    const auto path = zh::ui::FontFace::resolve_system_font("sans-serif");
    std::ifstream input(path, std::ios::binary);
    auto bytes = std::make_shared<std::vector<std::uint8_t>>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return zh::ui::FontFace::from_memory(bytes, 18, "Data/English/SessionTest.ttf");
}

zh::platform::PlatformEvent key(SDL_Scancode scancode)
{
    zh::platform::PlatformEvent event;
    event.type = zh::platform::EventType::key_down;
    event.physical_key.value = static_cast<std::uint16_t>(scancode);
    return event;
}

void test_physical_navigation_and_local_internet()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::UiSession session(recorder, zh::ui::TextLayout(test_face()));
    check(session.handle(key(SDL_SCANCODE_DOWN)), "down navigation failed");
    check(session.handle(key(SDL_SCANCODE_DOWN)), "second down navigation failed");
    check(session.handle(key(SDL_SCANCODE_DOWN)), "third down navigation failed");
    check(session.selected_menu_item() == 3, "physical scancode did not select Internet item");
    check(session.handle(key(SDL_SCANCODE_RETURN)), "activation failed");
    check(session.last_action() == "Internet services are unavailable in this build", "Internet did not resolve locally");
    check(session.handle(key(SDL_SCANCODE_A)), "printable key event failed");
    check(session.text().empty(), "physical key synthesized text");
}

void test_text_editing_composition_focus_and_boundaries()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::UiSession session(recorder, zh::ui::TextLayout(test_face()));
    session.focus_text_field(zh::ui::TextField::chat);
    zh::platform::PlatformEvent edit;
    edit.type = zh::platform::EventType::text_editing;
    edit.text = "ni"; edit.editing_start = 0; edit.editing_length = 2;
    check(session.handle(edit) && session.composition() == "ni", "IME pre-edit failed");
    zh::platform::PlatformEvent input;
    input.type = zh::platform::EventType::text_input;
    input.text = "N\xc3\xad";
    check(session.handle(input), "UTF-8 commit failed");
    check(session.text() == "N\xc3\xad" && session.composition().empty(), "commit/composition state wrong");
    check(session.handle(key(SDL_SCANCODE_LEFT)), "cursor-left failed");
    input.text = "!";
    check(session.handle(input) && session.text() == "N!\xc3\xad", "cursor insertion failed");
    check(session.handle(key(SDL_SCANCODE_BACKSPACE)) && session.text() == "N\xc3\xad", "UTF-8 backspace failed");
    edit.text = "x"; edit.editing_start = 2; edit.editing_length = 0;
    check(!session.handle(edit), "out-of-range composition accepted");
    zh::platform::PlatformEvent lost; lost.type = zh::platform::EventType::focus_lost;
    check(session.handle(lost) && !session.focused() && session.composition().empty(), "focus loss did not clear transient state");
    input.text = "ignored";
    check(session.handle(input) && session.text() == "N\xc3\xad", "unfocused input changed text");
    zh::platform::PlatformEvent gained; gained.type = zh::platform::EventType::focus_gained;
    check(session.handle(gained) && session.focused(), "focus recovery failed");
    input.text = std::string(257, 'x');
    check(!session.handle(input) && session.last_error().find("256") != std::string::npos, "oversized input accepted");
}

void test_representative_flows_resize_and_snapshot()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::UiSession session(recorder, zh::ui::TextLayout(test_face()));
    const std::vector<std::pair<zh::ui::Flow, std::string>> flows{
        {zh::ui::Flow::main_menu, "Campaign Multiplayer Options"},
        {zh::ui::Flow::loading, "Loading operation"},
        {zh::ui::Flow::tooltip, "A tooltip long enough to wrap across lines for recorder evidence"},
        {zh::ui::Flow::subtitle, "Subtitle caf\xc3\xa9 with mixed text that wraps"},
        {zh::ui::Flow::caption, "Caption text"},
        {zh::ui::Flow::save_name, "A-save-name-that-needs-truncation"},
        {zh::ui::Flow::player_name, "Player One"},
        {zh::ui::Flow::game_name, "Generals Game"},
        {zh::ui::Flow::chat, std::string("missing ") + std::string("\xf4\x8f\xbf\xbf", 4)},
    };
    for (const auto& [flow, text] : flows) check(session.record_flow(flow, text), "representative flow failed");
    zh::platform::PlatformEvent resize; resize.type = zh::platform::EventType::window_resized;
    resize.width = 1024; resize.height = 768;
    check(session.handle(resize), "resize event failed");
    check(session.record_flow(zh::ui::Flow::main_menu, "Resized menu"), "resized flow failed");
    resize.width = 20000;
    check(!session.handle(resize), "oversized resize accepted");

    const auto snapshot = device.snapshot();
    std::ifstream expected(std::string(ZH_SOURCE_DIR) + "/tests/ui/snapshots/representative-ui.markers");
    check(static_cast<bool>(expected), "representative snapshot marker file missing");
    for (std::string marker; std::getline(expected, marker);)
        if (!marker.empty() && marker[0] != '#') check(snapshot.find(marker) != std::string::npos, marker.c_str());
    check(snapshot.find("flow=tooltip") < snapshot.find("flow=subtitle"), "flow order changed");
    check(snapshot.find("fallback=yes") != std::string::npos, "missing-glyph stream lacks fallback evidence");
    check(snapshot.find("ui-resize 1280x720 -> 1024x768") != std::string::npos, "session resize was not recorded");
}
} // namespace

int main()
{
    try {
        test_physical_navigation_and_local_internet();
        test_text_editing_composition_focus_and_boundaries();
        test_representative_flows_resize_and_snapshot();
        std::cout << "UI session tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
