#include "zh/audio/manager.h"
#include "zh/data/inventory.h"
#include "zh/data/vfs.h"

#include <filesystem>
#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
    if (argc != 5) {
        std::cerr << "usage: audio_corpus ZH_ROOT GENERALS_ROOT LANGUAGE MANIFEST\n";
        return 2;
    }
    try {
        const auto vfs = zh::data::VirtualFileSystem::mount({argv[1], argv[2], argv[3], {}});
        const auto manifest = zh::data::load_corpus_manifest(argv[4]);
        zh::audio::AudioManager manager(vfs, 4); manager.configure_output(false);
        std::vector<float> sink(256 * 2);
        std::size_t count = 0;
        for (const auto& requirement : manifest.required) {
            if (requirement.format != zh::data::AssetFormat::wav && requirement.format != zh::data::AssetFormat::mp3) continue;
            const auto metadata = zh::audio::probe_audio(vfs, requirement.logical_name);
            zh::audio::PlayRequest request; request.logical_path = requirement.logical_name; request.streaming = requirement.format == zh::data::AssetFormat::mp3;
            const auto handle = manager.play(request); manager.render(sink.data(), 256); manager.stop(handle); manager.render(sink.data(), 1);
            manager.drain_completions();
            std::cout << "audio corpus: " << requirement.logical_name << " encoding=" << metadata.encoding
                      << " channels=" << metadata.channels << " rate=" << metadata.sample_rate << '\n';
            ++count;
        }
        if (count == 0) throw zh::audio::AudioError("audio corpus manifest contains no audio requirements");
        std::cout << "audio corpus verification ok: " << count << " logical assets; null sink playable\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "audio corpus error: " << error.what() << '\n';
        return 1;
    }
}
