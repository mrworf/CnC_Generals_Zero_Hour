#pragma once

#include "zh/renderer/contract.h"

namespace zh::original_runtime {

// Public, backend-agnostic representation of the three D3D8 volume passes.
// It owns no shadow geometry or source object and performs no presentation.
struct VolumeStencilProtocol {
  renderer::PipelineDesc increment;
  renderer::PipelineDesc decrement;
  renderer::PipelineDesc composite;
};

// `shadow_mask` names reserved non-shadow stencil bits. The original volume
// counter therefore reads the complement and never writes color in either
// volume pass; the final rectangle uses DESTCOLOR/ZERO under LEQUAL stencil.
VolumeStencilProtocol make_volume_stencil_protocol(
    renderer::ShaderHandle vertex, renderer::ShaderHandle fragment,
    renderer::TextureFormat color_format, renderer::UInt8 shadow_mask);

} // namespace zh::original_runtime
