#include "zh/singleplayer/game_flow.h"

#include "zh/video/decoder.h"
#include "zh/video/resolver.h"

#include <array>
#include <utility>

namespace zh::singleplayer {
namespace {

world::Faction world_faction(Faction faction)
{
    switch (faction) {
    case Faction::usa: return world::Faction::usa;
    case Faction::china: return world::Faction::china;
    case Faction::gla: return world::Faction::gla;
    }
    return world::Faction::neutral;
}

world::WorldScene scene_for(const Request& request)
{
    world::WorldScene scene;
    scene.map_name = request.scenario;
    world::WorldItem terrain;
    terrain.label = "terrain:" + request.scenario;
    terrain.family = world::GeometryFamily::terrain;
    terrain.vertex_count = 6;
    terrain.index_count = 6;
    terrain.material.texture_stage_count = 2;
    scene.items.push_back(terrain);
    world::WorldItem army;
    army.label = "army:" + std::string(faction_name(request.faction));
    army.family = world::GeometryFamily::faction_geometry;
    army.faction = world_faction(request.faction);
    army.vertex_count = 12;
    army.index_count = 18;
    army.casts_shadow = true;
    scene.items.push_back(army);
    world::WorldItem marker;
    marker.label = "selection-marker";
    marker.family = world::GeometryFamily::selection_marker;
    marker.material.blending = true;
    scene.items.push_back(marker);
    return scene;
}

std::vector<effects::EffectRequest> representative_effects()
{
    return {
        {"Data/INI/ParticleSystem.ini", "particle-sprite", "particle-point-list", 12, 24.0F},
        {"Art/Textures/Projected.dds", "projected-decal", "projected-texture", 6, 16.0F},
        {"Post/SceneColor", "post-process", "scene-post-effect", 3, 16.0F},
    };
}

} // namespace

GameFlow::GameFlow(renderer::GpuDevice& device, const data::VirtualFileSystem& vfs,
    Request request, foundation::EnvironmentLookup environment)
    : device_(device), vfs_(vfs), session_(std::move(request), &vfs_, std::move(environment)),
      ui_(device_), world_(device_), effects_(device_), audio_(vfs_)
{
}

GameFlow::~GameFlow() { teardown(); }

void GameFlow::record_ui(std::string label, ui::ScreenTransition transition)
{
    const ui::Rect viewport{0, 0, 1280, 720};
    ui::Scene scene{1280, 720, transition, {
        {"english:" + std::move(label), {32, 32, 480, 48}, viewport, 0xffffffffU,
            ui::BlendMode::alpha, ui::Layer::content},
        {"cursor", {640, 360, 16, 16}, viewport, 0xffffffffU,
            ui::BlendMode::alpha, ui::Layer::cursor},
    }};
    if (const auto result = ui_.record(scene); !result) throw FlowError("single-player UI flow failed: " + result.error);
}

void GameFlow::warn(std::string message)
{
    warnings_.push_back(std::move(message));
    device_.record_marker("single-player warning=" + warnings_.back());
}

FlowReport GameFlow::run(Outcome outcome)
{
    if (ran_) throw FlowError("single-player game flow can run only once; construct a new flow after unload");
    ran_ = true;
    try {
        events_.push_back("main-menu");
        record_ui("Main Menu", ui::ScreenTransition::menu_enter);
        session_.start();
        events_.push_back("loading");
        record_ui("Loading " + session_.request().scenario, ui::ScreenTransition::loading_begin);

        if (const auto result = world_.load(scene_for(session_.request())); !result)
            throw FlowError("single-player world load failed: " + result.error);
        world_loaded_ = true;
        if (const auto result = effects_.load(representative_effects()); !result)
            throw FlowError("single-player effects load failed: " + result.error);
        effects_loaded_ = true;

        audio_.configure_output(false);
        const std::array audio_assets{
            std::pair{"Audio/English/mission-speech.wav", audio::AudioKind::speech},
            std::pair{"Audio/music.wav", audio::AudioKind::music},
            std::pair{"Audio/effect.wav", audio::AudioKind::effect},
        };
        for (const auto& [logical, kind] : audio_assets) {
            if (vfs_.find(logical) == nullptr) {
                warn("optional audio absent:" + std::string(logical));
                continue;
            }
            audio::PlayRequest play;
            play.logical_path = logical;
            play.kind = kind;
            play.streaming = kind != audio::AudioKind::effect;
            play.loop = kind == audio::AudioKind::music;
            (void)audio_.play(play);
            events_.push_back("audio:" + std::string(logical));
        }

        try {
            const auto movie = video::resolve_localized_movie(vfs_, "Movies/Briefing.bik", "English");
            device_.record_marker("video-selected logical=" + movie);
            events_.push_back("movie:" + movie);
        } catch (const video::VideoError& error) {
            warn(std::string("optional movie absent:") + error.what());
        }

        record_ui("Loading complete", ui::ScreenTransition::loading_complete);
        session_.advance(session_.request().final_tick / 2);
        if (const auto result = world_.record_frame(session_.snapshot().tick); !result)
            throw FlowError("single-player world frame failed: " + result.error);
        if (const auto result = effects_.record_frame(session_.snapshot().tick); !result)
            throw FlowError("single-player effects frame failed: " + result.error);
        events_.push_back("gameplay:" + std::to_string(session_.snapshot().tick));

        session_.set_paused(true);
        audio_.set_paused(true);
        events_.push_back("pause-menu");
        record_ui("Pause / Options");
        device_.record_marker("single-player options language=English audio=enabled video=enabled");
        session_.set_paused(false);
        audio_.set_paused(false);
        session_.save("autosave", true);
        session_.load("autosave");
        session_.write_replay("last-match");
        session_.verify_replay("last-match");
        events_.push_back("save-load-replay");

        session_.finish(outcome);
        events_.push_back(outcome == Outcome::victory ? "victory" : "defeat");
        record_ui(outcome == Outcome::victory ? "Victory" : "Defeat");

        FlowReport report;
        report.scenario = session_.request().scenario;
        report.mode = session_.request().mode;
        report.faction = session_.request().faction;
        report.outcome = outcome;
        report.final_tick = session_.snapshot().tick;
        report.final_crc = persistence::snapshot_crc32(session_.snapshot());
        report.events = events_;
        report.events.insert(report.events.end(), session_.events().begin(), session_.events().end());
        report.warnings = warnings_;
        teardown();
        report.events.push_back("return-to-menu");
        report.events.push_back("clean-exit");
        return report;
    } catch (...) {
        teardown();
        throw;
    }
}

void GameFlow::teardown() noexcept
{
    if (effects_loaded_) { (void)effects_.teardown(); effects_loaded_ = false; }
    if (world_loaded_) { (void)world_.teardown(); world_loaded_ = false; }
    audio_.shutdown();
    session_.shutdown();
}

} // namespace zh::singleplayer
