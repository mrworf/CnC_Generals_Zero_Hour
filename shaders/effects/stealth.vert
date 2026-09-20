#version 450
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 0) out vec3 world_normal;
layout(location = 1) out vec4 screen_position;
layout(set = 1, binding = 0) uniform Frame { mat4 view_projection; } frame_data;
layout(set = 1, binding = 1) uniform Object { mat4 model; } object_data;
void main() {
    vec4 world = object_data.model * vec4(in_position, 1.0);
    screen_position = frame_data.view_projection * world;
    gl_Position = screen_position;
    world_normal = mat3(object_data.model) * in_normal;
}
