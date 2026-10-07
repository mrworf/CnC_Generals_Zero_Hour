$input a_position, a_color0, a_texcoord0
$output v_project
#include <bgfx_shader.sh>
uniform mat4 u_project;
void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_project = mul(u_project, vec4(a_position, 1.0)).xyz;
}
