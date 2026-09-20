#version 450
layout(location = 0) in vec2 texture_uv;
layout(location = 1) in vec4 projected_position;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D normal_texture;
layout(set = 2, binding = 1) uniform sampler2D reflection_texture;
layout(set = 3, binding = 0) uniform Material { vec4 tint; } material_data;
layout(set = 3, binding = 1) uniform Effect { vec4 distortion; } effect_data;
void main() {
    vec2 normal_offset = texture(normal_texture, texture_uv).rg * 2.0 - 1.0;
    vec2 projected_uv = projected_position.xy / projected_position.w * 0.5 + 0.5;
    vec4 reflected = texture(reflection_texture, projected_uv + normal_offset * effect_data.distortion.x);
    out_color = reflected * material_data.tint;
}
