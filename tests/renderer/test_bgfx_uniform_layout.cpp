#include "zh/platform/bgfx_device.h"
#include "zh/renderer/bgfx_uniform_layout.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
using namespace zh::renderer;
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
std::vector<char> read(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    check(bool(input), "generated fixture input missing");
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void write(const std::filesystem::path &path, const std::vector<char> &bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), bytes.size());
    check(bool(output), "generated fixture output failed");
}
UInt32 word(const std::vector<char> &bytes, std::size_t offset) {
    check(offset + 4 <= bytes.size(), "fixture word outside envelope");
    UInt32 value = 0;
    for (unsigned i = 0; i < 4; ++i)
        value |= UInt32(UInt8(bytes[offset + i])) << (8 * i);
    return value;
}
void put(std::vector<char> &bytes, std::size_t offset, UInt32 value) {
    check(offset + 4 <= bytes.size(), "fixture word outside envelope");
    for (unsigned i = 0; i < 4; ++i)
        bytes[offset + i] = char(value >> (8 * i));
}
std::size_t payload(const std::vector<char> &bytes) {
    auto position = std::size_t{22};
    const auto count = UInt8(bytes[20]) | (UInt32(UInt8(bytes[21])) << 8);
    for (UInt32 i = 0; i < count; ++i)
        position += 1U + UInt8(bytes[position]) + 10U;
    return position + 4;
}
std::vector<BgfxUniformBlockLayout> layout(const std::filesystem::path &root, const char *family,
                                           ShaderStage stage) {
    std::vector<BgfxUniformBlockLayout> result;
    check(decode_bgfx_uniform_layout(read(root / (std::string(family) + ".bin")), stage, result),
          "compiler layout rejected");
    const auto manifest = read(root / (std::string(family) + ".json"));
    std::map<std::string, UInt32> bindings;
    check(read_bgfx_uniform_bindings({manifest.data(), manifest.size()}, bindings) &&
              associate_bgfx_uniform_bindings(result, bindings),
          "compiler manifest binding rejected");
    const auto sidecar = read(root / (std::string(family) + ".layout"));
    check(validate_bgfx_uniform_layout_sidecar({sidecar.data(), sidecar.size()}, stage, result),
          "compiler sidecar rejected");
    return result;
}
std::vector<char> modeled(unsigned blocks, unsigned depth = 0, bool overlap = false,
                          bool duplicate = false, bool cycle = false, UInt32 array_count = 1,
                          bool duplicate_field = false, unsigned field_count = 1,
                          bool reflection = false, unsigned extra_root = 0) {
    if (duplicate_field)
        field_count = 2;
    std::vector<UInt32> words{0x07230203, 0x00010000, 0, 1000, 0};
    const auto emit = [&](unsigned op, std::initializer_list<UInt32> values) {
        words.push_back((UInt32(values.size() + 1) << 16) | op);
        words.insert(words.end(), values);
    };
    const auto named = [&](unsigned op, std::initializer_list<UInt32> values, const std::string &text) {
        const auto at = words.size();
        words.push_back(0);
        words.insert(words.end(), values);
        for (std::size_t i = 0; i <= text.size(); i += 4) {
            UInt32 packed = 0;
            for (unsigned j = 0; j < 4 && i + j < text.size(); ++j)
                packed |= UInt32(UInt8(text[i + j])) << (j * 8);
            words.push_back(packed);
        }
        words[at] = (UInt32(words.size() - at) << 16) | op;
    };
    named(15, {4, 8}, "main");
    named(5, {7}, "zh_uniforms");
    if (extra_root)
        named(5, {10}, extra_root == 1 ? "zh_uniforms" : "foreign_root");
    for (unsigned i = 0; i < field_count; ++i)
        named(6, {4, i}, i == 0 || duplicate_field ? "live" : "field_" + std::to_string(i));
    for (unsigned i = 0; i < blocks; ++i)
        named(6, {5, i}, "block_" + std::to_string(duplicate && i == 1 ? 0 : i));
    emit(22, {1, 32});
    emit(23, {2, 1, 4});
    emit(21, {9, 32, 0});
    emit(43, {9, 3, array_count});
    UInt32 field_type = 2;
    for (unsigned i = 0; i < depth; ++i) {
        emit(28, {20 + i, cycle && i == 0 ? 20 : field_type, 3});
        emit(71, {20 + i, 6, 16});
        field_type = 20 + i;
    }
    words.push_back((UInt32(field_count + 2) << 16) | 30);
    words.push_back(4);
    for (unsigned i = 0; i < field_count; ++i)
        words.push_back(field_type);
    const auto root_at = words.size();
    words.push_back((UInt32(blocks + 2) << 16) | 30);
    words.push_back(5);
    for (unsigned i = 0; i < blocks; ++i)
        words.push_back(4);
    check(words.size() - root_at == blocks + 2, "modeled root capacity broken");
    emit(32, {6, 2, 5});
    emit(59, {6, 7, 2});
    if (extra_root)
        emit(59, {6, 10, 2});
    emit(71, {5, 2});
    emit(71, {7, 34, 0});
    emit(71, {7, 33, 1});
    if (extra_root) {
        emit(71, {10, 34, 0});
        emit(71, {10, 33, 1});
    }
    for (unsigned i = 0; i < field_count; ++i)
        emit(72, {4, i, 35, i * 16U});
    for (unsigned i = 0; i < blocks; ++i)
        emit(72, {5, i, 35, overlap && i == 1 ? 0 : i * field_count * 16U});
    std::vector<char> result(22, 0);
    std::memcpy(result.data(), "FSH", 3);
    result[3] = 12;
    if (reflection) {
        result[20] = char(blocks * field_count);
        result[21] = char((blocks * field_count) >> 8);
        for (unsigned block = 0; block < blocks; ++block)
            for (unsigned field = 0; field < field_count; ++field) {
                const auto path = "ZhStageUniforms.block_" + std::to_string(block) + "." +
                                  (field == 0 ? "live" : "field_" + std::to_string(field));
                result.push_back(char(path.size()));
                result.insert(result.end(), path.begin(), path.end());
                const auto offset = (block * field_count + field) * 16;
                result.insert(result.end(), {18, 1, char(offset), char(offset >> 8), 1, 0, 0, 0, 0, 0});
            }
    }
    const auto begin = result.size();
    result.resize(begin + 4 + words.size() * 4 + 4, 0);
    put(result, begin, UInt32(words.size() * 4));
    for (unsigned i = 0; i < words.size(); ++i)
        put(result, begin + 4 + i * 4, words[i]);
    return result;
}
void cpu() {
    const std::filesystem::path root = ZH_BGFX_SHADER_DIR;
    for (unsigned generation = 0; generation < 2; ++generation) {
        const auto fragment = layout(root, "renderer/original_applied_3.frag", ShaderStage::fragment);
        check(fragment.size() == 1 && fragment[0].origin == 0 && fragment[0].extent == 224,
              "dead-prefix source origin compacted");
        const auto ops = std::find_if(fragment[0].fields.begin(), fragment[0].fields.end(),
                                      [](const auto &field) { return field.name == "stage_ops"; });
        check(ops != fragment[0].fields.end() && ops->offset == 128 && ops->count == 2 &&
                  ops->extent == 32,
              "integer array source bytes repacked");
        const auto vertex = layout(root, "renderer/uniform_origin_probe.vert", ShaderStage::vertex);
        check(vertex.size() == 2 && vertex[0].origin == 0 && vertex[0].extent == 160 &&
                  vertex[1].origin == 160 && vertex[1].source_binding == 2 && vertex[1].extent == 32,
              "nonzero later block origin/binding lost");
        const auto composite = std::find_if(vertex[0].fields.begin(), vertex[0].fields.end(),
                                            [](const auto &field) { return field.name == "composite"; });
        check(composite != vertex[0].fields.end() && composite->offset == 16 &&
                  composite->extent == 64 && composite->kind == 4,
              "matrix/dead-prefix ABI lost");
        const auto empty = layout(root, "renderer/acceptance.vert", ShaderStage::vertex);
        check(empty.empty(), "resource-free shader gained uniform fields");
        const auto dead = layout(root, "effects/post_effect.vert", ShaderStage::vertex);
        check(dead.empty(), "optimized-out whole source block gained runtime reads");
        const auto canonical = encode_bgfx_uniform_layout(ShaderStage::vertex, vertex);
        for (const auto &bad :
             {std::string{}, canonical.substr(0, canonical.size() - 1),
              canonical + "block duplicate 0 0 16\n", std::string("zh-bgfx-uniform-layout 2 vertex 0\n"),
              std::string(4097, 'x')})
            check(!validate_bgfx_uniform_layout_sidecar(bad, ShaderStage::vertex, vertex),
                  "malformed sidecar admitted");
        check(!validate_bgfx_uniform_layout_sidecar(canonical, ShaderStage::fragment, vertex),
              "stage mismatch admitted");
        auto bindings = std::map<std::string, UInt32>{{"frame_data", 0}, {"object_data", 0}};
        auto candidate = vertex;
        check(!associate_bgfx_uniform_bindings(candidate, bindings) && candidate[1].source_binding == 2,
              "duplicate binding changed accepted layout");
        bindings = {{"unknown", 0}, {"object_data", 2}};
        check(!associate_bgfx_uniform_bindings(candidate, bindings), "unknown block admitted");
        bindings = {{"frame_data", 0}, {"object_data", 4}};
        check(!associate_bgfx_uniform_bindings(candidate, bindings), "capacity overflow admitted");
        for (const char *bad_manifest :
             {"{\"instance\":\"a\",\"source_binding\":5}",
              "{\"instance\":\"a\",\"source_binding\":999999999999999999999}",
              "{\"instance\":\"a\",\"source_binding\":0,\"instance\":\"a\",\"source_binding\":1}",
              "{\"instance\":\"bad name\",\"source_binding\":0}",
              "{\"instance\":\"a\",\"source_binding\":-1}"}) {
            auto parsed = bindings;
            check(!read_bgfx_uniform_bindings(bad_manifest, parsed) && parsed == bindings,
                  "malformed binding parser published partial map");
        }
        const auto original = read(root / "renderer/original_applied_3.frag.bin");
        const auto reject = [&](const std::vector<char> &bytes,
                                ShaderStage stage = ShaderStage::fragment) {
            auto output = vertex;
            check(!decode_bgfx_uniform_layout(bytes, stage, output) && output.size() == 2 &&
                      output[1].origin == 160 && output[1].source_binding == 2,
                  "bad binary published partial layout");
        };
        reject({});
        reject(std::vector<char>(original.begin(), original.end() - 1));
        reject(std::vector<char>(4 * 1024 * 1024 + 1, 'x'));
        reject(original, ShaderStage::vertex);
        auto bad = original;
        bad[20] = 1;
        bad[21] = 4;
        reject(bad); // 1025 reflected fields.
        bad = original;
        const auto reflection = 23U + UInt8(bad[22]);
        bad[reflection + 2] = 0;
        bad[reflection + 3] = 0;
        reject(bad); // Absolute underflow.
        bad = original;
        bad[reflection + 2] = char(255);
        bad[reflection + 3] = char(255);
        reject(bad);
        bad = original;
        bad[reflection + 1] = 3;
        reject(bad); // Array count mismatch.
        bad = original;
        bad[reflection + 4] = 3;
        reject(bad); // Register extent mismatch.
        bad = original;
        bad[reflection] = 19;
        reject(bad); // Matrix/vector mismatch.
        bad = original;
        bad[23 + 16] = 'z';
        reject(bad); // Unknown source block.
        bad = original;
        put(bad, payload(bad) + 12, 65537);
        reject(bad);
        bad = original;
        put(bad, payload(bad) + 20, 0);
        reject(bad); // Zero instruction length.
        bad = original;
        for (auto at = payload(bad) + 20; at < payload(bad) + word(bad, payload(bad) - 4);) {
            const auto header = word(bad, at), op = header & 65535, count = header >> 16;
            if (op == 71 && count == 4 && word(bad, at + 8) == 34) {
                put(bad, at + 12, 2);
                break;
            }
            at += count * 4;
        }
        reject(bad);
        bad = original;
        bad.push_back(0);
        reject(bad); // Trailing envelope data.
        std::vector<BgfxUniformBlockLayout> maximum;
        check(decode_bgfx_uniform_layout(modeled(4), ShaderStage::fragment, maximum) &&
                  maximum.size() == 4 && maximum[3].origin == 48,
              "four-block capacity rejected");
        reject(modeled(5));
        reject(modeled(2, 0, true));
        reject(modeled(2, 0, false, true));
        reject(modeled(1, 33));
        reject(modeled(1, 1, false, false, true));
        check(decode_bgfx_uniform_layout(modeled(1, 29), ShaderStage::fragment, maximum),
              "bounded depth maximum rejected");
        check(decode_bgfx_uniform_layout(modeled(1, 1, false, false, false, 4096), ShaderStage::fragment,
                                         maximum) &&
                  maximum[0].extent == 65536,
              "bounded array/extent maximum rejected");
        reject(modeled(1, 1, false, false, false, 4097));
        reject(modeled(1, 1, false, false, false, UINT32_MAX));
        reject(modeled(1, 0, false, false, false, 1, true));
        check(decode_bgfx_uniform_layout(modeled(4, 0, false, false, false, 1, false, 256, true),
                                         ShaderStage::fragment, maximum) &&
                  maximum.size() == 4 && maximum[3].fields.size() == 256,
              "1024-reflection/256-field maximum rejected");
        reject(modeled(1, 0, false, false, false, 1, false, 257));
        auto overflow = modeled(1);
        put(overflow, overflow.size() - 8, UINT32_MAX - 15);
        reject(overflow);
        auto duplicate_offset = modeled(1);
        const auto bytes = word(duplicate_offset, 22);
        const auto end = 26U + bytes;
        duplicate_offset.insert(duplicate_offset.begin() + end, 20, 0);
        put(duplicate_offset, 22, bytes + 20);
        put(duplicate_offset, end, (5U << 16) | 72);
        put(duplicate_offset, end + 4, 4);
        put(duplicate_offset, end + 8, 0);
        put(duplicate_offset, end + 12, 35);
        put(duplicate_offset, end + 16, 0);
        reject(duplicate_offset);
        auto foreign_root = modeled(1);
        const std::string root_name = "zh_uniforms";
        const auto root_at =
            std::search(foreign_root.begin(), foreign_root.end(), root_name.begin(), root_name.end());
        check(root_at != foreign_root.end(), "modeled root name missing");
        *root_at = 'x';
        reject(foreign_root); // A foreign UBO is not an optimized-out source UBO.
        reject(modeled(1, 0, false, false, false, 1, false, 1, false, 1)); // Duplicate known root.
        reject(modeled(1, 0, false, false, false, 1, false, 1, false, 2)); // Known then foreign.
        auto foreign_then_known = modeled(1, 0, false, false, false, 1, false, 1, false, 1);
        const auto first_root = std::search(foreign_then_known.begin(), foreign_then_known.end(),
                                            root_name.begin(), root_name.end());
        check(first_root != foreign_then_known.end(), "mixed modeled root name missing");
        *first_root = 'x';
        reject(foreign_then_known);
    }
}

struct TempRoot {
    std::filesystem::path path;
    TempRoot() {
        std::string pattern =
            (std::filesystem::temp_directory_path() / "zh-uniform-origin-XXXXXX").string();
        std::vector<char> name(pattern.begin(), pattern.end());
        name.push_back(0);
        const auto created = mkdtemp(name.data());
        check(created, "temporary fixture root failed");
        path = created;
        std::filesystem::create_directory(path / "renderer");
        for (const char *family : {"uniform_origin_probe.vert", "uniform_origin_probe.frag",
                                   "original_applied_d1.vert", "original_applied_3.frag"})
            for (const char *extension : {".bin", ".json", ".layout"})
                std::filesystem::copy_file(std::filesystem::path(ZH_BGFX_SHADER_DIR) / "renderer" /
                                               (std::string(family) + extension),
                                           path / "renderer" / (std::string(family) + extension));
    }
    ~TempRoot() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};
template <class T> BufferHandle buffer(BgfxGpuDevice &device, const T &value, BufferUsage usage) {
    auto handle = device.create_buffer({sizeof(value), usage, true}, "exact source bytes");
    check(handle && device.upload({handle, sizeof(value), 0, sizeof(value)}, &value),
          "exact source upload failed");
    return handle;
}
struct Vertex {
    float xyz[3];
    UInt32 diffuse;
    float uv[2];
};
static_assert(sizeof(Vertex) == 24);
PipelineDesc pipeline(ShaderHandle vs, ShaderHandle fs) {
    PipelineDesc state;
    state.vertex_shader = vs;
    state.fragment_shader = fs;
    state.vertex_layout = VertexLayout::original_fvf;
    state.original_fvf.stride = sizeof(Vertex);
    state.original_fvf.attribute_count = 3;
    state.original_fvf.attributes[0] = {0, VertexElementFormat::float3, 0};
    state.original_fvf.attributes[1] = {2, VertexElementFormat::ubyte4_norm, 12};
    state.original_fvf.attributes[2] = {4, VertexElementFormat::float2, 16};
    state.color_format = TextureFormat::bgra8;
    state.raster.cull = CullMode::none;
    return state;
}
void physical() {
    for (unsigned generation = 0; generation < 2; ++generation) {
        TempRoot root;
        BgfxOptions options;
        options.shader_root = root.path;
        BgfxGpuDevice device(options);
        const auto vs = device.create_shader(
            {ShaderStage::vertex, "renderer/uniform_origin_probe.vert", 3, 0}, "origin vertex");
        const auto fs = device.create_shader(
            {ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1}, "origin fragment");
        check(vs && fs, device.last_error().c_str());
        const auto program =
            device.create_pipeline(PipelineKey(pipeline(vs, fs)), "origin matrix/multiblock program");
        check(bool(program), device.last_error().c_str());
        struct Frame {
            std::array<float, 4> dead{};
            std::array<float, 16> matrix{};
            std::array<float, 8> hole{};
            std::array<Int32, 4> enabled{};
            std::array<float, 8> uv{};
        } frame;
        static_assert(sizeof(Frame) == 160);
        static_assert(offsetof(Frame, matrix) == 16 && offsetof(Frame, enabled) == 112 &&
                      offsetof(Frame, uv) == 128);
        frame.matrix[0] = frame.matrix[5] = frame.matrix[10] = frame.matrix[15] = 1;
        frame.enabled[0] = 1;
        frame.uv[4] = 0.625f;
        frame.uv[5] = 0.125f;
        struct Pair {
            std::array<float, 4> dead{};
            std::array<float, 4> live{};
        } object, pass;
        object.live[3] = 0;
        pass.live = {0.5f, 1, 0.25f, 1};
        struct Color {
            std::array<float, 4> dead{}, tint{}, hole{};
            std::array<Int32, 4> enabled{};
        } color;
        static_assert(sizeof(Pair) == 32 && offsetof(Pair, live) == 16);
        static_assert(sizeof(Color) == 64 && offsetof(Color, tint) == 16 &&
                      offsetof(Color, enabled) == 48);
        color.tint = {1, 0.5f, 1, 0.5f};
        color.enabled[0] = 1;
        const auto frame_buffer = buffer(device, frame, BufferUsage::uniform);
        const auto object_buffer = buffer(device, object, BufferUsage::uniform);
        const auto color_buffer = buffer(device, color, BufferUsage::uniform);
        const auto pass_buffer = buffer(device, pass, BufferUsage::uniform);
        const std::array<Vertex, 3> vertices{{{{-.8f, -.8f, .5f}, 0xffffffff, {0, 0}},
                                              {{.8f, -.8f, .5f}, 0xffffffff, {0, 0}},
                                              {{0, .8f, .5f}, 0xffffffff, {0, 0}}}};
        const auto vb = buffer(device, vertices, BufferUsage::vertex);
        TextureDesc image;
        image.width = image.height = 4;
        image.format = TextureFormat::rgba8;
        const auto texture = device.create_texture(image, "nontrivial atlas quadrant");
        std::array<UInt8, 64> atlas{};
        for (unsigned y = 0; y < 4; ++y)
            for (unsigned x = 0; x < 4; ++x) {
                const auto at = (y * 4 + x) * 4;
                atlas[at] = x >= 2 && y < 2 ? 200 : 20;
                atlas[at + 1] = x >= 2 && y < 2 ? 100 : 10;
                atlas[at + 2] = x >= 2 && y < 2 ? 80 : 4;
                atlas[at + 3] = 128;
            }
        check(texture && device.upload_texture({texture, 4, 4, 16, 64}, atlas.data()),
              "atlas upload failed");
        SamplerDesc sampler_desc;
        sampler_desc.min_filter = sampler_desc.mag_filter = Filter::nearest;
        const auto sampler = device.create_sampler(sampler_desc, "exact nearest sample");
        image.width = image.height = 64;
        image.sampled = false;
        image.render_target = true;
        image.format = TextureFormat::bgra8;
        const auto target = device.create_texture(image, "accepted pixel target");
        image.format = TextureFormat::depth24_stencil8;
        const auto depth = device.create_texture(image, "accepted depth target");
        RenderPassDesc render;
        render.color_targets[0] = target;
        render.color_target_count = 1;
        render.depth_target = depth;
        render.width = render.height = 64;
        render.clear_color = {0, 0, 0, 1};
        DrawDesc draw;
        draw.pipeline = program;
        draw.vertex_buffer = vb;
        draw.vertex_or_index_count = 3;
        draw.vertex_bindings.uniform_count = 3;
        draw.vertex_bindings.uniforms[0] = {frame_buffer, 0, sizeof(frame)};
        draw.vertex_bindings.uniforms[1] = draw.vertex_bindings.uniforms[0];
        draw.vertex_bindings.uniforms[2] = {object_buffer, 0, sizeof(object)};
        draw.fragment_bindings.uniform_count = 4;
        for (unsigned i = 0; i < 3; ++i)
            draw.fragment_bindings.uniforms[i] = {color_buffer, 0, sizeof(color)};
        draw.fragment_bindings.uniforms[3] = {pass_buffer, 0, sizeof(pass)};
        draw.fragment_bindings.texture_count = 1;
        draw.fragment_bindings.textures[0] = texture;
        draw.fragment_bindings.samplers[0] = sampler;
        const auto execute = [&] {
            check(device.begin_pass(render, "exact-origin physical witness"), "physical begin failed");
            check(device.draw(draw), device.last_error().c_str());
            check(device.end_pass(), "physical end failed");
            return device.readback_rgba(target);
        };
        const auto pixels = execute();
        const auto center = (32U * 64U + 32U) * 4;
        check(pixels.size() == 64 * 64 * 4 && pixels[center] == 100 && pixels[center + 1] == 50 &&
                  pixels[center + 2] == 20 && pixels[center + 3] == 64,
              "dead-prefix/hole/int/matrix/array/multiblock pixels differ");
        const auto accepted_count = device.live_resource_count();
        const auto sidecar_path = root.path / "renderer/uniform_origin_probe.frag.layout";
        const auto sidecar = read(sidecar_path);
        for (const auto &bad :
             {std::vector<char>{}, std::vector<char>(sidecar.begin(), sidecar.end() - 1),
              std::vector<char>(4097, 'x')}) {
            write(sidecar_path, bad);
            check(!device.create_shader(
                      {ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1},
                      "rejected candidate") &&
                      device.live_resource_count() == accepted_count &&
                      device.readback_rgba(target) == pixels,
                  "metadata rejection changed accepted resource/target identity");
        }
        write(sidecar_path, sidecar);
        const auto retry = device.create_shader(
            {ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1}, "clean metadata retry");
        check(retry && retry != fs && execute() == pixels, "metadata clean retry lost accepted program");
        device.destroy(retry);
        std::filesystem::rename(sidecar_path, sidecar_path.string() + ".saved");
        check(!device.create_shader({ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1},
                                    "missing sidecar") &&
                  device.live_resource_count() == accepted_count &&
                  device.readback_rgba(target) == pixels,
              "missing metadata changed accepted state");
        std::filesystem::rename(sidecar_path.string() + ".saved", sidecar_path);
        check(!device.create_shader({ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 3, 1},
                                    "wrong source count") &&
                  device.live_resource_count() == accepted_count,
              "source-binding mismatch published native resource");
        const auto manifest_path = root.path / "renderer/uniform_origin_probe.frag.json";
        const auto manifest = read(manifest_path);
        for (const auto &replacement : std::array<std::pair<const char *, const char *>, 4>{
                 {{"\"source_binding\":3", "\"source_binding\":2"},
                  {"\"source_binding\":3", "\"source_binding\":0"},
                  {"\"instance\":\"pass_data\"", "\"instance\":\"color_data\""},
                  {"\"stage\":\"fragment\"", "\"stage\":\"vertex\""}}}) {
            std::string stale(manifest.begin(), manifest.end());
            const auto at = stale.find(replacement.first);
            check(at != std::string::npos, "manifest fixture token absent");
            stale.replace(at, std::strlen(replacement.first), replacement.second);
            write(manifest_path, {stale.begin(), stale.end()});
            check(!device.create_shader(
                      {ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1},
                      "stale manifest") &&
                      device.live_resource_count() == accepted_count &&
                      device.readback_rgba(target) == pixels,
                  "stale/duplicate binding/stage metadata changed accepted state");
        }
        write(manifest_path, manifest);
        const auto binary_path = root.path / "renderer/uniform_origin_probe.frag.bin";
        const auto binary = read(binary_path);
        auto malformed = binary;
        for (std::size_t cursor = 22; cursor < payload(binary) - 4;) {
            const auto length = UInt8(binary[cursor++]);
            const std::string name(binary.data() + cursor, length);
            cursor += length;
            if (name.rfind("ZhStageUniforms.", 0) == 0) {
                malformed[cursor + 2] = char(255);
                malformed[cursor + 3] = char(255);
                break;
            }
            cursor += 10;
        }
        write(binary_path, malformed);
        check(!device.create_shader({ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1},
                                    "forged reflected range") &&
                  device.live_resource_count() == accepted_count &&
                  device.readback_rgba(target) == pixels,
              "forged reflected field changed accepted state");
        write(binary_path, binary);
        auto foreign_binary = binary;
        const std::string root_identifier = "zh_uniforms";
        const auto foreign_at = std::search(foreign_binary.begin(), foreign_binary.end(),
                                            root_identifier.begin(), root_identifier.end());
        check(foreign_at != foreign_binary.end(), "physical Uniform provider name absent");
        *foreign_at = 'x'; // Debug-name-only mutation: the actual live UBO remains present.
        write(binary_path, foreign_binary);
        const auto empty_sidecar = encode_bgfx_uniform_layout(ShaderStage::fragment, {});
        write(sidecar_path, {empty_sidecar.begin(), empty_sidecar.end()});
        check(!device.create_shader({ShaderStage::fragment, "renderer/uniform_origin_probe.frag", 4, 1},
                                    "foreign live Uniform provider") &&
                  device.live_resource_count() == accepted_count &&
                  device.readback_rgba(target) == pixels,
              "foreign live Uniform provider admitted as optimized-out absence");
        write(binary_path, binary);
        write(sidecar_path, sidecar);

        // Exact original source UBO byte offsets: no source shader or caller ABI
        // is repacked to make this control pass.
        const auto original_vs = device.create_shader(
            {ShaderStage::vertex, "renderer/original_applied_d1.vert", 1, 0}, "original matrix rows");
        const auto original_fs =
            device.create_shader({ShaderStage::fragment, "renderer/original_applied_3.frag", 1, 2},
                                 "original dead-prefix stages");
        check(original_vs && original_fs, device.last_error().c_str());
        const auto original_program = device.create_pipeline(
            PipelineKey(pipeline(original_vs, original_fs)), "original exact fragment bytes");
        check(bool(original_program), device.last_error().c_str());
        std::array<UInt32, 200> transform_words{};
        const auto set_floats = [](auto &words, unsigned byte, const std::array<float, 4> &values) {
            check(byte + sizeof(values) <= sizeof(words), "fixture source ABI overflow");
            std::memcpy(reinterpret_cast<char *>(words.data()) + byte, values.data(), sizeof(values));
        };
        for (unsigned matrix_byte : {0U, 64U, 128U, 192U, 256U})
            for (unsigned row = 0; row < 4; ++row) {
                std::array<float, 4> values{};
                values[row] = 1;
                set_floats(transform_words, matrix_byte + row * 16, values);
            }
        const auto transform_buffer = buffer(device, transform_words, BufferUsage::uniform);
        std::array<UInt32, 56> fragment_words{};
        set_floats(fragment_words, 64, {1, 0, 0, 1});
        set_floats(fragment_words, 80, {0, 1, 0, 0});
        fragment_words[32] = fragment_words[33] = fragment_words[34] =
            1; // stage0 select texture RGB/alpha.
        fragment_words[36] = fragment_words[37] = 3;
        fragment_words[38] = 1; // stage1 modulate white.
        fragment_words[40] = fragment_words[42] = 2;
        fragment_words[44] = fragment_words[46] = 1;
        fragment_words[45] = fragment_words[47] = 2;
        const auto fragment_buffer = buffer(device, fragment_words, BufferUsage::uniform);
        auto original_vertices = vertices;
        for (auto &vertex : original_vertices) {
            vertex.uv[0] = .625f;
            vertex.uv[1] = .125f;
        }
        const auto original_vb = buffer(device, original_vertices, BufferUsage::vertex);
        TextureDesc white_desc;
        white_desc.width = white_desc.height = 1;
        white_desc.format = TextureFormat::rgba8;
        const auto white = device.create_texture(white_desc, "second-stage white");
        const std::array<UInt8, 4> white_pixel{255, 255, 255, 255};
        check(white && device.upload_texture({white, 1, 1, 4, 4}, white_pixel.data()),
              "second stage upload failed");
        draw.pipeline = original_program;
        draw.vertex_buffer = original_vb;
        draw.vertex_bindings = {};
        draw.vertex_bindings.uniform_count = 1;
        draw.vertex_bindings.uniforms[0] = {transform_buffer, 0, sizeof(transform_words)};
        draw.fragment_bindings = {};
        draw.fragment_bindings.uniform_count = 1;
        draw.fragment_bindings.uniforms[0] = {fragment_buffer, 0, sizeof(fragment_words)};
        draw.fragment_bindings.texture_count = 2;
        draw.fragment_bindings.textures[0] = texture;
        draw.fragment_bindings.textures[1] = white;
        draw.fragment_bindings.samplers[0] = draw.fragment_bindings.samplers[1] = sampler;
        const auto original_pixels = execute();
        check(original_pixels[center] == 200 && original_pixels[center + 1] == 100 &&
                  original_pixels[center + 2] == 80 && original_pixels[center + 3] == 128,
              "original texture quadrant/stage integer bytes differ");
        set_floats(fragment_words, 96, {1, 4, .6f, 0});
        check(device.upload({fragment_buffer, sizeof(fragment_words), 0, sizeof(fragment_words)},
                            fragment_words.data()),
              "alpha payload upload failed");
        const auto rejected_alpha = execute();
        check(rejected_alpha[center] == 0 && rejected_alpha[center + 1] == 0 &&
                  rejected_alpha[center + 2] == 0 && rejected_alpha[center + 3] == 255,
              "original alpha-test source bytes did not reject");
        set_floats(fragment_words, 96, {1, 4, .4f, 0});
        set_floats(fragment_words, 80, {0, 1, 1, 0});
        check(device.upload({fragment_buffer, sizeof(fragment_words), 0, sizeof(fragment_words)},
                            fragment_words.data()),
              "fog payload upload failed");
        const auto fog = execute();
        check(fog[center] >= 227 && fog[center] <= 228 && fog[center + 1] == 50 &&
                  fog[center + 2] == 40 && fog[center + 3] == 128,
              "original fog/alpha source byte range differs");
        device.destroy(original_program);
        device.destroy(original_vs);
        device.destroy(original_fs);
        device.destroy(original_vb);
        device.destroy(transform_buffer);
        device.destroy(fragment_buffer);
        device.destroy(white);
        device.destroy(program);
        device.destroy(fs);
        device.destroy(vs);
        device.destroy(vb);
        device.destroy(frame_buffer);
        device.destroy(object_buffer);
        device.destroy(color_buffer);
        device.destroy(pass_buffer);
        device.destroy(texture);
        device.destroy(sampler);
        device.destroy(target);
        device.destroy(depth);
        check(device.wait_idle() && device.live_resource_count() == 0,
              "origin generation resource residual");
    }
}
} // namespace
int main(int argc, char **argv) {
    try {
        cpu();
        if (argc == 2 && std::string(argv[1]) == "--gpu")
            physical();
        std::cout << "exact uniform origins: admitted generated controls\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
