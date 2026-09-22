#include "zh/platform/bgfx_device.h"

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

template <class F> std::string failure(F action)
{
    try { action(); } catch (const std::exception& error) { return error.what(); }
    throw std::runtime_error("expected construction failure");
}

void test_options()
{
    zh::renderer::BgfxOptions options;
    options.shader_root = ZH_BGFX_SHADER_DIR;
    options.driver = "direct3d12";
    check(failure([&] { zh::renderer::BgfxGpuDevice device(options); }).find("must be 'vulkan'") != std::string::npos,
        "non-Vulkan backend did not fail closed");
    options.driver = "vulkan";
    options.shader_root.clear();
    check(failure([&] { zh::renderer::BgfxGpuDevice device(options); }).find("shader root") != std::string::npos,
        "empty shader root did not fail closed");
}

void test_resources()
{
    using namespace zh::renderer;
    struct Vertex { float x, y; std::uint32_t color; float u, v; };
    BgfxOptions options;
    options.shader_root = ZH_BGFX_SHADER_DIR;
    for (int generation = 0; generation != 2; ++generation) {
        BgfxGpuDevice device(options);
        check(bgfx::getRendererType() == bgfx::RendererType::Vulkan, "bgfx did not select Vulkan");
        check(!device.create_texture({}, "invalid texture"), "zero-extent texture accepted");
        check(device.last_error().find("nonzero") != std::string::npos, "texture error unavailable");

        TextureDesc desc;
        desc.width = 8; desc.height = 8;
        check(device.supports_texture_format(desc.format, desc.dimension, true, false), "RGBA8 unsupported");
        auto texture = device.create_texture(desc, "owned test texture");
        check(bool(texture), "physical texture allocation failed");
        std::array<unsigned char, 8 * 8 * 4> pixels{};
        pixels.fill(17);
        check(device.upload_texture({texture, 8, 8, 8 * 4, pixels.size(), 0}, pixels.data()),
            "texture upload failed");
        check(!device.upload_texture({texture, 8, 8, 7, pixels.size(), 0}, pixels.data()),
            "short row pitch accepted");
        device.destroy(texture);
        check(!device.upload_texture({texture, 8, 8, 8 * 4, pixels.size(), 0}, pixels.data()),
            "stale texture accepted");
        auto replacement = device.create_texture(desc, "replacement");
        check(bool(replacement) && replacement != texture, "texture generation did not advance");
        device.destroy(replacement);

        auto buffer = device.create_buffer({32, BufferUsage::vertex, true}, "owned test buffer");
        check(bool(buffer), "buffer creation failed");
        check(device.upload({buffer, 32, 0, 4}, pixels.data()), "buffer upload failed");
        check(!device.upload({buffer, 32, 31, 4}, pixels.data()), "overflow buffer upload accepted");
        device.destroy(buffer);
        check(!device.upload({buffer, 32, 0, 4}, pixels.data()), "stale buffer accepted");
        check(!device.clear_viewport({}), "unimplemented target clear silently accepted");
        check(!device.pass_active(), "pass unexpectedly active");
        check(device.wait_idle(), "wait_idle failed");

        check(!device.create_shader({ShaderStage::vertex, "../outside.vert", 0, 0}, "escape"),
            "shader path traversal accepted");
        check(!device.create_shader({ShaderStage::fragment, "renderer/acceptance.vert", 0, 0}, "wrong stage"),
            "wrong shader stage accepted");
        auto vertex_shader = device.create_shader({ShaderStage::vertex, "renderer/acceptance.vert", 0, 0}, "acceptance vertex");
        auto fragment_shader = device.create_shader({ShaderStage::fragment, "renderer/acceptance.frag", 0, 0}, "acceptance fragment");
        check(bool(vertex_shader) && bool(fragment_shader), "physical shader creation failed");
        auto ui_uniform_shader = device.create_shader({ShaderStage::vertex, "renderer/ui.vert", 1, 0},
            "nested UI uniform reflection");
        check(bool(ui_uniform_shader), device.last_error().c_str());
        device.destroy(ui_uniform_shader);
        std::size_t shader_families = 0;
        for (const auto& file : std::filesystem::recursive_directory_iterator(ZH_BGFX_SHADER_DIR)) {
            if (file.path().extension() != ".bin") continue;
            auto name = std::filesystem::relative(file.path(), ZH_BGFX_SHADER_DIR).string();
            name.resize(name.size() - 4);
            const auto stage = name.size() >= 5 && name.compare(name.size() - 5, 5, ".vert") == 0
                ? ShaderStage::vertex : ShaderStage::fragment;
            auto family = device.create_shader({stage, name, 4, 16}, "complete physical family closure");
            check(bool(family), device.last_error().c_str());
            device.destroy(family);
            ++shader_families;
        }
        check(shader_families == 39, "pinned shader family count changed");
        PipelineDesc pipeline_desc;
        pipeline_desc.vertex_shader = vertex_shader; pipeline_desc.fragment_shader = fragment_shader;
        pipeline_desc.vertex_layout = VertexLayout::position_color_uv;
        pipeline_desc.color_format = TextureFormat::bgra8;
        pipeline_desc.raster.cull = CullMode::none;
        auto pipeline = device.create_pipeline(PipelineKey(pipeline_desc), "acceptance pipeline");
        check(bool(pipeline), "physical program creation failed");
        auto vertex_buffer = device.create_buffer({3 * sizeof(Vertex), BufferUsage::vertex, true}, "ordered trace vertices");
        const std::array<Vertex,3> triangle{{
            {-0.9f, 0.9f, 0xff00ff00U, 0, 0},
            {-0.6f, 0.9f, 0xff00ff00U, 1, 0},
            {-0.75f, 0.6f, 0xff00ff00U, 0.5f, 1},
        }};
        check(device.upload({vertex_buffer, sizeof(triangle), 0, sizeof(triangle)}, triangle.data()),
            "vertex upload failed");
        DrawDesc draw;
        draw.pipeline = pipeline; draw.vertex_buffer = vertex_buffer; draw.vertex_or_index_count = 3;
        check(!device.draw(draw), "inactive draw accepted");

        TextureDesc color_desc;
        color_desc.width = 160; color_desc.height = 120;
        color_desc.format = TextureFormat::bgra8; color_desc.render_target = true;
        auto color = device.create_texture(color_desc, "clear test color");
        auto depth_desc = color_desc;
        depth_desc.format = TextureFormat::depth24_stencil8;
        auto depth = device.create_texture(depth_desc, "clear test depth-stencil");
        check(bool(color) && bool(depth), "physical clear targets unavailable");
        RenderPassDesc pass;
        pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
        pass.width = 160; pass.height = 120; pass.target_generation = 5;
        pass.clear_color = {1, 0, 0, 1}; pass.clear_depth = 1.0f;
        pass.color_load = AttachmentLoad::load;
        check(!device.begin_pass(pass, "uninitialized color load"), "uninitialized color LOAD accepted");
        pass.color_load = AttachmentLoad::clear;
        pass.depth_load = AttachmentLoad::load;
        check(!device.begin_pass(pass, "uninitialized depth load"), "uninitialized depth LOAD accepted");
        pass.depth_load = AttachmentLoad::clear;
        check(device.begin_pass(pass, "outer clear"), "outer pass failed");
        check(device.pass_active() && device.active_pass_extent() == std::pair<UInt32,UInt32>{160,120},
            "active pass state/extent wrong");
        check(!device.begin_pass(pass, "illegal nested pass"), "nested pass accepted");
        check(device.draw(draw), "pre-camera source-shaped draw failed");
        check(!device.set_viewport({40.5f,30,80,60,0,1}), "fractional viewport silently rounded");
        check(!device.set_viewport({40,30,80,60,0.25f,1}), "unsupported depth range silently ignored");
        check(device.set_viewport({40,30,80,60,0,1}), "camera viewport failed");
        ViewportClearDesc inset;
        inset.color_target = color; inset.depth_target = depth;
        inset.target_generation = 5; inset.x = 40; inset.y = 30;
        inset.width = 80; inset.height = 60;
        inset.color = inset.depth = inset.stencil = true;
        inset.color_value = {0, 0, 1, 1}; inset.depth_value = 1; inset.stencil_value = 0;
        auto stale = inset; stale.target_generation = 4;
        check(!device.clear_viewport(stale), "stale target generation accepted");
        check(device.clear_viewport(inset), "ordered inset clear failed");
        check(device.draw(draw), "post-camera source-shaped draw failed");
        check(device.end_pass(), "end pass failed");
        check(!device.end_pass(), "inactive end pass accepted");
        auto result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U, "clear readback failed");
        const auto pixel = [&](int x, int y) { return &result[(y * 160 + x) * 4]; };
        const auto red = [&](int x, int y) {
            const auto* p = pixel(x,y); return p[0] == 255 && p[1] == 0 && p[2] == 0 && p[3] == 255;
        };
        const auto blue = [&](int x, int y) {
            const auto* p = pixel(x,y); return p[0] == 0 && p[1] == 0 && p[2] == 255 && p[3] == 255;
        };
        const auto green = [&](int x, int y) {
            const auto* p = pixel(x,y); return p[0] == 0 && p[1] == 255 && p[2] == 0 && p[3] == 255;
        };
        check(red(0,0) && red(39,30) && blue(40,30) && blue(119,89)
            && red(120,89) && red(159,119), "outer/inset ordered color pixels differ");
        check(green(20,15) && green(50,37), "source-shaped draw/clear/draw pixels differ");

        pass.color_load = pass.depth_load = AttachmentLoad::load;
        check(device.begin_pass(pass, "preserving load"), "initialized LOAD rejected");
        auto stencil_only = inset;
        stencil_only.color = false; stencil_only.depth = false; stencil_only.stencil = true;
        stencil_only.stencil_value = 3;
        check(device.clear_viewport(stencil_only), "stencil-only clear failed");
        auto depth_only = inset;
        depth_only.color = false; depth_only.depth = true; depth_only.stencil = false;
        depth_only.depth_value = 0.5f;
        check(device.clear_viewport(depth_only), "depth-only clear failed");
        check(device.end_pass(), "preservation pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && red(0,0) && blue(40,30)
            && green(20,15) && green(50,37) && red(159,119),
            "independent depth/stencil clear changed color attachment");

        check(device.begin_pass(pass, "view budget"), "post-readback pass failed");
        for (UInt32 view = 1; view < RendererLimits::ordered_views; ++view)
            check(device.clear_viewport(stencil_only), "view budget rejected a legal view");
        check(!device.clear_viewport(stencil_only), "view budget exhaustion accepted");
        check(device.end_pass() && device.wait_idle(), "view-budget frame did not end");
        check(device.begin_pass(pass, "reclaimed view budget"), "view budget did not reset next frame");
        check(device.end_pass() && device.wait_idle(), "reclaimed frame did not end");

        auto stencil_writer_desc = pipeline_desc;
        stencil_writer_desc.blend.color_write_mask = 0;
        stencil_writer_desc.depth_stencil.depth_test = false;
        stencil_writer_desc.depth_stencil.depth_write = false;
        stencil_writer_desc.depth_stencil.stencil_test = true;
        stencil_writer_desc.depth_stencil.stencil_compare = CompareOp::always;
        stencil_writer_desc.depth_stencil.stencil_reference = 0x80;
        stencil_writer_desc.depth_stencil.stencil_write_mask = 0x80;
        stencil_writer_desc.depth_stencil.depth_pass = StencilOp::replace;
        auto stencil_writer = device.create_pipeline(PipelineKey(stencil_writer_desc), "MSB-only stencil writer");
        auto stencil_verifier_desc = stencil_writer_desc;
        stencil_verifier_desc.blend.color_write_mask = 0x0f;
        stencil_verifier_desc.depth_stencil.stencil_compare = CompareOp::equal;
        stencil_verifier_desc.depth_stencil.stencil_reference = 0x87;
        stencil_verifier_desc.depth_stencil.stencil_write_mask = 0;
        stencil_verifier_desc.depth_stencil.depth_pass = StencilOp::keep;
        auto stencil_verifier = device.create_pipeline(PipelineKey(stencil_verifier_desc), "preserved low stencil bits verifier");
        check(bool(stencil_writer) && bool(stencil_verifier), "stencil programs unavailable");
        pass.color_load = pass.depth_load = AttachmentLoad::clear;
        check(device.begin_pass(pass, "stencil write-mask proof"), "stencil proof pass failed");
        auto full_stencil = inset;
        full_stencil.x = full_stencil.y = 0;
        full_stencil.width = 160; full_stencil.height = 120;
        full_stencil.color = full_stencil.depth = false; full_stencil.stencil = true;
        full_stencil.stencil_value = 7;
        check(device.clear_viewport(full_stencil), "outer stencil 7 clear failed");
        check(device.clear_viewport(inset), "inset stencil 0 clear failed");
        check(device.set_viewport({0,0,160,120,0,1}), "full target stencil draw viewport failed");
        draw.pipeline = stencil_writer;
        check(device.draw(draw), "MSB-only stencil writer failed");
        draw.pipeline = stencil_verifier;
        check(device.draw(draw), "stencil low-bits verifier failed");
        check(device.end_pass(), "stencil proof pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,15) && blue(50,37) && red(0,0),
            "public stencil write mask did not preserve low bits");
        device.destroy(stencil_writer); device.destroy(stencil_verifier);

        struct OriginalVertex { float xyz[3]; std::uint32_t diffuse; float uv[2]; };
        static_assert(sizeof(OriginalVertex) == 24);
        auto original_vertex = device.create_shader({ShaderStage::vertex, "renderer/original_fvf_probe.vert", 0, 0},
            "original indexed FVF vertex");
        check(bool(original_vertex), device.last_error().c_str());
        auto original_pipeline_desc = pipeline_desc;
        original_pipeline_desc.vertex_shader = original_vertex;
        original_pipeline_desc.vertex_layout = VertexLayout::original_fvf;
        original_pipeline_desc.original_fvf.stride = sizeof(OriginalVertex);
        original_pipeline_desc.original_fvf.attribute_count = 3;
        original_pipeline_desc.original_fvf.attributes[0] = {0,VertexElementFormat::float3,0};
        original_pipeline_desc.original_fvf.attributes[1] = {2,VertexElementFormat::ubyte4_norm,12};
        original_pipeline_desc.original_fvf.attributes[2] = {4,VertexElementFormat::float2,16};
        auto original_pipeline = device.create_pipeline(PipelineKey(original_pipeline_desc), "original indexed FVF program");
        check(bool(original_pipeline), device.last_error().c_str());
        const std::array<OriginalVertex,4> original_vertices{{
            {{0,0,0.5f},0, {0,0}},
            {{-0.9f,0.9f,0.5f},0xff00ff00U,{0,0}},
            {{-0.6f,0.9f,0.5f},0xff00ff00U,{1,0}},
            {{-0.75f,0.6f,0.5f},0xff00ff00U,{0.5f,1}},
        }};
        const std::array<std::uint16_t,4> original_indices{{99,0,1,2}};
        auto original_vb = device.create_buffer({sizeof(original_vertices),BufferUsage::vertex,true}, "original vertices");
        auto original_ib = device.create_buffer({sizeof(original_indices),BufferUsage::index,true}, "original indices");
        check(device.upload({original_vb,sizeof(original_vertices),0,sizeof(original_vertices)},original_vertices.data())
            && device.upload({original_ib,sizeof(original_indices),0,sizeof(original_indices)},original_indices.data()),
            "original indexed upload failed");
        DrawDesc original_draw;
        original_draw.pipeline = original_pipeline;
        original_draw.vertex_buffer = original_vb;
        original_draw.index_buffer = original_ib;
        original_draw.index_element_size = IndexElementSize::uint16;
        original_draw.first_index = 1;
        original_draw.base_vertex = 1;
        original_draw.vertex_or_index_count = 3;
        pass.color_load = pass.depth_load = AttachmentLoad::clear;
        check(device.begin_pass(pass, "original FVF indexed base vertex"), "original FVF pass failed");
        check(device.draw(original_draw), device.last_error().c_str());
        auto out_of_range = original_draw;
        out_of_range.first_index = 2;
        check(!device.draw(out_of_range), "out-of-range original index accepted");
        check(device.end_pass(), "original FVF pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,15) && red(150,110),
            "original FVF indexed/base-vertex pixels differ");
        device.destroy(original_vb); device.destroy(original_ib);
        device.destroy(original_pipeline); device.destroy(original_vertex);

        auto video_vertex = device.create_shader({ShaderStage::vertex, "renderer/video.vert", 1, 0}, "video vertex");
        auto video_fragment = device.create_shader({ShaderStage::fragment, "renderer/video.frag", 0, 1}, "video fragment");
        check(bool(video_vertex) && bool(video_fragment), device.last_error().c_str());
        auto video_pipeline_desc = pipeline_desc;
        video_pipeline_desc.vertex_shader = video_vertex;
        video_pipeline_desc.fragment_shader = video_fragment;
        auto video_pipeline = device.create_pipeline(PipelineKey(video_pipeline_desc), "bound video pipeline");
        check(bool(video_pipeline), device.last_error().c_str());
        auto frame_uniform = device.create_buffer({16, BufferUsage::uniform, true}, "frame viewport uniform");
        const std::array<float,4> viewport_uniform{160,120,0,0};
        check(device.upload({frame_uniform, 16, 0, 16}, viewport_uniform.data()), "viewport uniform upload failed");
        TextureDesc sample_desc;
        sample_desc.width = sample_desc.height = 2;
        auto sample = device.create_texture(sample_desc, "sampled green image");
        const std::array<unsigned char,16> green_texels{
            0,255,0,255, 0,255,0,255, 0,255,0,255, 0,255,0,255};
        check(device.upload_texture({sample, 2, 2, 8, 16, 0}, green_texels.data()), "green sample upload failed");
        auto sampler = device.create_sampler({}, "green nearest sampler");
        check(bool(sampler), "sampled image sampler creation failed");
        const std::array<Vertex,3> video_triangle{{
            {0,0,0xffffffffU,0,0}, {160,0,0xffffffffU,1,0}, {0,120,0xffffffffU,0,1}}};
        auto video_vertices = device.create_buffer({sizeof(video_triangle), BufferUsage::vertex, true}, "video pixels");
        check(device.upload({video_vertices,sizeof(video_triangle),0,sizeof(video_triangle)},video_triangle.data()),
            "video vertex upload failed");
        DrawDesc video_draw;
        video_draw.pipeline = video_pipeline;
        video_draw.vertex_buffer = video_vertices;
        video_draw.vertex_or_index_count = 3;
        video_draw.vertex_bindings.uniform_count = 1;
        video_draw.vertex_bindings.uniforms[0] = {frame_uniform,0,16};
        video_draw.fragment_bindings.texture_count = 1;
        video_draw.fragment_bindings.textures[0] = sample;
        video_draw.fragment_bindings.samplers[0] = sampler;
        pass.color_load = pass.depth_load = AttachmentLoad::clear;
        check(device.begin_pass(pass, "uniform and texture physical proof"), "video proof pass failed");
        auto missing_uniform = video_draw;
        missing_uniform.vertex_bindings.uniforms[0].buffer = {};
        check(!device.draw(missing_uniform), "missing reflected uniform accepted");
        auto missing_texture = video_draw;
        missing_texture.fragment_bindings.textures[0] = {};
        check(!device.draw(missing_texture), "missing reflected texture accepted");
        check(device.draw(video_draw), device.last_error().c_str());
        check(device.end_pass(), "video proof end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,20) && red(150,110),
            "reflected uniform/texture bindings did not drive physical pixels");
        auto blended_desc = video_pipeline_desc;
        blended_desc.blend.enabled = true;
        blended_desc.blend.source_color = BlendFactor::src_alpha;
        blended_desc.blend.destination_color = BlendFactor::inv_src_alpha;
        auto blended_pipeline = device.create_pipeline(PipelineKey(blended_desc), "source alpha blend");
        check(bool(blended_pipeline), device.last_error().c_str());
        auto half_alpha_texels = green_texels;
        for (std::size_t pixel_index = 3; pixel_index < half_alpha_texels.size(); pixel_index += 4)
            half_alpha_texels[pixel_index] = 128;
        check(device.upload_texture({sample,2,2,8,16,0}, half_alpha_texels.data()), "alpha texture update failed");
        video_draw.pipeline = blended_pipeline;
        check(device.begin_pass(pass, "source alpha physical proof"), "blend pass failed");
        check(device.draw(video_draw), device.last_error().c_str());
        check(device.end_pass(), "blend pass end failed");
        result = device.readback_rgba(color);
        const auto* blended = pixel(20,20);
        check(blended[0] >= 125 && blended[0] <= 130 && blended[1] >= 125 && blended[1] <= 130
            && blended[2] == 0 && red(150,110), "public bgfx source-alpha blend pixels differ");
        device.destroy(blended_pipeline);
        auto ui_vertex_shader = device.create_shader({ShaderStage::vertex,"renderer/ui.vert",1,0}, "UI vertex");
        auto ui_fragment_shader = device.create_shader({ShaderStage::fragment,"renderer/ui.frag",1,1}, "UI fragment");
        check(bool(ui_vertex_shader) && bool(ui_fragment_shader), device.last_error().c_str());
        auto ui_pipeline_desc = pipeline_desc;
        ui_pipeline_desc.vertex_shader = ui_vertex_shader;
        ui_pipeline_desc.fragment_shader = ui_fragment_shader;
        auto ui_pipeline = device.create_pipeline(PipelineKey(ui_pipeline_desc), "UI half-pixel alpha pipeline");
        auto ui_alpha = device.create_buffer({16,BufferUsage::uniform,true}, "UI alpha reference");
        check(bool(ui_pipeline) && bool(ui_alpha), device.last_error().c_str());
        const std::array<float,4> reject_alpha{{0.75f,0,0,0}};
        check(device.upload({ui_alpha,16,0,16},reject_alpha.data()), "UI alpha reference upload failed");
        auto ui_draw = video_draw;
        ui_draw.pipeline = ui_pipeline;
        ui_draw.fragment_bindings.uniform_count = 1;
        ui_draw.fragment_bindings.uniforms[0] = {ui_alpha,0,16};
        check(device.begin_pass(pass,"UI alpha rejection"), "UI alpha pass failed");
        check(device.draw(ui_draw), device.last_error().c_str());
        check(device.end_pass(), "UI alpha pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && red(20,20),
            "UI alpha-reference discard changed background");
        check(device.upload_texture({sample,2,2,8,16,0},green_texels.data()), "UI opaque texture update failed");
        const std::array<float,4> allow_alpha{{0,0,0,0}};
        check(device.upload({ui_alpha,16,0,16},allow_alpha.data()), "UI permissive alpha upload failed");
        check(device.begin_pass(pass,"UI half-pixel green proof"), "UI pixel pass failed");
        check(device.draw(ui_draw), device.last_error().c_str());
        check(device.end_pass(), "UI pixel pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,20) && red(150,110),
            "UI half-pixel/texture/alpha pixels differ");
        device.destroy(ui_alpha); device.destroy(ui_pipeline);
        device.destroy(ui_vertex_shader); device.destroy(ui_fragment_shader);
        struct PointVertex { float xyz[3]; std::uint32_t color; };
        auto point_vertex_shader = device.create_shader({ShaderStage::vertex,"renderer/points.vert",2,0}, "point vertex");
        auto point_fragment_shader = device.create_shader({ShaderStage::fragment,"renderer/points.frag",1,1}, "point fragment");
        check(bool(point_vertex_shader) && bool(point_fragment_shader), device.last_error().c_str());
        auto point_pipeline_desc = pipeline_desc;
        point_pipeline_desc.vertex_shader = point_vertex_shader;
        point_pipeline_desc.fragment_shader = point_fragment_shader;
        point_pipeline_desc.vertex_layout = VertexLayout::point_sprite;
        point_pipeline_desc.topology = PrimitiveTopology::point_list;
        point_pipeline_desc.uses_point_size = true;
        auto point_pipeline = device.create_pipeline(PipelineKey(point_pipeline_desc), "uniform-driven point");
        check(bool(point_pipeline), device.last_error().c_str());
        auto point_vb = device.create_buffer({sizeof(PointVertex),BufferUsage::vertex,true}, "point vertex bytes");
        const PointVertex point_vertex{{0,0,0.5f},0xffffffffU};
        check(device.upload({point_vb,sizeof(point_vertex),0,sizeof(point_vertex)},&point_vertex),
            "point vertex upload failed");
        std::array<float,20> frame_matrix{};
        std::array<float,20> object_matrix{};
        for (int diagonal : {0,5,10,15}) frame_matrix[diagonal] = object_matrix[diagonal] = 1;
        object_matrix[16] = 7;
        auto point_frame = device.create_buffer({sizeof(frame_matrix),BufferUsage::uniform,true}, "point frame");
        auto point_object = device.create_buffer({sizeof(object_matrix),BufferUsage::uniform,true}, "point object");
        auto point_material = device.create_buffer({16,BufferUsage::uniform,true}, "point material");
        const std::array<float,4> point_alpha{0,0,0,0};
        check(device.upload({point_frame,sizeof(frame_matrix),0,sizeof(frame_matrix)},frame_matrix.data())
            && device.upload({point_object,sizeof(object_matrix),0,sizeof(object_matrix)},object_matrix.data())
            && device.upload({point_material,16,0,16},point_alpha.data()), "point uniform upload failed");
        check(device.upload_texture({sample,2,2,8,16,0},green_texels.data()), "point texture restore failed");
        DrawDesc point_draw;
        point_draw.pipeline = point_pipeline;
        point_draw.vertex_buffer = point_vb;
        point_draw.vertex_or_index_count = 1;
        point_draw.point_size = 7;
        point_draw.vertex_bindings.uniform_count = 2;
        point_draw.vertex_bindings.uniforms[0] = {point_frame,0,sizeof(frame_matrix)};
        point_draw.vertex_bindings.uniforms[1] = {point_object,0,sizeof(object_matrix)};
        point_draw.fragment_bindings.uniform_count = 1;
        point_draw.fragment_bindings.uniforms[0] = {point_material,0,16};
        point_draw.fragment_bindings.texture_count = 1;
        point_draw.fragment_bindings.textures[0] = sample;
        point_draw.fragment_bindings.samplers[0] = sampler;
        check(device.begin_pass(pass, "point and multi-block physical proof"), "point pass failed");
        auto fractional_point = point_draw;
        fractional_point.point_size = 7.5f;
        check(!device.draw(fractional_point), "unrepresentable fractional point size accepted");
        check(device.draw(point_draw), device.last_error().c_str());
        check(device.end_pass(), "point pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(80,60) && red(70,60),
            "point-size or multi-block uniform pixels differ");
        device.destroy(point_vb); device.destroy(point_frame); device.destroy(point_object);
        device.destroy(point_material); device.destroy(point_pipeline);
        device.destroy(point_vertex_shader); device.destroy(point_fragment_shader);
        struct WorldVertex { float xyz[3], normal[3], uv[2]; };
        const std::array<WorldVertex,3> world_triangle{{
            {{-0.9f,0.9f,0.5f},{0,0,1},{0,0}},
            {{-0.6f,0.9f,0.5f},{0,0,1},{1,0}},
            {{-0.75f,0.6f,0.5f},{0,0,1},{0.5f,1}},
        }};
        auto world_vertex_shader = device.create_shader({ShaderStage::vertex,"renderer/world.vert",2,0}, "world vertex");
        auto world_fragment_shader = device.create_shader({ShaderStage::fragment,"renderer/world.frag",1,4}, "world fragment");
        check(bool(world_vertex_shader) && bool(world_fragment_shader), device.last_error().c_str());
        auto world_pipeline_desc = pipeline_desc;
        world_pipeline_desc.vertex_shader = world_vertex_shader;
        world_pipeline_desc.fragment_shader = world_fragment_shader;
        world_pipeline_desc.vertex_layout = VertexLayout::world_mesh;
        auto world_pipeline = device.create_pipeline(PipelineKey(world_pipeline_desc), "world material");
        auto world_vertices = device.create_buffer({sizeof(world_triangle),BufferUsage::vertex,true}, "world vertex bytes");
        check(bool(world_pipeline) && bool(world_vertices)
            && device.upload({world_vertices,sizeof(world_triangle),0,sizeof(world_triangle)},world_triangle.data()),
            "world pipeline or vertices unavailable");
        auto world_frame = device.create_buffer({64,BufferUsage::uniform,true}, "world view projection");
        auto world_object = device.create_buffer({64,BufferUsage::uniform,true}, "world model");
        auto world_material = device.create_buffer({16,BufferUsage::uniform,true}, "world material color");
        const std::array<float,16> identity{{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}};
        const std::array<float,4> world_diffuse{{0,0,0,0}};
        check(device.upload({world_frame,64,0,64},identity.data())
            && device.upload({world_object,64,0,64},identity.data())
            && device.upload({world_material,16,0,16},world_diffuse.data()), "world uniform upload failed");
        DrawDesc world_draw;
        world_draw.pipeline = world_pipeline;
        world_draw.vertex_buffer = world_vertices;
        world_draw.vertex_or_index_count = 3;
        world_draw.vertex_bindings.uniform_count = 2;
        world_draw.vertex_bindings.uniforms[0] = {world_frame,0,64};
        world_draw.vertex_bindings.uniforms[1] = {world_object,0,64};
        world_draw.fragment_bindings.uniform_count = 1;
        world_draw.fragment_bindings.uniforms[0] = {world_material,0,16};
        world_draw.fragment_bindings.texture_count = 4;
        for (int texture_index = 0; texture_index != 4; ++texture_index) {
            world_draw.fragment_bindings.textures[texture_index] = sample;
            world_draw.fragment_bindings.samplers[texture_index] = sampler;
        }
        check(device.begin_pass(pass,"world material physical proof"), "world pass failed");
        check(device.draw(world_draw), device.last_error().c_str());
        check(device.end_pass(), "world pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,15) && red(150,110),
            "world material texture/matrix pixels differ");
        device.destroy(world_vertices); device.destroy(world_frame); device.destroy(world_object);
        device.destroy(world_material); device.destroy(world_pipeline);
        device.destroy(world_vertex_shader); device.destroy(world_fragment_shader);
        struct EffectVertex { float xyz[3], uv[2]; };
        const std::array<EffectVertex,3> effect_triangle{{
            {{-0.9f,0.9f,0},{0,0}}, {{-0.6f,0.9f,0},{1,0}}, {{-0.75f,0.6f,0},{0.5f,1}},
        }};
        auto effect_vertex_shader = device.create_shader({ShaderStage::vertex,"effects/post_effect.vert",1,0}, "effect vertex");
        auto effect_fragment_shader = device.create_shader({ShaderStage::fragment,"effects/post_effect.frag",1,1}, "effect fragment");
        check(bool(effect_vertex_shader) && bool(effect_fragment_shader), device.last_error().c_str());
        auto effect_pipeline_desc = pipeline_desc;
        effect_pipeline_desc.vertex_shader = effect_vertex_shader;
        effect_pipeline_desc.fragment_shader = effect_fragment_shader;
        effect_pipeline_desc.vertex_layout = VertexLayout::water;
        auto effect_pipeline = device.create_pipeline(PipelineKey(effect_pipeline_desc), "post-effect pipeline");
        auto effect_vertices = device.create_buffer({sizeof(effect_triangle),BufferUsage::vertex,true}, "effect vertices");
        check(bool(effect_pipeline) && bool(effect_vertices)
            && device.upload({effect_vertices,sizeof(effect_triangle),0,sizeof(effect_triangle)},effect_triangle.data()),
            "effect pipeline or vertices unavailable");
        auto effect_frame = device.create_buffer({16,BufferUsage::uniform,true}, "effect frame");
        auto effect_scale = device.create_buffer({16,BufferUsage::uniform,true}, "effect scale");
        const std::array<float,4> unity{{1,1,1,1}};
        check(device.upload({effect_frame,16,0,16},viewport_uniform.data())
            && device.upload({effect_scale,16,0,16},unity.data()), "effect uniforms unavailable");
        DrawDesc effect_draw;
        effect_draw.pipeline = effect_pipeline;
        effect_draw.vertex_buffer = effect_vertices;
        effect_draw.vertex_or_index_count = 3;
        effect_draw.vertex_bindings.uniform_count = 1;
        effect_draw.vertex_bindings.uniforms[0] = {effect_frame,0,16};
        effect_draw.fragment_bindings.uniform_count = 1;
        effect_draw.fragment_bindings.uniforms[0] = {effect_scale,0,16};
        effect_draw.fragment_bindings.texture_count = 1;
        effect_draw.fragment_bindings.textures[0] = sample;
        effect_draw.fragment_bindings.samplers[0] = sampler;
        SamplerDesc no_mip_desc;
        no_mip_desc.maximum_lod = 0.0f;
        auto no_mip_sampler = device.create_sampler(no_mip_desc, "one-level source no-mip sampler");
        SamplerDesc unsupported_lod_desc;
        unsupported_lod_desc.maximum_lod = 1.0f;
        auto unsupported_lod_sampler = device.create_sampler(unsupported_lod_desc, "unsupported bounded LOD sampler");
        check(bool(no_mip_sampler) && bool(unsupported_lod_sampler), "LOD sampler allocation failed");
        check(device.begin_pass(pass,"effect material physical proof"), "effect pass failed");
        effect_draw.fragment_bindings.samplers[0] = unsupported_lod_sampler;
        check(!device.draw(effect_draw) && device.last_error().find("unsupported sampler LOD") != std::string::npos,
            "unsupported bounded sampler LOD accepted");
        effect_draw.fragment_bindings.samplers[0] = no_mip_sampler;
        check(device.draw(effect_draw), device.last_error().c_str());
        check(device.end_pass(), "effect pass end failed");
        result = device.readback_rgba(color);
        check(result.size() == 160U * 120U * 4U && green(20,15) && red(150,110),
            "one-mip LOD0 effect sampler/color-scale pixels differ");
        device.destroy(no_mip_sampler); device.destroy(unsupported_lod_sampler);
        device.destroy(effect_vertices); device.destroy(effect_frame); device.destroy(effect_scale);
        device.destroy(effect_pipeline); device.destroy(effect_vertex_shader); device.destroy(effect_fragment_shader);
        device.destroy(video_vertices); device.destroy(sampler); device.destroy(sample);
        device.destroy(frame_uniform); device.destroy(video_pipeline);
        device.destroy(video_vertex); device.destroy(video_fragment);
        device.destroy(color); device.destroy(depth);
        device.destroy(vertex_buffer);
        device.destroy(pipeline); device.destroy(vertex_shader); device.destroy(fragment_shader);
    }
}

void test_shader_envelope_negatives()
{
    using namespace zh::renderer;
    const auto root = std::filesystem::temp_directory_path()
        / ("zh-m30-envelope-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto owned = root / "renderer";
    std::filesystem::create_directories(owned);
    struct Cleanup {
        std::filesystem::path root;
        ~Cleanup() {
            std::error_code error;
            std::filesystem::remove(root / "renderer" / "ui.vert.bin", error);
            std::filesystem::remove(root / "renderer" / "ui.vert.json", error);
            std::filesystem::remove(root / "renderer", error);
            std::filesystem::remove(root, error);
        }
    } cleanup{root};
    const auto source = std::filesystem::path(ZH_BGFX_SHADER_DIR) / "renderer";
    std::filesystem::copy_file(source / "ui.vert.json", owned / "ui.vert.json");
    std::ifstream input(source / "ui.vert.bin", std::ios::binary);
    const std::vector<char> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    check(bytes.size() > 64, "pinned nested-uniform shader is missing");
    BgfxOptions options;
    options.shader_root = root;
    BgfxGpuDevice device(options);
    const auto write_binary = [&](const std::vector<char>& payload) {
        std::ofstream output(owned / "ui.vert.bin", std::ios::binary | std::ios::trunc);
        output.write(payload.data(), static_cast<std::streamsize>(payload.size()));
        check(bool(output), "failed to write owned malformed shader fixture");
    };
    write_binary(bytes);
    auto valid = device.create_shader({ShaderStage::vertex, "renderer/ui.vert", 1, 0}, "valid copied envelope");
    check(bool(valid), device.last_error().c_str());
    device.destroy(valid);
    auto malformed = bytes;
    const std::string nested = "ZhStageUniforms.frame_data.viewport";
    const auto position = std::search(malformed.begin(), malformed.end(), nested.begin(), nested.end());
    check(position != malformed.end(), "expected length-prefixed reflected field is absent");
    *(position + std::string("ZhStageUniforms").size()) = '-';
    write_binary(malformed);
    check(!device.create_shader({ShaderStage::vertex, "renderer/ui.vert", 1, 0}, "invalid name"),
        "unknown reflected identifier character accepted");
    check(device.last_error().find("invalid or colliding") != std::string::npos,
        "malformed reflected identifier did not fail at envelope validation");
    malformed = bytes;
    malformed[20] = static_cast<char>(0xff);
    malformed[21] = static_cast<char>(0xff);
    write_binary(malformed);
    check(!device.create_shader({ShaderStage::vertex, "renderer/ui.vert", 1, 0}, "bad count"),
        "oversized reflected uniform count accepted");
    write_binary(bytes);
    {
        std::ofstream manifest(owned / "ui.vert.json", std::ios::binary | std::ios::trunc);
        manifest << "{\"stage\":\"fragment\",\"textures\":[],\"uniform_blocks\":[]}";
    }
    check(!device.create_shader({ShaderStage::vertex, "renderer/ui.vert", 1, 0}, "wrong manifest stage"),
        "shader/manifest stage mismatch accepted");
}

void test_window_presentation()
{
    using namespace zh::renderer;
    check(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
    SDL_Window* window = SDL_CreateWindow("M30 public bgfx presentation", 160, 120, SDL_WINDOW_RESIZABLE);
    check(window != nullptr, SDL_GetError());
    for (int generation = 0; generation != 2; ++generation) {
        BgfxOptions options;
        options.shader_root = ZH_BGFX_SHADER_DIR;
        bool suspended = false;
        bool unavailable = false;
        options.pixel_extent_query = [&](SDL_Window* target, int* width, int* height) {
            if (unavailable) return false;
            if (suspended) { *width = *height = 0; return true; }
            return SDL_GetWindowSizeInPixels(target, width, height);
        };
        BgfxGpuDevice device(options);
        check(!device.claim_window(nullptr), "null SDL window accepted");
        check(device.claim_window(window), device.last_error().c_str());
        check(!device.claim_window(window), "double SDL window claim accepted");
        check(!device.present({}), "stale presentation source accepted");
        TextureDesc color_desc;
        color_desc.width = 160; color_desc.height = 120;
        color_desc.format = TextureFormat::bgra8;
        color_desc.render_target = true;
        auto color = device.create_texture(color_desc, "SDL3 presentation source");
        check(bool(color), "presentation target allocation failed");
        auto depth_desc = color_desc;
        depth_desc.format = TextureFormat::depth24_stencil8;
        auto depth = device.create_texture(depth_desc, "SDL3 presentation depth");
        check(bool(depth), "presentation depth allocation failed");
        check(!device.present(color), "uninitialized presentation source accepted");
        RenderPassDesc pass;
        pass.color_targets[0] = color; pass.color_target_count = 1;
        pass.depth_target = depth;
        pass.width = 160; pass.height = 120;
        pass.clear_color = {0,1,0,1};
        check(device.begin_pass(pass, "SDL3 green frame"), "presentation source pass failed");
        check(!device.present(color), "mid-pass present accepted");
        check(device.end_pass(), "presentation source end failed");
        check(device.present(color), device.last_error().c_str());
        suspended = true;
        check(device.present(color), "zero-pixel SDL window suspension failed");
        suspended = false;
        unavailable = true;
        check(!device.present(color) && device.last_error().find("pixel size is unavailable") != std::string::npos,
            "SDL pixel-size query failure was not propagated");
        unavailable = false;
        check(device.present(color), "presentation did not recover after zero extent/query failure");
        if (generation == 0) {
            int previous_width = 0, previous_height = 0;
            check(SDL_GetWindowSizeInPixels(window, &previous_width, &previous_height), SDL_GetError());
            check(SDL_SetWindowSize(window, 200, 140) && SDL_SyncWindow(window), SDL_GetError());
            int resized_width = 0, resized_height = 0;
            check(SDL_GetWindowSizeInPixels(window, &resized_width, &resized_height), SDL_GetError());
            check(resized_width > 0 && resized_height > 0
                && (resized_width != previous_width || resized_height != previous_height),
                "SDL3 pixel extent did not change for public bgfx swap-chain resize");
        }
        check(device.present(color), device.last_error().c_str());
        device.release_window();
        check(!device.present(color), "present after release accepted");
        check(device.claim_window(window), device.last_error().c_str());
        check(device.present(color), device.last_error().c_str());
        device.release_window();
        device.destroy(color); device.destroy(depth);
        check(device.wait_idle(), "presentation lifecycle idle failed");
    }
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

} // namespace

int main(int argc, char** argv)
{
    try {
        test_options();
        if (argc > 1 && std::string(argv[1]) == "--gpu") {
            test_resources();
            test_shader_envelope_negatives();
            test_window_presentation();
        }
        std::cout << "bgfx resource/device contract tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
