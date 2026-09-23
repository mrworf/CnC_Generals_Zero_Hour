#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
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

class TestTerrain final : public HeightMapRenderObjClass
{
public:
	void On_Frame_Update() override { ++updates; }
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
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		TestTerrain terrain;
		require(terrain.initHeightData(map->getDrawWidth(), map->getDrawHeight(), map, NULL, TRUE) == 0,
			"original terrain scene attachment map binding failed");
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

		alternate.Add_Render_Object(&terrain);
		require(terrain.Peek_Scene() == &alternate && alternate.registrations == 1,
			"original terrain update registration missing");
		alternate.Remove_Render_Object(&terrain);
		require(!terrain.Peek_Scene() && alternate.unregistrations == 1,
			"original terrain update unregister missing");
		alternate.Add_Render_Object(&terrain);
		alternate.Remove_Render_Object(&terrain);
		require(alternate.registrations == 2 && alternate.unregistrations == 2 && !terrain.Peek_Scene(),
			"original terrain scene re-entry changed update ownership");
		terrain.freeMapResources();
		edge.release_source_buffers();
		display.reset();
		require(device.snapshot().find("draw pipeline=") == std::string::npos &&
			device.resource_counts().total() == 0,
			"original terrain scene attachment submitted or retained Recording resources");
	}
	map->Release_Ref();
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original terrain scene attachment: bounds=8x8 registrations=2 generations=2 resources=0");
}
