#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WW3D2/scene.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
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
	try { operation(); }
	catch (const std::runtime_error &) { return true; }
	return false;
}

class TestTerrain final : public HeightMapRenderObjClass
{
public:
	void On_Frame_Update() override { ++updates; }
	void forceMalformedProp(bool enabled)
	{
		m_propBuffer = enabled ? reinterpret_cast<W3DPropBuffer *>(this) : NULL;
	}
	Int updates = 0;
};

class RegistrationScene final : public SimpleSceneClass
{
public:
	void Register(RenderObjClass *object, RegType kind) override
	{
		if (kind == ON_FRAME_UPDATE) ++registrations;
		SimpleSceneClass::Register(object, kind);
	}
	void Unregister(RenderObjClass *object, RegType kind) override
	{
		if (kind == ON_FRAME_UPDATE) ++unregistrations;
		SimpleSceneClass::Unregister(object, kind);
	}
	Int registrations = 0;
	Int unregistrations = 0;
};
}

extern "C" void zh_probe_terrain_scene_attachment()
{
	const char *path = std::getenv("ZH_M22_TERRAIN_SCENE_ATTACHMENT_MAP");
	require(path, "original terrain scene attachment map missing");
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original terrain scene attachment map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, FALSE));
	input.close();
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	const UnsignedByte saved_shrouded = TheWritableGlobalData->m_shroudAlpha;
	const UnsignedByte saved_fogged = TheWritableGlobalData->m_fogAlpha;
	const UnsignedByte saved_clear = TheWritableGlobalData->m_clearAlpha;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	TheWritableGlobalData->m_shroudAlpha = 17;
	TheWritableGlobalData->m_fogAlpha = 91;
	TheWritableGlobalData->m_clearAlpha = 203;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		Display *saved_display = TheDisplay;
		TheDisplay = display.get();
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted pre-bootstrap calls");
		display->init();
		TestTerrain terrain;
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted a missing terrain map");
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted an uninitialized map");
		require(terrain.initHeightData(map->getDrawWidth(), map->getDrawHeight(), map, NULL, TRUE) == 0,
			"original terrain scene attachment map binding failed");
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted a detached map");
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted a detached terrain map");
		AABoxClass box;
		SphereClass sphere;
		terrain.Get_Obj_Space_Bounding_Box(box);
		terrain.Get_Obj_Space_Bounding_Sphere(sphere);
		const float max_height = 63.0f * MAP_HEIGHT_SCALE;
		require(box.Center.X == 4.0f * MAP_XY_FACTOR && box.Center.Y == 4.0f * MAP_XY_FACTOR &&
			box.Center.Z == max_height * 0.5f && box.Extent.X == 4.0f * MAP_XY_FACTOR &&
			box.Extent.Y == 4.0f * MAP_XY_FACTOR && box.Extent.Z == max_height * 0.5f &&
			sphere.Center.X == box.Center.X && sphere.Center.Y == box.Center.Y &&
			sphere.Center.Z == box.Center.Z && sphere.Radius > 0.0f,
			"original terrain scene attachment bounds changed");

		W3DDisplay::m_3DScene->Add_Render_Object(&terrain);
		require(terrain.Peek_Scene() == W3DDisplay::m_3DScene,
			"original terrain primary scene attachment missing");
		const auto baseline = device.resource_counts();
		terrain.notifyShroudChanged();
		terrain.notifyShroudChanged();
		require(device.resource_counts() == baseline,
			"original no-prop shroud notification created Recording resources");
		W3DShroud *shroud = terrain.getShroud();
		const Int cells_x = shroud->getNumShroudCellsX();
		const Int cells_y = shroud->getNumShroudCellsY();
		require(cells_x > 1 && cells_y > 1,
			"original display shroud generated dimensions missing");
		const W3DShroudLevel initial = shroud->getShroudLevel(0, 0);
		display->clearShroud();
		display->clearShroud();
		require(shroud->getShroudLevel(0, 0) == initial,
			"original display clearShroud mutated native no-op grid");
		display->setShroudLevel(0, 0, CELLSHROUD_SHROUDED);
		display->setShroudLevel(cells_x - 1, cells_y - 1, CELLSHROUD_FOGGED);
		display->setShroudLevel(0, 0, CELLSHROUD_CLEAR);
		require(shroud->getShroudLevel(0, 0) == 203 &&
			shroud->getShroudLevel(cells_x - 1, cells_y - 1) == 91,
			"original display shroud category-to-alpha or edge mapping changed");
		display->clearShroud();
		require(shroud->getShroudLevel(0, 0) == 203,
			"original display repeated clear changed a written cell");
		auto rejects_without_mutation = [&]() {
			const W3DShroudLevel before = shroud->getShroudLevel(0, 0);
			return rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_FOGGED); }) &&
				shroud->getShroudLevel(0, 0) == before;
		};
		require(rejected([&]() { display->setShroudLevel(-1, 0, CELLSHROUD_FOGGED); }) &&
			rejected([&]() { display->setShroudLevel(cells_x, cells_y - 1, CELLSHROUD_FOGGED); }) &&
			rejected([&]() { display->setShroudLevel(0, cells_y, CELLSHROUD_FOGGED); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_COUNT); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, static_cast<CellShroudStatus>(-1)); }) &&
			shroud->getShroudLevel(0, 0) == 203,
			"original display shroud accepted an invalid cell or category");
		TheDisplay = saved_display;
		const bool foreign_display = rejects_without_mutation();
		TheDisplay = display.get();
		HeightMapRenderObjClass *saved_height_map = TheHeightMap;
		TheHeightMap = NULL;
		const bool missing_height_map = rejects_without_mutation();
		TheHeightMap = saved_height_map;
		BaseHeightMapRenderObjClass *saved_owner = TheTerrainRenderObject;
		TheTerrainRenderObject = NULL;
		const bool missing_owner = rejected([&]() { terrain.notifyShroudChanged(); });
		const bool missing_display_owner = rejects_without_mutation();
		TheTerrainRenderObject = saved_owner;
		W3DAssetManager *saved_assets = W3DDisplay::m_assetManager;
		W3DDisplay::m_assetManager = NULL;
		const bool missing_assets = rejected([&]() { terrain.notifyShroudChanged(); });
		const bool missing_display_assets = rejects_without_mutation();
		W3DDisplay::m_assetManager = saved_assets;
		RTS3DScene *saved_scene = W3DDisplay::m_3DScene;
		W3DDisplay::m_3DScene = NULL;
		const bool missing_scene = rejected([&]() { terrain.notifyShroudChanged(); });
		const bool missing_display_scene = rejects_without_mutation();
		W3DDisplay::m_3DScene = saved_scene;
		terrain.forceMalformedProp(true);
		const bool malformed_prop = rejected([&]() { terrain.notifyShroudChanged(); });
		const bool malformed_display_prop = rejects_without_mutation();
		terrain.forceMalformedProp(false);
		require(foreign_display && missing_height_map && missing_owner &&
			missing_display_owner && missing_assets && missing_display_assets &&
			missing_scene && missing_display_scene && malformed_prop &&
			malformed_display_prop &&
			device.resource_counts() == baseline,
			"original terrain shroud notification accepted a broken owner or prop");
		terrain.notifyShroudChanged();
		display->setShroudLevel(0, 0, CELLSHROUD_SHROUDED);
		require(shroud->getShroudLevel(0, 0) == 17 &&
			device.resource_counts() == baseline,
			"original display shroud provider retry failed");
		RegistrationScene alternate;
		bool duplicate_rejected = false;
		try { terrain.Notify_Added(W3DDisplay::m_3DScene); }
		catch (const std::runtime_error &) { duplicate_rejected = true; }
		bool mismatched_add_rejected = false;
		try { terrain.Notify_Added(&alternate); }
		catch (const std::runtime_error &) { mismatched_add_rejected = true; }
		bool mismatched_remove_rejected = false;
		try { terrain.Notify_Removed(&alternate); }
		catch (const std::runtime_error &) { mismatched_remove_rejected = true; }
		require(duplicate_rejected && mismatched_add_rejected && mismatched_remove_rejected &&
			terrain.Peek_Scene() == W3DDisplay::m_3DScene && alternate.registrations == 0 &&
			alternate.unregistrations == 0,
			"original terrain scene negative attachment changed publication");
		W3DDisplay::m_3DScene->Remove_Render_Object(&terrain);
		require(!terrain.Peek_Scene(), "original terrain primary scene detach retained owner");
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted primary scene detachment");
		require(rejected([&]() { display->clearShroud(); }) && rejects_without_mutation(),
			"original display shroud accepted primary scene detachment");

		alternate.Add_Render_Object(&terrain);
		require(terrain.Peek_Scene() == &alternate && alternate.registrations == 1,
			"original terrain update registration missing");
		alternate.Remove_Render_Object(&terrain);
		require(!terrain.Peek_Scene() && alternate.unregistrations == 1,
			"original terrain update unregister missing");
		alternate.Add_Render_Object(&terrain);
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted a foreign scene");
		require(rejected([&]() { display->clearShroud(); }) && rejects_without_mutation(),
			"original display shroud accepted a foreign scene");
		alternate.Remove_Render_Object(&terrain);
		require(alternate.registrations == 2 && alternate.unregistrations == 2 && !terrain.Peek_Scene(),
			"original terrain scene re-entry changed update ownership");
		terrain.freeMapResources();
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted a released map");
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted a released map");
		edge.release_source_buffers();
		TheDisplay = saved_display;
		display.reset();
		require(device.snapshot().find("draw pipeline=") == std::string::npos &&
			device.resource_counts().total() == 0,
			"original terrain scene attachment submitted or retained Recording resources");
	}
	map->Release_Ref();
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	TheWritableGlobalData->m_shroudAlpha = saved_shrouded;
	TheWritableGlobalData->m_fogAlpha = saved_fogged;
	TheWritableGlobalData->m_clearAlpha = saved_clear;
	std::puts("original terrain scene attachment: bounds=8x8 registrations=2 shroud=2 display-cells=3 generations=2 resources=0");
}
