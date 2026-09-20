#include "zh/data/vfs.h"
#include "zh/original_data.h"
#include "zh/original_resources.h"
#include "zh/renderer/recording_device.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace {
int failures = 0;
void check(bool condition, std::string_view message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

void write_bytes(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void write_text(const std::filesystem::path& path, std::string_view text)
{
    write_bytes(path, {text.begin(), text.end()});
}

bool zero(const zh::original_resources::OwnershipCounts& counts)
{
    return counts.loaders == 0 && counts.assets == 0 && counts.scenes == 0 && counts.lights == 0 &&
        counts.callbacks == 0 && counts.layouts == 0 && counts.windows == 0 && counts.fonts == 0 &&
        counts.audio_definitions == 0 && counts.workers == 0 && counts.device_acquisitions == 0;
}
}

int main()
{
    using namespace zh::original_resources;
    const auto root = std::filesystem::temp_directory_path() /
        ("zh-original-resources-cpu-" + std::to_string(static_cast<long long>(::getpid())));
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    const auto zh_root = root / "zh";
    const auto generals_root = root / "generals";
    std::filesystem::create_directories(generals_root);
    write_text(zh_root / "Art/model.w3d", "W3DMODEL vertices=3 indices=3\n");
    write_text(zh_root / "Art/walk.w3a", "W3DANIM frames=12\n");
    std::vector<std::uint8_t> texture(24, 0x7f);
    texture[0] = 'Z'; texture[1] = 'H'; texture[2] = 'W'; texture[3] = 'T';
    texture[4] = 2; texture[5] = 0; texture[6] = 2; texture[7] = 0;
    write_bytes(zh_root / "Art/skin.zhwt", texture);
    const auto vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    const zh::original_data::LogicalFiles files(vfs);
    const std::vector<ResourceRequest> resources{
        {ResourceKind::model, "Art/model.w3d"},
        {ResourceKind::texture, "Art/skin.zhwt"},
        {ResourceKind::animation, "Art/walk.w3a"},
    };

    {
        zh::renderer::RecordingGpuDevice device;
        CpuPresentation presentation(files, device);
        check(presentation.initialize(resources), presentation.last_error());
        const auto live = presentation.counts();
        check(live.loaders == 3 && live.assets == 3 && live.scenes == 1 && live.lights == 1,
            "source loaders/assets/scene/light not owned");
        check(live.device_acquisitions == 0, "headless CPU consumer acquired a device");
        check(presentation.supports(Capability::cpu_resources) && presentation.supports(Capability::recorded_drawing),
            "CPU/recording capabilities absent");
        check(!presentation.supports(Capability::physical_device) && !presentation.supports(Capability::web_browser),
            "headless provider exposed device/browser capability");
        check(presentation.record_scene(), presentation.last_error());
        const auto snapshot = device.snapshot();
        check(snapshot.find("original W3D CPU initialize") != std::string::npos, "initialization marker missing");
        check(snapshot.find("original W3D scene light=default assets=3") != std::string::npos, "scene producer missing");
        check(snapshot.find("begin_pass label=\"original W3D CPU scene\"") != std::string::npos,
            "original scene did not reach recording device");
        check(snapshot.find("draw pipeline=") != std::string::npos, "original scene emitted no draw");
        presentation.shutdown();
        presentation.shutdown();
        check(zero(presentation.counts()), "normal/repeated shutdown retained ownership");
        check(device.resource_counts().total() == 0, "normal shutdown retained recording resources");
    }

    for (std::size_t stage = 1; stage <= 6; ++stage) {
        zh::renderer::RecordingGpuDevice device;
        CpuPresentation presentation(files, device);
        check(!presentation.initialize(resources, stage), "injected failure was accepted");
        check(presentation.last_error().find("injected CPU presentation failure") != std::string_view::npos,
            "injected failure diagnostic missing");
        check(zero(presentation.counts()), "partial failure retained ownership");
        check(device.resource_counts().total() == 0, "partial failure retained recording resources");
    }

    {
        zh::renderer::RecordingGpuDevice device;
        CpuPresentation presentation(files, device);
        auto duplicate = resources; duplicate.push_back(resources.front());
        check(!presentation.initialize(duplicate), "duplicate resource accepted");
        check(presentation.last_error().find("duplicate") != std::string_view::npos, "duplicate diagnostic missing");
        check(!presentation.record_scene() && presentation.last_error().find("not initialized") != std::string_view::npos,
            "record before initialization accepted");
        auto missing = resources; missing[0].logical_name = "Art/missing.w3d";
        check(!presentation.initialize(missing), "missing resource accepted");
        check(presentation.last_error().find("missing original logical file") != std::string_view::npos,
            "missing resource diagnostic lost logical context");
        write_text(zh_root / "Art/bad.w3d", "not a model\n");
    }

    // A newly mounted malformed fixture proves validation without mutating an active session.
    const auto malformed_vfs = zh::data::VirtualFileSystem::mount({zh_root, generals_root, "English", {}});
    const zh::original_data::LogicalFiles malformed_files(malformed_vfs);
    zh::renderer::RecordingGpuDevice malformed_device;
    CpuPresentation malformed(malformed_files, malformed_device);
    auto bad = resources; bad[0].logical_name = "Art/bad.w3d";
    check(!malformed.initialize(bad), "malformed model accepted");
    check(malformed.last_error().find("malformed W3D model") != std::string_view::npos,
        "malformed model diagnostic missing");
    check(zero(malformed.counts()) && malformed_device.resource_counts().total() == 0,
        "malformed resource retained state");
    check(std::string(provider_cpu_identity()) == "OriginalCpuPresentation.cpp", "CPU provider witness");

    std::filesystem::remove_all(root, ignored);
    if (failures != 0) return 1;
    std::cout << "original-resources CPU: ok providers=" << provider_cpu_identity()
              << " loaders=3 assets=3 devices=0\n";
    return 0;
}
