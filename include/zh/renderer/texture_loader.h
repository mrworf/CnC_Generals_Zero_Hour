#pragma once

#include "zh/renderer/contract.h"

#include <string>
#include <vector>

namespace zh::renderer {

struct TextureData {
    TextureDesc descriptor;
    std::vector<UInt8> bytes;
    bool premultiplied_alpha = false;
    bool decoded_bc_fallback = false;
};

struct TextureLoadResult {
    TextureData texture;
    std::string error;
    explicit operator bool() const noexcept { return error.empty(); }
};

// Parses a complete in-memory DDS or TGA stream. DDS support is deliberately
// limited to the source-required DXT1/2/3/4/5 family; TGA support is limited to
// uncompressed 24/32-bit true color. No filesystem or backend API is touched.
TextureLoadResult parse_texture(const std::vector<UInt8>& bytes, bool backend_supports_bc);

} // namespace zh::renderer
