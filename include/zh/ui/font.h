#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace zh::ui {

class FontError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct GlyphMetrics {
    std::uint32_t glyph_index = 0;
    int width = 0;
    int height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
    int bitmap_pitch = 0;
    const unsigned char* bitmap = nullptr;
};

class FontFace {
public:
    FontFace() = default;
    static FontFace from_memory(std::shared_ptr<const std::vector<std::uint8_t>> bytes,
        unsigned pixel_height, std::string logical_name, long face_index = 0);
    static FontFace from_system(std::string_view family, unsigned pixel_height);
    static std::string resolve_system_font(std::string_view family);

    bool valid() const noexcept { return static_cast<bool>(impl_); }
    bool has_glyph(char32_t codepoint) const;
    GlyphMetrics render_glyph(char32_t codepoint) const;
    std::string source() const;
    std::size_t retained_byte_count() const noexcept;

private:
    struct Impl;
    explicit FontFace(std::shared_ptr<Impl> impl) : impl_(std::move(impl)) {}
    std::shared_ptr<Impl> impl_;
};

struct LayoutOptions {
    int maximum_width = 0;
    std::size_t maximum_lines = 1;
    bool wrap = false;
    bool truncate_with_ellipsis = false;
    int atlas_width = 512;
    int atlas_height = 512;
};

struct PositionedGlyph {
    char32_t codepoint = 0;
    std::uint32_t glyph_index = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
    int atlas_x = 0;
    int atlas_y = 0;
    std::size_t face_index = 0;
    bool replacement = false;
};

struct TextRun {
    std::vector<PositionedGlyph> glyphs;
    std::vector<std::uint8_t> atlas_alpha;
    int atlas_width = 0;
    int atlas_height = 0;
    std::size_t line_count = 0;
    bool wrapped = false;
    bool truncated = false;
    bool used_fallback = false;
    std::vector<std::string> diagnostics;
};

class TextLayout {
public:
    explicit TextLayout(FontFace primary, std::vector<FontFace> fallbacks = {});
    TextRun layout(std::string_view utf8, const LayoutOptions& options) const;

private:
    FontFace primary_;
    std::vector<FontFace> fallbacks_;
};

enum class LocaleLayoutDecision { simple_left_to_right, requires_shaping, requires_bidirectional_layout };
LocaleLayoutDecision selected_english_layout_decision() noexcept;
std::string_view selected_english_layout_reason() noexcept;

} // namespace zh::ui
