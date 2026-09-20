#include "zh/ui/renderer.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

zh::ui::UiElement element(std::string label, zh::ui::Rect bounds, zh::ui::Layer layer,
    zh::ui::BlendMode blend = zh::ui::BlendMode::alpha)
{
    return {std::move(label), bounds, {0, 0, 640, 480}, 0xffffffffU, blend, layer};
}

void test_order_clip_blend_and_half_pixel()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::Scene scene{640, 480, zh::ui::ScreenTransition::menu_enter, {
        element("cursor", {90, 80, 16, 16}, zh::ui::Layer::cursor, zh::ui::BlendMode::additive),
        element("panel", {-10, 10, 100, 50}, zh::ui::Layer::background, zh::ui::BlendMode::opaque),
        element("button", {20, 40, 120, 24}, zh::ui::Layer::content),
        element("tooltip", {18, 70, 180, 40}, zh::ui::Layer::overlay),
    }};
    scene.elements[1].clip = {0, 0, 640, 480};
    check(recorder.record(scene), "valid UI scene failed");
    const auto snapshot = device.snapshot();
    check(snapshot.find("ui-transition menu-enter") != std::string::npos, "menu transition missing");
    check(snapshot.find("clip=0.00,10.00,90.00,50.00 vertex=-0.50,9.50") != std::string::npos,
        "clip or half-pixel placement changed");
    const auto panel = snapshot.find("label=panel");
    const auto button = snapshot.find("label=button");
    const auto tooltip = snapshot.find("label=tooltip");
    const auto cursor = snapshot.find("label=cursor");
    check(panel < button && button < tooltip && tooltip < cursor, "UI/cursor layer order changed");
    check(snapshot.find("blend=opaque") != std::string::npos, "opaque blend missing");
    check(snapshot.find("blend=additive") != std::string::npos, "additive blend missing");
}

void test_loading_resize_and_clipped_out()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::Scene first{320, 200, zh::ui::ScreenTransition::loading_begin,
        {element("outside", {500, 500, 20, 20}, zh::ui::Layer::content)}};
    first.elements[0].clip = {0, 0, 320, 200};
    check(recorder.record(first), "loading scene failed");
    zh::ui::Scene second{800, 600, zh::ui::ScreenTransition::loading_complete,
        {element("loading-complete", {0, 0, 800, 600}, zh::ui::Layer::background)}};
    second.elements[0].clip = {0, 0, 800, 600};
    check(recorder.record(second), "resized loading scene failed");
    const auto snapshot = device.snapshot();
    check(snapshot.find("ui-clipped-out label=outside") != std::string::npos, "clipped-out marker missing");
    check(snapshot.find("ui-resize 320x200 -> 800x600") != std::string::npos, "resize sequencing missing");
    check(snapshot.find("ui-transition loading-complete") != std::string::npos, "loading completion missing");
}

void test_failures_recover_and_internet_is_local()
{
    zh::renderer::RecordingGpuDevice device;
    zh::ui::UiRecorder recorder(device);
    zh::ui::Scene invalid{0, 480, {}, {}};
    check(!recorder.record(invalid), "zero viewport accepted");
    check(recorder.last_error().find("viewport") != std::string::npos, "viewport error not actionable");
    invalid = {640, 480, {}, {element("bad", {0, 0, 10, 10}, zh::ui::Layer::content)}};
    invalid.elements[0].bounds.x = std::numeric_limits<float>::quiet_NaN();
    check(!recorder.record(invalid), "NaN geometry accepted");
    invalid = {640, 480, {}, {element("", {0, 0, 10, 10}, zh::ui::Layer::content)}};
    check(!recorder.record(invalid), "empty label accepted");
    zh::ui::Scene valid{640, 480, {}, {element("recovered", {0, 0, 10, 10}, zh::ui::Layer::content)}};
    check(recorder.record(valid), "recorder did not recover");
    const auto unavailable = recorder.select_internet_action();
    check(unavailable.explanation == "Internet services are unavailable in this build", "local explanation changed");
    check(!unavailable.network_attempted && !unavailable.waiting, "Internet action attempted work or waited");
}
} // namespace

int main()
{
    try {
        test_order_clip_blend_and_half_pixel();
        test_loading_resize_and_clipped_out();
        test_failures_recover_and_internet_is_local();
        std::cout << "UI renderer tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
