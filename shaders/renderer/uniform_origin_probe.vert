#version 450
// Generated-only physical admission control; not an original tree program.
layout(location=0) in vec3 in_position;
layout(location=4) in vec2 in_uv;
layout(location=0) out vec2 probe_uv;
layout(set=1,binding=0,std140) uniform OriginFrame {
    vec4 dead_prefix;
    mat4 composite;
    vec4 dead_hole[2];
    ivec4 enabled;
    vec4 uv_bias[2];
} frame_data;
layout(set=1,binding=2,std140) uniform OriginObject {
    vec4 dead_prefix;
    vec4 offset;
} object_data;
void main() {
    gl_Position=frame_data.composite*vec4(in_position,1.0)+object_data.offset;
    probe_uv=in_uv+frame_data.uv_bias[frame_data.enabled.x].xy;
}
