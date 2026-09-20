#version 450
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec2 texture_uv;
layout(location = 1) out vec4 projected_position;
layout(set = 1, binding = 0) uniform Frame { mat4 view_projection; vec4 time; } frame_data;
layout(set = 1, binding = 1) uniform Object { mat4 model; vec4 wave; } object_data;
void main() {
    vec3 position = in_position;
    position.z += sin(position.x * object_data.wave.x + frame_data.time.x) * object_data.wave.y;
    projected_position = frame_data.view_projection * object_data.model * vec4(position, 1.0);
    gl_Position = projected_position;
    texture_uv = in_uv + frame_data.time.xx * object_data.wave.zw;
}
