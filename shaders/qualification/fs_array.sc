$input v_color0, v_texcoord0
#include <bgfx_shader.sh>
SAMPLER2DARRAY(s_color, 0);
uniform vec4 u_layer;
void main()
{
    gl_FragColor = texture2DArray(s_color, vec3(v_texcoord0, u_layer.x)) * v_color0;
}
