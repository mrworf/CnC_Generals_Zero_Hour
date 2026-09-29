#include "zh/renderer/recording_device.h"

#include <array>
#include <iostream>
#include <new>
#include <stdexcept>
#include <vector>

using namespace zh::renderer;
namespace {
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
// An existing consumer implementing only the ordinary interface still links
// and rejects the new capability without invoking any ordinary operation.
struct OrdinaryOnlyDevice final:GpuDevice {
    unsigned calls=0;
    BufferHandle create_buffer(const BufferDesc&,std::string_view) override {++calls;return {};}
    TextureHandle create_texture(const TextureDesc&,std::string_view) override {++calls;return {};}
    SamplerHandle create_sampler(const SamplerDesc&,std::string_view) override {++calls;return {};}
    ShaderHandle create_shader(const ShaderDesc&,std::string_view) override {++calls;return {};}
    PipelineHandle create_pipeline(const PipelineKey&,std::string_view) override {++calls;return {};}
    ValidationResult upload(const UploadDesc&,const void*) override {++calls;return {};}
    ValidationResult upload_texture(const TextureUploadDesc&,const void*) override {++calls;return {};}
    ValidationResult begin_pass(const RenderPassDesc&,std::string_view) override {++calls;return {};}
    ValidationResult draw(const DrawDesc&) override {++calls;return {};}
    ValidationResult end_pass() override {++calls;return {};}
    ValidationResult present(TextureHandle) override {++calls;return {};}
    void destroy(BufferHandle) override {++calls;}
    void destroy(TextureHandle) override {++calls;}
    void destroy(SamplerHandle) override {++calls;}
    void destroy(ShaderHandle) override {++calls;}
    void destroy(PipelineHandle) override {++calls;}
    const std::string& last_error() const noexcept override {static const std::string empty;return empty;}
    bool pass_active() const noexcept override {return false;}
    void record_marker(std::string_view) override {++calls;}
};
struct Scene {
    RecordingGpuDevice device;
    BufferHandle buffer, index;
    TextureHandle color,depth;
    SamplerHandle sampler;
    ShaderHandle vertex,fragment;
    PipelineHandle pipeline;
    Scene() {
        buffer=device.create_buffer({12,BufferUsage::vertex,true},"prior vertices");
        index=device.create_buffer({12,BufferUsage::index,true},"prior indices");
        color=device.create_texture({4,4,1,1,TextureDimension::texture_2d,TextureFormat::rgba8,true,true},"prior color");
        depth=device.create_texture({4,4,1,1,TextureDimension::texture_2d,TextureFormat::depth24_stencil8,true,false},"prior depth");
        sampler=device.create_sampler({},"prior sampler");
        vertex=device.create_shader({ShaderStage::vertex,"generated.vert",0,0},"prior vertex");
        fragment=device.create_shader({ShaderStage::fragment,"generated.frag",0,0},"prior fragment");
        PipelineDesc desc; desc.vertex_shader=vertex; desc.fragment_shader=fragment;
        pipeline=device.create_pipeline(PipelineKey(desc),"prior pipeline");
        check(buffer && index && color && depth && sampler && vertex && fragment && pipeline,"scene create");
    }
    RenderPassDesc pass() const {
        RenderPassDesc result;
        result.color_targets[0]=color; result.color_target_count=1; result.depth_target=depth;
        result.width=4; result.height=4; result.target_generation=7; return result;
    }
    DrawDesc draw() const {
        DrawDesc result; result.pipeline=pipeline; result.vertex_buffer=buffer;
        result.index_buffer=index; result.vertex_or_index_count=3; return result;
    }
    ViewportClearDesc clear() const {
        ViewportClearDesc result; result.color_target=color; result.depth_target=depth;
        result.target_generation=7; result.width=4; result.height=4; result.color=true; result.depth=true;
        return result;
    }
};
DeviceTransactionDesc desc(DeviceTransactionMode mode=DeviceTransactionMode::idle_preparation) {
    return {mode,7,128,512,1024*1024,mode==DeviceTransactionMode::frame_commands ? 16U : 0U};
}
struct Baseline {
    std::string commands;
    ResourceCounts resources;
    std::vector<UInt8> buffer,index,texture,last_draw;
    explicit Baseline(const Scene& s):commands(s.device.snapshot()),resources(s.device.resource_counts()),
        buffer(s.device.buffer_bytes(s.buffer)),index(s.device.buffer_bytes(s.index)),
        texture(s.device.texture_bytes(s.color)),last_draw(s.device.last_draw_index_bytes()) {}
    void assert_same(const Scene& s) const {
        check(commands==s.device.snapshot(),"rollback command baseline");
        check(resources==s.device.resource_counts(),"rollback resource baseline");
        check(buffer==s.device.buffer_bytes(s.buffer) && index==s.device.buffer_bytes(s.index),"rollback prior buffer bytes/identity");
        check(texture==s.device.texture_bytes(s.color),"rollback prior texture bytes/identity");
        check(last_draw==s.device.last_draw_index_bytes(),"rollback last indexed draw bytes");
        check(!s.device.pass_active() && s.device.active_pass_extent()==std::pair<UInt32,UInt32>{0,0},"rollback phase/extent");
        check(s.device.sampler_descriptor(s.sampler).maximum_anisotropy==1,"rollback sampler identity");
        check(s.device.pipeline_descriptor(s.pipeline).vertex_shader==s.vertex,"rollback pipeline/shader identity");
    }
};
void test_idle_and_handles() {
    Scene s; const Baseline baseline(s); DeviceTransactionToken token;
    check(s.device.begin_device_transaction(desc(),token),"idle admission");
    const auto candidate=s.device.create_buffer({8,BufferUsage::uniform,true},"candidate");
    const auto candidate_texture=s.device.create_texture({2,2},"candidate texture");
    const auto candidate_sampler=s.device.create_sampler({},"candidate sampler");
    const auto candidate_shader=s.device.create_shader({ShaderStage::vertex,"candidate.vert",0,0},"candidate shader");
    PipelineDesc candidate_desc;candidate_desc.vertex_shader=candidate_shader;candidate_desc.fragment_shader=s.fragment;
    const auto candidate_pipeline=s.device.create_pipeline(PipelineKey(candidate_desc),"candidate pipeline");
    check(candidate_texture && candidate_sampler && candidate_shader && candidate_pipeline,"candidate resource graph");
    const std::array<UInt8,12> bytes{{9,8,7}};
    check(candidate && s.device.upload({s.buffer,12,0,12},bytes.data()),"idle resource/bytes");
    s.device.destroy(s.sampler); s.device.destroy(s.pipeline); s.device.destroy(s.vertex);
    s.device.destroy(s.fragment);s.device.destroy(s.depth);s.device.destroy(s.color);s.device.destroy(s.index);
    s.device.record_marker("unpublished idle candidate");
    check(s.device.snapshot()==baseline.commands,"partial journal exposed");
    check(s.device.abort_device_transaction(token),"idle abort"); baseline.assert_same(s);
    check(s.device.buffer_bytes(candidate).empty(),"aborted candidate still live");
    check(!s.device.describe_texture_format(candidate_texture),"aborted texture still live");
    check(!s.device.abort_device_transaction(token) && !s.device.commit_device_transaction(token),"finish replay");
    const auto retry=s.device.create_buffer({8,BufferUsage::uniform,true},"ordinary retry");
    check(retry && retry!=candidate,"aborted handle aliased ordinary retry"); s.device.destroy(retry);
    check(s.device.begin_device_transaction(desc(),token),"idle retry admission");
    const auto accepted=s.device.create_buffer({8,BufferUsage::uniform,true},"accepted retry");
    check(accepted && accepted!=candidate,"aborted handle aliased transaction retry");
    s.device.record_marker("accepted idle candidate");
    check(s.device.commit_device_transaction(token),"idle commit");
    check(!s.device.commit_device_transaction(token) && !s.device.abort_device_transaction(token),"committed owner replay");
    check(s.device.snapshot().find("accepted idle candidate")!=std::string::npos &&
        s.device.snapshot().find("unpublished idle candidate")==std::string::npos,"idle publish once");
}
void test_modes_and_tokens() {
    Scene s; DeviceTransactionToken token;
    for (unsigned op=0;op<6;++op) {
        const Baseline baseline(s); check(s.device.begin_device_transaction(desc(),token),"mode admission");
        bool result=true;
        switch(op) {
        case 0:result=s.device.begin_pass(s.pass(),"forbidden");break;
        case 1:result=s.device.set_viewport({0,0,4,4,0,1});break;
        case 2:result=s.device.clear_viewport(s.clear());break;
        case 3:result=s.device.draw(s.draw());break;
        case 4:result=s.device.end_pass();break;
        case 5:result=s.device.present(s.color);break;
        }
        check(!result && !s.device.commit_device_transaction(token),"idle frame operation accepted");
        baseline.assert_same(s); check(s.device.abort_device_transaction(token),"mode abort");baseline.assert_same(s);
    }
    check(s.device.begin_device_transaction(desc(),token),"token admission");
    auto unchanged=token;
    check(!s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),unchanged),"overlapping mode accepted");
    check(unchanged.device==token.device && unchanged.sequence==token.sequence,"failed admission changed output token");
    for (unsigned field=0;field<4;++field) {
        auto wrong=token;
        if (field==0) ++wrong.device;
        if (field==1) ++wrong.sequence;
        if (field==2) ++wrong.generation;
        if (field==3) wrong.mode=DeviceTransactionMode::frame_commands;
        check(!s.device.commit_device_transaction(wrong) && !s.device.abort_device_transaction(wrong),"mismatched owner accepted");
    }
    check(s.device.commit_device_transaction(token),"wrong token mutated real owner");
    check(s.device.begin_pass(s.pass(),"ordinary active pass"),"ordinary pass");
    check(!s.device.begin_device_transaction(desc(),token),"active ordinary pass checkpoint accepted");
    check(s.device.end_pass(),"ordinary behavior changed");
}

// Each step independently faults before mutation, including post-pass release.
bool frame_step(Scene& s,unsigned step,BufferHandle& candidate) {
    static const std::array<UInt8,12> bytes{{1,2,3,4}};
    static const std::array<UInt8,64> pixels{{5,6,7,8}};
    switch(step) {
    case 0:candidate=s.device.create_buffer({16,BufferUsage::uniform,true},"frame candidate");return bool(candidate);
    case 1:return s.device.upload({s.buffer,12,0,12},bytes.data());
    case 2:return s.device.upload_texture({s.color,4,4,16,64,0},pixels.data());
    case 3:s.device.record_marker("ordered frame marker");return true;
    case 4:return s.device.begin_pass(s.pass(),"candidate frame");
    case 5:return s.device.set_viewport({0,0,4,4,0,1});
    case 6:return s.device.clear_viewport(s.clear());
    case 7:return s.device.draw(s.draw());
    case 8:return s.device.end_pass();
    case 9:return s.device.present(s.color);
    case 10:s.device.destroy(candidate);return s.device.buffer_bytes(candidate).empty();
    }
    return false;
}
void test_faulted_frames() {
    for (unsigned fault=0;fault<11;++fault) {
        Scene s; const Baseline baseline(s); DeviceTransactionToken token;
        s.device.fail_transaction_operation_after(fault);
        check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"frame admission");
        BufferHandle candidate;
        for (unsigned step=0;step<=fault;++step) {
            try { const bool ok=frame_step(s,step,candidate); if (step<fault) check(ok,"pre-fault operation failed"); }
            catch (const std::runtime_error&) { check(step==fault && step==3,"unexpected marker exception"); }
        }
        check(!s.device.commit_device_transaction(token),"injected failure committed");
        check(s.device.abort_device_transaction(token),"fault abort");baseline.assert_same(s);
        const auto aborted=candidate;
        check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"frame retry admission");
        for (unsigned step=0;step<11;++step) check(frame_step(s,step,candidate),"clean frame retry");
        check(!aborted || aborted!=candidate,"faulted frame handle alias");
        check(s.device.snapshot()==baseline.commands,"partial frame visible");
        check(s.device.commit_device_transaction(token),"frame retry commit");
        const auto counts=s.device.operation_counts();
        check(counts.passes==1 && counts.draws==1 && counts.presents==1 && counts.failures==0,"retry emitted twice/error as successful journal");
        auto loaded=s.pass(); loaded.color_load=loaded.depth_load=AttachmentLoad::load;
        check(s.device.begin_pass(loaded,"accepted initialization"),"commit target initialization");
        check(s.device.end_pass(),"accepted initialization end");
    }
}
void test_bounds_and_checkpoint() {
    Scene s; const Baseline baseline(s); DeviceTransactionToken token{99,88,77,DeviceTransactionMode::frame_commands};
    s.device.fail_next_transaction_checkpoint();
    check(!s.device.begin_device_transaction(desc(),token) && token.device==99,"checkpoint failure/output");baseline.assert_same(s);
    for (unsigned bad=0;bad<13;++bad) {
        auto d=desc();
        if (bad==0) d.generation=0;
        if (bad==1) d.commands=4097;
        if (bad==2) d.resources=4097;
        if (bad==3) d.bytes=RendererLimits::maximum_upload_bytes+1;
        if (bad==4) d.views=1;
        if (bad==5) d.mode=static_cast<DeviceTransactionMode>(2);
        if (bad==6) d.bytes=1;
        if (bad==7) d.resources=1;
        if (bad==8) d.commands=0;
        if (bad==9) d.resources=0;
        if (bad==10) d.bytes=0;
        if (bad==11) {d.mode=DeviceTransactionMode::frame_commands;d.views=0;}
        if (bad==12) {d.mode=DeviceTransactionMode::frame_commands;d.views=257;}
        check(!s.device.begin_device_transaction(d,token),"bad/baseline-over-budget admission");baseline.assert_same(s);
    }
    auto d=desc();d.commands=1;
    check(s.device.begin_device_transaction(d,token),"bounded command admission");
    s.device.record_marker("one");bool threw=false;
    try {s.device.record_marker("bound plus one");} catch(const std::runtime_error&) {threw=true;}
    check(threw && !s.device.commit_device_transaction(token),"command bound plus one");
    check(s.device.abort_device_transaction(token),"bounded abort");baseline.assert_same(s);
    d=desc(DeviceTransactionMode::frame_commands);d.views=1;
    check(s.device.begin_device_transaction(d,token) && s.device.begin_pass(s.pass(),"one view"),"view bound admission");
    check(!s.device.draw(s.draw()),"view bound plus one");
    check(s.device.abort_device_transaction(token),"view abort");baseline.assert_same(s);
    check(s.device.begin_device_transaction(desc(),token),"consumed checkpoint fault retry");
    s.device.fail_next_buffer_create();
    check(!s.device.create_buffer({8,BufferUsage::uniform,true},"fault"),"ordinary injected fault ignored");
    check(!s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),"ordinary fault rollback");baseline.assert_same(s);
    check(s.device.begin_device_transaction(desc(),token),"fault retry");
    check(bool(s.device.create_buffer({8,BufferUsage::uniform,true},"retry")),"fault restored by abort");
    check(s.device.abort_device_transaction(token),"fault retry abort");baseline.assert_same(s);
    { Scene fresh;const Baseline exact(fresh);
      d=desc();d.resources=8; // exact eight slots; prior aborts intentionally keep tombstones.
      check(fresh.device.begin_device_transaction(d,token),"exact resource capacity admission");
      PipelineDesc p;p.vertex_shader=fresh.vertex;p.fragment_shader=fresh.fragment;
      check(fresh.device.create_pipeline(PipelineKey(p),"bounded cache hit")==fresh.pipeline,"cache hit charged a new resource");
      check(!fresh.device.create_sampler({},"resource bound plus one"),"resource bound plus one");
      check(fresh.device.abort_device_transaction(token),"resource capacity abort");exact.assert_same(fresh); }
    check(s.device.begin_device_transaction(desc(),token),"byte capacity admission");
    const UInt8 one=0;
    check(!s.device.upload({s.buffer,12,0,RendererLimits::maximum_upload_bytes+1},&one),"byte capacity plus one");
    check(s.device.abort_device_transaction(token),"byte capacity abort");baseline.assert_same(s);
    check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"incomplete frame admission");
    check(s.device.begin_pass(s.pass(),"unfinished"),"unfinished frame begin");
    check(!s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),"active pass committed");baseline.assert_same(s);
    for (unsigned fault=0;fault<2;++fault) {
        check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token) &&
            s.device.begin_pass(s.pass(),"attached retirement rejection"),"attached rejection admission");
        if (fault) s.device.fail_next_transaction_diagnostic_allocation();
        bool threw=false;
        try {s.device.destroy(s.color);} catch(const std::bad_alloc&) {threw=true;}
        check(threw==bool(fault),"diagnostic allocation injection not reached");
        check(s.device.describe_texture_format(s.color)==TextureFormat::rgba8,"attached rejection destroyed target");
        check(!s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),"attached rejection did not poison/abort");
        baseline.assert_same(s);
    }
}
void test_resource_faults_and_absence() {
    OrdinaryOnlyDevice unsupported;DeviceTransactionToken absent{9,8,7,DeviceTransactionMode::frame_commands};
    for (auto mode:{DeviceTransactionMode::idle_preparation,DeviceTransactionMode::frame_commands}) {
        check(!unsupported.supports_device_transactions(mode) &&
            !unsupported.begin_device_transaction(desc(mode),absent) &&
            !unsupported.commit_device_transaction(absent) && !unsupported.abort_device_transaction(absent),"unsupported default opened");
    }
    check(unsupported.calls==0 && absent.device==9,"unsupported admission mutated consumer/token");
    for (unsigned fault=0;fault<6;++fault) {
        Scene s;const Baseline baseline(s);DeviceTransactionToken token;
        check(s.device.begin_device_transaction(desc(),token),"resource fault admission");
        s.device.record_marker("partial before resource fault");
        bool failed=false;
        if (fault==0) {s.device.fail_next_texture_create();failed=!s.device.create_texture({2,2},"faulted texture");}
        if (fault==1) {s.device.fail_next_sampler_create();failed=!s.device.create_sampler({},"faulted sampler");}
        if (fault==2) {s.device.fail_next_shader_create();failed=!s.device.create_shader({ShaderStage::vertex,"fault.vert",0,0},"faulted shader");}
        if (fault==3) {s.device.fail_next_pipeline_create();PipelineDesc p;p.vertex_shader=s.vertex;p.fragment_shader=s.fragment;
            failed=!s.device.create_pipeline(PipelineKey(p),"faulted pipeline");}
        if (fault==4) {s.device.fail_next_texture_upload();std::array<UInt8,64> bytes{};
            failed=!s.device.upload_texture({s.color,4,4,16,64,0},bytes.data());}
        if (fault==5) {s.device.fail_next_buffer_upload();std::array<UInt8,12> bytes{};
            failed=!s.device.upload({s.buffer,12,0,12},bytes.data());}
        check(failed && !s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),"resource fault committed");
        baseline.assert_same(s);
        check(s.device.begin_device_transaction(desc(),token),"resource fault clean retry");
        check(bool(s.device.create_texture({2,2},"retry texture")) && bool(s.device.create_sampler({},"retry sampler")) &&
            bool(s.device.create_shader({ShaderStage::vertex,"retry.vert",0,0},"retry shader")),"resource fault restored on retry");
        std::array<UInt8,64> bytes{};check(s.device.upload_texture({s.color,4,4,16,64,0},bytes.data()),"texture upload fault restored");
        check(s.device.upload({s.buffer,12,0,12},bytes.data()),"buffer upload fault restored");
        PipelineDesc p;p.vertex_shader=s.vertex;p.fragment_shader=s.fragment;
        check(bool(s.device.create_pipeline(PipelineKey(p),"retry pipeline")),"pipeline fault restored");
        check(s.device.abort_device_transaction(token),"resource retry abort");baseline.assert_same(s);
    }
}
void test_uninitialized_rollback_and_destruction() {
    Scene s;DeviceTransactionToken token;
    check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"initialization transaction");
    check(s.device.begin_pass(s.pass(),"partial initialized frame") && s.device.end_pass(),"partial initialization");
    check(s.device.abort_device_transaction(token),"initialization abort");
    auto loaded=s.pass(); loaded.color_load=loaded.depth_load=AttachmentLoad::load;
    check(!s.device.begin_pass(loaded,"must remain uninitialized"),"rollback left target initialized");
    { RecordingGpuDevice owner;check(owner.begin_device_transaction(desc(),token),"destruction admission");
      check(bool(owner.create_buffer({8,BufferUsage::uniform,true},"uncommitted destruction")),"destruction candidate"); }
}
void test_initialized_bytes_and_view_baseline() {
    Scene s;std::array<UInt8,64> pixels{};pixels.fill(42);
    check(s.device.upload_texture({s.color,4,4,16,64,0},pixels.data()),"initialized baseline texture");
    check(s.device.begin_pass(s.pass(),"prior draw") && s.device.draw(s.draw()) && s.device.end_pass(),"prior index/pass baseline");
    const Baseline baseline(s);DeviceTransactionToken token;
    check(s.device.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"initialized transaction");
    pixels.fill(17);check(s.device.upload_texture({s.color,4,4,16,64,0},pixels.data()),"initialized mutation");
    check(s.device.begin_pass(s.pass(),"candidate") && s.device.draw(s.draw()) && s.device.end_pass() && s.device.present(s.color),"candidate frame");
    check(s.device.abort_device_transaction(token),"initialized abort");baseline.assert_same(s);
    auto loaded=s.pass();loaded.color_load=loaded.depth_load=AttachmentLoad::load;
    check(s.device.begin_pass(loaded,"restored initialized target") && s.device.end_pass(),"prior initialization lost");
    RecordingGpuDevice bounded(256,3);
    const auto color=bounded.create_texture({2,2,1,1,TextureDimension::texture_2d,TextureFormat::rgba8,true,false},"view color");
    const auto depth=bounded.create_texture({2,2,1,1,TextureDimension::texture_2d,TextureFormat::depth16,true,false},"view depth");
    RenderPassDesc p;p.color_targets[0]=color;p.color_target_count=1;p.depth_target=depth;p.width=p.height=2;
    check(bounded.begin_pass(p,"prior view") && bounded.end_pass(),"prior view budget");
    check(bounded.begin_device_transaction(desc(DeviceTransactionMode::frame_commands),token),"view baseline admission");
    check(bounded.begin_pass(p,"candidate view") && bounded.end_pass() && bounded.present(color),"candidate view/present");
    check(bounded.abort_device_transaction(token),"view baseline abort");
    check(bounded.begin_pass(p,"second view") && bounded.end_pass() && bounded.begin_pass(p,"third view") && bounded.end_pass(),"view budget not restored");
    check(!bounded.begin_pass(p,"fourth view"),"abort/present incorrectly reset prior view baseline");
}
void test_camera_capacity() {
    Scene s;const Baseline baseline(s);DeviceTransactionToken token;
    auto camera=desc();camera.capacity=DeviceTransactionCapacity::camera_startup;
    camera.bytes=CameraStartupTransactionLimits::maximum_bytes;
    check(desc().capacity==DeviceTransactionCapacity::ordinary && camera.bytes==738197504,
        "camera formula/default capacity changed");
    for (unsigned bad=0;bad<12;++bad) {
        auto rejected=camera;
        if (bad==0) ++rejected.bytes;
        if (bad==1) rejected.bytes=0;
        if (bad==2) rejected.capacity=static_cast<DeviceTransactionCapacity>(255);
        if (bad==3) {rejected.mode=DeviceTransactionMode::frame_commands;rejected.views=1;}
        if (bad==4) rejected.generation=0;
        if (bad==5) rejected.views=1;
        if (bad==6) rejected.commands=0;
        if (bad==7) rejected.commands=4097;
        if (bad==8) rejected.resources=0;
        if (bad==9) rejected.resources=4097;
        if (bad==10) rejected.capacity=DeviceTransactionCapacity::ordinary;
        if (bad==11) rejected.mode=static_cast<DeviceTransactionMode>(255);
        DeviceTransactionToken output{99,98,97,DeviceTransactionMode::frame_commands};
        check(!s.device.begin_device_transaction(rejected,output) && output.device==99
            && output.sequence==98,"camera profile rejection published token");baseline.assert_same(s);
    }
    for (bool commit : {false,true}) {
        check(s.device.begin_device_transaction(camera,token),"camera exact ceiling admission");
        auto output=token;auto rejected=camera;rejected.capacity=static_cast<DeviceTransactionCapacity>(255);
        check(!s.device.begin_device_transaction(rejected,output) && output.sequence==token.sequence,
            "camera nested rejection changed owner");
        check(commit ? s.device.commit_device_transaction(token) : s.device.abort_device_transaction(token),
            "camera nested rejection poisoned real owner");baseline.assert_same(s);
    }
    check(s.device.begin_device_transaction(camera,token)
        && !s.device.create_buffer({RendererLimits::maximum_upload_bytes+1,BufferUsage::vertex,true},"oversized")
        && !s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),
        "camera profile widened individual resource limit");baseline.assert_same(s);
    check(s.device.begin_device_transaction(camera,token)
        && !s.device.upload({s.buffer,RendererLimits::maximum_upload_bytes+1,0,
            RendererLimits::maximum_upload_bytes+1},reinterpret_cast<const void*>(1))
        && !s.device.commit_device_transaction(token) && s.device.abort_device_transaction(token),
        "camera profile widened individual upload limit");baseline.assert_same(s);

    RecordingGpuDevice combined;
    std::array<UInt8,8> bytes{};std::array<UInt8,16> pixels{};
    const auto buffer=combined.create_buffer({bytes.size(),BufferUsage::vertex,true},"camera");
    const auto texture=combined.create_texture({2,2},"sibling");
    check(buffer && texture && combined.upload_texture({texture,2,2,8,pixels.size(),0},pixels.data()),
        "camera combined generated ownership");
    const auto commands=combined.snapshot();const auto resources=combined.resource_counts();
    const auto prior=combined.buffer_bytes(buffer);const auto prior_texture=combined.texture_bytes(texture);
    const UInt64 retained=bytes.size()+6+7+pixels.size(); // exact retained bytes and both labels
    camera.bytes=retained-1;
    check(!combined.begin_device_transaction(camera,token) && combined.snapshot()==commands,
        "camera retained baseline was not charged");
    camera.bytes=retained+bytes.size()+pixels.size();
    for (bool extra : {true,false}) {
        check(combined.begin_device_transaction(camera,token),"camera combined exact budget");
        bytes.fill(17);pixels.fill(42);
        check(combined.upload({buffer,bytes.size(),0,bytes.size()},bytes.data())
            && combined.upload_texture({texture,2,2,8,pixels.size(),0},pixels.data()),
            "camera combined exact upload charge");
        if (extra) check(!combined.upload({buffer,bytes.size(),0,1},bytes.data())
            && !combined.commit_device_transaction(token),"camera combined byte+1 escaped");
        check(combined.abort_device_transaction(token) && combined.snapshot()==commands
            && combined.resource_counts()==resources && combined.buffer_bytes(buffer)==prior
            && combined.texture_bytes(texture)==prior_texture,"camera combined rollback/retry identity");
    }
    combined.destroy(buffer);combined.destroy(texture);
    check(combined.resource_counts()==ResourceCounts{},"camera combined teardown residual");
}
}
int main() {
    try {
        for (unsigned generation=0;generation<2;++generation) {
            test_idle_and_handles();test_modes_and_tokens();test_faulted_frames();
            test_bounds_and_checkpoint();test_resource_faults_and_absence();test_uninitialized_rollback_and_destruction();
            test_initialized_bytes_and_view_baseline();
            test_camera_capacity();
        }
        std::cout<<"Recording transactions: two generations passed\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
