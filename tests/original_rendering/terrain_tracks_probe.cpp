#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <climits>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename Operation> bool rejected(Operation operation)
{
	try { operation(); } catch (const std::runtime_error &) { return true; }
	return false;
}
unsigned draws(const std::string &trace)
{
	unsigned result = 0;
	for (std::size_t at = trace.find("draw pipeline="); at != std::string::npos;
		at = trace.find("draw pipeline=", at + 1)) ++result;
	return result;
}
unsigned occurrences_before(const std::string &trace, const char *needle, std::size_t end)
{
	unsigned result = 0;
	for (std::size_t at = trace.find(needle); at != std::string::npos && at < end;
		at = trace.find(needle, at + 1)) ++result;
	return result;
}
}

extern "C" void zh_probe_terrain_tracks()
{
	const char *map = std::getenv("ZH_M22_TERRAIN_TRACKS_MAP");
	require(map, "original terrain-track map fixture missing");
	const Int saved_modules = TheGlobalData->m_maxTerrainTracks;
	const Real saved_partition = TheGlobalData->m_partitionCellSize;
	const Bool saved_marks = TheGlobalData->m_makeTrackMarks;
	const Int saved_edges = TheGlobalData->m_maxTankTrackEdges;
	const Int saved_opaque = TheGlobalData->m_maxTankTrackOpaqueEdges;
	const Int saved_fade = TheGlobalData->m_maxTankTrackFadeDelay;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	TheWritableGlobalData->m_makeTrackMarks = TRUE;
	TheWritableGlobalData->m_maxTankTrackEdges = 8;
	TheWritableGlobalData->m_maxTankTrackOpaqueEdges = 4;
	TheWritableGlobalData->m_maxTankTrackFadeDelay = 1;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		const Int module_count = generation + 1;
		TheWritableGlobalData->m_maxTerrainTracks = module_count;
		zh::original_runtime::OriginalGpuEdge edge(device);
		{
			W3DDisplay display; display.init();
			Display *saved_display = TheDisplay; TheDisplay = &display;
			TerrainVisual *saved_visual = TheTerrainVisual;
			W3DTerrainVisual visual; TheTerrainVisual = &visual;
			View *saved_view = TheTacticalView; W3DView *view = NULL;
			try {
				display.setWidth(32); display.setHeight(24); visual.init();
				require(TheTerrainTracksRenderObjClassSystem &&
					TheTerrainTracksRenderObjClassSystem->hasPendingGpuResources(),
					"original map-less track owner acquired resources");
				require(visual.load(AsciiString(map)) && !TheTerrainTracksRenderObjClassSystem->hasPendingGpuResources(),
					"original track map load did not reacquire source buffers");
				auto *tracks = TheTerrainTracksRenderObjClassSystem;
				const std::string allocated = device.snapshot();
				require(allocated.find("label=\"original WW3D 16-bit index buffer\" size=84 usage=1 dynamic=true") != std::string::npos &&
					allocated.find(std::string("label=\"original WW3D vertex buffer\" size=") +
						(module_count == 1 ? "384" : "768") + " usage=0 dynamic=true") != std::string::npos,
					"original terrain-track source buffer descriptors failed");
				tracks->ReleaseResources();
				device.fail_next_buffer_create();
				require(rejected([&] { tracks->ReAcquireResources(); }) && tracks->hasPendingGpuResources(),
					"original terrain-track create rollback was not pending");
				tracks->ReAcquireResources();
				require(!tracks->hasPendingGpuResources(), "original terrain-track create retry failed");
				view = new W3DView; TheTacticalView = view; view->init(); display.attachView(view);
				view->setWidth(32); view->setHeight(24); view->setDefaultView(0, 0, 1);
				auto *track = tracks->bindTrack(TheHeightMap, MAP_XY_FACTOR, "");
				require(track && tracks->ownsActiveModule(track) &&
					static_cast<void *>(track->Peek_Scene()) == static_cast<void *>(W3DDisplay::m_3DScene),
					"original terrain-track module singleton/publication failed");
				auto *sibling = module_count == 2 ? tracks->bindTrack(TheHeightMap, MAP_XY_FACTOR, "") : NULL;
				if (sibling) require(tracks->ownsActiveModule(sibling) &&
					static_cast<void *>(sibling->Peek_Scene()) == static_cast<void *>(W3DDisplay::m_3DScene),
					"original terrain-track sibling publication failed");
				require((module_count == 1 ? !sibling : !!sibling) &&
					!tracks->bindTrack(TheHeightMap, MAP_XY_FACTOR, ""),
					"original terrain-track capacity boundary failed");
				require(rejected([&] { tracks->init(NULL); }),
					"original terrain-track foreign scene bootstrap was accepted");
				track->addEdgeToTrack(0, 0); track->addEdgeToTrack(24, 0); track->addEdgeToTrack(48, 0);
				if (sibling) { sibling->addEdgeToTrack(0, 8); sibling->addEdgeToTrack(24, 8); sibling->addEdgeToTrack(48, 8); }
				zh::renderer::TextureDesc target;
				target.width = 32; target.height = 24; target.format = zh::renderer::TextureFormat::bgra8;
				target.render_target = true; target.sampled = false;
				auto color = device.create_texture(target, "original terrain tracks color");
				target.format = zh::renderer::TextureFormat::depth24_stencil8;
				auto depth = device.create_texture(target, "original terrain tracks depth");
				require(color && depth, "original terrain-track targets missing");
				auto frame = [&] { edge.bind_frame_targets(color, depth, 32, 24); display.draw();
					require(!WW3D::Is_Rendering() && !device.pass_active(), "original terrain-track frame retained state"); };
				const std::string before = device.snapshot(); frame();
				const std::string active = device.snapshot().substr(before.size());
				const std::size_t track_marker = active.find("original TerrainTracksRenderObjClassSystem::flush");
				require(draws(active) == 2 + module_count && active.find("original RTS3DScene::Render map terrain") < track_marker &&
					active.find("DX8Wrapper::Draw indexed first=0 count=6 base=0", track_marker) != std::string::npos,
					"original terrain-track flush ordering/range failed");
				device.fail_buffer_upload_after(occurrences_before(active, "upload ", track_marker));
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active() &&
					tracks->ownsActiveModule(track), "original terrain-track upload rollback lost module");
				device.fail_draw_after(draws(active.substr(0, track_marker)));
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active(),
					"original terrain-track draw rollback failed");
				frame();
				TheWritableGlobalData->m_makeTrackMarks = FALSE;
				const std::string disabled_before = device.snapshot(); frame();
				require(draws(device.snapshot().substr(disabled_before.size())) == 2,
					"original disabled terrain tracks emitted a draw");
				TheWritableGlobalData->m_makeTrackMarks = TRUE;
				tracks->unbindTrack(track); if (sibling) tracks->unbindTrack(sibling); tracks->Reset();
				const std::string expired_before = device.snapshot(); frame();
				require(draws(device.snapshot().substr(expired_before.size())) == 2,
					"original expired terrain tracks emitted a draw");
				device.destroy(depth); device.destroy(color);
			} catch (...) { TheTacticalView = saved_view; TheTerrainVisual = saved_visual; TheDisplay = saved_display; throw; }
			TheTacticalView = saved_view; TheTerrainVisual = saved_visual; TheDisplay = saved_display;
		}
		edge.release_source_buffers();
	}
	// The source vertex offset is 16-bit.  Verify an impossible cardinality
	// unwinds before any track owner or Recording resource is published.
	TheWritableGlobalData->m_maxTerrainTracks = INT_MAX;
	{
		zh::renderer::RecordingGpuDevice overflow_device;
		zh::original_runtime::OriginalGpuEdge edge(overflow_device);
		W3DDisplay display; display.init();
		Display *saved_display = TheDisplay; TheDisplay = &display;
		TerrainVisual *saved_visual = TheTerrainVisual;
		W3DTerrainVisual visual; TheTerrainVisual = &visual;
		try {
			require(rejected([&] { visual.init(); }) && !TheTerrainTracksRenderObjClassSystem,
				"original terrain-track overflow owner was published");
		} catch (...) { TheTerrainVisual = saved_visual; TheDisplay = saved_display; throw; }
		TheTerrainVisual = saved_visual; TheDisplay = saved_display;
		edge.release_source_buffers();
		require(overflow_device.resource_counts().total() == 0,
			"original terrain-track overflow retained Recording resources");
	}
	TheWritableGlobalData->m_maxTerrainTracks = saved_modules;
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	TheWritableGlobalData->m_makeTrackMarks = saved_marks;
	TheWritableGlobalData->m_maxTankTrackEdges = saved_edges;
	TheWritableGlobalData->m_maxTankTrackOpaqueEdges = saved_opaque;
	TheWritableGlobalData->m_maxTankTrackFadeDelay = saved_fade;
	require(device.resource_counts().total() == 0, "original terrain-track teardown retained resources");
	std::puts("original terrain tracks: single=1 multi=2 capacity=1 retry=2 disabled=1 expired=1 generations=2 resources=0");
}
