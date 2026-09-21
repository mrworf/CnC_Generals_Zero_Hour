#include "zh/platform/sdl_gpu_device.h"
#include "zh/effects/recorder.h"
#include "zh/ui/renderer.h"
#include "zh/world/recorder.h"
#include "zh/video/player.h"

#include <SDL3/SDL.h>

#include <array>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace zh::renderer;

void check(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }

struct Vertex { float x, y; std::uint32_t color; float u, v; };

class GeneratedVideoDecoder final : public zh::video::VideoDecoder {
public:
    GeneratedVideoDecoder()
    {
        metadata_.width = 64; metadata_.height = 48; metadata_.frame_rate = 30.0;
        metadata_.video_codec = "project-owned-rgba";
    }
    const zh::video::VideoMetadata& metadata() const noexcept override { return metadata_; }
    const std::string& logical_path() const noexcept override { return logical_; }
    bool read_frame(zh::video::VideoFrame& frame) override
    {
        if (done_) { eos_ = true; return false; }
        frame.width = metadata_.width; frame.height = metadata_.height; frame.timestamp_seconds = 0.0;
        frame.rgba.resize(static_cast<std::size_t>(frame.width) * frame.height * 4U);
        for (std::size_t index = 0; index < frame.rgba.size(); index += 4) {
            frame.rgba[index] = static_cast<std::uint8_t>((index / 4U) % frame.width * 4U);
            frame.rgba[index + 1] = static_cast<std::uint8_t>((index / 4U) / frame.width * 5U);
            frame.rgba[index + 2] = 128; frame.rgba[index + 3] = 255;
        }
        done_ = true;
        return true;
    }
    std::vector<zh::video::VideoAudioChunk> drain_audio() override { return {}; }
    bool end_of_stream() const noexcept override { return eos_; }
    void close() noexcept override { eos_ = true; }
private:
    zh::video::VideoMetadata metadata_;
    std::string logical_ = "project-owned/M14-generated-frame";
    bool done_ = false;
    bool eos_ = false;
};

void render_generation(SDL_Window* window, int generation)
{
    SdlGpuOptions options;
    options.shader_root = ZH_GPU_SHADER_DIR;
    options.debug = true;
    SdlGpuDevice device(options);
    check(device.capabilities().backend == "vulkan", "SDL_GPU did not select Vulkan");
    check(!device.claim_window(nullptr) && device.last_error().find("null") != std::string::npos,
        "null window was not rejected");
    check(device.claim_window(window), device.last_error());
    check(!device.claim_window(window) && device.last_error().find("already claimed") != std::string::npos,
        "duplicate window claim was not rejected");
    check(!device.present({}) && device.last_error().find("source texture") != std::string::npos,
        "stale presentation source was not rejected");

    const auto vertex_shader = device.create_shader({ShaderStage::vertex, "renderer/acceptance.vert", 0, 0}, "M14 acceptance vertex");
    const auto fragment_shader = device.create_shader({ShaderStage::fragment, "renderer/acceptance.frag", 0, 0}, "M14 acceptance fragment");
    const auto vertices = device.create_buffer({sizeof(Vertex) * 3, BufferUsage::vertex, true}, "M14 acceptance vertices");
    TextureDesc color_desc;
    color_desc.width = 320; color_desc.height = 240; color_desc.render_target = true;
    const auto color = device.create_texture(color_desc, "M14 acceptance color");
    auto depth_desc = color_desc;
    depth_desc.format = TextureFormat::depth24_stencil8; depth_desc.sampled = false;
    const auto depth = device.create_texture(depth_desc, "M14 acceptance depth");
    PipelineDesc pipeline_desc;
    pipeline_desc.vertex_shader = vertex_shader; pipeline_desc.fragment_shader = fragment_shader;
    pipeline_desc.vertex_layout = VertexLayout::position_color_uv;
    pipeline_desc.raster.cull = CullMode::none;
    auto unsupported_pipeline = pipeline_desc;
    unsupported_pipeline.topology = PrimitiveTopology::triangle_fan;
    check(!device.create_pipeline(PipelineKey(unsupported_pipeline), "unsupported triangle fan")
            && device.last_error().find("CPU-expanded") != std::string::npos,
        "triangle fan was not rejected with the CPU-expansion contract");
    const auto pipeline = device.create_pipeline(PipelineKey(pipeline_desc), "M14 acceptance pipeline");
    check(vertex_shader && fragment_shader && vertices && color && depth && pipeline, device.last_error());

    const std::array<Vertex, 3> triangle{{
        {-0.8F, 0.7F, 0xff2020ffU, 0.0F, 0.0F},
        {0.8F, 0.7F, 0xff20ff20U, 1.0F, 0.0F},
        {0.0F, -0.8F, 0xffff2020U, 0.5F, 1.0F},
    }};
    check(device.upload({vertices, sizeof(triangle), 0, sizeof(triangle)}, triangle.data()), device.last_error());
    RenderPassDesc pass;
    pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
    pass.width = 320; pass.height = 240;
    DrawDesc draw;
    draw.pipeline = pipeline; draw.vertex_buffer = vertices; draw.vertex_or_index_count = 3;
    const std::array<const char*, 16> scenes{{
        "ui", "font", "terrain", "faction-usa", "faction-china", "faction-gla", "water", "particles",
        "shadows", "roads-decals", "trees", "stealth", "wwshade", "movie", "resize", "focus",
    }};
    const auto started = std::chrono::steady_clock::now();
    for (const char* scene : scenes) {
        const std::string label = "M14 generation " + std::to_string(generation) + " scene " + scene;
        check(device.begin_pass(pass, label), device.last_error());
        if (scene == scenes.front()) {
            check(!device.begin_pass(pass, "nested") && device.last_error().find("already active") != std::string::npos,
                "nested render pass was not rejected");
        }
        device.record_marker(std::string("scene=") + scene
            + " handedness=left depth=0..1 winding=clockwise half-pixel=-0.5 color=argb8 alpha=premultiplied"
              " fog=view-space bias=explicit origin=top-left av=ordered");
        check(device.draw(draw), device.last_error());
        check(device.end_pass(), device.last_error());
        check(device.present(color), device.last_error());
        std::cout << "scene=" << scene << " status=pass generation=" << generation << '\n';
    }
    // B2 public attachment semantics: the original WW3D source will select
    // these loads/clears in B3. Exercise the physical edge independently now.
    const unsigned frame_width=generation==1 ? 160U : 240U;
    const unsigned frame_height=generation==1 ? 120U : 180U;
    TextureDesc frame_desc=color_desc;
    frame_desc.width=frame_width; frame_desc.height=frame_height;
    const auto frame_color=device.create_texture(frame_desc,"M22 source-clear color");
    frame_desc.format=TextureFormat::depth24_stencil8; frame_desc.sampled=false;
    const auto frame_depth=device.create_texture(frame_desc,"M22 source-clear depth");
    check(frame_color && frame_depth,device.last_error());
    RenderPassDesc frame{};
    frame.color_targets[0]=frame_color; frame.color_target_count=1;
    frame.depth_target=frame_depth; frame.width=frame_width; frame.height=frame_height;
    frame.color_load=AttachmentLoad::load;
    check(!device.begin_pass(frame,"load untouched color") &&
        device.last_error().find("uninitialized color")!=std::string::npos,
        "uninitialized Vulkan color load accepted");
    check(!device.present(frame_color),"uninitialized color presentation accepted");
    frame.color_load=AttachmentLoad::clear; frame.depth_load=AttachmentLoad::load;
    check(!device.begin_pass(frame,"load untouched depth") &&
        device.last_error().find("uninitialized depth")!=std::string::npos,
        "uninitialized Vulkan depth load accepted");
    frame.depth_load=AttachmentLoad::clear;
    frame.clear_color={0.125F,0.25F,0.375F,0.5F}; frame.clear_depth=0.25F;
    check(device.begin_pass(frame,"M22 explicit full-target color alpha depth clear"),device.last_error());
    check(!device.begin_pass(frame,"midpass clear"),"midpass Vulkan clear accepted");
    check(device.draw(draw),device.last_error()); // original z=0.5 fails depth <=0.25
    check(device.end_pass(),device.last_error());
    frame.color_load=AttachmentLoad::load; frame.depth_load=AttachmentLoad::load;
    check(device.begin_pass(frame,"M22 preserve color and depth"),device.last_error());
    check(device.draw(draw),device.last_error());
    check(device.end_pass(),device.last_error());
    const auto preserved=device.readback_rgba(frame_color);
    check(preserved.size()==frame_width*frame_height*4U,device.last_error());
    const auto corner=preserved.data();
    const auto frame_center=preserved.data()+((frame_height/2U)*frame_width+(frame_width/2U))*4U;
    for (unsigned channel=0;channel<4;++channel)
        check(std::abs(static_cast<int>(frame_center[channel])-static_cast<int>(corner[channel]))<=1,
            "depth LOAD did not preserve source clear's occlusion");
    check(std::abs(static_cast<int>(corner[0])-32)<=1 && std::abs(static_cast<int>(corner[1])-64)<=1 &&
        std::abs(static_cast<int>(corner[2])-96)<=1 && std::abs(static_cast<int>(corner[3])-128)<=1,
        "full-target source clear lost RGBA, including destination alpha");
    frame.depth_load=AttachmentLoad::clear; frame.clear_depth=1.0F;
    check(device.begin_pass(frame,"M22 preserve color reset depth"),device.last_error());
    check(device.draw(draw),device.last_error());
    check(device.end_pass(),device.last_error());
    const auto revealed=device.readback_rgba(frame_color);
    const auto* lit=revealed.data()+((frame_height/2U)*frame_width+(frame_width/2U))*4U;
    check(std::abs(static_cast<int>(lit[0])-static_cast<int>(corner[0]))>20 ||
          std::abs(static_cast<int>(lit[1])-static_cast<int>(corner[1]))>20,
        "depth CLEAR did not permit same geometry after source depth LOAD");
    frame.clear_color[3]=std::numeric_limits<float>::infinity();
    check(!device.begin_pass(frame,"invalid alpha"),"infinite Vulkan clear alpha accepted");
    frame.clear_color[3]=0.5F;
    check(device.present(frame_color),device.last_error());
    device.destroy(frame_color);
    check(!device.begin_pass(frame,"stale color attachment") &&
        device.last_error().find("stale")!=std::string::npos,
        "destroyed GPU color generation remained loadable");
    frame_desc.format=TextureFormat::rgba8; frame_desc.sampled=true;
    const auto replacement=device.create_texture(frame_desc,"M22 next color generation");
    check(replacement && replacement!=frame_color,device.last_error());
    frame.color_targets[0]=replacement;
    check(!device.begin_pass(frame,"new uninitialized generation") &&
        device.last_error().find("uninitialized color")!=std::string::npos,
        "replacement target inherited prior generation's LOAD contents");
    device.destroy(replacement); device.destroy(frame_depth);
    std::cout << "original-frame-attachment-clear-load-pixels=pass generation="<<generation
              << " extent="<<frame_width<<"x"<<frame_height<<'\n';
    // Backend parity for original WW3D: its shared mesh index buffers are
    // 16-bit and each draw carries its own first index and base vertex.
    const auto indexed_vertices = device.create_buffer({sizeof(Vertex) * 4, BufferUsage::vertex, true}, "M22 indexed vertices");
    const auto indexed_indices = device.create_buffer({sizeof(std::uint16_t) * 4, BufferUsage::index, true}, "M22 16-bit indices");
    check(indexed_vertices && indexed_indices, device.last_error());
    const std::array<Vertex, 4> indexed_geometry{{
        {0.0F, 0.0F, 0xff000000U, 0.0F, 0.0F}, triangle[0], triangle[1], triangle[2],
    }};
    const std::array<std::uint16_t, 4> original_indices{{0, 0, 1, 2}};
    check(device.upload({indexed_vertices, sizeof(indexed_geometry), 0, sizeof(indexed_geometry)}, indexed_geometry.data()), device.last_error());
    check(device.upload({indexed_indices, sizeof(original_indices), 0, sizeof(original_indices)}, original_indices.data()), device.last_error());
    DrawDesc indexed_draw = draw;
    indexed_draw.vertex_buffer = indexed_vertices;
    indexed_draw.index_buffer = indexed_indices;
    indexed_draw.index_element_size = IndexElementSize::uint16;
    indexed_draw.first_index = 1;
    indexed_draw.base_vertex = 1;
    check(device.begin_pass(pass, "M22 16-bit indexed contract"), device.last_error());
    indexed_draw.first_index = 3;
    check(!device.draw(indexed_draw) && device.last_error().find("index buffer") != std::string::npos,
        "out-of-range original index offset was not rejected");
    indexed_draw.first_index = 1;
    check(device.draw(indexed_draw), device.last_error());
    check(device.end_pass(), device.last_error());
    check(device.present(color), device.last_error());
    device.destroy(indexed_indices);
    device.destroy(indexed_vertices);
    // Canonical original XYZ + diffuse + UV1: original FVFInfoClass reports
    // offsets 0, 12, 16 and stride 24. Prove SDL consumes those original
    // indexed bytes directly, not the fixed world_mesh reinterpretation.
    struct OriginalVertex { float xyz[3]; std::uint32_t diffuse; float uv[2]; };
    static_assert(sizeof(OriginalVertex) == 24);
    PipelineDesc original_pipeline = pipeline_desc;
    original_pipeline.vertex_shader = device.create_shader(
        {ShaderStage::vertex, "renderer/original_fvf_probe.vert", 0, 0}, "source FVF layout probe");
    original_pipeline.vertex_layout = VertexLayout::original_fvf;
    original_pipeline.original_fvf.stride = sizeof(OriginalVertex);
    original_pipeline.original_fvf.attribute_count = 3;
    original_pipeline.original_fvf.attributes[0] = {0, VertexElementFormat::float3, 0};
    original_pipeline.original_fvf.attributes[1] = {2, VertexElementFormat::ubyte4_norm, 12};
    original_pipeline.original_fvf.attributes[2] = {4, VertexElementFormat::float2, 16};
    const auto original_pipeline_handle = device.create_pipeline(PipelineKey(original_pipeline), "original FVF indexed layout");
    check(original_pipeline.vertex_shader && original_pipeline_handle, device.last_error());
    const std::array<OriginalVertex, 4> source_vertices{{
        {{0.0F, 0.0F, 0.5F}, 0xff000000U, {0.0F, 0.0F}},
        {{-0.8F, 0.7F, 0.5F}, 0xff2020ffU, {0.0F, 0.0F}},
        {{0.8F, 0.7F, 0.5F}, 0xff20ff20U, {1.0F, 0.0F}},
        {{0.0F, -0.8F, 0.5F}, 0xffff2020U, {0.5F, 1.0F}},
    }};
    const auto source_vb = device.create_buffer({sizeof(source_vertices), BufferUsage::vertex, true}, "original FVF bytes");
    const auto source_ib = device.create_buffer({sizeof(original_indices), BufferUsage::index, true}, "original FVF indices");
    check(source_vb && source_ib, device.last_error());
    check(device.upload({source_vb, sizeof(source_vertices), 0, sizeof(source_vertices)}, source_vertices.data()), device.last_error());
    check(device.upload({source_ib, sizeof(original_indices), 0, sizeof(original_indices)}, original_indices.data()), device.last_error());
    indexed_draw.pipeline = original_pipeline_handle;
    indexed_draw.vertex_buffer = source_vb;
    indexed_draw.index_buffer = source_ib;
    check(device.begin_pass(pass, "original FVF indexed Vulkan layout"), device.last_error());
    check(device.draw(indexed_draw), device.last_error());
    check(device.end_pass(), device.last_error());
    const auto fvf_pixels = device.readback_rgba(color);
    check(fvf_pixels.size() == 320U * 240U * 4U, device.last_error());
    const auto pixel = [&](unsigned x, unsigned y, unsigned channel) {
        return fvf_pixels[(y * 320U + x) * 4U + channel];
    };
    const unsigned center = pixel(160, 120, 0) + pixel(160, 120, 1) + pixel(160, 120, 2);
    const unsigned clear = pixel(0, 0, 0) + pixel(0, 0, 1) + pixel(0, 0, 2);
    check(center > clear + 25, "original indexed FVF layout did not shade center against clear corner");
    std::cout << "original-fvf-indexed-pixels=pass generation=" << generation
              << " center-rgb-sum=" << center << " clear-rgb-sum=" << clear << '\n';
    check(device.present(color), device.last_error());
    device.destroy(source_ib); device.destroy(source_vb);
    device.destroy(original_pipeline_handle); device.destroy(original_pipeline.vertex_shader);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started).count();
    check(!device.end_pass() && device.last_error().find("no render pass") != std::string::npos,
        "end without pass was not rejected");
    check(device.wait_idle(), device.last_error());
    check(elapsed < 10000, "representative M14 scene matrix exceeded 10 second functional threshold");

    std::vector<zh::effects::EffectRequest> effects{
        {"Art/Water/WaterPlane.w3d", "water-surface", "water-and-reflection", 3, 24.0F},
        {"Data/INI/ParticleSystem.ini", "particle-sprite", "particle-point-list", 4, 24.0F},
        {"Art/Textures/Projected.dds", "projected-decal", "projected-texture", 5, 24.0F},
        {"Art/Textures/BumpEnv.dds", "bump-environment", "bump-environment", 6, 24.0F},
        {"Art/Textures/Stealth.dds", "stealth-shell", "stealth-distortion", 7, 24.0F},
        {"Post/SceneColor", "post-process", "scene-post-effect", 8, 24.0F},
        {"Shaders/fterrain.pso", "wwshade-terrain", "wwshade-multipass", 9, 24.0F},
    };
    zh::effects::EffectsRecorder effects_recorder(device);
    check(effects_recorder.load(std::move(effects)), effects_recorder.last_error());
    check(effects_recorder.record_frame(static_cast<UInt32>(generation)), effects_recorder.last_error());
    check(device.present_last(), device.last_error());
    check(effects_recorder.teardown(), effects_recorder.last_error());

    {
        zh::ui::UiRecorder ui_recorder(device);
        zh::ui::Scene ui_scene;
        ui_scene.width = 320; ui_scene.height = 240; ui_scene.transition = zh::ui::ScreenTransition::menu_enter;
        ui_scene.elements = {
            {"menu-panel", {16, 16, 288, 160}, {0, 0, 320, 240}, 0xff304060U, zh::ui::BlendMode::opaque, zh::ui::Layer::background},
            {"english-font-sample", {32, 48, 224, 32}, {0, 0, 320, 240}, 0xffffffffU, zh::ui::BlendMode::alpha, zh::ui::Layer::content},
            {"cursor", {140, 108, 16, 16}, {0, 0, 320, 240}, 0xffffffffU, zh::ui::BlendMode::additive, zh::ui::Layer::cursor},
        };
        check(ui_recorder.record(ui_scene), ui_recorder.last_error());
        check(device.present_last(), device.last_error());
    }

    {
        zh::world::WorldScene world_scene;
        world_scene.map_name = "project-owned-M14-scene";
        world_scene.camera.viewport_width = 320;
        world_scene.camera.viewport_height = 240;
        for (unsigned family = 0; family <= static_cast<unsigned>(zh::world::GeometryFamily::selection_marker); ++family) {
            zh::world::WorldItem item;
            item.label = "family-" + std::to_string(family);
            item.family = static_cast<zh::world::GeometryFamily>(family);
            if (item.family == zh::world::GeometryFamily::faction_geometry) item.faction = zh::world::Faction::usa;
            item.vertex_count = 6; item.index_count = 6;
            item.material.fog.enabled = item.family == zh::world::GeometryFamily::terrain;
            item.material.alpha_test.enabled = item.family == zh::world::GeometryFamily::tree;
            item.material.blending = item.family == zh::world::GeometryFamily::road_decal;
            world_scene.items.push_back(item);
        }
        for (auto faction : {zh::world::Faction::usa, zh::world::Faction::china, zh::world::Faction::gla}) {
            zh::world::WorldItem item;
            item.label = "faction-" + std::string(zh::world::faction_name(faction));
            item.family = zh::world::GeometryFamily::faction_geometry;
            item.faction = faction; item.vertex_count = 6; item.index_count = 6;
            world_scene.items.push_back(item);
        }
        zh::world::WorldRecorder world_recorder(device);
        check(world_recorder.load(std::move(world_scene)), world_recorder.last_error());
        check(world_recorder.record_frame(static_cast<UInt32>(generation)), world_recorder.last_error());
        check(device.present_last(), device.last_error());
        check(world_recorder.teardown(), world_recorder.last_error());
    }

    {
        zh::video::NullVideoAudioSink audio;
        auto decoder = std::make_unique<GeneratedVideoDecoder>();
        zh::video::VideoPlayer player(std::move(decoder), audio, device, 320, 240);
        player.advance(0.0);
        check(player.presented_frames() == 1, "project-owned movie frame did not reach SDL_GPU presentation");
        player.advance(0.0);
        check(player.state() == zh::video::PlaybackState::completed, "project-owned movie did not complete");
    }

    std::cout << "generation=" << generation << " backend=" << device.capabilities().backend
              << " device=" << device.capabilities().device_name
              << " driver=" << device.capabilities().driver_name << " " << device.capabilities().driver_version
              << " sdl=" << device.capabilities().sdl_version
              << " bc=" << device.capabilities().bc1 << device.capabilities().bc2 << device.capabilities().bc3
              << " scenes=" << scenes.size() << " elapsed_ms=" << elapsed << '\n';

    device.destroy(pipeline); device.destroy(vertices); device.destroy(fragment_shader); device.destroy(vertex_shader);
    device.destroy(depth); device.destroy(color);
    check(device.readback_rgba(color).empty() && device.last_error().find("live RGBA8") != std::string::npos,
        "stale original FVF readback was not rejected after device resource teardown");
    device.release_window();
}

} // namespace

int main()
{
    SDL_Window* window = nullptr;
    try {
        check(SDL_Init(SDL_INIT_VIDEO), std::string("SDL video init failed: ") + SDL_GetError());
        window = SDL_CreateWindow("Zero Hour M14 SDL_GPU acceptance", 320, 240, SDL_WINDOW_RESIZABLE);
        check(window != nullptr, std::string("SDL window creation failed: ") + SDL_GetError());
        render_generation(window, 1);
        check(SDL_SetWindowSize(window, 400, 300) && SDL_SyncWindow(window),
            std::string("resize transition failed: ") + SDL_GetError());
        check(SDL_SetWindowFullscreen(window, true) && SDL_SyncWindow(window),
            std::string("fullscreen enter failed: ") + SDL_GetError());
        check(SDL_SetWindowFullscreen(window, false) && SDL_SyncWindow(window),
            std::string("fullscreen leave failed: ") + SDL_GetError());
        SDL_HideWindow(window);
        SDL_ShowWindow(window);
        render_generation(window, 2);
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        std::cout << "SDL_GPU resource/pass/presentation/resize/focus/fullscreen/recreation: ok\n";
        return 0;
    } catch (const std::exception& error) {
        if (window) SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        std::cerr << error.what() << '\n';
        return 1;
    }
}
