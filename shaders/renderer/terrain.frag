#version 450
layout(location = 0) in vec3 world_normal;
layout(location = 1) in vec2 texture_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D base_texture;
layout(set = 2, binding = 1) uniform sampler2D light_map;
layout(set = 3, binding = 0) uniform Material { vec4 diffuse; } material_data;
void main() {
    float lighting = max(normalize(world_normal).z, 0.2);
    out_color = texture(base_texture, texture_uv) * texture(light_map, texture_uv) * material_data.diffuse * lighting;
}
