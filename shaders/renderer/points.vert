#version 450
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 vertex_color;
layout(set = 1, binding = 0) uniform Frame { mat4 view_projection; vec4 viewport; } frame_data;
layout(set = 1, binding = 1) uniform Object { mat4 model; vec4 point_size; } object_data;
void main() {
    vec4 view_position = object_data.model * vec4(in_position, 1.0);
    gl_Position = frame_data.view_projection * view_position;
    gl_PointSize = max(1.0, object_data.point_size.x / max(gl_Position.w, 0.001));
    vertex_color = in_color;
}
