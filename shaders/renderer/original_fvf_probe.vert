#version 450

// Device-layout probe only: B3B owns source-selected material shading.
layout(location = 0) in vec3 source_position;
layout(location = 2) in vec4 source_diffuse;
layout(location = 4) in vec2 source_uv;
layout(location = 0) out vec4 vertex_color;

void main()
{
    gl_Position = vec4(source_position, 1.0);
    vertex_color = source_diffuse + vec4(source_uv * 0.0, 0.0, 0.0);
}
