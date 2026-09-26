#pragma once

#include <array>

namespace zh::original_runtime {

// Exact Trees.nvv c4–7, c8–18, c32 and c33, supplied by the terrain owner.
// This separate std140 ABI never interprets the packed slots as mesh normals.
struct alignas(16) TreeVertexUniform {
    std::array<std::array<float,4>,4> composite{};
    std::array<std::array<float,4>,11> sway{};
    std::array<float,4> shroud_offset{};
    std::array<float,4> shroud_scale{};
};
static_assert(sizeof(TreeVertexUniform)==17*16);

} // namespace zh::original_runtime
