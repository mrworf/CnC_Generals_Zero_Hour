#include "zh/ui/font.h"
#include "zh/data/vfs.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

std::shared_ptr<const std::vector<std::uint8_t>> font_bytes()
{
    const auto path = zh::ui::FontFace::resolve_system_font("sans-serif");
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("could not open Fontconfig test face");
    auto bytes = std::make_shared<std::vector<std::uint8_t>>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return bytes;
}

void test_memory_face_retains_bytes_and_metrics()
{
    const auto bytes = font_bytes();
    auto face = zh::ui::FontFace::from_memory(bytes, 18, "Data/English/TestFont.ttf");
    check(face.retained_byte_count() == bytes->size(), "memory face did not retain bytes");
    check(face.source() == "memory:Data/English/TestFont.ttf", "logical font source changed");
    check(face.has_glyph(U'A'), "test face lacks ASCII glyph");
    const auto glyph = face.render_glyph(U'A');
    check(glyph.glyph_index != 0 && glyph.advance > 0, "glyph metrics were not populated");
    check(glyph.width >= 0 && glyph.height >= 0 && glyph.bitmap_pitch >= glyph.width, "glyph bitmap metrics invalid");
}

void test_system_fallback_wrap_truncate_and_mixed_text()
{
    auto face = zh::ui::FontFace::from_system("Arial Unicode MS", 18);
    check(face.source().find("fontconfig:Arial Unicode MS -> ") == 0, "Fontconfig selection not logged");
    zh::ui::TextLayout layout(face);
    auto wrapped = layout.layout("Menu tooltip caf\xc3\xa9 subtitle caption", {90, 4, true, false, 256, 256});
    check(wrapped.wrapped && wrapped.line_count > 1, "mixed text did not wrap");
    check(!wrapped.glyphs.empty() && wrapped.atlas_alpha.size() == 256U * 256U, "atlas was not produced");
    auto truncated = layout.layout("player-name-that-is-too-long", {55, 1, false, true, 256, 256});
    check(truncated.truncated && !truncated.glyphs.empty(), "long name was not truncated");
    check(truncated.glyphs.back().codepoint == U'\u2026' || truncated.glyphs.back().codepoint == U'.',
        "truncation did not end in ellipsis fallback");
}

void test_missing_glyph_and_failures()
{
    auto primary = zh::ui::FontFace::from_memory(font_bytes(), 16, "test.ttf");
    zh::ui::TextLayout layout(primary);
    const std::string unsupported("\xf4\x8f\xbf\xbf", 4);
    const auto fallback = layout.layout(unsupported, {100, 1, false, false, 128, 128});
    check(fallback.used_fallback && fallback.glyphs[0].replacement, "missing glyph did not use deliberate replacement");
    bool failed = false;
    try { (void)zh::ui::FontFace::from_memory(std::make_shared<const std::vector<std::uint8_t>>(), 16, "empty"); }
    catch (const zh::ui::FontError&) { failed = true; }
    check(failed, "empty font bytes accepted");
    failed = false;
    try {
        auto corrupt = std::make_shared<const std::vector<std::uint8_t>>(16, 0xffU);
        (void)zh::ui::FontFace::from_memory(corrupt, 16, "corrupt");
    } catch (const zh::ui::FontError&) { failed = true; }
    check(failed, "corrupt font accepted");
    failed = false;
    try { (void)layout.layout(std::string("\xc0\xaf", 2), {100, 1}); }
    catch (const zh::ui::FontError&) { failed = true; }
    check(failed, "malformed UTF-8 accepted");
    failed = false;
    try { (void)layout.layout("A", {100, 1, false, false, 1, 1}); }
    catch (const zh::ui::FontError& error) { failed = std::string(error.what()).find("atlas") != std::string::npos; }
    check(failed, "impossible atlas accepted without actionable error");
    failed = false;
    try { (void)layout.layout("A", {0, 1}); }
    catch (const zh::ui::FontError&) { failed = true; }
    check(failed, "invalid width accepted");
}

void test_english_locale_gate()
{
    check(zh::ui::selected_english_layout_decision() == zh::ui::LocaleLayoutDecision::simple_left_to_right,
        "English unexpectedly enabled shaping/bidi dependencies");
    check(zh::ui::selected_english_layout_reason().find("neither contextual") != std::string_view::npos,
        "locale decision lacks evidence rationale");
}

void test_language_selection_prefers_vfs_memory_face()
{
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-ui-font-selection-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh_root = root / "zh";
    const auto generals_root = root / "generals";
    std::filesystem::create_directories(zh_root / "Data/English");
    std::filesystem::create_directories(generals_root);
    const auto system_path = zh::ui::FontFace::resolve_system_font("sans-serif");
    std::filesystem::copy_file(system_path, zh_root / "Data/English/Local.ttf");
    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    const auto selection = zh::ui::parse_language_font_selection(
        "UnicodeFontName = Arial Unicode MS\nLocalFontFile = Data/English/Local.ttf\n");
    auto face = zh::ui::load_selected_font(selection, &vfs, 16);
    check(face.source() == "memory:Data/English/Local.ttf", "LocalFontFile did not take precedence through VFS");
    check(face.retained_byte_count() > 0, "VFS font bytes were not retained");
    bool failed = false;
    try { (void)zh::ui::load_selected_font(selection, nullptr, 16); }
    catch (const zh::ui::FontError& error) { failed = std::string(error.what()).find("mounted VFS") != std::string::npos; }
    check(failed, "LocalFontFile without VFS did not fail clearly");
    failed = false;
    try { (void)zh::ui::parse_language_font_selection("DrawGroupInfoFontSize=12\n"); }
    catch (const zh::ui::FontError&) { failed = true; }
    check(failed, "font-less language configuration accepted");
    std::filesystem::remove_all(root, ignored);
}
} // namespace

int main()
{
    try {
        test_memory_face_retains_bytes_and_metrics();
        test_system_fallback_wrap_truncate_and_mixed_text();
        test_missing_glyph_and_failures();
        test_english_locale_gate();
        test_language_selection_prefers_vfs_memory_face();
        std::cout << "UI font tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
