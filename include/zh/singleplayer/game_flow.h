#pragma once

#include "zh/audio/manager.h"
#include "zh/effects/recorder.h"
#include "zh/singleplayer/session.h"
#include "zh/ui/renderer.h"
#include "zh/world/recorder.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace zh::singleplayer {

class FlowError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct FlowReport {
    std::string scenario;
    Mode mode = Mode::skirmish;
    Faction faction = Faction::usa;
    Outcome outcome = Outcome::victory;
    foundation::UInt32 final_tick = 0;
    foundation::UInt32 final_crc = 0;
    std::vector<std::string> events;
    std::vector<std::string> warnings;
};

class GameFlow {
public:
    GameFlow(renderer::GpuDevice& device, const data::VirtualFileSystem& vfs,
        Request request, foundation::EnvironmentLookup environment = {});
    ~GameFlow();

    GameFlow(const GameFlow&) = delete;
    GameFlow& operator=(const GameFlow&) = delete;

    FlowReport run(Outcome outcome);

private:
    void record_ui(std::string label, ui::ScreenTransition transition = ui::ScreenTransition::none);
    void warn(std::string message);
    void teardown() noexcept;

    renderer::GpuDevice& device_;
    const data::VirtualFileSystem& vfs_;
    Session session_;
    ui::UiRecorder ui_;
    world::WorldRecorder world_;
    effects::EffectsRecorder effects_;
    audio::AudioManager audio_;
    std::vector<std::string> events_;
    std::vector<std::string> warnings_;
    bool world_loaded_ = false;
    bool effects_loaded_ = false;
    bool ran_ = false;
};

} // namespace zh::singleplayer
