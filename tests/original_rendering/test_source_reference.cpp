#include "original_gpu_edge.h"
#include "texture.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include "zh/original_process.h"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

namespace {
using namespace zh::renderer;
using Edge=zh::original_runtime::OriginalGpuEdge;
void check(bool value,const char* category) { if(!value) throw std::runtime_error(category); }
struct Texture final:TextureClass {
    unsigned& destroyed;
    unsigned initializations=0;
    explicit Texture(unsigned& counter):TextureClass("generated-reference",nullptr,MIP_LEVELS_1),destroyed(counter) {}
    ~Texture() override { ++destroyed; }
    void Init() override { ++initializations;TextureClass::Init(); }
    unsigned last_accessed() const { return LastAccessed; }
};
Texture* source(Edge& edge,unsigned& destroyed,bool ready=true)
{
    auto* result=NEW_REF(Texture,(destroyed));
    unsigned mips=1;
    const auto handle=edge.create_texture(WW3D_FORMAT_A8R8G8B8,4,4,mips);
    std::array<unsigned,16> pixels{};pixels.fill(0xff00ff00);
    edge.upload_texture(handle,0,4,4,16,pixels.data(),sizeof(pixels));
    edge.publish_texture(result,handle);
    if(ready) result->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,4,4);
    return result;
}
void sampling(Edge& edge,TextureBaseClass* texture)
{
    edge.select_texture(0,texture);
    for(auto state:{Edge::FilterStageState::min_filter,Edge::FilterStageState::mag_filter,
        Edge::FilterStageState::mip_filter,Edge::FilterStageState::address_u,Edge::FilterStageState::address_v})
        edge.set_filter_stage_state(0,state,0);
}
void recording()
{
    RecordingGpuDevice device;
    unsigned destroyed=0;
    {
        Edge edge(device);
        auto* texture=source(edge,destroyed);
        const auto handle=edge.texture_handle(texture);
        const auto revision=edge.source_revision();
        const auto accessed=texture->last_accessed();
        std::array<TextureBaseClass*,65> units{};units.fill(texture);
        Edge::SourceReferenceToken token{11,12,13,14};
        for(unsigned bad=0;bad<5;++bad) {
            auto* pointer=bad==2?reinterpret_cast<TextureBaseClass*>(1):bad==3?nullptr:texture;
            const auto refs=texture->Num_Refs();
            check(!edge.begin_source_references(edge.generation()+(bad==0),bad==1?nullptr:&pointer,
                bad==4?0:1,token),"invalid source admission accepted");
            check(token.owner==11 && token.sequence==12 && token.generation==13 && token.units==14
                && texture->Num_Refs()==refs && edge.source_revision()==revision
                && texture->last_accessed()==accessed && !texture->initializations,
                "invalid admission changed source or token");
        }
        auto* unready=source(edge,destroyed,false);TextureBaseClass* unready_provider=unready;
        check(!edge.begin_source_references(edge.generation(),&unready_provider,1,token),"unready source admitted");
        unready->Release_Ref();
        check(!edge.begin_source_references(edge.generation(),units.data(),65,token)
            && texture->Num_Refs()==1,"source bound+1 changed refs");
        check(edge.begin_source_references(edge.generation(),units.data(),64,token)
            && texture->Num_Refs()==65 && texture->last_accessed()==accessed
            && !texture->initializations,"exact source capacity rejected or performed lazy work");
        const auto owner=token;
        check(!edge.begin_source_references(edge.generation(),units.data(),1,token)
            && token.sequence==owner.sequence,"nested admission poisoned owner");
        for(unsigned field=0;field<4;++field) {
            auto foreign=owner;
            if(field==0) ++foreign.owner;if(field==1) ++foreign.sequence;
            if(field==2) ++foreign.generation;if(field==3) --foreign.units;
            check(!edge.finish_source_references(foreign) && !edge.cancel_source_references(foreign)
                && edge.reserved_source_reference_count()==64,"foreign token consumed owner");
        }
        const auto before_finish=device.snapshot();
        check(edge.finish_source_references(owner) && !edge.finish_source_references(owner)
            && texture->Num_Refs()==65 && device.snapshot()==before_finish,"finish released or touched device");
        check(!edge.begin_source_references(edge.generation(),units.data(),1,token)
            && texture->Num_Refs()==65,"full queue admission changed refs");
        check(!edge.drain_source_references(edge.generation()+1) && edge.queued_source_reference_count()==64,
            "foreign generation drained queue");
        check(edge.drain_source_references(edge.generation()) && texture->Num_Refs()==1
            && edge.texture_handle(texture)==handle && edge.source_reference_release_count()==64,
            "surviving source identity changed");
        const auto empty=device.snapshot();
        check(edge.drain_source_references(edge.generation()) && device.snapshot()==empty,"empty drain touched device");
        check(edge.begin_source_references(edge.generation(),units.data(),2,token),"sole reference admission");
        sampling(edge,texture);
        texture->Release_Ref();
        check(edge.cancel_source_references(token) && destroyed==1,"cancel deleted sole pinned source");
        for(unsigned fault=0;fault<4;++fault) {
            const auto baseline=device.snapshot();const auto count=device.resource_counts();
            const auto source_revision=edge.source_revision();
            if(fault==0) device.fail_next_transaction_checkpoint();
            if(fault==1) device.fail_transaction_operation_after(0);
            if(fault==2) edge.fail_next_source_reference_commit();
            if(fault==3) { device.fail_transaction_operation_after(0);device.fail_next_transaction_diagnostic_allocation(); }
            check(!edge.drain_source_references(edge.generation()) && edge.queued_source_reference_count()==2
                && edge.source_revision()==source_revision && edge.texture_handle(texture)==handle
                && texture->Num_Refs()==2 && destroyed==1 && device.resource_counts()==count
                && device.snapshot()==baseline,"source cleanup failure half-published");
        }
        check(edge.drain_source_references(edge.generation()) && destroyed==2
            && edge.queued_source_reference_count()==0 && device.resource_counts().textures==0,
            "sole reference retry did not retire exactly once");
        auto* first=source(edge,destroyed);auto* second=NEW_REF(Texture,(destroyed));
        edge.publish_texture_alias(second,first);second->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,4,4);
        TextureBaseClass* aliases[]{first,second,second};
        const auto shared=edge.texture_handle(first);
        check(edge.begin_source_references(edge.generation(),aliases,3,token),"alias admission");
        first->Release_Ref();check(edge.finish_source_references(token),"alias finish");
        check(edge.drain_source_references(edge.generation()) && destroyed==3 && second->Num_Refs()==1
            && edge.texture_handle(second)==shared && device.resource_counts().textures==1,
            "alias multiplicity prematurely retired native owner");
        TextureBaseClass* sole=second;
        check(edge.begin_source_references(edge.generation(),&sole,1,token),"last alias admission");
        second->Release_Ref();check(edge.shutdown_source_references() && destroyed==4
            && !edge.reserved_source_reference_count() && !edge.queued_source_reference_count()
            && !device.resource_counts().total(),"shutdown alias residual");
        auto* invalidated=source(edge,destroyed);TextureBaseClass* pointer=invalidated;
        check(edge.begin_source_references(edge.generation(),&pointer,1,token),"invalidation pin");
        invalidated->Invalidate();
        check(invalidated->Num_Refs()==1 && !invalidated->Is_Initialized()
            && !edge.queued_source_reference_count() && !edge.reserved_source_reference_count(),
            "invalidation failed to cancel and drain receiver pins");
        invalidated->Release_Ref();
        check(destroyed==5 && !device.resource_counts().total(),"immediate pre-teardown residual");
        auto* failure=source(edge,destroyed);pointer=failure;
        check(edge.begin_source_references(edge.generation(),&pointer,1,token),"invalidation failure pin");
        device.fail_next_transaction_checkpoint();bool caught=false;
        try { failure->Invalidate(); } catch(const std::runtime_error&) {caught=true;}
        check(caught && failure->Is_Initialized() && failure->Num_Refs()==2
            && edge.queued_source_reference_count()==1,"failed invalidation mutated receiver");
        failure->Release_Ref(); // only the queue owns the receiver when entering Invalidate.
        failure->Invalidate();
        check(destroyed==6 && !device.resource_counts().total(),"sole receiver invalidation lifetime");
        auto* shutdown=source(edge,destroyed);pointer=shutdown;
        check(edge.begin_source_references(edge.generation(),&pointer,1,token),"shutdown failure pin");
        shutdown->Release_Ref();device.fail_next_transaction_checkpoint();
        check(!edge.shutdown_source_references() && edge.queued_source_reference_count()==1 && destroyed==6,
            "shutdown failure abandoned pins");
        check(edge.shutdown_source_references() && destroyed==7 && !device.resource_counts().total(),
            "shutdown retry residual");
        auto* missing=NEW_REF(Texture,(destroyed));
        const auto fallback=edge.missing_texture();
        edge.publish_texture(missing,fallback,true);missing->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,4,4);
        pointer=missing;check(edge.begin_source_references(edge.generation(),&pointer,1,token),"missing source pin");
        missing->Release_Ref();check(edge.finish_source_references(token)
            && edge.drain_source_references(edge.generation()) && destroyed==8
            && device.resource_counts().textures==1,"shared missing owner retired by source");
    }
    check(!device.resource_counts().total(),"source generation residual");
    {
        Edge edge(device);auto* shutdown=source(edge,destroyed);TextureBaseClass* pointer=shutdown;
        Edge::SourceReferenceToken token;
        check(edge.begin_source_references(edge.generation(),&pointer,1,token),"destructor pin admission");
        shutdown->Release_Ref(); // active reservation must cancel/drain in owner destruction.
    }
    check(destroyed==9 && !device.resource_counts().total(),"active reservation destructor residual");
}
struct Scene {
    BgfxGpuDevice& device;ShaderHandle vs,fs;PipelineHandle pipeline;
    BufferHandle uniform,vertices,indices;TextureHandle color,depth;SamplerHandle sampler;
    explicit Scene(BgfxGpuDevice& owner):device(owner)
    {
        vs=device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"reference viewport");
        fs=device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"reference source");
        PipelineDesc state;state.vertex_shader=vs;state.fragment_shader=fs;
        state.vertex_layout=VertexLayout::position_color_uv;state.color_format=TextureFormat::bgra8;
        state.raster.cull=CullMode::none;
        pipeline=device.create_pipeline(PipelineKey(state),"reference sample");
        std::array<float,4> viewport{64,64,0,0};
        struct Vertex {float x,y;UInt32 diffuse;float u,v;};
        std::array<Vertex,3> geometry{{{0,0,0xffffffff,0,0},{128,0,0xffffffff,2,0},{0,128,0xffffffff,0,2}}};
        std::array<UInt16,3> order{0,1,2};
        uniform=device.create_buffer({sizeof(viewport),BufferUsage::uniform,true},"reference viewport");
        vertices=device.create_buffer({sizeof(geometry),BufferUsage::vertex,true},"reference geometry");
        indices=device.create_buffer({sizeof(order),BufferUsage::index,true},"reference indices");
        check(vs && fs && pipeline && uniform && vertices && indices
            && device.upload({uniform,sizeof(viewport),0,sizeof(viewport)},viewport.data())
            && device.upload({vertices,sizeof(geometry),0,sizeof(geometry)},geometry.data())
            && device.upload({indices,sizeof(order),0,sizeof(order)},order.data()),"reference sample resources");
        TextureDesc target;target.width=target.height=64;target.render_target=true;target.format=TextureFormat::bgra8;
        color=device.create_texture(target,"reference target");target.format=TextureFormat::depth24_stencil8;
        depth=device.create_texture(target,"reference depth");SamplerDesc nearest;
        nearest.min_filter=nearest.mag_filter=nearest.mip_filter=Filter::nearest;
        sampler=device.create_sampler(nearest,"reference sampling");
        check(color && depth && sampler,"reference sample targets");
    }
    std::vector<UInt8> render(TextureHandle sample)
    {
        RenderPassDesc pass;pass.color_targets[0]=color;pass.color_target_count=1;
        pass.depth_target=depth;pass.width=pass.height=64;
        DrawDesc draw;draw.pipeline=pipeline;draw.vertex_buffer=vertices;draw.index_buffer=indices;
        draw.index_element_size=IndexElementSize::uint16;draw.vertex_or_index_count=3;
        draw.vertex_bindings.uniform_count=1;draw.vertex_bindings.uniforms[0]={uniform,0,16};
        draw.fragment_bindings.texture_count=1;draw.fragment_bindings.textures[0]=sample;
        draw.fragment_bindings.samplers[0]=sampler;
        check(device.begin_pass(pass,"source identity sample") && device.draw(draw) && device.end_pass(),
            "reference sample draw");
        return device.readback_rgba(color);
    }
    ~Scene(){device.destroy(pipeline);device.destroy(vs);device.destroy(fs);device.destroy(uniform);
        device.destroy(vertices);device.destroy(indices);device.destroy(color);device.destroy(depth);device.destroy(sampler);}
};
void physical()
{
    BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
    BgfxGpuDevice device(options);unsigned destroyed=0;
    {
        Scene scene(device);
        Edge edge(device);auto* texture=source(edge,destroyed);TextureBaseClass* pointer=texture;
        const auto handle=edge.texture_handle(texture);
        const auto pixels=scene.render(handle);const unsigned center=(32*64+32)*4;
        check(pixels.size()==64*64*4 && pixels[center]==0 && pixels[center+1]==255
            && pixels[center+2]==0,"physical accepted pixel baseline");
        sampling(edge,texture);
        Edge::SourceReferenceToken token;
        check(edge.begin_source_references(edge.generation(),&pointer,1,token),"physical source pin");
        texture->Release_Ref();
        const auto frames=device.native_frame_advance_count();const auto retires=device.native_retirement_destroy_count();
        check(edge.finish_source_references(token) && device.native_frame_advance_count()==frames
            && device.native_retirement_destroy_count()==retires,"physical finish called native boundary");
        edge.fail_next_source_reference_commit();
        check(!edge.drain_source_references(edge.generation()) && edge.texture_handle(texture)==handle
            && edge.queued_source_reference_count()==1 && destroyed==0,"physical rejection changed identity");
        check(scene.render(handle)==pixels,"physical rejection changed accepted sampled pixels");
        const auto retry_frames=device.native_frame_advance_count();
        const auto retry_retires=device.native_retirement_destroy_count();
        check(edge.drain_source_references(edge.generation()) && destroyed==1
            && device.native_frame_advance_count()==retry_frames && device.native_retirement_destroy_count()==retry_retires,
            "physical source drain made immediate native calls");
    }
    check(device.wait_idle() && device.live_resource_count()==0,"physical source residual");
}
void terminal_models()
{
    for(unsigned after=0;after<3;++after) {
        const auto child=fork();check(child>=0,"terminal model fork");
        if(!child) {
            std::set_terminate([]{std::fputs("modeled source-reference terminal failure\n",stderr);std::_Exit(77);});
            RecordingGpuDevice device;Edge edge(device);unsigned destroyed=0;
            auto* texture=source(edge,destroyed);TextureBaseClass* pointer=texture;Edge::SourceReferenceToken token;
            check(edge.begin_source_references(edge.generation(),&pointer,1,token),"terminal pin");
            texture->Release_Ref();check(edge.finish_source_references(token),"terminal finish");
            if(after==2) {
                edge.fail_next_source_reference_commit();
                edge.~OriginalGpuEdge(); // Mandatory shutdown must terminate, never return with pins.
                std::_Exit(1);
            }
            if(after==1) check(edge.drain_source_references(edge.generation()) && destroyed==1
                && !edge.queued_source_reference_count() && !device.resource_counts().total(),"terminal committed disposition");
            else {
                edge.fail_next_source_reference_commit();
                check(!edge.drain_source_references(edge.generation()) && destroyed==0
                    && edge.queued_source_reference_count()==1 && edge.texture_handle(texture),"terminal failed disposition");
            }
            // Deliberately no retry, replacement generation or normal teardown.
            std::terminate();
        }
        int status=0;check(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==77,
            "terminal model returned normally or failed disposition");
    }
}
}
int main(int argc,char** argv)
{
    try {
        struct Services {
            Services(){char reason[64]{};check(zh::original_process::initialize_services(0,reason,sizeof(reason)),"source services");}
            ~Services(){zh::original_process::shutdown_services();}
        } services;
        for(unsigned generation=0;generation<2;++generation)
            if(argc==2 && std::string(argv[1])=="--gpu") physical();else recording();
        if(argc==1) terminal_models();
        std::cout<<"original source reference: ok\n";return 0;
    } catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
