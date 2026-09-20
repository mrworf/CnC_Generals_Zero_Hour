#version 450
layout(location = 0) in vec4 vertex_color;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D particle_texture;
layout(set = 3, binding = 0) uniform Material { vec4 alpha_reference; } material_data;
void main() {
    vec4 color = texture(particle_texture, gl_PointCoord) * vertex_color;
    if (color.a < material_data.alpha_reference.x) discard;
    out_color = color;
}
