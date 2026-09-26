#include "zh/renderer/bgfx_uniform_layout.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "usage: uniform-layout binary stage output-or---check\n";
        return 1;
    }
    const std::string stage_name = argv[2];
    if (stage_name != "vertex" && stage_name != "fragment")
        return 1;
    const auto stage =
        stage_name == "vertex" ? zh::renderer::ShaderStage::vertex : zh::renderer::ShaderStage::fragment;
    std::ifstream input(argv[1], std::ios::binary | std::ios::ate);
    if (!input || input.tellg() < 28 || input.tellg() > 4 * 1024 * 1024)
        return 1;
    std::vector<char> data(static_cast<std::size_t>(input.tellg()));
    input.seekg(0);
    input.read(data.data(), data.size());
    std::vector<zh::renderer::BgfxUniformBlockLayout> blocks;
    const auto result = zh::renderer::decode_bgfx_uniform_layout(data, stage, blocks);
    if (!input || !result) {
        std::cerr << "uniform layout admission failed\n";
        return 1;
    }
    std::string manifest_path = argv[1];
    manifest_path.resize(manifest_path.size() - 4);
    manifest_path += ".json";
    std::ifstream manifest_input(manifest_path, std::ios::binary | std::ios::ate);
    if (!manifest_input || manifest_input.tellg() <= 0 || manifest_input.tellg() > 65536)
        return 1;
    std::string manifest(static_cast<std::size_t>(manifest_input.tellg()), '\0');
    manifest_input.seekg(0);
    manifest_input.read(manifest.data(), manifest.size());
    std::map<std::string, zh::renderer::UInt32> bindings;
    if (!manifest_input || !zh::renderer::read_bgfx_uniform_bindings(manifest, bindings) ||
        !zh::renderer::associate_bgfx_uniform_bindings(blocks, bindings))
        return 1;
    const auto encoded = zh::renderer::encode_bgfx_uniform_layout(stage, blocks);
    if (std::string(argv[3]) == "--check") {
        std::string path = argv[1];
        path.resize(path.size() - 4);
        path += ".layout";
        std::ifstream sidecar(path, std::ios::binary | std::ios::ate);
        if (!sidecar || sidecar.tellg() <= 0 || sidecar.tellg() > 4096)
            return 1;
        std::string actual(static_cast<std::size_t>(sidecar.tellg()), '\0');
        sidecar.seekg(0);
        sidecar.read(actual.data(), actual.size());
        return sidecar && zh::renderer::validate_bgfx_uniform_layout_sidecar(actual, stage, blocks) ? 0
                                                                                                    : 1;
    }
    std::ofstream output(argv[3], std::ios::binary | std::ios::trunc);
    output << encoded;
    return output ? 0 : 1;
}
