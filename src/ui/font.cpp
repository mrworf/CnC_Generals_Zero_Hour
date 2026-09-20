#include "zh/ui/font.h"

#include "zh/data/vfs.h"

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cstring>
#include <limits>
#include <sstream>
#include <utility>

namespace zh::ui {
namespace {

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r");
    value = value.substr(first, last - first + 1);
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"')
            || (value.front() == '\'' && value.back() == '\'')))
        value = value.substr(1, value.size() - 2);
    return value;
}

std::string ft_error(std::string_view operation, FT_Error code)
{
    return std::string(operation) + " failed with FreeType error " + std::to_string(code);
}

std::vector<char32_t> decode_utf8(std::string_view text)
{
    std::vector<char32_t> result;
    for (std::size_t index = 0; index < text.size();) {
        const auto first = static_cast<unsigned char>(text[index]);
        char32_t value = 0;
        std::size_t length = 0;
        if (first <= 0x7fU) { value = first; length = 1; }
        else if ((first & 0xe0U) == 0xc0U) { value = first & 0x1fU; length = 2; }
        else if ((first & 0xf0U) == 0xe0U) { value = first & 0x0fU; length = 3; }
        else if ((first & 0xf8U) == 0xf0U) { value = first & 0x07U; length = 4; }
        else throw FontError("text layout: malformed UTF-8 leading byte at offset " + std::to_string(index));
        if (index + length > text.size()) throw FontError("text layout: truncated UTF-8 at offset " + std::to_string(index));
        for (std::size_t continuation = 1; continuation < length; ++continuation) {
            const auto byte = static_cast<unsigned char>(text[index + continuation]);
            if ((byte & 0xc0U) != 0x80U)
                throw FontError("text layout: malformed UTF-8 continuation at offset " + std::to_string(index + continuation));
            value = (value << 6U) | (byte & 0x3fU);
        }
        const bool overlong = (length == 2 && value < 0x80U) || (length == 3 && value < 0x800U)
            || (length == 4 && value < 0x10000U);
        if (overlong || value > 0x10ffffU || (value >= 0xd800U && value <= 0xdfffU))
            throw FontError("text layout: invalid Unicode scalar at offset " + std::to_string(index));
        result.push_back(value);
        index += length;
    }
    return result;
}

struct SelectedGlyph {
    char32_t codepoint;
    std::size_t face_index;
    bool replacement;
};

} // namespace

struct FontFace::Impl {
    ~Impl()
    {
        if (face) FT_Done_Face(face);
        if (library) FT_Done_FreeType(library);
    }

    FT_Library library = nullptr;
    FT_Face face = nullptr;
    std::shared_ptr<const std::vector<std::uint8_t>> retained_bytes;
    std::string source;
};

FontFace FontFace::from_memory(std::shared_ptr<const std::vector<std::uint8_t>> bytes,
    unsigned pixel_height, std::string logical_name, long face_index)
{
    if (!bytes || bytes->empty()) throw FontError("font memory face: bytes are empty");
    if (pixel_height == 0 || pixel_height > 512) throw FontError("font memory face: pixel height must be within 1..512");
    if (logical_name.empty()) throw FontError("font memory face: logical name is empty");
    if (bytes->size() > static_cast<std::size_t>(std::numeric_limits<FT_Long>::max()))
        throw FontError("font memory face: byte length exceeds FreeType limit");
    auto impl = std::make_shared<Impl>();
    if (const auto error = FT_Init_FreeType(&impl->library)) throw FontError(ft_error("FT_Init_FreeType", error));
    impl->retained_bytes = std::move(bytes);
    impl->source = "memory:" + std::move(logical_name);
    const auto error = FT_New_Memory_Face(impl->library, impl->retained_bytes->data(),
        static_cast<FT_Long>(impl->retained_bytes->size()), face_index, &impl->face);
    if (error) throw FontError(ft_error("FT_New_Memory_Face(" + impl->source + ")", error));
    if (const auto size_error = FT_Set_Pixel_Sizes(impl->face, 0, pixel_height))
        throw FontError(ft_error("FT_Set_Pixel_Sizes(" + impl->source + ")", size_error));
    return FontFace(std::move(impl));
}

std::string FontFace::resolve_system_font(std::string_view family)
{
    if (family.empty()) throw FontError("Fontconfig family is empty");
    if (!FcInit()) throw FontError("Fontconfig initialization failed");
    FcPattern* pattern = FcPatternCreate();
    if (!pattern) throw FontError("Fontconfig pattern allocation failed");
    const std::string family_copy(family);
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(family_copy.c_str()));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult result = FcResultNoMatch;
    FcPattern* match = FcFontMatch(nullptr, pattern, &result);
    FcPatternDestroy(pattern);
    if (!match) throw FontError("Fontconfig found no fallback for family '" + family_copy + "'");
    FcChar8* file = nullptr;
    const auto status = FcPatternGetString(match, FC_FILE, 0, &file);
    if (status != FcResultMatch || !file) {
        FcPatternDestroy(match);
        throw FontError("Fontconfig match for family '" + family_copy + "' has no file");
    }
    const std::string path(reinterpret_cast<const char*>(file));
    FcPatternDestroy(match);
    return path;
}

FontFace FontFace::from_system(std::string_view family, unsigned pixel_height)
{
    if (pixel_height == 0 || pixel_height > 512) throw FontError("system font: pixel height must be within 1..512");
    const auto path = resolve_system_font(family);
    auto impl = std::make_shared<Impl>();
    if (const auto error = FT_Init_FreeType(&impl->library)) throw FontError(ft_error("FT_Init_FreeType", error));
    impl->source = "fontconfig:" + std::string(family) + " -> " + path;
    if (const auto error = FT_New_Face(impl->library, path.c_str(), 0, &impl->face))
        throw FontError(ft_error("FT_New_Face(" + path + ")", error));
    if (const auto error = FT_Set_Pixel_Sizes(impl->face, 0, pixel_height))
        throw FontError(ft_error("FT_Set_Pixel_Sizes(" + path + ")", error));
    return FontFace(std::move(impl));
}

bool FontFace::has_glyph(char32_t codepoint) const
{
    return impl_ && FT_Get_Char_Index(impl_->face, static_cast<FT_ULong>(codepoint)) != 0;
}

GlyphMetrics FontFace::render_glyph(char32_t codepoint) const
{
    if (!impl_) throw FontError("render glyph: font face is not initialized");
    const auto index = FT_Get_Char_Index(impl_->face, static_cast<FT_ULong>(codepoint));
    if (index == 0) throw FontError("render glyph: selected face has no glyph for U+" + std::to_string(codepoint));
    if (const auto error = FT_Load_Glyph(impl_->face, index, FT_LOAD_DEFAULT))
        throw FontError(ft_error("FT_Load_Glyph(" + impl_->source + ")", error));
    if (const auto error = FT_Render_Glyph(impl_->face->glyph, FT_RENDER_MODE_NORMAL))
        throw FontError(ft_error("FT_Render_Glyph(" + impl_->source + ")", error));
    const auto& slot = *impl_->face->glyph;
    if (slot.bitmap.pixel_mode != FT_PIXEL_MODE_GRAY && slot.bitmap.width != 0 && slot.bitmap.rows != 0)
        throw FontError("render glyph: FreeType did not produce an 8-bit grayscale bitmap");
    return {index, static_cast<int>(slot.bitmap.width), static_cast<int>(slot.bitmap.rows),
        slot.bitmap_left, slot.bitmap_top, static_cast<int>(slot.advance.x / 64), slot.bitmap.pitch, slot.bitmap.buffer};
}

std::string FontFace::source() const { return impl_ ? impl_->source : std::string{}; }
std::size_t FontFace::retained_byte_count() const noexcept
{
    return impl_ && impl_->retained_bytes ? impl_->retained_bytes->size() : 0;
}

FontSelection parse_language_font_selection(std::string_view language_ini)
{
    if (language_ini.size() > 1024U * 1024U) throw FontError("language font configuration exceeds 1 MiB");
    FontSelection selection;
    std::istringstream lines{std::string(language_ini)};
    for (std::string line; std::getline(lines, line);) {
        const auto comment = line.find_first_of(";#");
        if (comment != std::string::npos) line.resize(comment);
        const auto equals = line.find('=');
        if (equals == std::string::npos) continue;
        const auto key = trim(line.substr(0, equals));
        const auto value = trim(line.substr(equals + 1));
        if (key == "UnicodeFontName") selection.unicode_font_name = value;
        else if (key == "LocalFontFile") selection.local_font_file = value;
    }
    if (selection.local_font_file.empty() && selection.unicode_font_name.empty())
        throw FontError("language font configuration has neither LocalFontFile nor UnicodeFontName");
    return selection;
}

FontFace load_selected_font(const FontSelection& selection, const data::VirtualFileSystem* vfs,
    unsigned pixel_height, std::size_t maximum_font_bytes)
{
    if (!selection.local_font_file.empty()) {
        if (!vfs) throw FontError("LocalFontFile '" + selection.local_font_file + "' requires a mounted VFS");
        const auto* resource = vfs->find(selection.local_font_file);
        if (!resource) throw FontError("LocalFontFile is missing from VFS: " + selection.local_font_file);
        if (resource->size == 0 || resource->size > maximum_font_bytes)
            throw FontError("LocalFontFile size is outside configured bound: " + selection.local_font_file);
        auto bytes = std::make_shared<std::vector<std::uint8_t>>(
            vfs->read_prefix(selection.local_font_file, static_cast<std::size_t>(resource->size)));
        if (bytes->size() != resource->size)
            throw FontError("LocalFontFile read was incomplete: " + selection.local_font_file);
        return FontFace::from_memory(std::move(bytes), pixel_height, selection.local_font_file);
    }
    if (selection.unicode_font_name.empty()) throw FontError("UnicodeFontName is empty and no LocalFontFile was selected");
    return FontFace::from_system(selection.unicode_font_name, pixel_height);
}

TextLayout::TextLayout(FontFace primary, std::vector<FontFace> fallbacks)
    : primary_(std::move(primary)), fallbacks_(std::move(fallbacks))
{
    if (!primary_.valid()) throw FontError("text layout: primary face is not initialized");
    for (const auto& face : fallbacks_)
        if (!face.valid()) throw FontError("text layout: fallback face is not initialized");
}

TextRun TextLayout::layout(std::string_view utf8, const LayoutOptions& options) const
{
    if (options.maximum_width <= 0) throw FontError("text layout: maximum width must be positive");
    if (options.maximum_lines == 0 || options.maximum_lines > 1024) throw FontError("text layout: maximum lines must be within 1..1024");
    if (options.atlas_width <= 0 || options.atlas_height <= 0 || options.atlas_width > 8192 || options.atlas_height > 8192)
        throw FontError("text layout: atlas dimensions must be within 1..8192");
    const auto scalars = decode_utf8(utf8);
    if (scalars.size() > 65536) throw FontError("text layout: text exceeds 65536 Unicode scalars");
    std::vector<FontFace> faces;
    faces.push_back(primary_);
    faces.insert(faces.end(), fallbacks_.begin(), fallbacks_.end());
    const auto select = [&](char32_t requested) -> SelectedGlyph {
        for (std::size_t index = 0; index < faces.size(); ++index)
            if (faces[index].has_glyph(requested)) return {requested, index, false};
        for (char32_t replacement : {U'\ufffd', U'?'})
            for (std::size_t index = 0; index < faces.size(); ++index)
                if (faces[index].has_glyph(replacement)) return {replacement, index, true};
        throw FontError("text layout: no fallback glyph for U+" + std::to_string(requested));
    };

    TextRun run;
    run.atlas_width = options.atlas_width;
    run.atlas_height = options.atlas_height;
    run.atlas_alpha.assign(static_cast<std::size_t>(options.atlas_width) * options.atlas_height, 0);
    int pen_x = 0;
    int baseline = 0;
    int line_height = 0;
    int shelf_x = 0;
    int shelf_y = 0;
    int shelf_height = 0;
    run.line_count = scalars.empty() ? 0 : 1;

    const auto append = [&](char32_t requested, bool ellipsis) -> bool {
        const auto selected = select(requested);
        const auto metrics = faces[selected.face_index].render_glyph(selected.codepoint);
        const int advance = std::max(1, metrics.advance);
        if (pen_x + advance > options.maximum_width) return false;
        if (shelf_x + metrics.width > options.atlas_width) {
            shelf_x = 0;
            shelf_y += shelf_height + 1;
            shelf_height = 0;
        }
        if (shelf_y + metrics.height > options.atlas_height)
            throw FontError("text layout: glyph atlas capacity exceeded at U+" + std::to_string(requested));
        if (metrics.width > 0 && metrics.height > 0) {
            for (int row = 0; row < metrics.height; ++row) {
                const auto* source = metrics.bitmap + static_cast<std::ptrdiff_t>(row) * metrics.bitmap_pitch;
                auto* destination = run.atlas_alpha.data() + static_cast<std::size_t>(shelf_y + row) * options.atlas_width + shelf_x;
                std::memcpy(destination, source, static_cast<std::size_t>(metrics.width));
            }
        }
        run.glyphs.push_back({ellipsis ? U'\u2026' : requested, metrics.glyph_index, pen_x + metrics.bearing_x,
            baseline - metrics.bearing_y, metrics.width, metrics.height, metrics.bearing_x, metrics.bearing_y,
            advance, shelf_x, shelf_y, selected.face_index, selected.replacement});
        if (selected.face_index != 0 || selected.replacement) {
            run.used_fallback = true;
            run.diagnostics.push_back("fallback U+" + std::to_string(requested) + " -> " + faces[selected.face_index].source());
        }
        pen_x += advance;
        line_height = std::max(line_height, std::max(advance, metrics.height + std::max(0, metrics.bearing_y)));
        shelf_x += metrics.width + 1;
        shelf_height = std::max(shelf_height, metrics.height);
        return true;
    };

    for (const auto scalar : scalars) {
        if (scalar == U'\n') {
            if (run.line_count >= options.maximum_lines) { run.truncated = true; break; }
            ++run.line_count; run.wrapped = true; pen_x = 0; baseline += std::max(1, line_height); line_height = 0;
            continue;
        }
        if (append(scalar, false)) continue;
        if (options.wrap && run.line_count < options.maximum_lines) {
            ++run.line_count; run.wrapped = true; pen_x = 0; baseline += std::max(1, line_height); line_height = 0;
            if (append(scalar, false)) continue;
        }
        run.truncated = true;
        if (options.truncate_with_ellipsis) {
            const auto ellipsis = primary_.has_glyph(U'\u2026') ? U'\u2026' : U'.';
            while (!run.glyphs.empty() && !append(ellipsis, true)) {
                pen_x -= run.glyphs.back().advance;
                run.glyphs.pop_back();
            }
        }
        break;
    }
    return run;
}

LocaleLayoutDecision selected_english_layout_decision() noexcept
{
    return LocaleLayoutDecision::simple_left_to_right;
}

std::string_view selected_english_layout_reason() noexcept
{
    return "The selected English corpus is left-to-right and requires neither contextual substitution nor bidi paragraph reordering";
}

} // namespace zh::ui
