#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 0) out vec4 vertex_color;

void main()
{
    gl_Position = vec4(in_position, 0.5, 1.0);
    vertex_color = in_color + vec4(in_uv * 0.0, 0.0, 0.0);
}
