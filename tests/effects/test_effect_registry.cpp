#include "zh/effects/registry.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace zh;

namespace {

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

std::string read_file(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open " + path.string());
    std::ostringstream text;
    text << input.rdbuf();
    return text.str();
}

void test_canonical_mapping_and_compiled_sources()
{
    const auto registry = effects::effect_registry();
    check(effects::validate_effect_registry(registry), "canonical effects registry rejected");
    check(registry.size == static_cast<std::size_t>(effects::EffectKind::count), "effects registry size changed");
    for (std::size_t index = 0; index < registry.size; ++index) {
        const auto& entry = registry.entries[index];
        const auto vertex_source = std::filesystem::path(ZH_SOURCE_DIR) / "shaders" / entry.vertex_source;
        const auto fragment_source = std::filesystem::path(ZH_SOURCE_DIR) / "shaders" / entry.fragment_source;
        const auto vertex_output = std::filesystem::path(ZH_BINARY_DIR) / "generated/shaders" / (std::string(entry.vertex_source) + ".spv");
        const auto fragment_output = std::filesystem::path(ZH_BINARY_DIR) / "generated/shaders" / (std::string(entry.fragment_source) + ".spv");
        check(std::filesystem::is_regular_file(vertex_source), "mapped vertex source is missing");
        check(std::filesystem::is_regular_file(fragment_source), "mapped fragment source is missing");
        check(std::filesystem::is_regular_file(vertex_output), "mapped vertex SPIR-V is missing");
        check(std::filesystem::is_regular_file(fragment_output), "mapped fragment SPIR-V is missing");
    }
    const auto& points = registry.entries[static_cast<std::size_t>(effects::EffectKind::particle_points)];
    check(points.point_size && points.premultiplied_alpha && points.source_blend == renderer::BlendFactor::one,
        "point-size/premultiplied particle mapping changed");
    const auto& projected = registry.entries[static_cast<std::size_t>(effects::EffectKind::projected_texture)];
    check(projected.depth_bias != 0.0F, "projected depth bias was lost");
    const auto& multipass = registry.entries[static_cast<std::size_t>(effects::EffectKind::wwshade_multipass)];
    check(multipass.pass_count == 2, "WWShade multipass ordering contract changed");
    check(registry.entries[static_cast<std::size_t>(effects::EffectKind::water)].render_target_input
        && registry.entries[static_cast<std::size_t>(effects::EffectKind::stealth)].render_target_input
        && registry.entries[static_cast<std::size_t>(effects::EffectKind::post_effect)].render_target_input,
        "render-target effect dependency changed");
}

void test_logical_corpus_completeness()
{
    effects::EffectCorpusCoverage corpus;
    const auto text = read_file(std::filesystem::path(ZH_SOURCE_DIR) / "data/corpus/effects-manifest.tsv");
    check(effects::parse_effect_corpus(text, effects::effect_registry(), corpus), "canonical effects corpus rejected");
    check(corpus.entries.size() == effects::effect_registry().size, "effects corpus is incomplete");

    auto incomplete = text;
    incomplete.erase(incomplete.rfind("effect\t"));
    check(!effects::parse_effect_corpus(incomplete, effects::effect_registry(), corpus), "incomplete effects corpus accepted");
    check(!effects::parse_effect_corpus(text + "effect\tUnknown/Asset.bin\tunknown-material\tunknown-program\n",
        effects::effect_registry(), corpus), "unknown corpus effect accepted");
}

void test_actionable_lookup_and_registry_rejections()
{
    const effects::EffectMapping* mapping = nullptr;
    auto result = effects::find_effect(effects::effect_registry(), "missing-effect", "Art/Unknown.W3D", "broken-material", mapping);
    check(!result && result.error.find("Art/Unknown.W3D") != std::string::npos
        && result.error.find("broken-material") != std::string::npos, "unknown effect omitted logical request context");

    const auto canonical = effects::effect_registry();
    std::vector<effects::EffectMapping> entries(canonical.entries, canonical.entries + canonical.size);
    entries[1].kind = effects::EffectKind::water;
    check(!effects::validate_effect_registry({entries.data(), entries.size()}), "duplicate effect family accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[1].point_size = false;
    check(!effects::validate_effect_registry({entries.data(), entries.size()}), "point effect without point size accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[1].source_blend = renderer::BlendFactor::src_alpha;
    check(!effects::validate_effect_registry({entries.data(), entries.size()}), "invalid premultiplied blend accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[2].depth_bias = 0.0F;
    check(!effects::validate_effect_registry({entries.data(), entries.size()}), "projected effect without depth bias accepted");
    entries.assign(canonical.entries, canonical.entries + canonical.size);
    entries[6].pass_count = 1;
    check(!effects::validate_effect_registry({entries.data(), entries.size()}), "single-pass WWShade mapping accepted");
}

} // namespace

int main()
{
    try {
        test_canonical_mapping_and_compiled_sources();
        test_logical_corpus_completeness();
        test_actionable_lookup_and_registry_rejections();
        std::cout << "effect registry tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
