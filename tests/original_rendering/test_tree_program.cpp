#include "original_gpu_edge.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "shader.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include "zh/original_process.h"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

struct DynamicFrameGeneratedProbeAccess {
    static unsigned offset(const DynamicVBAccessClass &access) { return access.VertexBufferOffset; }
    static unsigned offset(const DynamicIBAccessClass &access) { return access.IndexBufferOffset; }
};

namespace {
using namespace zh::renderer;
using Edge=zh::original_runtime::OriginalGpuEdge;
using Uniform=zh::original_runtime::TreeVertexUniform;
struct TreeVertex { float position[3],packed[3]; std::uint32_t diffuse; float uv[2]; };
static_assert(sizeof(TreeVertex)==36);
void check(bool okay,const char* category) { if (!okay) throw std::runtime_error(category); }
template<class F> bool rejected(F call) { try { call(); } catch(const std::runtime_error&) { return true; } return false; }

Uniform constants()
{
    Uniform result;
    for(unsigned i=0;i<4;++i) result.composite[i][i]=1;
    result.sway[1]={0.5f,0,0,0};
    result.sway[10]={0.1f,0.2f,-0.05f,0};
    result.shroud_offset={0.5f,0.5f,0,0};
    result.shroud_scale={1,1,1,1};
    return result;
}
std::array<TreeVertex,3> vertices(float slot=1)
{
    return {{{{-0.4f,-0.4f,0.75f},{slot,0.5f,0.25f},0x8064c832,{0.625f,0.125f}},
             {{ 0.4f,-0.4f,0.75f},{slot,0.5f,0.25f},0x8064c832,{0.625f,0.125f}},
             {{ 0.0f, 0.4f,0.75f},{slot,0.5f,0.25f},0x8064c832,{0.625f,0.125f}}}};
}
void write(DX8VertexBufferClass* vb,const std::array<TreeVertex,3>& data)
{
    VertexBufferClass::WriteLockClass lock(vb);
    std::memcpy(lock.Get_Vertex_Array(),data.data(),sizeof(data));
}
void source_state()
{
    ShaderClass shader;
    shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
    shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
    DX8Wrapper::Set_Shader(shader);
    DX8Wrapper::Set_Material(nullptr);
    DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
    DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
    DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
    DX8Wrapper::Apply_Render_State_Changes();
}

void recording_generation()
{
    RecordingGpuDevice device;
    {
        Edge edge(device);
        source_state();
        auto* vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNDUV1,3));
        auto data=vertices(); write(vb,data);
        auto program=constants();
        const auto accepted=edge.prepare_tree_state(vb,program);
        edge.validate_prepared_state(accepted);
        const auto resources=device.resource_counts();
        const auto pipeline=device.pipeline_descriptor(accepted.pipeline);
        check(pipeline.original_fvf.stride==sizeof(TreeVertex),"tree exact FVF stride");
        const auto bytes=device.buffer_bytes(accepted.vertex_bindings.uniforms[0].buffer);
        check(bytes.size()==sizeof(program) && !std::memcmp(bytes.data(),&program,sizeof(program)),
            "tree source constants were changed");
        for(unsigned fault=0;fault<6;++fault) {
            if(fault==0) device.fail_next_shader_create();
            if(fault==1) device.fail_next_pipeline_create();
            if(fault==2) device.fail_next_buffer_create();
            if(fault==3) device.fail_buffer_create_after(1);
            if(fault==4) device.fail_next_buffer_upload();
            if(fault==5) device.fail_buffer_upload_after(1);
            check(rejected([&]{edge.prepare_tree_state(vb,program);}),"tree candidate fault admitted");
            edge.validate_prepared_state(accepted);
            check(device.resource_counts()==resources &&
                device.buffer_bytes(accepted.vertex_bindings.uniforms[0].buffer)==bytes,
                "tree candidate fault changed accepted program");
        }
        for(float bad:{-1.0f,11.0f,1.5f,std::numeric_limits<float>::infinity()}) {
            data[0].packed[0]=bad; write(vb,data);
            check(rejected([&]{edge.prepare_tree_state(vb,program);}),"tree invalid slot admitted");
            edge.validate_prepared_state(accepted);
            check(device.resource_counts()==resources,"tree invalid slot allocated resources");
        }
        for(unsigned bad=0;bad<4;++bad) {
            data=vertices();
            if(bad==0) data[0].position[0]=std::numeric_limits<float>::quiet_NaN();
            if(bad==1) data[0].packed[1]=std::numeric_limits<float>::infinity();
            if(bad==2) data[0].packed[2]=std::numeric_limits<float>::quiet_NaN();
            if(bad==3) data[0].uv[0]=std::numeric_limits<float>::infinity();
            write(vb,data);
            check(rejected([&]{edge.prepare_tree_state(vb,program);}),
                "tree nonfinite source bytes admitted");
            edge.validate_prepared_state(accepted);
            check(device.resource_counts()==resources &&
                device.buffer_bytes(accepted.vertex_bindings.uniforms[0].buffer)==bytes,
                "tree nonfinite source bytes changed accepted program");
        }
        data=vertices(10); write(vb,data);
        for(unsigned bad=0;bad<5;++bad) {
            auto invalid=program;
            if(bad==0) invalid.composite[1][2]=std::numeric_limits<float>::quiet_NaN();
            if(bad==1) invalid.sway[0][0]=1;
            if(bad==2) invalid.sway[10][3]=1;
            if(bad==3) invalid.shroud_scale[0]=0;
            if(bad==4) invalid.shroud_offset[0]=std::numeric_limits<float>::infinity();
            check(rejected([&]{edge.prepare_tree_state(vb,invalid);}),"tree invalid constants admitted");
            edge.validate_prepared_state(accepted);
        }
        auto* wrong=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNUV1,3));
        check(rejected([&]{edge.prepare_tree_state(wrong,program);}) &&
            rejected([&]{edge.prepare_tree_state(nullptr,program);}),"tree missing/wrong FVF admitted");
        wrong->Release_Ref();
        const auto retry=edge.prepare_tree_state(vb,program);
        edge.validate_prepared_state(retry);
        check(rejected([&]{edge.validate_prepared_state(accepted);}) &&
            device.resource_counts()==resources,"tree retry retained superseded candidate");
        auto forged=retry; ++forged.generation;
        check(rejected([&]{edge.validate_prepared_state(forged);}),"tree stale generation admitted");
        check(device.snapshot().find("draw ")==std::string::npos,"tree preparation dispatched triangles");

        std::array<TextureClass*,2> textures{};
        for(unsigned stage=0;stage<2;++stage) {
            textures[stage]=NEW_REF(TextureClass,(stage?"recording-shroud":"recording-atlas",nullptr,MIP_LEVELS_1));
            unsigned mips=1;
            const auto texture=edge.create_texture(WW3D_FORMAT_A8R8G8B8,1,1,mips);
            const std::array<unsigned char,4> pixel{255,255,255,255};
            edge.upload_texture(texture,0,1,1,4,pixel.data(),pixel.size());
            edge.publish_texture(textures[stage],texture);
            textures[stage]->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,1,1);
            DX8Wrapper::Set_Texture(stage,textures[stage]);
        }
        DX8Wrapper::Apply_Render_State_Changes();
        for(unsigned stage=0;stage<2;++stage) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLOROP,D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLORARG1,D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLORARG2,D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_ALPHAARG2,D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_TEXCOORDINDEX,stage);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
        }
        const auto textured=edge.prepare_tree_state(vb,program);
        const auto texturedResources=device.resource_counts();
        const auto texturedBytes=device.buffer_bytes(textured.vertex_bindings.uniforms[0].buffer);
        check(textured.texture_mask==3 && textured.fragment_bindings.texture_count==2 &&
            rejected([&]{Edge::map_applied_state(DX8_FVF_XYZNDUV1);}),
            "tree generated UV1 relaxed generic source UV rule");
        for(unsigned bad=0;bad<4;++bad) {
            const unsigned stage=bad==3?0:1;
            const unsigned key=bad==2?D3DTSS_TEXTURETRANSFORMFLAGS:D3DTSS_TEXCOORDINDEX;
            const unsigned value=bad==0?2:bad==1?D3DTSS_TCI_CAMERASPACEPOSITION|1:
                bad==2?D3DTTFF_COUNT2:1;
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,key,value);
            check(rejected([&]{edge.prepare_tree_state(vb,program);}) &&
                device.resource_counts()==texturedResources &&
                device.buffer_bytes(textured.vertex_bindings.uniforms[0].buffer)==texturedBytes,
                "tree wrong mapping changed accepted program");
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,key,
                key==D3DTSS_TEXTURETRANSFORMFLAGS?D3DTTFF_DISABLE:stage);
        }
        const auto mappedRetry=edge.prepare_tree_state(vb,program);
        edge.validate_prepared_state(mappedRetry);
        {
            TextureDesc target;target.width=4;target.height=4;target.render_target=true;target.sampled=false;
            auto color=device.create_texture(target,"immutable program target");
            target.format=TextureFormat::depth24_stencil8;
            auto depth=device.create_texture(target,"immutable program depth");
            edge.bind_frame_targets(color,depth,4,4);
            edge.bind_frame_targets(color,color,4,4);
            const auto invalidProbe=device.snapshot();
            check(!edge.probe_tree_frame_admission() && device.snapshot()==invalidProbe,
                "immutable admission accepted wrong target format or mutated baseline");
            edge.bind_frame_targets(color,depth,4,4);
            Edge::ImmutableTreeSnapshot immutable;
            immutable.vertex=program;
            immutable.pipeline.vertex_layout=VertexLayout::original_fvf;
            immutable.pipeline.original_fvf=Edge::layout_for_fvf(DX8_FVF_XYZNDUV1);
            immutable.pipeline.raster.cull=CullMode::none;
            immutable.fragment.alpha_parameters={1,static_cast<float>(CompareOp::greater_equal),96.0f/255.0f,0};
            immutable.textures={textures[0],textures[1]};
            for(unsigned stage=0;stage<2;++stage) {
                immutable.fragment.stage_ops[stage]={static_cast<int>(Edge::CombinerOp::modulate),
                    static_cast<int>(stage?Edge::CombinerOp::select_second:Edge::CombinerOp::modulate),1,0};
                immutable.fragment.stage_args[stage]={static_cast<int>(Edge::CombinerArg::texture),
                    static_cast<int>(stage?Edge::CombinerArg::current:Edge::CombinerArg::diffuse),
                    static_cast<int>(Edge::CombinerArg::texture),static_cast<int>(stage?Edge::CombinerArg::current:Edge::CombinerArg::diffuse)};
            }
            auto owned=edge.prepare_immutable_tree_program(vb,immutable);
            const auto descriptor=device.pipeline_descriptor(owned->state().pipeline);
            check(!descriptor.blend.enabled && descriptor.depth_stencil.depth_write
                && descriptor.depth_stencil.depth_compare==CompareOp::less_equal && descriptor.raster.cull==CullMode::none,
                "immutable program changed native alpha-test-only depth/cull/blend semantics");
            const auto fragment=device.buffer_bytes(owned->state().fragment_bindings.uniforms[0].buffer);
            check(fragment.size()==sizeof(immutable.fragment)
                && !std::memcmp(fragment.data(),&immutable.fragment,sizeof(immutable.fragment)),
                "immutable program changed alpha0x60/GEQUAL or exact native stage combiners");
            const auto ownedResources=device.resource_counts();
            const auto ownedIdentity=owned->state();
            const std::array<int,2> ownedRefs{textures[0]->Num_Refs(),textures[1]->Num_Refs()};
            auto stalePin=immutable;
            stalePin.textures[0]=reinterpret_cast<TextureBaseClass*>(1);
            check(rejected([&]{edge.prepare_immutable_tree_program(vb,stalePin);})
                && device.resource_counts()==ownedResources && textures[0]->Num_Refs()==ownedRefs[0]
                && textures[1]->Num_Refs()==ownedRefs[1],"immutable nonresident pin changed ownership");
            for(unsigned fault=0;fault<7;++fault) {
                if(fault==0) device.fail_next_sampler_create();
                if(fault==1) device.fail_next_shader_create();
                if(fault==2) device.fail_next_pipeline_create();
                if(fault==3) device.fail_next_buffer_create();
                if(fault==4) device.fail_buffer_create_after(1);
                if(fault==5) device.fail_next_buffer_upload();
                if(fault==6) device.fail_buffer_upload_after(1);
                check(rejected([&]{edge.prepare_immutable_tree_program(vb,immutable);})
                    && edge.immutable_tree_program_current(*owned) && device.resource_counts()==ownedResources
                    && textures[0]->Num_Refs()==ownedRefs[0] && textures[1]->Num_Refs()==ownedRefs[1]
                    && owned->state().serial==ownedIdentity.serial,
                    "immutable candidate fault damaged accepted resource identity");
            }
            device.fail_next_transaction_checkpoint();
            check(!edge.probe_tree_frame_admission() && edge.immutable_tree_program_current(*owned)
                && device.resource_counts()==ownedResources,"immutable probe rejection changed phase");
            const auto beforeProbe=device.snapshot();
            check(edge.probe_tree_frame_admission() && device.snapshot()==beforeProbe
                && device.resource_counts()==ownedResources,"immutable probe touched accepted targets/resources");
            (void)edge.prepare_tree_state(vb,program); // Existing mutable owner changes independently.
            DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_TEXCOORDINDEX,0);
            check(edge.immutable_tree_program_current(*owned)
                && owned->state().pipeline==ownedIdentity.pipeline && owned->state().serial==ownedIdentity.serial
                && device.buffer_bytes(owned->state().fragment_bindings.uniforms[0].buffer)==fragment,
                "generic preparation/stage mutation invalidated immutable program");
            DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_TEXCOORDINDEX,1);
            const auto bound=edge.bind_vertex(vb);
            DX8VertexBufferClass *dynamicVertex=nullptr;
            DX8IndexBufferClass *dynamicIndex=nullptr;
            {
                DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,4);
                DynamicVBAccessClass::WriteLockClass lock(&access);
                std::memset(lock.Get_Formatted_Vertex_Array(),0x35,4*sizeof(VertexFormatXYZNDUV2));
                dynamicVertex=static_cast<DX8VertexBufferClass *>(access.Peek_Buffer());
            }
            {
                DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,4);
                DynamicIBAccessClass::WriteLockClass lock(&access);
                std::fill(lock.Get_Index_Array(),lock.Get_Index_Array()+4,3);
                dynamicIndex=static_cast<DX8IndexBufferClass *>(access.Peek_Buffer());
            }
            const auto dynamicVertexHandle=edge.bind_vertex(dynamicVertex),dynamicIndexHandle=edge.bind_index(dynamicIndex);
            const auto dynamicVertexBytes=device.buffer_bytes(dynamicVertexHandle),dynamicIndexBytes=device.buffer_bytes(dynamicIndexHandle);
            const auto dynamicVertexCapacity=dynamicVertex->Get_Vertex_Count(),dynamicIndexCapacity=dynamicIndex->Get_Index_Count();
            const auto frameBaseline=edge.prepare_tree_state(vb,program);
            const auto sourceBaseline=DX8Wrapper::Inspect_Source_State();
            const auto frameResources=device.resource_counts();
            const auto frameCommands=device.snapshot();
            const auto frameBytes=device.buffer_bytes(bound);
            const auto frameRefs=vb->Num_Refs();
            check(edge.begin_tree_source_frame() && edge.tree_source_frame_pending(),
                "source frame checkpoint rejected exact backend/target");
            check(!edge.begin_tree_source_frame(),"nested source frame changed original ownership");
            edge.begin_source_frame(true,true,0,0,0,1);
            Matrix4x4 changedWorld(true);changedWorld[0].W=1;
            DX8Wrapper::Set_Transform(D3DTS_WORLD,changedWorld);
            DX8Wrapper::Set_Texture(1,nullptr);
            DX8Wrapper::Set_Material(nullptr);
            write(vb,vertices(0));
            (void)edge.bind_vertex(vb);
            {
                DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,4);
                DynamicVBAccessClass::WriteLockClass lock(&access);
                std::memset(lock.Get_Formatted_Vertex_Array(),0x61,4*sizeof(VertexFormatXYZNDUV2));
                (void)edge.bind_vertex(access.Peek_Buffer());
            }
            {
                DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,4);
                DynamicIBAccessClass::WriteLockClass lock(&access);
                std::fill(lock.Get_Index_Array(),lock.Get_Index_Array()+4,5);
                (void)edge.bind_index(access.Peek_Buffer());
            }
            {
                DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,6000);
                DynamicVBAccessClass::WriteLockClass lock(&access);
                std::memset(lock.Get_Formatted_Vertex_Array(),0x73,6000*sizeof(VertexFormatXYZNDUV2));
                (void)edge.bind_vertex(access.Peek_Buffer());
            }
            {
                DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,6000);
                DynamicIBAccessClass::WriteLockClass lock(&access);
                std::fill(lock.Get_Index_Array(),lock.Get_Index_Array()+6000,7);
                (void)edge.bind_index(access.Peek_Buffer());
            }
            check(rejected([&]{DynamicVBAccessClass unsupported(BUFFER_TYPE_DYNAMIC_SORTING,DX8_FVF_XYZNDUV2,4);}) &&
                rejected([&]{DynamicIBAccessClass unsupported(BUFFER_TYPE_DYNAMIC_SORTING,4);}),
                "tree source frame admitted sorting producer");
            edge.end_source_frame(false);
            // The latest fallible presentation command must poison the whole
            // frame, after all preceding source/device mutation has occurred.
            device.fail_transaction_operation_after(0);
            check(!device.present(color) && !edge.commit_tree_source_frame()
                && edge.abort_tree_source_frame() && !edge.abort_tree_source_frame(),
                "late presentation failure escaped source/device rollback");
            const auto restored=DX8Wrapper::Inspect_Source_State();
            check(!edge.tree_source_frame_pending() && !device.pass_active()
                && device.snapshot()==frameCommands && device.resource_counts()==frameResources
                && device.buffer_bytes(bound)==frameBytes && vb->Num_Refs()==frameRefs
                && !std::memcmp(vb->Get_CPU_Vertex_Buffer(),frameBytes.data(),frameBytes.size())
                && restored.render==sourceBaseline.render && restored.stages==sourceBaseline.stages
                && restored.transforms.size()==sourceBaseline.transforms.size()
                && edge.immutable_tree_program_current(*owned),
                "late presentation abort changed accepted source/resource identity");
            edge.validate_prepared_state(frameBaseline);
            {
                DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,4);
                check(access.Peek_Buffer()==dynamicVertex && DynamicFrameGeneratedProbeAccess::offset(access)==4 &&
                    dynamicVertex->Get_Vertex_Count()==dynamicVertexCapacity && device.buffer_bytes(dynamicVertexHandle)==dynamicVertexBytes &&
                    !std::memcmp(dynamicVertex->Get_CPU_Vertex_Buffer(),dynamicVertexBytes.data(),dynamicVertexBytes.size()),
                    "dynamic vertex abort changed identity/capacity/offset/bytes");
            }
            {
                DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_DX8,4);
                check(access.Peek_Buffer()==dynamicIndex && DynamicFrameGeneratedProbeAccess::offset(access)==4 &&
                    dynamicIndex->Get_Index_Count()==dynamicIndexCapacity && device.buffer_bytes(dynamicIndexHandle)==dynamicIndexBytes &&
                    !std::memcmp(dynamicIndex->Get_CPU_Index_Buffer(),dynamicIndexBytes.data(),dynamicIndexBytes.size()),
                    "dynamic index abort changed identity/capacity/offset/bytes");
            }
            check(edge.begin_tree_source_frame(),"source frame clean retry rejected");
            edge.begin_source_frame(true,true,0,0,0,1);
            edge.end_source_frame(true);
            check(edge.commit_tree_source_frame() && !edge.commit_tree_source_frame()
                && edge.immutable_tree_program_current(*owned),
                "source frame success did not consume exactly once");
            owned.reset();
            device.destroy(depth);device.destroy(color);
        }
        textures[1]->Invalidate();
        check(rejected([&]{edge.validate_prepared_state(mappedRetry);}) &&
            rejected([&]{edge.prepare_tree_state(vb,program);}),"tree stale texture owner admitted");
        for(unsigned stage=0;stage<2;++stage) {
            DX8Wrapper::Set_Texture(stage,nullptr);
            textures[stage]->Invalidate();
            textures[stage]->Release_Ref();
        }
        vb->Release_Ref();
    }
    DynamicVBAccessClass::_Deinit();DynamicIBAccessClass::_Deinit();
    check(device.resource_counts().total()==0 && VertexBufferClass::Get_Total_Buffer_Count()==0,
        "tree program teardown retained resource");
}

void physical_generation()
{
    BgfxOptions options; options.shader_root=ZH_BGFX_SHADER_DIR;
    BgfxGpuDevice device(options);
    TextureDesc target; target.width=64; target.height=64; target.render_target=true; target.sampled=false;
    const auto color=device.create_texture(target,"tree program pixels");
    target.format=TextureFormat::depth24_stencil8;
    const auto depth=device.create_texture(target,"tree program depth");
    check(color && depth,"tree physical target creation");
    {
        Edge edge(device); source_state();
        auto* vb=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNDUV1,3));
        auto* ib=NEW_REF(DX8IndexBufferClass,(3));
        { IndexBufferClass::WriteLockClass lock(ib); auto* i=lock.Get_Index_Array(); i[0]=0;i[1]=1;i[2]=2; }
        const auto render=[&](float slot,const Uniform& uniform) {
            write(vb,vertices(slot));
            const auto state=edge.prepare_tree_state(vb,uniform);
            const auto vertex=edge.bind_vertex(vb),index=edge.bind_index(ib);
            RenderPassDesc pass; pass.color_targets[0]=color;pass.color_target_count=1;
            pass.depth_target=depth;pass.width=64;pass.height=64;
            check(device.begin_pass(pass,"exact tree program"),"tree physical pass");
            DrawDesc draw;draw.pipeline=state.pipeline;draw.vertex_buffer=vertex;draw.index_buffer=index;
            draw.vertex_or_index_count=3;draw.index_element_size=IndexElementSize::uint16;
            draw.vertex_bindings=state.vertex_bindings;
            draw.fragment_bindings=state.fragment_bindings;
            check(device.draw(draw),"tree physical draw");
            check(device.end_pass(),"tree physical end");
            return device.readback_rgba(color);
        };
        const auto zero=render(0,constants());
        const auto sway=render(1,constants());
        check(zero.size()==64*64*4 && sway.size()==zero.size(),"tree physical readback");
        unsigned compared=0;
        for(unsigned y=2;y<62;++y) for(unsigned x=2;x<54;++x) {
            const auto p=(y*64+x)*4,q=(y*64+x+8)*4;
            if(zero[p+3]==128) {
                check(std::abs(int(zero[p])-50)<=1 && std::abs(int(zero[p+1])-100)<=1 &&
                    std::abs(int(zero[p+2])-25)<=1,"tree darkening/packed color/alpha pixels");
                for(unsigned channel=0;channel<4;++channel)
                    check(zero[p+channel]==sway[q+channel],"tree height-relative exact sway pixels");
                ++compared;
            }
        }
        check(compared>100,"tree physical control emitted no source pixels");
        auto transformed=constants(); transformed.composite[0][3]=0.25f;
        const auto shifted=render(0,transformed);
        check(shifted==sway,"tree composite row orientation pixels");

        std::array<TextureClass*,2> source_textures{};
        std::array<std::array<unsigned char,4>,16> atlas{};
        for(unsigned y=0;y<4;++y) for(unsigned x=0;x<4;++x)
            atlas[y*4+x]=y<2?(x<2?std::array<unsigned char,4>{0,0,255,255}:
                std::array<unsigned char,4>{255,255,255,255}):
                (x<2?std::array<unsigned char,4>{255,0,0,255}:
                std::array<unsigned char,4>{0,255,0,255});
        std::array<std::array<unsigned char,4>,8> shroud{};
        for(unsigned i=0;i<8;++i) {
            const auto value=static_cast<unsigned char>(32+i*31);
            shroud[i]={value,value,value,255};
        }
        for(unsigned stage=0;stage<2;++stage) {
            unsigned mips=1,width=stage?8:4,height=stage?1:4;
            source_textures[stage]=NEW_REF(TextureClass,(stage?"generated-shroud":"generated-atlas",nullptr,MIP_LEVELS_1));
            const auto texture=edge.create_texture(WW3D_FORMAT_A8R8G8B8,width,height,mips);
            edge.upload_texture(texture,0,width,height,width*4,stage?
                static_cast<const void*>(shroud.data()):static_cast<const void*>(atlas.data()),width*height*4);
            edge.publish_texture(source_textures[stage],texture);
            source_textures[stage]->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,width,height);
            DX8Wrapper::Set_Texture(stage,source_textures[stage]);
            using Filter=Edge::FilterStageState;
            edge.set_filter_stage_state(stage,Filter::min_filter,1);
            edge.set_filter_stage_state(stage,Filter::mag_filter,1);
            edge.set_filter_stage_state(stage,Filter::mip_filter,1);
            edge.set_filter_stage_state(stage,Filter::address_u,1);
            edge.set_filter_stage_state(stage,Filter::address_v,1);
        }
        DX8Wrapper::Apply_Render_State_Changes();
        for(unsigned stage=0;stage<2;++stage) {
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLOROP,D3DTOP_MODULATE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLORARG1,D3DTA_TEXTURE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_COLORARG2,D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_ALPHAARG2,D3DTA_CURRENT);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
            DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_TEXCOORDINDEX,stage);
        }
        const auto texturedZero=render(0,constants());
        const auto texturedSway=render(1,constants());
        compared=0;
        unsigned darkest=255,brightest=0;
        for(unsigned y=2;y<62;++y) for(unsigned x=2;x<54;++x) {
            const auto p=(y*64+x)*4,q=(y*64+x+8)*4;
            if(texturedZero[p+3]==128) {
                // UV0=(.625,.125) selects the white top-right atlas quadrant. UV1 must sample
                // the unswayed source position, despite eight-pixel motion.
                for(unsigned channel=0;channel<4;++channel)
                    check(texturedZero[p+channel]==texturedSway[q+channel],
                        "tree atlas/unswayed shroud UV pixels");
                check(texturedZero[p]>0 && texturedZero[p+1]>texturedZero[p] &&
                    texturedZero[p]>texturedZero[p+2],"tree atlas UV selected wrong column");
                darkest=std::min(darkest,unsigned(texturedZero[p+1]));
                brightest=std::max(brightest,unsigned(texturedZero[p+1]));
                ++compared;
            }
        }
        check(compared>100 && brightest>darkest+20,"tree shroud control lacked position variation");
        for(unsigned stage=0;stage<2;++stage) {
            DX8Wrapper::Set_Texture(stage,nullptr);
            source_textures[stage]->Invalidate();
            source_textures[stage]->Release_Ref();
        }
        edge.release_source_buffers(); vb->Release_Ref();ib->Release_Ref();
    }
    device.destroy(depth);device.destroy(color);
    check(device.wait_idle() && device.live_resource_count()==0,"tree physical teardown");
}
}

int main(int argc,char** argv)
{
    try {
        // bgfx's worker and validation layer share the native global allocator.
        // Use shipping synchronization services, not the single-thread tool mode.
        struct Services {
            Services() { char reason[64]{}; check(zh::original_process::initialize_services(0,reason,sizeof(reason)),
                "tree program process services"); }
            ~Services() { zh::original_process::shutdown_services(); }
        } services;
        for(unsigned generation=0;generation<2;++generation)
            if(argc==2 && std::string(argv[1])=="--gpu") physical_generation();
            else recording_generation();
        std::cout<<"original tree program: ok\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
