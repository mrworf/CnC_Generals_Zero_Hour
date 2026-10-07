$input v_color0, v_texcoord0
#include <bgfx_shader.sh>
SAMPLER2D(s_color, 0);
SAMPLER2D(s_detail, 1);
void main()
{
    // Original fterrain.nvp: mul r0,t1,t0; mul r0,r0,v0.
    gl_FragColor = texture2D(s_color, v_texcoord0) * texture2D(s_detail, v_texcoord0) * v_color0;
}
