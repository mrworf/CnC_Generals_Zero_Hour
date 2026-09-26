#version 450

// Canonical GameClient/Shaders/Trees.nvv; the native pixel program is unused.
layout(location=0) in vec3 source_position;
// Keep the canonical attribute transport name; these three slots are not normals.
layout(location=1) in vec3 source_normal;
layout(location=2) in vec4 source_diffuse_bgra;
layout(location=4) in vec2 source_uv;
layout(location=0) out vec4 applied_diffuse;
layout(location=1) out vec4 applied_tex0;
layout(location=2) out vec4 applied_tex1;
layout(location=3) out float view_depth;
layout(location=4) out vec3 applied_specular;

layout(set=1,binding=0,std140) uniform OriginalTree {
    vec4 composite_rows[4];
    vec4 sway[11];
    vec4 shroud_offset;
    vec4 shroud_scale;
} source;

void main()
{
    vec4 original=vec4(source_position,1.0);
    vec4 displaced=original+(source_position.z-source_normal.z)*
        source.sway[int(source_normal.x)];
    gl_Position=vec4(dot(source.composite_rows[0],displaced),
        dot(source.composite_rows[1],displaced),
        dot(source.composite_rows[2],displaced),
        dot(source.composite_rows[3],displaced));
    applied_diffuse=vec4(source_diffuse_bgra.bgr*source_normal.y,
        source_diffuse_bgra.a);
    applied_tex0=vec4(source_uv,0.0,1.0);
    applied_tex1=(original+source.shroud_offset)*source.shroud_scale;
    applied_specular=vec3(0.0);
    view_depth=0.0; // The exact native tree pass disables fog.
}
