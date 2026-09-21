#version 450

// The original FVFInfoClass chooses the locations and offsets. Unbound FVF
// attributes are never repacked by this device-edge shader.
layout(location=0) in vec3 source_position;
#if defined(ORIGINAL_FVF_N1) || defined(ORIGINAL_FVF_N2)
layout(location=1) in vec3 source_normal;
#endif
#if defined(ORIGINAL_FVF_D1) || defined(ORIGINAL_FVF_D2)
layout(location=2) in vec4 source_diffuse_bgra;
#endif
layout(location=4) in vec4 source_uv0;
#if defined(ORIGINAL_FVF_D2) || defined(ORIGINAL_FVF_N2)
layout(location=5) in vec4 source_uv1;
#endif
layout(location=0) out vec4 applied_diffuse;
layout(location=1) out vec4 applied_tex0;
layout(location=2) out vec4 applied_tex1;
layout(location=3) out float view_depth;

layout(set=1,binding=0,std140) uniform OriginalTransforms {
    vec4 world_rows[4];
    vec4 view_rows[4];
    vec4 projection_rows[4];
    vec4 texture_rows[8];
    ivec4 coordinate_modes;
    ivec4 uv_indices;
    ivec4 transform_flags;
} source;

mat4 source_matrix(vec4 a, vec4 b, vec4 c, vec4 d)
{
    // A GLSL matrix constructed from source rows transposes the original
    // D3D row-vector matrix: M_glsl * v == v * M_original.
    return mat4(a,b,c,d);
}

vec4 selected_uv(int stage,vec4 camera_position,vec3 camera_normal)
{
    int index=source.uv_indices[stage];
    int mode=source.coordinate_modes[stage];
    vec4 coordinate=source_uv0;
#if defined(ORIGINAL_FVF_D2) || defined(ORIGINAL_FVF_N2)
    if (index==1) coordinate=source_uv1;
#endif
    if (mode==65536) coordinate=vec4(camera_normal,1.0);
    else if (mode==131072) coordinate=camera_position;
    else if (mode==196608) coordinate=vec4(reflect(normalize(camera_position.xyz),
        normalize(camera_normal)),1.0);
    if (source.transform_flags[stage]!=0) {
        int offset=stage*4;
        coordinate=source_matrix(source.texture_rows[offset],source.texture_rows[offset+1],
            source.texture_rows[offset+2],source.texture_rows[offset+3])*coordinate;
    }
    return coordinate;
}

void main()
{
    mat4 world=source_matrix(source.world_rows[0],source.world_rows[1],
        source.world_rows[2],source.world_rows[3]);
    mat4 view=source_matrix(source.view_rows[0],source.view_rows[1],
        source.view_rows[2],source.view_rows[3]);
    mat4 projection=source_matrix(source.projection_rows[0],source.projection_rows[1],
        source.projection_rows[2],source.projection_rows[3]);
    vec4 world_position=world*vec4(source_position,1.0);
    vec4 camera_position=view*world_position;
    gl_Position=projection*camera_position;
    view_depth=camera_position.z;
    vec3 camera_normal=vec3(0.0);
#if defined(ORIGINAL_FVF_N1) || defined(ORIGINAL_FVF_N2)
    camera_normal=mat3(view*world)*source_normal;
#endif
#if defined(ORIGINAL_FVF_D1) || defined(ORIGINAL_FVF_D2)
    applied_diffuse=source_diffuse_bgra.zyxw;
#else
    applied_diffuse=vec4(1.0);
#endif
    applied_tex0=selected_uv(0,camera_position,camera_normal);
    applied_tex1=selected_uv(1,camera_position,camera_normal);
}
