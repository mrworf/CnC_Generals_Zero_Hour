layout(location=0) in vec4 applied_diffuse;
layout(location=1) in vec4 applied_tex0;
layout(location=2) in vec4 applied_tex1;
layout(location=3) in float view_depth;
layout(location=0) out vec4 out_color;

#if (ORIGINAL_STAGE_MASK & 1)
layout(set=2,binding=0) uniform sampler2D source_stage0;
#endif
#if (ORIGINAL_STAGE_MASK & 2)
#if (ORIGINAL_STAGE_MASK & 1)
layout(set=2,binding=1) uniform sampler2D source_stage1;
#else
layout(set=2,binding=0) uniform sampler2D source_stage1;
#endif
#endif

layout(set=3,binding=0,std140) uniform OriginalShader {
    vec4 diffuse,ambient,specular,emissive;
    vec4 fog_color,fog_parameters,alpha_parameters;
    ivec4 material_sources;
    ivec4 stage_ops[2];
    ivec4 stage_args[2];
    vec4 bump[2];
} source;

vec4 stage_texture(int stage,vec4 coordinate)
{
    int flags=source.stage_ops[stage].w;
    vec2 uv=coordinate.xy;
    // Projected texture coordinates are selected by original mapper flags;
    // these are carried by the vertex stage's source transform record.
    if ((flags&256)!=0 && coordinate.z!=0.0) uv=coordinate.xy/coordinate.z;
#if (ORIGINAL_STAGE_MASK & 1)
    if (stage==0) return texture(source_stage0,uv);
#endif
#if (ORIGINAL_STAGE_MASK & 2)
    if (stage==1) return texture(source_stage1,uv);
#endif
    return vec4(0.0);
}

vec4 operand(int arg,vec4 diffuse,vec4 current,vec4 texture_color)
{
    if (arg==0) return diffuse;
    if (arg==1) return current;
    return texture_color;
}

vec4 combine(int op,vec4 first,vec4 second,vec4 current)
{
    if (op==0) return current;
    if (op==1) return first;
    if (op==2) return second;
    if (op==3) return first*second;
    return clamp(first+second,vec4(0.0),vec4(1.0));
}

bool alpha_pass(float value,int compare,float reference)
{
    if (compare==0) return false;
    if (compare==1) return value<reference;
    if (compare==2) return value==reference;
    if (compare==3) return value<=reference;
    if (compare==4) return value>reference;
    if (compare==5) return value!=reference;
    if (compare==6) return value>=reference;
    return true;
}

void main()
{
    vec4 diffuse=applied_diffuse;
    vec4 current=diffuse;
    for (int stage=0;stage<2;++stage) {
        ivec4 ops=source.stage_ops[stage];
        ivec4 args=source.stage_args[stage];
        vec4 coordinate=stage==0?applied_tex0:applied_tex1;
        vec4 texture_color=ops.z!=0?stage_texture(stage,coordinate):vec4(0.0);
        vec4 color=combine(ops.x,operand(args.x,diffuse,current,texture_color),
            operand(args.y,diffuse,current,texture_color),current);
        vec4 alpha=combine(ops.y,operand(args.z,diffuse,current,texture_color),
            operand(args.w,diffuse,current,texture_color),current);
        current=vec4(color.rgb,alpha.a);
    }
    if (source.alpha_parameters.x!=0.0 && !alpha_pass(current.a,
        int(source.alpha_parameters.y),source.alpha_parameters.z)) discard;
    if (source.fog_parameters.z!=0.0) {
        float amount=clamp((source.fog_parameters.y-view_depth)/
            (source.fog_parameters.y-source.fog_parameters.x),0.0,1.0);
        current.rgb=mix(source.fog_color.rgb,current.rgb,amount);
    }
    out_color=current;
}
