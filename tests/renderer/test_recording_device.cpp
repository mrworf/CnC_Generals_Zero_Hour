#include "zh/renderer/recording_device.h"

#include <array>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace zh::renderer;

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

struct Scene {
    RecordingGpuDevice device{2};
    ShaderHandle vertex;
    ShaderHandle fragment;
    PipelineHandle pipeline;
    BufferHandle vertices;
    TextureHandle color;
    TextureHandle depth;

    Scene()
    {
        vertex = device.create_shader({ShaderStage::vertex, "scene.vert", 1, 0}, "scene vertex");
        fragment = device.create_shader({ShaderStage::fragment, "scene.frag", 0, 0}, "scene fragment");
        PipelineDesc pipeline_desc;
        pipeline_desc.vertex_shader = vertex;
        pipeline_desc.fragment_shader = fragment;
        pipeline = device.create_pipeline(PipelineKey(pipeline_desc), "scene pipeline");
        vertices = device.create_buffer({12, BufferUsage::vertex, true}, "vertices");
        TextureDesc color_desc;
        color_desc.width = 16; color_desc.height = 16; color_desc.render_target = true;
        color = device.create_texture(color_desc, "color target");
        TextureDesc depth_desc = color_desc;
        depth_desc.format = TextureFormat::depth24_stencil8;
        depth = device.create_texture(depth_desc, "depth target");
    }

    RenderPassDesc pass() const
    {
        RenderPassDesc pass;
        pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
        pass.width = 16; pass.height = 16;
        return pass;
    }
};

void test_valid_stream_and_snapshot()
{
    Scene scene;
    const std::array<unsigned char, 4> bytes{1, 2, 3, 4};
    check(scene.device.upload({scene.vertices, 12, 4, bytes.size()}, bytes.data()), "valid upload failed");
    check(scene.device.buffer_bytes(scene.vertices)[6] == 3, "upload did not update CPU copy");
    const std::array<unsigned char, 16 * 16 * 4> pixels{};
    check(scene.device.upload_texture({scene.color, 16, 16, 64, pixels.size()}, pixels.data()),
        "valid RGBA texture upload failed");
    check(scene.device.begin_pass(scene.pass(), "main pass"), "valid pass failed");
    DrawDesc draw;
    draw.pipeline = scene.pipeline; draw.vertex_buffer = scene.vertices; draw.vertex_or_index_count = 3;
    auto uniforms = scene.device.create_buffer({64, BufferUsage::uniform, true}, "frame constants");
    draw.vertex_bindings.uniforms[0] = {uniforms, 0, 64}; draw.vertex_bindings.uniform_count = 1;
    check(scene.device.draw(draw), "valid draw failed");
    check(scene.device.end_pass(), "valid pass end failed");
    check(scene.device.present(scene.color), "valid presentation failed");
    const auto snapshot = scene.device.snapshot();
    check(snapshot.find("create_buffer B1 label=\"vertices\"") != std::string::npos, "buffer id was not normalized");
    check(snapshot.find("begin_pass") < snapshot.find("draw pipeline="), "draw order not retained");
    check(snapshot.find("draw pipeline=") < snapshot.find("end_pass"), "pass order not retained");
    check(snapshot.find("upload_texture") != std::string::npos && snapshot.find("present T") != std::string::npos,
        "texture upload or presentation was not recorded");
}

void test_lifetimes_bindings_and_ranges()
{
    Scene scene;
    const std::array<unsigned char, 4> bytes{};
    check(!scene.device.upload({scene.vertices, 11, 0, 4}, bytes.data()), "false destination size accepted");
    check(scene.device.last_error().find("vertices") != std::string::npos, "upload error lost label provenance");
    check(!scene.device.upload({scene.vertices, 12, 0, 4}, nullptr), "null upload accepted");
    const std::array<unsigned char, 16 * 16 * 4> pixels{};
    check(!scene.device.upload_texture({scene.color, 16, 16, 63, pixels.size()}, pixels.data()),
        "undersized texture row pitch accepted");
    scene.device.destroy(scene.vertices);
    check(!scene.device.upload({scene.vertices, 12, 0, 4}, bytes.data()), "destroyed buffer upload accepted");
    scene.device.destroy(scene.vertices);
    auto replacement = scene.device.create_buffer({12, BufferUsage::vertex, true}, "replacement");
    check(replacement && replacement != scene.vertices, "slot reuse did not change generation");

    check(scene.device.begin_pass(scene.pass(), "binding pass"), "pass failed");
    DrawDesc draw; draw.pipeline = scene.pipeline; draw.vertex_buffer = replacement; draw.vertex_or_index_count = 3;
    check(!scene.device.draw(draw), "missing required uniform accepted");
    check(scene.device.last_error().find("binding pass") != std::string::npos, "draw error lost pass provenance");
    auto uniform = scene.device.create_buffer({32, BufferUsage::uniform, true}, "uniform");
    draw.vertex_bindings.uniforms[0] = {uniform, 0, 32}; draw.vertex_bindings.uniform_count = 1;
    scene.device.destroy(uniform);
    check(!scene.device.draw(draw), "destroyed binding accepted");
    check(scene.device.end_pass(), "recovery end pass failed");
}

void test_pass_rules_and_unsupported_descriptors()
{
    Scene scene;
    check(!scene.device.create_buffer({}, "bad buffer"), "invalid descriptor accepted");
    check(scene.device.last_error().find("bad buffer") != std::string::npos, "create error lost label");
    check(!scene.device.end_pass(), "end without pass accepted");
    check(scene.device.begin_pass(scene.pass(), "outer"), "outer pass failed");
    check(!scene.device.begin_pass(scene.pass(), "nested"), "nested pass accepted");
    scene.device.destroy(scene.color);
    check(scene.device.last_error().find("attached to the active") != std::string::npos,
        "active render target destruction accepted");
    check(scene.device.end_pass(), "outer end failed");
    auto wrong = scene.pass(); wrong.width = 8;
    check(!scene.device.begin_pass(wrong, "wrong extent"), "attachment extent mismatch accepted");
    scene.device.destroy(scene.color);
    check(!scene.device.begin_pass(scene.pass(), "dead target"), "destroyed target accepted");
}

void test_original_camera_viewport_contract()
{
    Scene scene;
    ViewportDesc viewport{4,2,8,10,0.1F,0.9F};
    check(scene.device.active_pass_extent()==std::pair<UInt32,UInt32>{0,0},"inactive extent leaked");
    check(!scene.device.set_viewport(viewport),"viewport without pass accepted");
    check(scene.device.begin_pass(scene.pass(),"original camera viewport"),"camera pass rejected");
    check(scene.device.active_pass_extent()==std::pair<UInt32,UInt32>{16,16},"active extent incorrect");
    auto bad=viewport; bad.x=9;
    check(!scene.device.set_viewport(bad),"out-of-target viewport accepted");
    bad=viewport; bad.min_depth=0.95F;
    check(!scene.device.set_viewport(bad),"inverted depth range accepted");
    bad=viewport; bad.width=std::numeric_limits<float>::quiet_NaN();
    check(!scene.device.set_viewport(bad),"nonfinite viewport accepted");
    check(scene.device.snapshot().find("viewport=")==std::string::npos,"invalid viewport mutated state");
    check(scene.device.set_viewport(viewport),"bounded camera viewport rejected");
    check(scene.device.snapshot().find("viewport=4.000000,2.000000,8.000000,10.000000 depth=0.100000:0.900000")!=
        std::string::npos,"viewport/depth recording differs from source");
    check(scene.device.end_pass(),"camera pass end rejected");
    check(scene.device.active_pass_extent()==std::pair<UInt32,UInt32>{0,0},"ended extent leaked");
    check(!scene.device.set_viewport(viewport),"viewport after pass accepted");
}

void test_pipeline_cache_is_immutable_and_bounded()
{
    Scene scene;
    PipelineDesc first;
    first.vertex_shader = scene.vertex; first.fragment_shader = scene.fragment;
    auto reused = scene.device.create_pipeline(PipelineKey(first), "same state");
    check(reused == scene.pipeline && scene.device.pipeline_count() == 1, "equal pipeline was not reused");
    auto second = first; second.fog_enabled = true;
    check(static_cast<bool>(scene.device.create_pipeline(PipelineKey(second), "fog state")), "second unique pipeline rejected");
    auto third = first; third.blend.enabled = true;
    check(!scene.device.create_pipeline(PipelineKey(third), "overflow state"), "pipeline cache explosion accepted");
    check(scene.device.pipeline_count() == 2, "failed cache insert mutated cache");
    scene.device.destroy(scene.vertex);
    auto fourth = first; fourth.depth_stencil.depth_write = false;
    check(!scene.device.create_pipeline(PipelineKey(fourth), "stale shader"), "stale shader accepted");
}

void test_original_16_bit_index_range_and_base_vertex()
{
    Scene scene;
    auto index = scene.device.create_buffer({12, BufferUsage::index, true}, "original 16-bit indices");
    auto uniform = scene.device.create_buffer({64, BufferUsage::uniform, true}, "world constants");
    check(scene.device.begin_pass(scene.pass(), "source mesh"), "source pass failed");
    DrawDesc draw;
    draw.pipeline = scene.pipeline;
    draw.vertex_buffer = scene.vertices;
    draw.index_buffer = index;
    draw.index_element_size = IndexElementSize::uint16;
    draw.first_index = 2;
    draw.base_vertex = 4;
    draw.vertex_or_index_count = 3;
    draw.vertex_bindings.uniforms[0] = {uniform, 0, 64};
    draw.vertex_bindings.uniform_count = 1;
    check(scene.device.draw(draw), "valid original indexed range rejected");
    check(scene.device.snapshot().find("index_bits=16 first_index=2 base_vertex=4") != std::string::npos,
        "original index format or offsets lost from recording");
    draw.first_index = 4;
    check(!scene.device.draw(draw), "16-bit range beyond index buffer accepted");
    draw.first_index = 0;
    draw.index_element_size = static_cast<IndexElementSize>(3);
    check(!scene.device.draw(draw), "unsupported index element width accepted");
    draw.index_buffer = {};
    draw.index_element_size = IndexElementSize::uint32;
    draw.base_vertex = 1;
    check(!scene.device.draw(draw), "non-indexed base vertex accepted");
    check(scene.device.end_pass(), "source pass end failed");
}

void test_source_texture_format_capabilities()
{
    RecordingGpuDevice device;
    check(device.supports_texture_format(TextureFormat::bc1,TextureDimension::texture_2d,true,false),
        "recording device must expose BC1 source profile");
    check(!device.supports_texture_format(TextureFormat::bc1,TextureDimension::texture_2d,true,true),
        "compressed render target was reported supported");
    device.set_texture_format_supported(TextureFormat::bc1,false);
    check(!device.supports_texture_format(TextureFormat::bc1,TextureDimension::texture_2d,true,false),
        "unsupported BC1 profile was reported supported");
    TextureDesc desc;
    desc.width=4; desc.height=4; desc.format=TextureFormat::bc1;
    check(!device.create_texture(desc,"source BC1"), "unsupported texture profile was created");
    device.set_texture_format_supported(TextureFormat::bc1,true);
    auto texture=device.create_texture(desc,"source BC1 retry");
    check(static_cast<bool>(texture), "supported texture profile was rejected");
    device.destroy(texture);
    check(device.resource_counts().total()==0, "texture profile retry leaked resources");
    TextureDesc oversized=desc;
    oversized.width=0xFFFFFFFFu;
    check(!device.create_texture(oversized,"oversized source"),
        "oversized texture create was accepted despite the physical boundary");
    check(!device.supports_texture_format(static_cast<TextureFormat>(255),TextureDimension::texture_2d,true,false),
        "invalid format was reported supported");
}

void test_original_mip_upload_contract()
{
    RecordingGpuDevice device;
    TextureDesc desc;
    desc.width=8; desc.height=8; desc.mip_levels=2; desc.format=TextureFormat::bc1;
    const auto texture=device.create_texture(desc,"owned BC1 mips");
    check(static_cast<bool>(texture),"compressed texture creation failed");
    std::array<UInt8,32> top{};
    std::array<UInt8,8> lower{};
    for (unsigned i=0;i<top.size();++i) top[i]=static_cast<UInt8>(i+1);
    for (unsigned i=0;i<lower.size();++i) lower[i]=static_cast<UInt8>(80+i);
    TextureUploadDesc upload;
    upload.destination=texture; upload.width=8; upload.height=8; upload.row_pitch=16; upload.size=32;
    check(device.upload_texture(upload,top.data()),"BC1 first mip rejected");
    upload.mip_level=1; upload.width=4; upload.height=4; upload.row_pitch=8; upload.size=8;
    check(device.upload_texture(upload,lower.data()),"BC1 second mip rejected");
    check(device.texture_bytes(texture,0)==std::vector<UInt8>(top.begin(),top.end()),"BC1 source mip bytes changed");
    check(device.texture_bytes(texture,1)==std::vector<UInt8>(lower.begin(),lower.end()),"BC1 reduced mip bytes changed");
    upload.mip_level=2;
    check(!device.upload_texture(upload,lower.data()),"out-of-range mip accepted");
    upload.mip_level=1; upload.row_pitch=4;
    check(!device.upload_texture(upload,lower.data()),"short block row accepted");
    device.destroy(texture);
    check(device.resource_counts().total()==0,"mip texture leaked");
}

void test_original_sampler_lod_contract()
{
    RecordingGpuDevice device;
    SamplerDesc source;
    source.maximum_lod=0.0F;
    auto sampler=device.create_sampler(source,"original no-mip sampler");
    check(static_cast<bool>(sampler) && device.sampler_descriptor(sampler).maximum_lod==0.0F,
        "source no-mip clamp was not preserved");
    device.destroy(sampler);
    source.maximum_lod=-1.0F;
    check(!device.create_sampler(source,"invalid original LOD"),"negative sampler LOD was accepted");
    source.maximum_lod=1000.0F;
    device.fail_next_sampler_create();
    check(!device.create_sampler(source,"injected sampler failure"),"injected sampler failure was ignored");
    sampler=device.create_sampler(source,"retry sampler");
    check(static_cast<bool>(sampler),"sampler retry failed");
    device.destroy(sampler);
    check(device.resource_counts().total()==0,"sampler contract leaked resources");
}

void test_original_frame_attachment_semantics()
{
    Scene scene;
    auto pass=scene.pass();
    pass.color_load=AttachmentLoad::load;
    check(!scene.device.begin_pass(pass,"uninitialized color"),"uninitialized color LOAD accepted");
    check(!scene.device.present(scene.color),"uninitialized presentation accepted");
    pass.color_load=AttachmentLoad::clear;
    pass.depth_load=AttachmentLoad::load;
    check(!scene.device.begin_pass(pass,"uninitialized depth"),"uninitialized depth LOAD accepted");
    pass.depth_load=AttachmentLoad::clear;
    pass.clear_color={0.125F,0.25F,0.375F,0.5F};
    pass.clear_depth=0.75F;
    check(scene.device.begin_pass(pass,"authored full target clear"),"source color/alpha/depth clear rejected");
    check(!scene.device.begin_pass(pass,"midpass clear"),"midpass clear accepted");
    check(!scene.device.present(scene.color),"midpass presentation accepted");
    check(scene.device.end_pass(),"source clear pass end failed");
    auto loaded=scene.pass();
    loaded.color_load=AttachmentLoad::load;
    loaded.depth_load=AttachmentLoad::load;
    check(scene.device.begin_pass(loaded,"preserve frame"),"initialized attachment LOAD rejected");
    check(scene.device.end_pass(),"preserve frame pass end failed");
    check(scene.device.present(scene.color),"completed source color presentation rejected");
    const auto snapshot=scene.device.snapshot();
    check(snapshot.find("color_load=0 depth_load=0 clear=0.125000,0.250000,0.375000,0.500000,0.750000")!=std::string::npos,
        "source full target color/alpha/depth values lost");
    check(snapshot.find("color_load=1 depth_load=1")!=std::string::npos,"source LOAD decisions lost");
    const auto before=scene.device.active_pass_extent();
    check(before.first==0 && before.second==0,"pass owner leaked after end");
    loaded.clear_color[3]=std::numeric_limits<float>::quiet_NaN();
    check(!scene.device.begin_pass(loaded,"nonfinite alpha"),"nonfinite alpha accepted");
    loaded.clear_color[3]=1.01F;
    check(!scene.device.begin_pass(loaded,"alpha out of range"),"out of range alpha accepted");
    loaded.clear_color[3]=1.0F;
    loaded.clear_depth=-0.1F;
    check(!scene.device.begin_pass(loaded,"negative depth"),"out of range depth accepted");
    loaded.clear_depth=1.0F;
    loaded.depth_load=static_cast<AttachmentLoad>(99);
    check(!scene.device.begin_pass(loaded,"bad load op"),"unsupported load op accepted");
    scene.device.destroy(scene.color);
    auto replacement=scene.device.create_texture({16,16,1,1,TextureDimension::texture_2d,TextureFormat::rgba8,true,false},"new generation");
    loaded.color_targets[0]=replacement; loaded.depth_load=AttachmentLoad::load;
    check(!scene.device.begin_pass(loaded,"stale generation"),"new target loaded old generation's contents");
}
} // namespace

int main()
{
    try {
        test_valid_stream_and_snapshot();
        test_lifetimes_bindings_and_ranges();
        test_pass_rules_and_unsupported_descriptors();
        test_original_camera_viewport_contract();
        test_pipeline_cache_is_immutable_and_bounded();
        test_original_16_bit_index_range_and_base_vertex();
        test_source_texture_format_capabilities();
        test_original_mip_upload_contract();
        test_original_sampler_lod_contract();
        test_original_frame_attachment_semantics();
        std::cout << "recording device tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
