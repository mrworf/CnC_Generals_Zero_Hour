$input v_project
#include <bgfx_shader.sh>
SAMPLER2D(s_color, 0);
void main()
{
    gl_FragColor = texture2D(s_color, v_project.xy / v_project.z);
}
