#include "zh/effects/recorder.h"

#include <algorithm>
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

std::vector<effects::EffectRequest> corpus_requests(bool reverse = false)
{
    effects::EffectCorpusCoverage corpus;
    const auto text = read_file(std::filesystem::path(ZH_SOURCE_DIR) / "data/corpus/effects-manifest.tsv");
    check(effects::parse_effect_corpus(text, effects::effect_registry(), corpus), "effects corpus parse failed");
    std::vector<effects::EffectRequest> requests;
    for (std::size_t index = 0; index < corpus.entries.size(); ++index) {
        const auto& entry = corpus.entries[index];
        requests.push_back({entry.logical_asset, entry.material, entry.legacy_effect,
            static_cast<renderer::UInt32>(3 + index), 24.0F});
    }
    if (reverse) std::reverse(requests.begin(), requests.end());
    return requests;
}

std::string record_corpus(bool reverse)
{
    renderer::RecordingGpuDevice device(32);
    effects::EffectsRecorder recorder(device);
    check(recorder.load(corpus_requests(reverse)), "effects load failed");
    check(recorder.owned_pipeline_count() == 8 && recorder.owned_pipeline_count() == device.pipeline_count(),
        "effects pipeline accounting changed");
    check(recorder.record_frame(7), "effects first frame failed");
    const auto stable_pipeline_count = device.pipeline_count();
    check(recorder.record_frame(8), "effects second frame failed");
    check(device.pipeline_count() == stable_pipeline_count, "effects pipeline cache grew between frames");
    check(recorder.teardown(), "effects teardown failed");
    check(device.resource_counts().total() == 0, "effects teardown leaked device resources");
    return device.snapshot();
}

void test_complete_deterministic_command_sequence()
{
    const auto ordered = record_corpus(false);
    const auto reversed = record_corpus(true);
    check(ordered == reversed, "caller effect order changed normalized command stream");
    for (unsigned value = 0; value < static_cast<unsigned>(effects::EffectKind::count); ++value) {
        const auto marker = "effect kind=" + std::string(effects::effect_kind_name(static_cast<effects::EffectKind>(value)));
        check(ordered.find(marker) != std::string::npos, "effect family missing from recorded stream");
    }
    const auto source = ordered.find("begin_pass label=\"effects source dependency\"");
    const auto main = ordered.find("begin_pass label=\"effects main pass\"");
    const auto post = ordered.find("begin_pass label=\"effects post output\"");
    check(source < main && main < post, "render-target dependency order changed");
    const auto pass_one = ordered.find("legacy=wwshade-multipass pass=1/2");
    const auto pass_two = ordered.find("legacy=wwshade-multipass pass=2/2");
    check(pass_one < pass_two, "WWShade multipass order changed");
    check(ordered.find("point_size=24.000000") != std::string::npos, "particle point size missing");
    check(ordered.find("depth-bias=-1.000000") != std::string::npos, "projected texture depth bias missing");
    check(ordered.find("legacy=particle-point-list pass=1/1 depth-bias=0.000000 premultiplied=true") != std::string::npos,
        "premultiplied particle state missing");
    check(ordered.find("effects-render-target dependency source -> main-effects -> post-output") != std::string::npos,
        "render-target dependency marker missing");
}

void test_unknown_and_invalid_requests_are_actionable()
{
    renderer::RecordingGpuDevice device;
    effects::EffectsRecorder recorder(device);
    auto unknown = corpus_requests();
    unknown[0].legacy_effect = "missing-program";
    check(!recorder.load(unknown), "unknown required effect accepted");
    check(recorder.last_error().find(unknown[0].logical_asset) != std::string::npos
        && recorder.last_error().find(unknown[0].material) != std::string::npos,
        "unknown required effect omitted logical asset/material");
    check(device.resource_counts().total() == 0, "unknown effect allocated resources");

    auto duplicate = corpus_requests();
    duplicate.push_back(duplicate.front());
    check(!recorder.load(duplicate) && recorder.last_error().find("duplicates logical asset") != std::string::npos,
        "duplicate logical effect accepted");
    auto invalid = corpus_requests();
    invalid.front().vertex_count = 0;
    check(!recorder.load(invalid) && recorder.last_error().find("vertex count") != std::string::npos,
        "zero-sized effect geometry accepted");
}

void test_lifecycle_recovery_and_bounded_capacity()
{
    renderer::RecordingGpuDevice device(32);
    effects::EffectsRecorder recorder(device);
    check(!recorder.record_frame(0) && recorder.last_error().find("loaded effects") != std::string::npos,
        "frame before load accepted");
    check(recorder.load(corpus_requests()), "load after rejected frame failed");
    check(!recorder.load(corpus_requests()) && recorder.last_error().find("teardown") != std::string::npos,
        "double load accepted");
    check(recorder.record_frame(9), "valid frame failed");
    check(!recorder.record_frame(8) && recorder.last_error().find("regressed") != std::string::npos,
        "regressed effects tick accepted");
    check(recorder.record_frame(10), "recorder did not recover after regressed tick");
    check(recorder.teardown(), "teardown after recovery failed");
    check(!recorder.teardown(), "double teardown accepted");
    check(recorder.load(corpus_requests()), "reload after teardown failed");
    check(recorder.record_frame(0), "frame after reload failed");
    check(recorder.teardown(), "second teardown failed");

    renderer::RecordingGpuDevice bounded_device(1);
    effects::EffectsRecorder bounded(bounded_device);
    check(!bounded.load(corpus_requests()), "insufficient pipeline capacity accepted");
    check(bounded.last_error().find("capacity exceeded") != std::string::npos,
        "pipeline capacity failure was not actionable");
    check(!bounded.loaded() && bounded_device.resource_counts().total() == 0,
        "failed effects load leaked partial resources");
}

} // namespace

int main()
{
    try {
        test_complete_deterministic_command_sequence();
        test_unknown_and_invalid_requests_are_actionable();
        test_lifecycle_recovery_and_bounded_capacity();
        std::cout << "effect recorder tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
