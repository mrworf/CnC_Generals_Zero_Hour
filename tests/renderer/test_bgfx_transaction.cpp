#include "zh/platform/bgfx_device.h"
#include "../../src/renderer/bgfx_transaction_state.h"
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace zh::renderer;
namespace {
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
DeviceTransactionDesc budget(UInt64 generation=7)
{ return {DeviceTransactionMode::frame_commands,generation,128,256,8U*1024U*1024U,32}; }
void cpu() {
    using State=detail::BgfxTransactionState;
    check(detail::bounded_view_rectangle(32767,32767,65535,65535)
        && !detail::bounded_view_rectangle(32768,0,1,1)
        && !detail::bounded_view_rectangle(0,32768,1,1)
        && !detail::bounded_view_rectangle(0,0,65536,1)
        && !detail::bounded_view_rectangle(0,0,1,65536),"signed coordinate exact representation");
    const detail::BgfxCapturedSampler sample{8,7,6,5,4};
    check(detail::sampler_alias(sample,sample)==detail::BgfxSamplerAlias::identical,"identical stage alias");
    for (unsigned field=0;field<5;++field) {
        auto conflicting=sample;
        if (field==0) ++conflicting.stage;
        if (field==1) ++conflicting.uniform;
        if (field==2) ++conflicting.texture;
        if (field==3) ++conflicting.source_sampler;
        if (field==4) ++conflicting.flags;
        check(detail::sampler_alias(sample,conflicting)==detail::BgfxSamplerAlias::conflict,"conflicting stage alias");
    }
    auto other=sample;++other.stage;++other.uniform;
    check(detail::sampler_alias(sample,other)==detail::BgfxSamplerAlias::distinct,"independent native stages");
    const std::array<UInt8,16> payload{{1,2,3,4}};auto changed=payload;changed[15]=1;
    check(detail::same_uniform_payload(1,16,payload.data(),1,16,payload.data())
        && !detail::same_uniform_payload(1,16,payload.data(),1,16,changed.data())
        && !detail::same_uniform_payload(1,16,payload.data(),2,16,payload.data())
        && !detail::same_uniform_payload(1,16,payload.data(),1,15,payload.data()),"uniform alias exact bytes/count");
    for (UInt64 generation=1;generation<=2;++generation) {
        auto desc=budget(generation); State state;
        check(state.begin(desc,generation,2,2),"frame CPU admission");
        const auto token=state.token();
        check(state.retain_views(32) && !state.retain_views(1) && !state.finish(token,true)
            && state.finish(token,false),"view bound plus one/abort");
        check(state.begin(desc,generation,2,2),"frame CPU retry");
        auto foreign=state.token(); ++foreign.device;
        check(!state.finish(foreign,true) && !state.finish(foreign,false)
            && !state.begin(desc,generation,0,0),"foreign/nested owner");
        check(state.finish(state.token(),true) && !state.finish(token,false),"exactly once finish");
        for (unsigned category=0;category<10;++category) {
            auto bad=desc;
            if (category==0) bad.views=0;
            if (category==1) bad.views=257;
            if (category==2) bad.commands=4097;
            if (category==3) bad.resources=4097;
            if (category==4) bad.bytes=State::maximum_bytes+1;
            if (category==5) bad.generation=0;
            if (category==6) bad.mode=static_cast<DeviceTransactionMode>(255);
            if (category==7) bad.commands=0;
            if (category==8) bad.resources=0;
            if (category==9) bad.bytes=0;
            check(!state.begin(bad,generation,0,0) && !state.active(),"malformed frame descriptor");
        }
        auto maximum=desc;maximum.commands=4096;maximum.resources=4096;
        maximum.bytes=State::maximum_bytes;maximum.views=256;
        for (unsigned field=0;field<4;++field) {
            check(state.begin(maximum,generation,0,0),"exact maximum admission");
            bool extra=true;
            if (field==0) {for (unsigned i=0;i<4096;++i) check(state.charge(),"exact command maximum");extra=state.charge();}
            if (field==1) {check(state.retain_resources(4096),"exact resource maximum");extra=state.retain_resources(1);}
            if (field==2) {check(state.retain_bytes(State::maximum_bytes),"exact byte maximum");extra=state.retain_bytes(1);}
            if (field==3) {check(state.retain_views(256),"exact view maximum");extra=state.retain_views(1);}
            check(!extra && !state.finish(state.token(),true) && state.finish(state.token(),false),"maximum plus one escaped");
        }
    }
}
struct Scene {
    BgfxGpuDevice& device;
    ShaderHandle vs,fs; PipelineHandle pipeline;
    BufferHandle uniform,vertices,indices;
    TextureHandle sample,color,depth; SamplerHandle sampler;
    std::array<float,4> viewport{64,64,0,0};
    struct Vertex { float x,y; UInt32 color; float u,v; };
    std::array<Vertex,3> triangle{{{0,0,0xffffffffU,0,0},{128,0,0xffffffffU,2,0},{0,128,0xffffffffU,0,2}}};
    std::array<UInt16,3> index_bytes{0,1,2};
    std::array<UInt8,64> red{},green{};
    explicit Scene(BgfxGpuDevice& owner):device(owner) {
        vs=device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"generated vertex");
        fs=device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"generated fragment");
        PipelineDesc state; state.vertex_shader=vs; state.fragment_shader=fs;
        state.vertex_layout=VertexLayout::position_color_uv; state.color_format=TextureFormat::bgra8;
        state.raster.cull=CullMode::none;
        pipeline=device.create_pipeline(PipelineKey(state),"generated program");
        uniform=device.create_buffer({sizeof(viewport),BufferUsage::uniform,true},"uniform");
        vertices=device.create_buffer({sizeof(triangle),BufferUsage::vertex,true},"vertices");
        indices=device.create_buffer({sizeof(index_bytes),BufferUsage::index,true},"indices");
        check(vs && fs && pipeline && uniform && vertices && indices,"generated frame resources");
        check(device.upload({uniform,sizeof(viewport),0,sizeof(viewport)},viewport.data())
            && device.upload({vertices,sizeof(triangle),0,sizeof(triangle)},triangle.data())
            && device.upload({indices,sizeof(index_bytes),0,sizeof(index_bytes)},index_bytes.data()),"generated frame bytes");
        for (unsigned i=0;i<16;++i) {red[4*i]=255;red[4*i+3]=255;green[4*i+1]=255;green[4*i+3]=255;}
        sample=device.create_texture({4,4},"sample");
        check(sample && device.upload_texture({sample,4,4,16,64,0},red.data()),"generated sample");
        SamplerDesc sampling; sampling.min_filter=sampling.mag_filter=sampling.mip_filter=Filter::nearest;
        sampler=device.create_sampler(sampling,"sampler");
        TextureDesc target;target.width=target.height=64;target.render_target=true;target.format=TextureFormat::bgra8;
        color=device.create_texture(target,"color");target.format=TextureFormat::depth24_stencil8;
        depth=device.create_texture(target,"depth");check(color && depth && sampler,"generated targets");
    }
    RenderPassDesc pass() const {
        RenderPassDesc result; result.color_target_count=1;result.color_targets[0]=color;result.depth_target=depth;
        result.width=result.height=64;result.target_generation=7;return result;
    }
    DrawDesc draw() const {
        DrawDesc result;result.pipeline=pipeline;result.vertex_buffer=vertices;result.index_buffer=indices;
        result.index_element_size=IndexElementSize::uint16;result.vertex_or_index_count=3;
        result.vertex_bindings.uniform_count=1;result.vertex_bindings.uniforms[0]={uniform,0,sizeof(viewport)};
        result.fragment_bindings.texture_count=1;result.fragment_bindings.textures[0]=sample;
        result.fragment_bindings.samplers[0]=sampler;return result;
    }
    void frame() {
        check(device.begin_pass(pass(),"generated frame") && device.draw(draw()) && device.end_pass()
            && device.present(color),device.last_error().c_str());
    }
    std::vector<UInt8> accepted() {
        check(device.begin_pass(pass(),"ordinary baseline") && device.draw(draw()) && device.end_pass(),
            device.last_error().c_str());return device.readback_rgba(color);
    }
    void teardown() {
        device.destroy(pipeline);device.destroy(vs);device.destroy(fs);device.destroy(uniform);
        device.destroy(vertices);device.destroy(indices);device.destroy(sample);device.destroy(sampler);
        device.destroy(color);device.destroy(depth);
    }
};
void physical() {
    check(SDL_Init(SDL_INIT_VIDEO),"SDL generated video service");
    auto* window=SDL_CreateWindow("generated frame journal",64,64,SDL_WINDOW_HIDDEN);
    check(window,"SDL generated window");
    for (unsigned generation=0;generation<2;++generation) {
        bool query=true;int width=64,height=64;
        BgfxOptions options;options.shader_root=ZH_BGFX_TRANSACTION_SHADER_DIR;
        options.pixel_extent_query=[&](SDL_Window* owner,int* w,int* h) {
            check(owner==window,"foreign extent query");*w=width;*h=height;return query;
        };
        BgfxGpuDevice device(options);check(device.claim_window(window),device.last_error().c_str());
        Scene scene(device);const auto baseline=scene.accepted();const auto resources=device.live_resource_count();
        DeviceTransactionToken token;
        const auto assert_prior=[&]() {
            check(!device.pass_active() && device.active_pass_extent()==std::pair<UInt32,UInt32>{0,0}
                && device.live_resource_count()==resources,"frame shadow/resource baseline");
            check(device.readback_rgba(scene.color)==baseline,"rejected candidate altered pixels");
        };
        for (unsigned fault=0;fault<13;++fault) {
            DeviceTransactionToken unchanged{99,98,97,DeviceTransactionMode::idle_preparation};
            device.fail_transaction_checkpoint_copy_after(fault);
            bool threw=false;
            try {(void)device.begin_device_transaction(budget(),unchanged);}catch (const std::bad_alloc&) {threw=true;}
            check(threw && unchanged.device==99 && !device.staged_frame_command_count(),"partial checkpoint published owner");
            assert_prior();
        }
        for (unsigned stage=0;stage<3;++stage) for (unsigned fault=0;fault<(stage==0 ? 1U : stage==1 ? 6U : 4U);++fault) {
            check(device.begin_device_transaction(budget(),token),"frozen copy fault admission");
            if (stage>0) check(device.begin_pass(scene.pass(),"copy prefix"),"copy prefix begin");
            if (stage>1) check(device.draw(scene.draw()) && device.end_pass(),"copy prefix draw/end");
            const auto packets=device.staged_frame_command_count(),owned=device.live_owned_native_reference_count();
            const auto phase=device.pass_active();const auto extent=device.active_pass_extent();
            device.fail_transaction_frame_copy_after(fault);bool threw=false;
            try {if (stage==0) (void)device.begin_pass(scene.pass(),"copy failure");
                if (stage==1) (void)device.draw(scene.draw());if (stage==2) (void)device.present(scene.color);
            }catch (const std::bad_alloc&) {threw=true;}
            check(threw && device.staged_frame_command_count()==packets && device.live_owned_native_reference_count()==owned
                && device.pass_active()==phase && device.active_pass_extent()==extent
                && !device.commit_device_transaction(token) && device.abort_device_transaction(token),"copy exception published shadow/native state");
            assert_prior();
        }
        for (unsigned category=0;category<3;++category) {
            auto bounded=budget();
            if (category==0) bounded.commands=1;
            if (category==1) bounded.views=1;
            if (category==2) bounded.resources=static_cast<UInt32>(resources)+1;
            check(device.begin_device_transaction(bounded,token) && device.begin_pass(scene.pass(),"exact first boundary"),"bounded frame begin");
            const auto packets=device.staged_frame_command_count(),owned=device.live_owned_native_reference_count();
            bool rejected=category==0 ? !device.end_pass() : category==1 ? !device.set_viewport({0,0,64,64,0,1}) : !device.draw(scene.draw());
            check(rejected && device.staged_frame_command_count()==packets && device.live_owned_native_reference_count()==owned
                && !device.commit_device_transaction(token) && device.abort_device_transaction(token),"bound plus one published partial state");assert_prior();
        }
        for (unsigned fault=0;fault<6;++fault) {
            check(device.begin_device_transaction(budget(),token),"operation fault admission");
            const auto frames=device.native_frame_advance_count();
            device.fail_transaction_operation_after(fault);
            for (unsigned step=0;step<=fault;++step) {
                const auto phase=device.pass_active();const auto extent=device.active_pass_extent();
                const auto packets=device.staged_frame_command_count();
                bool result=false;
                if (step==0) result=device.begin_pass(scene.pass(),"fault begin");
                if (step==1) result=device.set_viewport({0,0,64,64,0,1});
                if (step==2) {ViewportClearDesc clear;clear.color_target=scene.color;clear.target_generation=7;
                    clear.width=clear.height=16;clear.color=true;result=device.clear_viewport(clear);}
                if (step==3) result=device.draw(scene.draw());
                if (step==4) result=device.end_pass();
                if (step==5) result=device.present(scene.color);
                check(result==(step<fault),"operation fault boundary mismatch");
                if (step==fault) check(device.pass_active()==phase && device.active_pass_extent()==extent
                    && device.staged_frame_command_count()==packets,"fault published partial frame shadow");
            }
            check(!device.commit_device_transaction(token) && device.abort_device_transaction(token)
                && device.native_frame_advance_count()==frames,"operation fault completed frame");assert_prior();
        }
        for (unsigned fault=0;fault<3;++fault) {
            check(device.begin_device_transaction(budget(),token),"publication fault admission");
            device.fail_transaction_native_publication_after(fault);
            const auto frames=device.native_frame_advance_count();
            const bool begun=device.begin_pass(scene.pass(),"publication begin");
            check(begun==(fault>0),"FBO publication fault mismatch");
            if (begun) {
                const bool drawn=device.draw(scene.draw());check(drawn==(fault>1),"geometry publication fault mismatch");
                if (drawn) check(device.end_pass() && !device.present(scene.color),"presentation publication fault mismatch");
            }
            check(!device.commit_device_transaction(token) && device.abort_device_transaction(token)
                && device.native_frame_advance_count()==frames,"publication fault advanced frame");assert_prior();
        }
        // Actual compiled vertex/fragment providers share texture stage and
        // uniform identifier; native aliases must agree, not last-writer win.
        const auto fixture=[](const char* name) {return std::string(name);};
        const auto alias_vs=device.create_shader({ShaderStage::vertex,fixture("transaction_alias.vert"),1,1},"alias vertex");
        const auto distinct_vs=device.create_shader({ShaderStage::vertex,fixture("transaction_distinct.vert"),1,1},"distinct sampler vertex");
        const auto alias_fs=device.create_shader({ShaderStage::fragment,fixture("transaction_alias.frag"),1,1},"alias fragment");
        PipelineDesc alias_state;alias_state.vertex_shader=alias_vs;alias_state.fragment_shader=alias_fs;
        alias_state.vertex_layout=VertexLayout::position_color_uv;alias_state.color_format=TextureFormat::bgra8;
        alias_state.raster.cull=CullMode::none;
        const auto alias_pipeline=device.create_pipeline(PipelineKey(alias_state),"alias program");
        alias_state.vertex_shader=distinct_vs;
        const auto distinct_pipeline=device.create_pipeline(PipelineKey(alias_state),"distinct sampler program");
        const auto other_sampler=device.create_sampler({},"conflicting source sampler");
        const auto other_texture=device.create_texture({4,4},"conflicting source texture");
        check(other_texture && device.upload_texture({other_texture,4,4,16,64,0},scene.red.data()),"alias texture bytes");
        const auto other_uniform=device.create_buffer({16,BufferUsage::uniform,true},"conflicting uniform");
        const std::array<float,4> different{64,64,1,0};
        check(alias_vs && alias_fs && distinct_vs && alias_pipeline && distinct_pipeline && other_sampler && other_uniform
            && device.upload({other_uniform,16,0,16},different.data()),"alias providers");
        auto alias=scene.draw();alias.pipeline=alias_pipeline;
        alias.vertex_bindings.texture_count=1;alias.vertex_bindings.textures[0]=scene.sample;
        alias.vertex_bindings.samplers[0]=scene.sampler;
        alias.fragment_bindings.uniform_count=1;alias.fragment_bindings.uniforms[0]={scene.uniform,0,16};
        for (unsigned category=0;category<4;++category) {
            auto conflicting=alias;
            if (category==0) conflicting.fragment_bindings.samplers[0]=other_sampler;
            if (category==1) conflicting.fragment_bindings.textures[0]=other_texture;
            if (category==2) conflicting.fragment_bindings.uniforms[0]={other_uniform,0,16};
            if (category==3) conflicting.pipeline=distinct_pipeline;
            check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"alias rejection"),"alias admission");
            const auto packets=device.staged_frame_command_count(),owned=device.live_owned_native_reference_count();
            check(!device.draw(conflicting) && device.staged_frame_command_count()==packets
                && device.live_owned_native_reference_count()==owned && !device.commit_device_transaction(token)
                && device.abort_device_transaction(token),"conflicting alias published candidate");
            check(device.readback_rgba(scene.color)==baseline,"alias rejection changed pixels");
        }
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"equal alias")
            && device.draw(alias) && device.end_pass() && device.present(scene.color)
            && device.commit_device_transaction(token),"identical aliases were not canonicalized");
        check(device.readback_rgba(scene.color)==baseline,"canonical alias pixels");
        device.destroy(alias_pipeline);device.destroy(distinct_pipeline);device.destroy(alias_vs);device.destroy(distinct_vs);
        device.destroy(alias_fs);device.destroy(other_sampler);device.destroy(other_texture);device.destroy(other_uniform);
        for (unsigned category=0;category<8;++category) {
            const auto submitted=device.bounded_submission_count(),frames=device.native_frame_advance_count();
            check(device.begin_device_transaction(budget(),token),"frame fault admission");
            check(device.begin_pass(scene.pass(),"faulted candidate") && device.draw(scene.draw()),"staged candidate draw");
            bool result=true;
            if (category==0) {auto bad=scene.draw();bad.first_index=3;result=device.draw(bad);}
            if (category==1) {auto bad=scene.draw();bad.vertex_bindings.uniforms[0].offset=1;result=device.draw(bad);}
            if (category==2) result=device.set_viewport({0.5f,0,63,64,0,1});
            if (category==3) {device.fail_transaction_operation_after(0);result=device.end_pass();}
            if (category==4) {check(device.end_pass(),"query end");query=false;result=device.present(scene.color);query=true;}
            if (category==5) {check(device.end_pass(),"extent end");width=65536;result=device.present(scene.color);width=64;}
            if (category==6) {check(device.end_pass() && device.present(scene.color),"duplicate prefix");result=device.present(scene.color);}
            if (category==7) {auto stale=scene.draw();device.destroy(scene.sample);result=device.draw(stale);}
            check(!result && !device.commit_device_transaction(token),"faulted batch committed");
            const auto destroys=device.native_retirement_destroy_count();
            check(device.abort_device_transaction(token) && !device.pass_active()
                && device.live_resource_count()==resources && device.bounded_submission_count()==submitted
                && device.native_frame_advance_count()==frames && device.native_retirement_destroy_count()==destroys,
                "abort changed accepted ownership/frame");
            check(device.readback_rgba(scene.color)==baseline,"late rejection touched accepted pixels");
        }
        const auto submitted=device.bounded_submission_count();
        check(device.begin_device_transaction(budget(),token),"clean frame admission");
        scene.frame();const auto staged=device.staged_frame_command_count();
        check(staged==5 && device.bounded_submission_count()==submitted,"staging submitted commands");
        device.fail_next_transaction_submission();
        check(!device.commit_device_transaction(token) && device.staged_frame_command_count()==staged,
            "native rejection consumed immutable batch");
        const auto retired_before_commit=device.native_retirement_destroy_count();
        check(device.commit_device_transaction(token) && device.bounded_submission_count()==submitted+1
            && !device.commit_device_transaction(token) && !device.abort_device_transaction(token),"retry did not emit once");
        check(device.pending_native_retirement_count()==4 && device.native_retirement_destroy_count()==retired_before_commit,
            "frame commit destroyed or lost typed candidates");
        check(device.readback_rgba(scene.color)==baseline,"committed frame pixels differ");
        check(!device.pending_native_retirement_count() && device.native_retirement_destroy_count()==retired_before_commit+4,
            "ordinary boundary did not retire exactly four acquired units");
        device.record_marker("empty ordinary retirement boundary");
        check(device.native_retirement_destroy_count()==retired_before_commit+4,"typed candidate retired twice");
        check(device.begin_device_transaction(budget(),token),"external encoder rejection admission");scene.frame();
        const auto before_overlap=device.bounded_submission_count(),frames_before_overlap=device.native_frame_advance_count();
        auto* overlapping=bgfx::begin(true);check(overlapping,"overlap encoder unavailable");
        check(!device.commit_device_transaction(token) && device.staged_frame_command_count()==5
            && device.bounded_submission_count()==before_overlap && device.native_frame_advance_count()==frames_before_overlap,
            "actual native rejection consumed batch");
        bgfx::end(overlapping);
        check(!device.commit_device_transaction(token) && device.abort_device_transaction(token)
            && device.native_frame_advance_count()==frames_before_overlap,"external encoder end manufactured readiness");
        check(device.wait_idle() && device.readback_rgba(scene.color)==baseline,"external owner ordinary boundary altered pixels");
        check(device.begin_device_transaction(budget(),token),"external owner fresh retry admission");scene.frame();
        check(device.commit_device_transaction(token) && device.bounded_submission_count()==before_overlap+1,"external owner retry emit count");
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"unfinished")
            && device.end_pass() && !device.commit_device_transaction(token) && device.abort_device_transaction(token),"unfinished frame completed");assert_prior();
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"diagnostic prefix"),"diagnostic admission");
        device.fail_next_transaction_diagnostic_allocation();bool diagnostic_threw=false;
        try {(void)device.set_viewport({-1,0,64,64,0,1});}catch (const std::bad_alloc&) {diagnostic_threw=true;}
        check(diagnostic_threw && !device.commit_device_transaction(token) && device.abort_device_transaction(token),"diagnostic exception escaped poisoning");assert_prior();
        // Source order: earlier full red draw, later half-width green draw,
        // then selected blue clear. View-global state must not edit old draws.
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"ordered pixels")
            && device.draw(scene.draw()) && device.upload_texture({scene.sample,4,4,16,64,0},scene.green.data())
            && device.set_viewport({32,0,32,64,0,1}) && device.draw(scene.draw()),"ordered draw staging");
        ViewportClearDesc blue;blue.color_target=scene.color;blue.target_generation=7;blue.width=blue.height=16;
        blue.color=true;blue.color_value={0,0,1,1};
        check(device.clear_viewport(blue) && device.end_pass() && device.present(scene.color)
            && device.commit_device_transaction(token),"ordered pixel commit");
        const auto ordered=device.readback_rgba(scene.color);
        const auto pixel=[&](unsigned x,unsigned y,unsigned channel){return ordered[(y*64+x)*4+channel];};
        check(pixel(8,8,2)==255 && pixel(16,32,0)==255 && pixel(48,32,1)==255,"view/clear/draw source order differs");
        check(device.upload_texture({scene.sample,4,4,16,64,0},scene.red.data()) && scene.accepted()==baseline,"ordinary pixel restoration");
        auto prefix=scene.pass();prefix.clear_color={0,0,1,1};
        check(device.begin_pass(prefix,"accepted ordinary prefix") && device.end_pass()
            && device.begin_device_transaction(budget(),token),"ordinary prefix admission");scene.frame();
        check(device.abort_device_transaction(token),"ordinary prefix abort");
        const auto prefix_pixels=device.readback_rgba(scene.color);
        check(prefix_pixels[0]==0 && prefix_pixels[1]==0 && prefix_pixels[2]==255,
            "abort erased/changed accepted ordinary prefix");
        check(scene.accepted()==baseline,"ordinary prefix restoration");
        check(device.begin_pass(prefix,"ordinary prefix before commit") && device.end_pass()
            && device.begin_device_transaction(budget(),token),"ordinary prefix commit admission");scene.frame();
        check(device.commit_device_transaction(token) && device.readback_rgba(scene.color)==baseline,"prefix view ordering changed");
        check(SDL_SetWindowSize(window,80,72),"generated window resize");width=80;height=72;
        for (bool commit : {false,true}) {
            check(device.begin_device_transaction(budget(),token),"resize admission");scene.frame();
            check(device.staged_frame_command_count()==6,"resize command missing or shadow not restored");
            check(commit ? device.commit_device_transaction(token) : device.abort_device_transaction(token),"resize finish");
            check(device.readback_rgba(scene.color)==baseline,"resize touched source target pixels");
        }
        check(device.begin_device_transaction(budget(),token),"post-resize publication admission");scene.frame();
        check(device.staged_frame_command_count()==5 && device.commit_device_transaction(token),"accepted resize identity not retained");
        // Captured shader/program ownership and byte versions cannot retarget
        // when public slots are removed and identical native handles deduplicate.
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"captured provider replacement")
            && device.draw(scene.draw()),"provider capture");
        device.destroy(scene.pipeline);device.destroy(scene.vs);device.destroy(scene.fs);
        const auto retry_vs=device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"replacement vertex");
        const auto retry_fs=device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"replacement fragment");
        PipelineDesc retry_state;retry_state.vertex_shader=retry_vs;retry_state.fragment_shader=retry_fs;
        retry_state.vertex_layout=VertexLayout::position_color_uv;retry_state.color_format=TextureFormat::bgra8;
        retry_state.raster.cull=CullMode::none;
        const auto retry_pipeline=device.create_pipeline(PipelineKey(retry_state),"replacement program");
        check(retry_vs && retry_fs && retry_pipeline && retry_vs!=scene.vs && retry_fs!=scene.fs
            && retry_pipeline!=scene.pipeline,"captured provider handle generation reused");
        auto degenerate=scene.triangle;for (auto& vertex:degenerate) vertex.x=vertex.y=0;
        const std::array<float,4> tiny_viewport{1,1,0,0};const std::array<UInt16,3> degenerate_indices{0,0,0};
        check(device.upload({scene.uniform,sizeof(tiny_viewport),0,sizeof(tiny_viewport)},tiny_viewport.data())
            && device.upload({scene.vertices,sizeof(degenerate),0,sizeof(degenerate)},degenerate.data())
            && device.upload({scene.indices,sizeof(degenerate_indices),0,sizeof(degenerate_indices)},degenerate_indices.data()),"captured byte replacement");
        check(device.end_pass() && device.present(scene.color) && device.commit_device_transaction(token),"captured provider removal invalidated batch");
        scene.vs=retry_vs;scene.fs=retry_fs;scene.pipeline=retry_pipeline;
        check(device.readback_rgba(scene.color)==baseline,"captured shader/geometry/uniform bytes retargeted");
        check(device.upload({scene.uniform,sizeof(scene.viewport),0,sizeof(scene.viewport)},scene.viewport.data())
            && device.upload({scene.vertices,sizeof(scene.triangle),0,sizeof(scene.triangle)},scene.triangle.data())
            && device.upload({scene.indices,sizeof(scene.index_bytes),0,sizeof(scene.index_bytes)},scene.index_bytes.data()),"ordinary byte restoration");
        // Freeze the first sampled version, then replace and remove the source.
        check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"COW version"),"COW admission");
        check(device.draw(scene.draw()) && device.upload_texture({scene.sample,4,4,16,64,0},scene.green.data()),"COW staging");
        device.destroy(scene.sample);
        const auto replacement=device.create_texture({4,4},"removed source retry");
        check(replacement && replacement!=scene.sample
            && device.upload_texture({replacement,4,4,16,64,0},scene.green.data()),"removed handle reused captured generation");
        check(device.end_pass() && device.present(scene.color) && device.commit_device_transaction(token),"captured removal retargeted draw");
        check(device.readback_rgba(scene.color)==baseline,"captured COW version changed");
        device.destroy(replacement);
        // Use this fixture's already-owned presentation/CompleteFrame boundary
        // for immutable partial-chain capture, COW, handle removal and retry.
        for (const auto format : {TextureFormat::rgba8,TextureFormat::bgra8,TextureFormat::bgr5a1})
        for (const auto filter : {Filter::nearest,Filter::linear}) {
            TextureDesc partial;partial.width=8;partial.height=4;partial.mip_levels=2;partial.format=format;
            const auto source=device.create_texture(partial,"partial-chain immutable source");
            const unsigned bpp=format==TextureFormat::bgr5a1 ? 2 : 4;
            const auto bytes=[&](unsigned w,unsigned h,bool green) {
                std::vector<UInt8> result(w*h*bpp);
                for(unsigned i=0;i<w*h;++i) {
                    if (bpp==2) {const UInt16 value=green ? 0x83e0 : 0xfc00;result[i*2]=UInt8(value);result[i*2+1]=UInt8(value>>8);}
                    else {result[i*4+(green ? 1 : format==TextureFormat::bgra8 ? 2 : 0)]=255;result[i*4+3]=255;}
                }
                return result;
            };
            const auto base_bytes=bytes(8,4,false),last_bytes=bytes(4,2,true),changed=bytes(4,2,false);
            check(source && device.upload_texture({source,8,4,8*bpp,base_bytes.size(),0},base_bytes.data()),
                "partial mip base bytes");
            scene.sample=source;
            const auto unknown_prior=device.readback_rgba(scene.color);
            check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"unknown declared mip"),
                "unknown mip frame admission");
            const auto unknown_packets=device.staged_frame_command_count(),unknown_owned=device.live_owned_native_reference_count();
            check(!device.draw(scene.draw()) && device.staged_frame_command_count()==unknown_packets
                && device.live_owned_native_reference_count()==unknown_owned && !device.commit_device_transaction(token)
                && device.abort_device_transaction(token) && device.readback_rgba(scene.color)==unknown_prior,
                "unknown declared mip published draw/resource/target state");
            check(device.upload_texture({source,4,2,4*bpp,last_bytes.size(),1},last_bytes.data()),"partial mip last bytes");
            auto far=scene.triangle;for(auto& vertex:far) {vertex.u*=2048;vertex.v*=2048;}
            check(device.upload({scene.vertices,sizeof(far),0,sizeof(far)},far.data()),"partial mip minification bytes");
            SamplerDesc sampling;sampling.min_filter=sampling.mag_filter=Filter::nearest;sampling.mip_filter=filter;
            const auto range_sampler=device.create_sampler(sampling,"preserved partial mip policy");
            const auto prior_sampler=scene.sampler;scene.sampler=range_sampler;scene.sample=source;
            const auto ordinary=scene.accepted();const auto center=(32U*64U+32U)*4U;
            check(ordinary.size()==64*64*4 && ordinary[center]==0 && ordinary[center+1]==255
                && ordinary[center+2]==0 && ordinary[center+3]==255,"ordinary partial-chain pixel baseline");
            check(device.begin_device_transaction(budget(),token) && device.begin_pass(scene.pass(),"immutable declared mip range")
                && device.draw(scene.draw()) && device.upload_texture({source,4,2,4*bpp,changed.size(),1},changed.data()),
                "partial range capture/COW preparation");
            device.destroy(source);
            auto single=partial;single.mip_levels=1;
            const auto reused=device.create_texture(single,"different range removed-handle retry");
            check(reused && reused!=source,"partial source handle generation reused");device.destroy(reused);
            check(device.end_pass() && device.present(scene.color),"partial source frame completion");
            const auto captured=device.staged_frame_command_count(),emitted=device.bounded_submission_count();
            device.fail_next_transaction_submission();
            check(!device.commit_device_transaction(token) && device.staged_frame_command_count()==captured
                && device.bounded_submission_count()==emitted,"partial-range rejection consumed immutable phase");
            check(device.commit_device_transaction(token) && device.bounded_submission_count()==emitted+1
                && device.readback_rgba(scene.color)==ordinary && !device.pending_native_retirement_count(),
                "immutable partial range/COW/lease retry differs from ordinary pixels");
            scene.sample={};scene.sampler=prior_sampler;device.destroy(range_sampler);
        }
        check(device.upload({scene.vertices,sizeof(scene.triangle),0,sizeof(scene.triangle)},scene.triangle.data()),
            "partial mip ordinary geometry restoration");
        // Suspension still completes once, without presentation draws.
        width=height=0;check(device.begin_device_transaction(budget(),token),"suspension admission");
        check(device.present(scene.color) && device.staged_frame_command_count()==1
            && device.commit_device_transaction(token),"suspension completion");width=80;height=72;
        scene.teardown();device.release_window();
        check(device.wait_idle() && !device.live_resource_count() && !device.live_owned_native_reference_count(),"frame generation residual");
    }
    SDL_DestroyWindow(window);SDL_Quit();
}
}
int main(int argc,char** argv) {
    try {cpu();if (argc==2 && std::string(argv[1])=="--gpu") physical();
        std::cout<<"bounded bgfx frame journal: generated controls passed\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
