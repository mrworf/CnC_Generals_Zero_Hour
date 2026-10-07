$input v_color0, v_texcoord0
#include <bgfx_shader.sh>
SAMPLER2D(s_color, 0);
SAMPLER2D(s_detail, 1);
void main()
{
    // Original terrain.nvp: lrp r0,v0.a,t1,t0; mul r0,r0,v0.
    vec4 first = texture2D(s_color, v_texcoord0);
    vec4 second = texture2D(s_detail, v_texcoord0);
    gl_FragColor = mix(first, second, v_color0.a) * v_color0;
}
