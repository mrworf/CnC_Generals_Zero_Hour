#version 450
layout(location=0) in vec2 in_position;
layout(location=1) in vec4 in_color;
layout(location=2) in vec2 in_uv;
layout(location=0) out vec4 vertex_color;
layout(location=1) out vec2 texture_uv;
layout(set=1,binding=0) uniform Frame { vec4 viewport; } frame_data;
layout(set=2,binding=0) uniform sampler2D different_texture;
void main() {
    gl_Position=vec4(in_position/frame_data.viewport.xy*vec2(2,-2)+vec2(-1,1),0,1);
    vertex_color=in_color*texture(different_texture,vec2(0.25));
    texture_uv=in_uv;
}
