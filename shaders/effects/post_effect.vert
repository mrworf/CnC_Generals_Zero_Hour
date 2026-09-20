#version 450
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec2 texture_uv;
layout(set = 1, binding = 0) uniform Frame { vec4 viewport; } frame_data;
void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);
    texture_uv = in_uv + frame_data.viewport.zw * 0.0;
}
