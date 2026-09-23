#version 450

layout(location = 0) in vec3 source_position;

void main()
{
    gl_Position = vec4(source_position, 1.0);
}
