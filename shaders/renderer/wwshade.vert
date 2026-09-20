#version 450
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 0) out vec3 world_normal;
layout(location = 1) out vec2 base_uv;
layout(location = 2) out vec4 projected_uv;
layout(set = 1, binding = 0) uniform Frame { mat4 view_projection; } frame_data;
layout(set = 1, binding = 1) uniform Material { mat4 texture_projection; } material_data;
layout(set = 1, binding = 2) uniform Object { mat4 model; } object_data;
void main() {
    vec4 world = object_data.model * vec4(in_position, 1.0);
    gl_Position = frame_data.view_projection * world;
    world_normal = mat3(object_data.model) * in_normal;
    base_uv = in_uv;
    projected_uv = material_data.texture_projection * world;
}
