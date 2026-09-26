#version 450
layout(location=0) in vec2 probe_uv;
layout(location=0) out vec4 out_color;
layout(set=2,binding=0) uniform sampler2D probe_texture;
layout(set=3,binding=0,std140) uniform OriginColor {
    vec4 dead_prefix;
    vec4 tint;
    vec4 dead_hole;
    ivec4 enabled;
} color_data;
layout(set=3,binding=3,std140) uniform OriginPass {
    vec4 dead_prefix;
    vec4 factor;
} pass_data;
void main() {
    out_color=color_data.enabled.x!=0
        ?texture(probe_texture,probe_uv)*color_data.tint*pass_data.factor:vec4(0.0);
}
