#version 450

// The original FVFInfoClass chooses the locations and offsets. Unbound FVF
// attributes are never repacked by this device-edge shader.
layout(location=0) in vec3 source_position;
#if defined(ORIGINAL_FVF_N0) || defined(ORIGINAL_FVF_N1) || defined(ORIGINAL_FVF_N2)
layout(location=1) in vec3 source_normal;
#endif
#if defined(ORIGINAL_FVF_D1) || defined(ORIGINAL_FVF_D2)
layout(location=2) in vec4 source_diffuse_bgra;
#endif
#if !defined(ORIGINAL_FVF_N0)
layout(location=4) in vec4 source_uv0;
#endif
#if defined(ORIGINAL_FVF_D2) || defined(ORIGINAL_FVF_N2)
layout(location=5) in vec4 source_uv1;
#endif
layout(location=0) out vec4 applied_diffuse;
layout(location=1) out vec4 applied_tex0;
layout(location=2) out vec4 applied_tex1;
layout(location=3) out float view_depth;
layout(location=4) out vec3 applied_specular;

layout(set=1,binding=0,std140) uniform OriginalTransforms {
    vec4 world_rows[4];
    vec4 view_rows[4];
    vec4 projection_rows[4];
    vec4 texture_rows[8];
    ivec4 coordinate_modes;
    ivec4 uv_indices;
    ivec4 transform_flags;
    vec4 lit_diffuse,lit_ambient,lit_specular,lit_emissive;
    vec4 lit_global_ambient;
    ivec4 lit_switches;
    vec4 light_position_range[4];
    vec4 light_direction_attenuation0[4];
    vec4 light_diffuse_attenuation1[4];
    vec4 light_ambient_attenuation2[4];
    vec4 light_specular_type[4];
} source;

mat4 source_matrix(vec4 a, vec4 b, vec4 c, vec4 d)
{
    // The original DX8Wrapper transposes its row-major Matrix4x4 before
    // submitting it as a D3D row-vector matrix. Source snapshots keep the
    // pre-transpose original rows; GLSL must reconstruct that original matrix.
    return transpose(mat4(a,b,c,d));
}

vec4 selected_uv(int stage,vec4 camera_position,vec3 camera_normal)
{
    int index=source.uv_indices[stage];
    int mode=source.coordinate_modes[stage];
    vec4 coordinate=vec4(0.0,0.0,0.0,1.0);
#if !defined(ORIGINAL_FVF_N0)
    coordinate=source_uv0;
#endif
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
#if defined(ORIGINAL_FVF_N0) || defined(ORIGINAL_FVF_N1) || defined(ORIGINAL_FVF_N2)
    camera_normal=mat3(view*world)*source_normal;
#endif
    applied_specular=vec3(0.0);
#if defined(ORIGINAL_LIT)
    // D3D fixed-function lighting transforms normals by inverse-transpose
    // world-view; D3DRS_NORMALIZENORMALS controls subsequent normalization.
    camera_normal=transpose(inverse(mat3(view*world)))*source_normal;
    if (source.lit_switches.x!=0) camera_normal=normalize(camera_normal);
    vec3 ambient=source.lit_global_ambient.rgb;
    vec3 diffuse=vec3(0.0);
    vec3 specular=vec3(0.0);
    for (int i=0;i<4;++i) {
        float type=source.light_specular_type[i].w;
        if (type==0.0) continue;
        vec3 incoming;
        float attenuation=1.0;
        if (type==1.0) {
            vec3 light_camera=(view*vec4(source.light_position_range[i].xyz,1.0)).xyz;
            vec3 offset=light_camera-camera_position.xyz;
            float distance=length(offset);
            if (distance>source.light_position_range[i].w) continue;
            incoming=distance>0.0?offset/distance:vec3(0.0);
            float denominator=source.light_direction_attenuation0[i].w+
                source.light_diffuse_attenuation1[i].w*distance+
                source.light_ambient_attenuation2[i].w*distance*distance;
            if (denominator<=0.0) continue;
            attenuation=1.0/denominator;
        } else {
            incoming=normalize(-(view*vec4(source.light_direction_attenuation0[i].xyz,0.0)).xyz);
        }
        ambient+=source.light_ambient_attenuation2[i].rgb*attenuation;
        float incidence=max(dot(camera_normal,incoming),0.0);
        diffuse+=source.light_diffuse_attenuation1[i].rgb*incidence*attenuation;
        if (source.lit_switches.z!=0 && incidence>0.0) {
            vec3 viewer=source.lit_switches.y!=0?
                normalize(-camera_position.xyz):vec3(0.0,0.0,-1.0);
            vec3 half_vector=normalize(incoming+viewer);
            specular+=source.light_specular_type[i].rgb*
                pow(max(dot(camera_normal,half_vector),0.0),source.lit_specular.a)*attenuation;
        }
    }
    // Normal-bearing original FVF variants supply no COLOR1/COLOR2 attribute.
    // Direct3D falls back to the corresponding material term even when a
    // material-source selector requests an absent per-vertex color.
    applied_diffuse=vec4(clamp(source.lit_emissive.rgb+
        source.lit_ambient.rgb*ambient+source.lit_diffuse.rgb*diffuse,0.0,1.0),
        source.lit_diffuse.a);
    applied_specular=clamp(source.lit_specular.rgb*specular,0.0,1.0);
#else
#if defined(ORIGINAL_FVF_D1) || defined(ORIGINAL_FVF_D2)
    applied_diffuse=source_diffuse_bgra.zyxw;
#else
    applied_diffuse=vec4(1.0);
#endif
#endif
    applied_tex0=selected_uv(0,camera_position,camera_normal);
    applied_tex1=selected_uv(1,camera_position,camera_normal);
}
