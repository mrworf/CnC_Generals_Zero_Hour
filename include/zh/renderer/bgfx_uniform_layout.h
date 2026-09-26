#pragma once

#include "zh/renderer/contract.h"
#include <map>
#include <string>
#include <vector>

namespace zh::renderer {

struct BgfxUniformFieldLayout {
    std::string name;
    UInt32 offset = 0;
    UInt32 extent = 0;
    UInt32 count = 1;
    UInt32 kind = 2; // bgfx Vec4/Mat3/Mat4 reflection kind; integer vectors retain bits.
};
struct BgfxUniformBlockLayout {
    std::string instance;
    UInt32 origin = 0;
    UInt32 extent = 0;
    std::vector<BgfxUniformFieldLayout> fields;
    UInt32 source_binding = 0;
};

// Bounded, CPU-only admission of compiler-authoritative std140 source blocks.
// The envelope/header reflection can omit dead fields; it is never a block base.
ValidationResult decode_bgfx_uniform_layout(const std::vector<char> &binary, ShaderStage stage,
                                            std::vector<BgfxUniformBlockLayout> &output);
ValidationResult associate_bgfx_uniform_bindings(std::vector<BgfxUniformBlockLayout> &blocks,
                                                 const std::map<std::string, UInt32> &source_bindings);
ValidationResult read_bgfx_uniform_bindings(std::string_view manifest,
                                            std::map<std::string, UInt32> &source_bindings);

// Canonical versioned compiler-origin sidecar. Exact comparison rejects stale,
// duplicate, truncated and noncanonical metadata without publishing candidates.
std::string encode_bgfx_uniform_layout(ShaderStage stage,
                                       const std::vector<BgfxUniformBlockLayout> &blocks);
ValidationResult validate_bgfx_uniform_layout_sidecar(std::string_view sidecar, ShaderStage stage,
                                                      const std::vector<BgfxUniformBlockLayout> &blocks);

} // namespace zh::renderer
