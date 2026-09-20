#version 450
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 projected_uv;
layout(set = 1, binding = 0) uniform Frame { mat4 view_projection; } frame_data;
layout(set = 1, binding = 1) uniform Object { mat4 model; mat4 texture_projection; } object_data;
void main() {
    vec4 world = object_data.model * vec4(in_position, 1.0);
    gl_Position = frame_data.view_projection * world;
    projected_uv = object_data.texture_projection * world;
}
