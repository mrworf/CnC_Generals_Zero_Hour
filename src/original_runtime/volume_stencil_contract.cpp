#include "volume_stencil_contract.h"

#include <stdexcept>

namespace zh::original_runtime {

VolumeStencilProtocol make_volume_stencil_protocol(
    renderer::ShaderHandle vertex, renderer::ShaderHandle fragment,
    renderer::TextureFormat color_format, renderer::UInt8 shadow_mask)
{
  if (!vertex || !fragment || color_format == renderer::TextureFormat::depth16
      || color_format == renderer::TextureFormat::depth24_stencil8
      || color_format == renderer::TextureFormat::depth32)
    throw std::runtime_error("original volume stencil protocol requires public color and shader handles");

  renderer::PipelineDesc volume;
  volume.vertex_shader = vertex;
  volume.fragment_shader = fragment;
  volume.vertex_layout = renderer::VertexLayout::position_color_uv;
  volume.color_format = color_format;
  volume.depth_format = renderer::TextureFormat::depth24_stencil8;
  volume.raster.cull = renderer::CullMode::clockwise;
  volume.depth_stencil.depth_test = true;
  volume.depth_stencil.depth_write = false;
  volume.depth_stencil.depth_compare = renderer::CompareOp::less_equal;
  volume.depth_stencil.stencil_test = true;
  volume.depth_stencil.stencil_compare = renderer::CompareOp::greater_equal;
  volume.depth_stencil.stencil_reference = 0x80;
  volume.depth_stencil.stencil_read_mask = shadow_mask;
  volume.depth_stencil.stencil_write_mask = 0xff;
  volume.depth_stencil.stencil_fail = renderer::StencilOp::keep;
  volume.depth_stencil.depth_fail = renderer::StencilOp::keep;
  volume.depth_stencil.depth_pass = renderer::StencilOp::increment_clamp;
  volume.blend.color_write_mask = 0;

  VolumeStencilProtocol protocol;
  protocol.increment = volume;
  protocol.decrement = volume;
  protocol.decrement.raster.cull = renderer::CullMode::counter_clockwise;
  protocol.decrement.depth_stencil.depth_pass = renderer::StencilOp::decrement_clamp;
  protocol.composite = volume;
  protocol.composite.raster.cull = renderer::CullMode::none;
  protocol.composite.depth_stencil.depth_compare = renderer::CompareOp::always;
  protocol.composite.depth_stencil.stencil_compare = renderer::CompareOp::less_equal;
  protocol.composite.depth_stencil.stencil_reference = 1;
  protocol.composite.depth_stencil.stencil_read_mask = static_cast<renderer::UInt8>(~shadow_mask);
  protocol.composite.depth_stencil.stencil_write_mask = 0;
  protocol.composite.depth_stencil.depth_pass = renderer::StencilOp::keep;
  protocol.composite.blend.enabled = true;
  protocol.composite.blend.source_color = renderer::BlendFactor::dst_color;
  protocol.composite.blend.destination_color = renderer::BlendFactor::zero;
  protocol.composite.blend.source_alpha = renderer::BlendFactor::one;
  protocol.composite.blend.destination_alpha = renderer::BlendFactor::zero;
  protocol.composite.blend.color_write_mask = 0x0f;
  return protocol;
}

} // namespace zh::original_runtime
