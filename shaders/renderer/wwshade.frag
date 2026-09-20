#version 450
layout(location = 0) in vec3 world_normal;
layout(location = 1) in vec2 base_uv;
layout(location = 2) in vec4 projected_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D base_texture;
layout(set = 2, binding = 1) uniform sampler2D detail_texture;
layout(set = 2, binding = 2) uniform sampler2D projected_texture;
layout(set = 3, binding = 0) uniform FrameEffect { vec4 fog_color; } frame_effect;
layout(set = 3, binding = 1) uniform Material { vec4 diffuse; } material_data;
layout(set = 3, binding = 2) uniform ObjectEffect { vec4 fog_parameters; } object_effect;
layout(set = 3, binding = 3) uniform PassEffect { vec4 detail_scale; } pass_effect;
void main() {
    vec4 base = texture(base_texture, base_uv) * material_data.diffuse;
    vec4 detail = texture(detail_texture, base_uv * pass_effect.detail_scale.xy);
    vec4 projected = textureProj(projected_texture, projected_uv);
    float fog = clamp(object_effect.fog_parameters.x, 0.0, 1.0);
    out_color = mix(base * detail * projected * max(normalize(world_normal).z, 0.2), frame_effect.fog_color, fog);
}
