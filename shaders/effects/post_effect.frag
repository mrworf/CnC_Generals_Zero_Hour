#version 450
layout(location = 0) in vec2 texture_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D scene_texture;
layout(set = 3, binding = 0) uniform Effect { vec4 color_scale; } effect_data;
void main() { out_color = texture(scene_texture, texture_uv) * effect_data.color_scale; }
