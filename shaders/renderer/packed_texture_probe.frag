#version 450
layout(location = 0) in vec2 texture_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D source_texture;
void main() { out_color = texture(source_texture, texture_uv); }
