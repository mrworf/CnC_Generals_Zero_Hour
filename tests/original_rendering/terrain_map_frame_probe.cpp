#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

template <typename Operation>
bool rejected(Operation operation)
{
	try {
		operation();
	} catch (const std::runtime_error &) {
		return true;
	}
	return false;
}

unsigned draw_count(const std::string &trace)
{
	unsigned count = 0;
	for (std::size_t at = trace.find("draw pipeline="); at != std::string::npos;
		at = trace.find("draw pipeline=", at + 1)) ++count;
	return count;
}
}

extern "C" void zh_probe_terrain_map_frame()
{
	const char *map_path = std::getenv("ZH_M22_TERRAIN_MAP_FRAME_MAP");
	require(map_path, "original terrain map-frame fixture missing");
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		{
			W3DDisplay display;
			display.init();
			Display *saved_display = TheDisplay;
			TheDisplay = &display;
			display.setWidth(32);
			display.setHeight(24);
			TerrainVisual *saved_visual = TheTerrainVisual;
			W3DTerrainVisual visual;
			TheTerrainVisual = &visual;
			View *saved_tactical_view = TheTacticalView;
			W3DView *view = NULL;
			try {
				visual.init();
				require(visual.load(AsciiString(map_path)) && TheHeightMap &&
					TheHeightMap->getMap() && TheTerrainRenderObject == TheHeightMap &&
					TheHeightMap->doesNeedFullUpdate(),
					"original map frame did not publish pending terrain owner");
				view = new W3DView;
				TheTacticalView = view;
				view->init();
				display.attachView(view);
				view->setWidth(32);
				view->setHeight(24);
				view->setDefaultView(0, 0, 1);

				zh::renderer::TextureDesc target;
				target.width = 32;
				target.height = 24;
				target.render_target = true;
				target.sampled = false;
				target.format = zh::renderer::TextureFormat::bgra8;
				auto color = device.create_texture(target, "original terrain map-frame color");
				target.format = zh::renderer::TextureFormat::depth24_stencil8;
				auto depth = device.create_texture(target, "original terrain map-frame depth");
				require(color && depth, "original terrain map-frame targets missing");
				auto frame = [&]() {
					edge.bind_frame_targets(color, depth, 32, 24);
					display.draw();
					require(!WW3D::Is_Rendering() && !device.pass_active(),
						"original map frame retained a source frame");
				};

				const std::string before = device.snapshot();
				frame();
				const std::string first = device.snapshot().substr(before.size());
				require(!TheHeightMap->doesNeedFullUpdate() &&
					TheTerrainRenderObject->getShroud()->getShroudTexture() &&
					draw_count(first) == 2 &&
					first.find("original W3DShroud::render projected") != std::string::npos &&
					first.find("original W3DShroud::render projected") <
					first.find("original W3DView::updateView terrain center") &&
					first.find("original W3DView::updateView terrain center") <
					first.find("original RTS3DScene::Render map terrain") &&
					first.find("original RTS3DScene::Render map terrain") <
					first.find("draw pipeline="),
					"original map frame did not order shroud projection before terrain draw");
				const std::string stationary_before = device.snapshot();
				frame();
				require(draw_count(device.snapshot().substr(stationary_before.size())) == 2 &&
					!TheHeightMap->doesNeedFullUpdate(),
					"original stationary map frame changed terrain update state");

				device.fail_next_draw();
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active() &&
					TheHeightMap->getMap() && TheTerrainRenderObject->getShroud()->getShroudTexture(),
					"original map-frame draw abort damaged source owners");
				frame();

				TheTerrainRenderObject->getShroud()->setShroudFilter(FALSE);
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active() &&
					TheTerrainRenderObject->getShroud()->getShroudTexture(),
					"original stale shroud frame did not reject cleanly");
				TheTerrainRenderObject->getShroud()->setShroudFilter(TRUE);
				frame();
				require(TheTerrainRenderObject->getShroud()->getShroudTexture(),
					"original map frame did not recover shroud projection");
				W3DDisplay::m_3DScene->Remove_Render_Object(TheTerrainRenderObject);
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active() &&
					!TheTerrainRenderObject->Peek_Scene(),
					"original map frame accepted a detached terrain owner");
				W3DDisplay::m_3DScene->Add_Render_Object(TheTerrainRenderObject);
				frame();

				const Bool saved_water = TheGlobalData->m_useWaterPlane;
				TheWritableGlobalData->m_useWaterPlane = TRUE;
				require(rejected(frame) && !WW3D::Is_Rendering() && !device.pass_active(),
					"original map frame activated disabled sibling water route");
				TheWritableGlobalData->m_useWaterPlane = saved_water;
				frame();

				device.destroy(depth);
				device.destroy(color);
			} catch (...) {
				TheTacticalView = saved_tactical_view;
				TheTerrainVisual = saved_visual;
				TheDisplay = saved_display;
				throw;
			}
			TheTacticalView = saved_tactical_view;
			TheTerrainVisual = saved_visual;
			TheDisplay = saved_display;
		}
		require(!TheHeightMap && !TheTerrainRenderObject,
			"original map-frame visual teardown retained map owner");
			edge.release_source_buffers();
	}
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	require(device.resource_counts().total() == 0,
		"original map-frame re-entry retained Recording resources");
	std::puts("original terrain map frame: shroud-before-terrain=1 retry=2 siblings=0 generations=2 resources=0");
}
