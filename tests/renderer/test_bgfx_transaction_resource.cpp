#include "zh/platform/bgfx_device.h"
#include "../../src/renderer/bgfx_transaction_state.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace zh::renderer;
void check(bool value, const char* message)
{ if (!value) throw std::runtime_error(message); }

DeviceTransactionDesc desc()
{ return {DeviceTransactionMode::idle_preparation, 1, 128, 128, 4U*1024U*1024U, 0}; }

void cpu()
{
    using State = detail::BgfxTransactionState;
    for (UInt64 generation = 1; generation <= 2; ++generation) {
        auto good = desc(); good.generation = generation;
        for (unsigned category = 0; category < 12; ++category) {
            auto bad = good;
            switch (category) {
            case 0: bad.mode = DeviceTransactionMode::frame_commands; break;
            case 1: bad.mode = static_cast<DeviceTransactionMode>(255); break;
            case 2: bad.generation = 0; break;
            case 3: bad.commands = 0; break;
            case 4: bad.commands = State::maximum_commands+1; break;
            case 5: bad.resources = 0; break;
            case 6: bad.resources = State::maximum_resources+1; break;
            case 7: bad.bytes = 0; break;
            case 8: bad.bytes = State::maximum_bytes+1; break;
            case 9: bad.views = 1; break;
            case 10: bad.resources = 1; break;
            case 11: bad.bytes = 1; break;
            }
            State state;
            check(!state.begin(bad, generation, 2, 2) && !state.active(), "bad CPU admission mutated owner");
        }
        State state;
        check(!state.begin(good, 0, 0, 0), "zero device admitted");
        check(state.begin(good, generation, 2, 2), "CPU baseline admission failed");
        const auto token = state.token();
        check(!state.begin(good, generation, 2, 2), "nested CPU owner admitted");
        for (unsigned field = 0; field < 4; ++field) {
            auto stale = token;
            if (field == 0) ++stale.device;
            if (field == 1) ++stale.sequence;
            if (field == 2) ++stale.generation;
            if (field == 3) stale.mode = DeviceTransactionMode::frame_commands;
            check(!state.finish(stale, false) && state.matches(token), "stale token consumed CPU owner");
        }
        check(state.charge(3, 1) && state.retain_bytes(7) && state.retain_resources(2), "CPU capacity charge rejected");
        check(state.finish(token, true) && !state.finish(token, true) && !state.finish(token, false),
              "CPU finish was not exactly once");
        check(state.begin(good, generation, 0, 0) && state.token().sequence > token.sequence,
              "CPU retry reused sequence");
        const auto retry = state.token();
        state.poison();
        check(!state.finish(retry, true) && state.finish(retry, false), "CPU poisoned owner committed");
        for (unsigned limit = 0; limit < 5; ++limit) {
            State bounded;
            auto small = good; small.commands = 1; small.resources = 2; small.bytes = 2;
            check(bounded.begin(small, generation, 1, 1), "bounded CPU owner rejected");
            bool result = true;
            if (limit == 0) { check(bounded.charge(), "first command rejected"); result = bounded.charge(); }
            if (limit == 1) result = bounded.charge(2);
            if (limit == 2) result = bounded.charge(0, 2);
            if (limit == 3) result = bounded.retain_bytes(std::numeric_limits<UInt64>::max());
            if (limit == 4) result = bounded.retain_resources(std::numeric_limits<UInt32>::max());
            check(!result && bounded.failed() && !bounded.finish(bounded.token(), true)
                      && bounded.finish(bounded.token(), false), "CPU bound+1 partially accepted");
        }
    }
}

struct Scene {
    BgfxGpuDevice& device;
    ShaderHandle vs, fs;
    PipelineHandle pipeline;
    BufferHandle uniform, vertices, indices;
    TextureHandle sample, color, depth;
    SamplerHandle sampler;
    PipelineDesc state;
    std::array<float,4> viewport{64,64,0,0};
    struct Vertex { float x,y; UInt32 diffuse; float u,v; };
    std::array<Vertex,3> triangle{{{0,0,0xffffffff,0,0}, {128,0,0xffffffff,2,0}, {0,128,0xffffffff,0,2}}};
    std::array<UInt16,3> index_bytes{0,1,2};
    std::array<UInt8,64> red{}, green{};

    explicit Scene(BgfxGpuDevice& owner) : device(owner)
    {
        vs = device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"source viewport");
        fs = device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"sampled resource");
        check(vs && fs, "resource witness shaders unavailable");
        state.vertex_shader = vs; state.fragment_shader = fs;
        state.vertex_layout = VertexLayout::position_color_uv;
        state.color_format = TextureFormat::bgra8;
        state.raster.cull = CullMode::none;
        pipeline = device.create_pipeline(PipelineKey(state),"accepted resource program");
        uniform = device.create_buffer({sizeof(viewport),BufferUsage::uniform,true},"viewport bytes");
        vertices = device.create_buffer({sizeof(triangle),BufferUsage::vertex,true},"geometry bytes");
        indices = device.create_buffer({sizeof(index_bytes),BufferUsage::index,true},"index bytes");
        check(pipeline && uniform && vertices && indices
            && device.upload({uniform,sizeof(viewport),0,sizeof(viewport)},viewport.data())
            && device.upload({vertices,sizeof(triangle),0,sizeof(triangle)},triangle.data())
            && device.upload({indices,sizeof(index_bytes),0,sizeof(index_bytes)},index_bytes.data()), "accepted bytes unavailable");
        TextureDesc image; image.width = image.height = 4; image.mip_levels = 3;
        sample = device.create_texture(image,"accepted mip source");
        for (unsigned i = 0; i < 16; ++i) { red[i*4] = 255; red[i*4+3] = 255; green[i*4+1] = 255; green[i*4+3] = 255; }
        check(sample && device.upload_texture({sample,4,4,16,64,0},red.data())
            && device.upload_texture({sample,2,2,8,16,1},red.data())
            && device.upload_texture({sample,1,1,4,4,2},red.data()), "accepted mips unavailable");
        SamplerDesc sampling; sampling.min_filter = sampling.mag_filter = sampling.mip_filter = Filter::nearest;
        sampler = device.create_sampler(sampling,"nearest source");
        image.width = image.height = 64; image.mip_levels = 1;
        image.render_target = true; image.format = TextureFormat::bgra8;
        color = device.create_texture(image,"accepted target");
        image.format = TextureFormat::depth24_stencil8;
        depth = device.create_texture(image,"accepted depth");
        check(sampler && color && depth, "accepted targets unavailable");
    }
    std::vector<UInt8> render(unsigned mip = 0, bool refresh_vertices = true)
    {
        auto mip_triangle = triangle;
        const float scale = mip == 0 ? 1.0f : mip == 1 ? 32.0f : 64.0f;
        for (auto& vertex : mip_triangle) { vertex.u *= scale; vertex.v *= scale; }
        check(!refresh_vertices || device.upload({vertices,sizeof(mip_triangle),0,sizeof(mip_triangle)},mip_triangle.data()),
              "ordinary mip-selection vertices unavailable");
        RenderPassDesc pass; pass.color_targets[0] = color; pass.color_target_count = 1;
        pass.depth_target = depth; pass.width = pass.height = 64;
        DrawDesc draw; draw.pipeline = pipeline; draw.vertex_buffer = vertices;
        draw.index_buffer = indices; draw.index_element_size = IndexElementSize::uint16;
        draw.vertex_or_index_count = 3;
        draw.vertex_bindings.uniform_count = 1;
        draw.vertex_bindings.uniforms[0] = {uniform,0,sizeof(viewport)};
        draw.fragment_bindings.texture_count = 1;
        draw.fragment_bindings.textures[0] = sample;
        draw.fragment_bindings.samplers[0] = sampler;
        check(device.begin_pass(pass,"ordinary sampled-resource control") && device.draw(draw)
            && device.end_pass(), device.last_error().c_str());
        return device.readback_rgba(color);
    }
    void teardown()
    {
        device.destroy(pipeline); device.destroy(vs); device.destroy(fs);
        device.destroy(uniform); device.destroy(vertices); device.destroy(indices); device.destroy(sample);
        device.destroy(sampler); device.destroy(color); device.destroy(depth);
    }
};

void physical()
{
    for (unsigned generation = 0; generation < 2; ++generation) {
        BgfxOptions options; options.shader_root = ZH_BGFX_SHADER_DIR;
        BgfxGpuDevice device(options);
        check(device.supports_device_transactions(DeviceTransactionMode::idle_preparation)
            && device.supports_device_transactions(DeviceTransactionMode::frame_commands), "wrong native capability");
        // Fill the declared retirement capacity exactly, then prove a later
        // admission includes both retained tombstones and queued native owners.
        auto small = desc(); small.resources = 8; small.commands = 8;
        DeviceTransactionToken full_token;
        check(device.begin_device_transaction(small,full_token), "queue-full baseline admission failed");
        std::array<TextureHandle,8> retired{};
        for (auto& handle : retired) {
            handle = device.create_texture({4,4},"queue capacity candidate");
            check(bool(handle), "queue full candidate rejected before declared bound");
        }
        auto destroyed = device.native_retirement_destroy_count();
        auto frames = device.native_frame_advance_count();
        check(device.abort_device_transaction(full_token)
            && device.pending_native_retirement_count() == 8
            && device.native_retirement_destroy_count() == destroyed
            && device.native_frame_advance_count() == frames, "queue-full abort made native calls");
        DeviceTransactionToken unchanged{99,98,97,DeviceTransactionMode::frame_commands};
        small.resources = 15;
        check(!device.begin_device_transaction(small,unchanged) && unchanged.device == 99
            && unchanged.sequence == 98 && device.pending_native_retirement_count() == 8
            && device.native_retirement_destroy_count() == destroyed
            && device.native_frame_advance_count() == frames, "retirement bound+1 admission mutated baseline");
        small.resources = 16;
        check(device.begin_device_transaction(small,full_token), "exact retained ownership capacity rejected");
        check(!device.create_buffer({16,BufferUsage::uniform,true},"capacity plus one")
            && !device.commit_device_transaction(full_token)
            && device.abort_device_transaction(full_token)
            && device.pending_native_retirement_count() == 8, "retirement capacity did not poison before create");
        check(device.begin_device_transaction(small,full_token)
            && device.commit_device_transaction(full_token)
            && !device.commit_device_transaction(full_token)
            && device.native_retirement_destroy_count() == destroyed
            && device.native_frame_advance_count() == frames, "repeated finish drained native queue");
        device.record_marker("ordinary full-queue drain");
        check(device.pending_native_retirement_count() == 0
            && device.native_retirement_destroy_count() == destroyed+8
            && device.native_frame_advance_count() == frames, "ordinary full-queue drain differs");
        device.record_marker("idempotent empty drain");
        check(device.native_retirement_destroy_count() == destroyed+8, "native owner retired twice");
        for (const auto old : retired) check(!device.describe_texture_format(old), "aborted queue handle stayed live");
        auto reused = device.create_texture({4,4},"retired-slot ordinary retry");
        check(reused && reused != retired[0], "ordinary retry aliased aborted generation");
        device.destroy(reused);
        check(device.wait_idle() && device.live_resource_count() == 0, "queue capacity control residual");
        Scene scene(device);
        const auto accepted = scene.render();
        const auto count = device.live_resource_count();
        const auto center = (32U*64U+32U)*4U;
        check(accepted.size() == 64*64*4 && accepted[center] == 255 && accepted[center+1] == 0,
              "accepted red mip pixels differ");
        auto budget = desc(); budget.generation += generation;
        const auto finish_without_native_calls = [&](DeviceTransactionToken token, bool commit) {
            const auto destroyed = device.native_retirement_destroy_count();
            const auto frames = device.native_frame_advance_count();
            check(commit ? device.commit_device_transaction(token) : device.abort_device_transaction(token), "native finish rejected");
            check(device.native_retirement_destroy_count() == destroyed && device.native_frame_advance_count() == frames,
                  "native call reached allocation-free finish");
            check(!device.commit_device_transaction(token) && !device.abort_device_transaction(token), "native finish replayed");
        };
        DeviceTransactionToken token, sentinel{99,98,97,DeviceTransactionMode::frame_commands};
        for (bool commit : {false,true}) {
            check(device.begin_device_transaction(budget,token), "overlap preservation admission failed");
            for (unsigned category = 0; category < 8; ++category) {
                auto overlapping = budget;
                if (category == 1) overlapping.mode = DeviceTransactionMode::frame_commands;
                if (category == 2) overlapping.generation = 0;
                if (category == 3) overlapping.commands = detail::BgfxTransactionState::maximum_commands+1;
                if (category == 4) overlapping.resources = detail::BgfxTransactionState::maximum_resources+1;
                if (category == 5) overlapping.bytes = detail::BgfxTransactionState::maximum_bytes+1;
                if (category == 6) overlapping.views = 1;
                if (category == 7) overlapping.mode = static_cast<DeviceTransactionMode>(255);
                sentinel = {99,98,97,DeviceTransactionMode::frame_commands};
                check(!device.begin_device_transaction(overlapping,sentinel) && sentinel.device == 99
                    && sentinel.sequence == 98 && !device.commit_device_transaction(sentinel)
                    && !device.abort_device_transaction(sentinel), "overlap or foreign finish consumed owner");
            }
            for (unsigned field = 0; field < 4; ++field) {
                auto foreign = token;
                if (field == 0) ++foreign.device;
                if (field == 1) ++foreign.sequence;
                if (field == 2) ++foreign.generation;
                if (field == 3) foreign.mode = DeviceTransactionMode::frame_commands;
                check(!device.commit_device_transaction(foreign) && !device.abort_device_transaction(foreign),
                      "single-field foreign finish consumed owner");
            }
            device.record_marker("preserved admitted owner after nested rejection");
            const auto usable = device.create_buffer({16,BufferUsage::uniform,true},"preserved owner resource");
            check(usable && device.upload({usable,16,0,16},scene.viewport.data()), "rejected admission poisoned admitted owner");
            finish_without_native_calls(token,commit);
            if (commit) device.destroy(usable);
            else check(!device.upload({usable,16,0,16},scene.viewport.data()), "aborted owner candidate stayed live");
        }
        device.fail_next_transaction_checkpoint();
        check(!device.begin_device_transaction(budget,sentinel) && sentinel.device == 99 && sentinel.sequence == 98,
              "checkpoint failure changed output identity");
        check(device.live_resource_count() == count && scene.render() == accepted, "checkpoint failure changed pixels");
        for (UInt32 copy_boundary = 0; copy_boundary < 12; ++copy_boundary) {
            device.fail_transaction_checkpoint_copy_after(copy_boundary);
            const auto refs = device.live_owned_native_reference_count();
            const auto frames = device.native_frame_advance_count();
            sentinel = {99,98,97,DeviceTransactionMode::frame_commands};
            bool threw = false;
            try { (void)device.begin_device_transaction(budget,sentinel); }
            catch (const std::bad_alloc&) { threw = true; }
            check(threw && sentinel.device == 99 && sentinel.sequence == 98
                && device.live_resource_count() == count && device.live_owned_native_reference_count() == refs
                && device.native_frame_advance_count() == frames && !device.pass_active(),
                  "partial checkpoint copy failure changed accepted ownership");
            check(device.begin_device_transaction(budget,token) && device.abort_device_transaction(token)
                && scene.render() == accepted, "partial checkpoint copy failure prevented clean retry");
        }
        for (unsigned fault = 0; fault < 8; ++fault) {
            check(device.begin_device_transaction(budget,token), "resource fault admission failed");
            device.fail_transaction_operation_after(fault);
            auto candidate = device.create_buffer({16,BufferUsage::uniform,true},"candidate bytes");
            const std::array<UInt8,16> bytes{};
            if (candidate) (void)device.upload({candidate,16,0,16},bytes.data());
            TextureDesc image; image.width = image.height = 4;
            const auto texture = device.create_texture(image,"candidate texture");
            if (texture) (void)device.upload_texture({texture,4,4,16,64,0},scene.green.data());
            const auto sampler = device.create_sampler({},"candidate sampler");
            device.destroy(scene.pipeline);
            device.destroy(scene.vs);
            device.record_marker("candidate resource publication");
            (void)sampler;
            check(!device.commit_device_transaction(token), "injected resource attempt committed");
            finish_without_native_calls(token,false);
            check(device.live_resource_count() == count && scene.render() == accepted
                && device.create_pipeline(PipelineKey(scene.state),"identity retry") == scene.pipeline,
                "resource rollback lost accepted bytes/provider identity");
        }
        for (unsigned candidate_kind = 0; candidate_kind < 4; ++candidate_kind) {
            check(device.begin_device_transaction(budget,token), "native publication fault admission failed");
            device.fail_transaction_native_publication_after(0);
            if (candidate_kind == 0) { TextureDesc image; image.width = image.height = 4;
                check(!device.create_texture(image,"unpublished texture"), "native texture publication fault ignored"); }
            if (candidate_kind == 1)
                check(!device.upload_texture({scene.sample,4,4,16,64,0},scene.green.data()), "COW publication fault ignored");
            if (candidate_kind == 2)
                check(!device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"unpublished shader"), "shader publication fault ignored");
            if (candidate_kind == 3) { auto state = scene.state; state.depth_stencil.depth_write = false;
                check(!device.create_pipeline(PipelineKey(state),"unpublished program"), "program publication fault ignored"); }
            check(!device.commit_device_transaction(token), "native publication failure committed");
            finish_without_native_calls(token,false);
            check(device.live_resource_count() == count && scene.render() == accepted, "native publication failure leaked provider/pixels");
        }
        check(device.begin_device_transaction(budget,token), "COW abort admission failed");
        check(device.upload_texture({scene.sample,4,4,16,64,0},scene.green.data()), "staged COW upload failed");
        const std::array<Scene::Vertex,3> collapsed{};
        const std::array<UInt16,3> collapsed_indices{};
        const std::array<float,4> invalid_viewport{};
        check(device.upload({scene.vertices,sizeof(collapsed),0,sizeof(collapsed)},collapsed.data())
            && device.upload({scene.indices,sizeof(collapsed_indices),0,sizeof(collapsed_indices)},collapsed_indices.data())
            && device.upload({scene.uniform,sizeof(invalid_viewport),0,sizeof(invalid_viewport)},invalid_viewport.data()),
              "accepted-buffer rollback candidates failed");
        const auto aborted_texture = device.create_texture({4,4},"aborted identity");
        check(bool(aborted_texture), "candidate identity missing");
        finish_without_native_calls(token,false);
        check(device.pending_native_retirement_count() >= 2, "abort lost queued native ownership");
        const auto drains = device.native_retirement_destroy_count();
        device.record_marker("ordinary retirement drain");
        check(!device.pending_native_retirement_count() && device.native_retirement_destroy_count() > drains,
              "ordinary boundary did not drain retirements");
        check(scene.render(0,false) == accepted && device.describe_texture_format(scene.sample)
            && !device.describe_texture_format(aborted_texture), "COW abort changed accepted identity");
        check(device.begin_device_transaction(budget,token), "COW retry admission failed");
        const auto retry_texture = device.create_texture({4,4},"retry identity");
        check(retry_texture && retry_texture != aborted_texture, "native retry aliased candidate handle");
        device.destroy(retry_texture);
        check(device.upload_texture({scene.sample,4,4,16,64,0},scene.green.data())
            && device.upload_texture({scene.sample,2,2,8,16,1},scene.red.data()), "multiple COW retry failed");
        finish_without_native_calls(token,true);
        check(device.pending_native_retirement_count() >= 3 && device.describe_texture_format(scene.sample),
              "commit lost retired native ownership/accepted handle");
        const auto green = scene.render();
        check(green[center] == 0 && green[center+1] == 255 && device.live_resource_count() == count,
              "COW retry did not publish exact green resource");
        const auto survivor_mip_one = scene.render(1);
        const auto survivor_mip_two = scene.render(2);
        check(survivor_mip_one[center] == 255 && survivor_mip_one[center+1] == 0
            && survivor_mip_two[center] == 255 && survivor_mip_two[center+1] == 0,
            "COW changed surviving accepted mip pixels");
        check(scene.render() == green, "mip control changed accepted base image");
        for (const auto format : {TextureFormat::rgba8,TextureFormat::bgra8,TextureFormat::bgr5a1}) {
          for (const bool partial : {false,true}) {
            TextureDesc image; image.width = image.height = 4; image.mip_levels = 3; image.format = format;
            if (partial) {image.width=8;image.height=4;image.mip_levels=2;}
            const auto texture = device.create_texture(image,"padded source mip survivor");
            const unsigned pixel_bytes = format == TextureFormat::bgr5a1 ? 2 : 4;
            const auto padded = [&](unsigned width, unsigned height, bool green) {
                const auto pitch = width*pixel_bytes+4;
                std::vector<UInt8> result(pitch*height,0xa5);
                for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x) {
                    auto* pixel = result.data()+y*pitch+x*pixel_bytes;
                    if (pixel_bytes == 2) {
                        const UInt16 value = green ? 0x83e0 : 0xfc00;
                        pixel[0] = UInt8(value); pixel[1] = UInt8(value >> 8);
                    } else {
                        pixel[0] = pixel[1] = pixel[2] = 0; pixel[3] = 255;
                        pixel[green ? 1 : format == TextureFormat::bgra8 ? 2 : 0] = 255;
                    }
                }
                return result;
            };
            check(bool(texture), "padded source format unavailable");
            for (unsigned mip = 0; mip < image.mip_levels; ++mip) {
                const auto width = image.width >> mip,height=image.height >> mip;
                const auto bytes = padded(width,height,false);
                check(device.upload_texture({texture,width,height,width*pixel_bytes+4,bytes.size(),mip},bytes.data()),
                      "ordinary padded mip upload failed");
            }
            const auto original_sample = scene.sample; scene.sample = texture;
            const auto red_pixels = scene.render();
            check(red_pixels[center] == 255 && red_pixels[center+1] == 0, "padded source red pixels differ");
            const auto changed = padded(image.width,image.height,true);
            if (partial) {
                for (unsigned failure=0;failure<3;++failure) {
                    check(device.begin_device_transaction(budget,token),"partial chain fault admission");
                    if (failure==0) device.fail_transaction_native_publication_after(0);
                    else device.fail_transaction_operation_after(failure-1);
                    const auto uploaded=device.upload_texture({texture,image.width,image.height,image.width*pixel_bytes+4,changed.size(),0},changed.data());
                    if (failure==2) check(uploaded,"partial COW expected pre-marker publication");
                    else check(!uploaded,"partial COW fault was not reached");
                    device.record_marker("partial COW late operation fault");
                    check(!device.commit_device_transaction(token),"partial fault committed");
                    finish_without_native_calls(token,false);
                    check(device.live_resource_count()==count+1 && scene.render()==red_pixels
                        && scene.render(2)==red_pixels,"partial COW fault changed source identity/known mips");
                }
            }
            check(device.begin_device_transaction(budget,token)
                && device.upload_texture({texture,image.width,image.height,image.width*pixel_bytes+4,changed.size(),0},changed.data()),
                  "padded COW abort candidate failed");
            finish_without_native_calls(token,false);
            check(scene.render() == red_pixels, "padded COW abort changed accepted pixels");
            check(device.begin_device_transaction(budget,token)
                && device.upload_texture({texture,image.width,image.height,image.width*pixel_bytes+4,changed.size(),0},changed.data()),
                  "padded COW retry candidate failed");
            finish_without_native_calls(token,true);
            const auto base_pixels = scene.render();
            const auto mip_one = scene.render(1), mip_two = scene.render(2);
            check(base_pixels[center] == 0 && base_pixels[center+1] == 255
                && mip_one[center] == 255 && mip_one[center+1] == 0
                && mip_two[center] == 255 && mip_two[center+1] == 0,
                  "padded/packed COW changed surviving mip content");
            scene.sample = original_sample;
            device.destroy(texture);
            check(device.live_resource_count() == count && scene.render() == green, "format control changed surviving resource");
          }
        }
        // Complete successful publication of all five resource families, plus
        // fresh target resize without overwriting the accepted attachment.
        check(device.begin_device_transaction(budget,token), "five-family success admission failed");
        const auto candidate_vs = device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"candidate source vertex");
        const auto candidate_fs = device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"candidate source fragment");
        auto candidate_state = scene.state;
        candidate_state.vertex_shader = candidate_vs; candidate_state.fragment_shader = candidate_fs;
        const auto candidate_program = device.create_pipeline(PipelineKey(candidate_state),"candidate source program");
        const auto candidate_buffer = device.create_buffer({16,BufferUsage::uniform,true},"candidate source bytes");
        const auto candidate_sampler = device.create_sampler({},"candidate source sampler");
        const auto candidate_image = device.create_texture({4,4},"candidate source image");
        TextureDesc resized; resized.width = resized.height = 32; resized.render_target = true;
        const auto resized_target = device.create_texture(resized,"fresh resize identity");
        check(candidate_vs && candidate_fs && candidate_program && candidate_buffer && candidate_sampler
            && candidate_image && resized_target
            && device.upload({candidate_buffer,16,0,16},scene.viewport.data())
            && device.upload_texture({candidate_image,4,4,16,64,0},scene.green.data()), "five-family successful candidate failed");
        finish_without_native_calls(token,true);
        check(device.live_resource_count() == count+7 && resized_target != scene.color
            && device.describe_texture_format(scene.color) && device.describe_texture_format(resized_target)
            && device.create_pipeline(PipelineKey(candidate_state),"accepted candidate cache") == candidate_program,
              "complete publication changed identities or cache provider");
        const auto original_pipeline = scene.pipeline;
        const auto original_sample = scene.sample;
        const auto original_sampler = scene.sampler;
        scene.pipeline = candidate_program; scene.sample = candidate_image; scene.sampler = candidate_sampler;
        check(scene.render() == green, "published candidate program/texture did not render");
        scene.pipeline = original_pipeline; scene.sample = original_sample; scene.sampler = original_sampler;
        device.destroy(candidate_program); device.destroy(candidate_vs); device.destroy(candidate_fs);
        device.destroy(candidate_buffer); device.destroy(candidate_sampler); device.destroy(candidate_image);
        device.destroy(resized_target);
        check(device.live_resource_count() == count && scene.render() == green, "candidate cleanup changed surviving identity");
        // Native shader/program caches deduplicate binary/parent identities.
        // A successful create still owns one distinct increment to destroy.
        for (unsigned mode = 0; mode < 3; ++mode) {
            const auto references = device.live_owned_native_reference_count();
            const auto drain_count = device.native_retirement_destroy_count();
            check(device.begin_device_transaction(budget,token), "native alias admission failed");
            std::array<ShaderHandle,3> vertex_aliases{}, fragment_aliases{};
            std::array<PipelineHandle,3> program_aliases{};
            for (unsigned alias = 0; alias < 3; ++alias) {
                vertex_aliases[alias] = device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"native shared vertex");
                fragment_aliases[alias] = device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"native shared fragment");
                auto state = scene.state; state.vertex_shader = vertex_aliases[alias]; state.fragment_shader = fragment_aliases[alias];
                program_aliases[alias] = device.create_pipeline(PipelineKey(state),"native shared program");
                check(vertex_aliases[alias] && fragment_aliases[alias] && program_aliases[alias], "native alias creation failed");
            }
            check(device.live_owned_native_reference_count() == references+9, "native create units collapsed by handle equality");
            if (mode != 0) {
                if (mode == 2) { device.destroy(scene.pipeline); device.destroy(scene.vs); device.destroy(scene.fs); }
                for (unsigned alias = 0; alias < (mode == 1 ? 3U : 2U); ++alias) {
                    device.destroy(program_aliases[alias]);
                    device.destroy(vertex_aliases[alias]); device.destroy(fragment_aliases[alias]);
                }
            }
            finish_without_native_calls(token,mode != 0);
            check(device.pending_native_retirement_count() == 9
                && device.live_owned_native_reference_count() == references+9,
                  "shared native canceled ownership units not retained exactly");
            if (mode == 2) {
                scene.pipeline = program_aliases[2]; scene.vs = vertex_aliases[2]; scene.fs = fragment_aliases[2];
                scene.state.vertex_shader = scene.vs; scene.state.fragment_shader = scene.fs;
            }
            device.record_marker("shared-native ordinary drain");
            check(device.live_owned_native_reference_count() == references
                && device.native_retirement_destroy_count() == drain_count+9
                && !device.pending_native_retirement_count() && device.live_resource_count() == count,
                  "shared native drain leaked or retired an accepted reference");
            device.record_marker("shared-native second drain");
            check(device.native_retirement_destroy_count() == drain_count+9 && scene.render() == green,
                  "shared native alias retry lost accepted program or destroyed twice");
        }
        for (unsigned route = 0; route < 11; ++route) {
            check(device.begin_device_transaction(budget,token), "negative route admission failed");
            const auto frames = device.native_frame_advance_count();
            if (route == 0) check(!device.begin_pass({},"forbidden pass"), "idle pass admitted");
            if (route == 1) check(!device.set_viewport({}), "idle viewport admitted");
            if (route == 2) check(!device.clear_viewport({}), "idle clear admitted");
            if (route == 3) check(!device.draw({}), "idle draw admitted");
            if (route == 4) check(!device.end_pass(), "idle end admitted");
            if (route == 5) check(!device.present(scene.color), "idle present admitted");
            if (route == 6) check(!device.wait_idle(), "idle wait admitted");
            if (route == 7) check(device.readback_rgba(scene.color).empty(), "idle readback admitted");
            if (route == 8) check(!device.claim_window(nullptr), "idle window claim admitted");
            if (route == 9) device.release_window();
            if (route == 10) {
                std::vector<UInt8> target(64*64*4);
                check(!device.upload_texture({scene.color,64,64,256,target.size(),0},target.data()), "target upload admitted");
            }
            check(!device.commit_device_transaction(token) && device.native_frame_advance_count() == frames,
                  "negative route advanced or committed");
            finish_without_native_calls(token,false);
            check(device.readback_rgba(scene.color) == green, "idle rejection touched accepted target pixels");
        }
        check(device.begin_device_transaction(budget,token), "diagnostic admission failed");
        device.fail_next_transaction_diagnostic_allocation();
        bool threw = false;
        try { (void)device.create_sampler({Filter::nearest,Filter::nearest,Filter::nearest,
            AddressMode::repeat,AddressMode::repeat,AddressMode::repeat,0},"invalid diagnostic"); }
        catch (const std::bad_alloc&) { threw = true; }
        check(threw && !device.commit_device_transaction(token), "diagnostic allocation did not poison owner");
        finish_without_native_calls(token,false);
        check(device.begin_device_transaction(budget,token), "byte bound admission failed");
        check(!device.upload({scene.uniform,sizeof(scene.viewport),0,budget.bytes+1},scene.viewport.data())
            && !device.commit_device_transaction(token), "native byte bound+1 accepted");
        finish_without_native_calls(token,false);
        auto one_command = budget; one_command.commands = 1;
        check(device.begin_device_transaction(one_command,token), "command bound admission failed");
        device.record_marker("exact command bound"); device.record_marker("command plus one");
        check(!device.commit_device_transaction(token), "native command bound+1 accepted");
        finish_without_native_calls(token,false);
        check(scene.render() == green, "native bounds changed accepted pixels");
        check(device.begin_device_transaction(budget,token), "final retire admission failed");
        device.destroy(scene.pipeline); device.destroy(scene.vs); device.destroy(scene.fs);
        finish_without_native_calls(token,true);
        check(device.pending_native_retirement_count() == 3, "accepted retirement queue differs");
        check(device.wait_idle() && !device.pending_native_retirement_count(), "wait did not drain retired native resources");
        scene.pipeline = {}; scene.vs = {}; scene.fs = {};
        scene.teardown();
        check(device.wait_idle() && device.live_resource_count() == 0
            && device.live_owned_native_reference_count() == 0, "resource generation residual");
        // Teardown cancels an active attempt before the shipping shutdown path.
        check(device.begin_device_transaction(budget,token), "destruction cancellation admission failed");
        check(bool(device.create_texture({4,4},"destructor candidate")), "destruction candidate failed");
        for (unsigned alias = 0; alias < 2; ++alias) {
            const auto vs = device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"shutdown shared vertex");
            const auto fs = device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"shutdown shared fragment");
            auto state = scene.state; state.vertex_shader = vs; state.fragment_shader = fs;
            check(vs && fs && device.create_pipeline(PipelineKey(state),"shutdown shared program"),
                  "shutdown shared-native candidate failed");
        }
        check(device.live_owned_native_reference_count() == 7, "shutdown candidate ownership units differ");
    }
}
} // namespace

int main(int argc, char** argv)
{
    try {
        cpu();
        if (argc == 2 && std::string(argv[1]) == "--gpu") physical();
        std::cout << "bounded bgfx resource publication: generated controls passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
