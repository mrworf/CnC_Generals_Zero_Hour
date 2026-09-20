#version 450
layout(location = 0) in vec3 world_normal;
layout(location = 1) in vec2 texture_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D base_texture;
layout(set = 2, binding = 1) uniform sampler2D normal_texture;
layout(set = 2, binding = 2) uniform samplerCube environment_texture;
layout(set = 3, binding = 0) uniform Material { vec4 environment_mix; } material_data;
void main() {
    vec3 bump = texture(normal_texture, texture_uv).xyz * 2.0 - 1.0;
    vec3 normal = normalize(world_normal + bump);
    vec4 base = texture(base_texture, texture_uv);
    vec4 environment = texture(environment_texture, reflect(vec3(0.0, 0.0, -1.0), normal));
    out_color = mix(base, environment, material_data.environment_mix.x);
}
