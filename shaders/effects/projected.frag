#version 450
layout(location = 0) in vec4 projected_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D projected_texture;
layout(set = 3, binding = 0) uniform Material { vec4 tint; } material_data;
void main() { out_color = textureProj(projected_texture, projected_uv) * material_data.tint; }
