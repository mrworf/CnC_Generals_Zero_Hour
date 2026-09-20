#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 0) out vec4 vertex_color;
layout(location = 1) out vec2 texture_uv;
layout(set = 1, binding = 0) uniform Frame { vec4 viewport; } frame_data;

void main()
{
    vec2 ndc = in_position / frame_data.viewport.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    vertex_color = in_color;
    texture_uv = in_uv;
}
