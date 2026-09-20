#version 450

layout(location = 0) in vec3 world_normal;
layout(location = 1) in vec2 texture_uv;
layout(location = 0) out vec4 out_color;
layout(set = 2, binding = 0) uniform sampler2D texture0;
layout(set = 2, binding = 1) uniform sampler2D texture1;
layout(set = 2, binding = 2) uniform sampler2D texture2;
layout(set = 2, binding = 3) uniform sampler2D shadow_texture;
layout(set = 3, binding = 0) uniform Material { vec4 diffuse; } material_data;

void main()
{
    float light = max(abs(normalize(world_normal).z), 0.2);
    vec4 base = texture(texture0, texture_uv) + texture(texture1, texture_uv) * 0.0
        + texture(texture2, texture_uv) * 0.0 + texture(shadow_texture, texture_uv) * 0.0;
    out_color = base * light + material_data.diffuse * 0.001;
}
