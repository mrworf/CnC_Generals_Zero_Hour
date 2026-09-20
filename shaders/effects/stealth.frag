#version 450
layout(location = 0) in vec3 world_normal;
layout(location = 1) in vec4 screen_position;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D scene_texture;
layout(set = 2, binding = 1) uniform sampler2D distortion_texture;
layout(set = 3, binding = 0) uniform Effect { vec4 strength; } effect_data;
void main() {
    vec2 uv = screen_position.xy / screen_position.w * 0.5 + 0.5;
    vec2 offset = texture(distortion_texture, uv).rg * 2.0 - 1.0;
    float edge = 1.0 - abs(normalize(world_normal).z);
    out_color = texture(scene_texture, uv + offset * effect_data.strength.x) * vec4(1.0, 1.0, 1.0, edge);
}
