#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/MapReaderWriterInfo.h"
#include "GameClient/ClientRandomValue.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "WW3D2/scene.h"
#include "WWLib/RAMFILE.H"
#include "assetmgr.h"
#include "dx8fvf.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
		CachedFileInputStream input;
		require(input.open(AsciiString(path)), "original terrain scene attachment map unreadable");
		WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, FALSE));
		input.close();
		auto display = std::make_unique<W3DDisplay>();
		Display *saved_display = TheDisplay;
		TheDisplay = display.get();
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted pre-bootstrap calls");
		display->init();
		const char *assetPath = std::getenv("ZH_M22_TERRAIN_SCENE_ATTACHMENT_ASSET");
		require(assetPath, "original terrain tree packet path missing");
		std::ifstream assetInput(assetPath, std::ios::binary);
		require(assetInput.good(), "original terrain tree packet unreadable");
		std::vector<char> assetBytes(std::istreambuf_iterator<char>{assetInput},
			std::istreambuf_iterator<char>{});
		require(!assetBytes.empty(), "original terrain tree packet empty");
		RAMFileClass assetPacket(assetBytes.data(), static_cast<int>(assetBytes.size()));
		require(static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager)
			->Load_3D_Assets(assetPacket), "original terrain tree packet rejected");
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
		W3DTreeDrawModuleData tree_data;
		tree_data.m_modelName = "TEST.LITONE01";
		tree_data.m_textureName = "Tree0.tga";
		tree_data.m_framesToMoveOutward = 3;
		W3DTreeDrawModuleData alternate_tree_data;
		alternate_tree_data.m_modelName = "TEST.LITONE01";
		alternate_tree_data.m_textureName = "Tree1.tga";
		Coord3D tree_position;
		tree_position.set(12, 18, 2);
		const auto tree_one = static_cast<DrawableID>(101);
		const auto tree_two = static_cast<DrawableID>(102);
		const auto tree_three = static_cast<DrawableID>(103);
		const auto emptyTreeResources = device.resource_counts();
		UnsignedInt clientBefore[6]{};
		UnsignedInt clientAfter[6]{};
		UnsignedInt freshEquivalentWords[6]{};
		W3DTreeDrawModuleData noUvTree;
		noUvTree.m_modelName = "TEST.ZERO01";
		noUvTree.m_textureName = "Tree0.tga";
		CopyGameClientRandomState(clientBefore);
		require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0.2f, &noUvTree) &&
			terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0 &&
			device.resource_counts() == emptyTreeResources,
			"original tree accepted mesh without required source UVs");
		CopyGameClientRandomState(clientAfter);
		require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
			"original tree no-UV rejection consumed client RNG");
		for (const auto &capacity : {std::pair<const char *, Int>{"TEST.VFITTREE", 29997},
			std::pair<const char *, Int>{"TEST.IFITTREE", 3}}) {
			W3DTreeDrawModuleData fitTree;
			fitTree.m_modelName = capacity.first;
			fitTree.m_textureName = "Tree0.tga";
			require(terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &fitTree) &&
				terrain.peekTreeVertexSource() && terrain.peekTreeIndexSource() &&
				terrain.peekTreeVertexSource()->Get_Vertex_Count() == capacity.second &&
				terrain.peekTreeIndexSource()->Get_Index_Count() ==
					(capacity.second == 3 ? 59991 : 3),
				"original tree rejected valid source vertex/index budget boundary");
			terrain.removeTree(tree_one);
			require(device.resource_counts() == emptyTreeResources &&
				terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
				"original tree valid budget boundary retained resources");
		}
		for (const char *modelName : {"TEST.VCAPTREE", "TEST.ICAPTREE"}) {
			W3DTreeDrawModuleData capTree;
			capTree.m_modelName = modelName;
			capTree.m_textureName = "Tree0.tga";
			CopyGameClientRandomState(clientBefore);
			require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &capTree) &&
				device.resource_counts() == emptyTreeResources &&
				terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
				"original tree exceeded source vertex/index budget");
			CopyGameClientRandomState(clientAfter);
			require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
				"original tree budget rejection consumed client RNG");
		}
		CopyGameClientRandomState(freshEquivalentWords);
		for (const char *fault : {"geometry", "vertex", "index",
			"texture-create", "texture-upload", "texture-publish", "registry"}) {
			CopyGameClientRandomState(clientBefore);
			setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", fault, 1);
			require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0.2f, &tree_data),
				"original tree injected resource fault admitted owner");
			unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
			CopyGameClientRandomState(clientAfter);
			require(terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0 &&
				device.resource_counts() == emptyTreeResources &&
				std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
				"original tree injected failure retained resources or consumed client RNG");
		}
		for (Int failure = 0; failure != 6; ++failure) {
			CopyGameClientRandomState(clientBefore);
			if (failure == 0) device.fail_next_buffer_create();
			if (failure == 1) device.fail_next_buffer_upload();
			if (failure == 2) device.fail_buffer_upload_after(1);
			if (failure == 3) device.fail_next_texture_create();
			if (failure == 4) device.fail_next_texture_upload();
			if (failure == 5) device.fail_buffer_create_after(1);
			require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0.2f, &tree_data),
				"original tree Recording fault admitted owner");
			CopyGameClientRandomState(clientAfter);
			require(terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0 &&
				device.resource_counts() == emptyTreeResources &&
				std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
				"original tree Recording fault retained resource or client RNG");
		}
		require(!terrain.tryAddTree(INVALID_DRAWABLE_ID, tree_position, 1, 0, 0, &tree_data) &&
			!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, NULL) &&
			terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
			"original tree registry accepted invalid admission");
		setenv("ZH_M22_TREE_REGISTRY_FAIL_AT", "type", 1);
		require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data) &&
			terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
			"original tree type failure published owner");
		setenv("ZH_M22_TREE_REGISTRY_FAIL_AT", "instance", 1);
		require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data) &&
			terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
			"original tree instance failure published owner");
		unsetenv("ZH_M22_TREE_REGISTRY_FAIL_AT");
		CopyGameClientRandomState(clientBefore);
		require(std::memcmp(clientBefore, freshEquivalentWords, sizeof(clientBefore)) == 0,
			"original tree failed admissions changed fresh equivalent client generation");
		const Real expectedScale = PreviewGameClientRandomValueReal(freshEquivalentWords, 0.8f, 1.2f);
		const Int expectedSway = PreviewGameClientRandomValue(freshEquivalentWords, 0, 9);
		const bool firstAdmitted = terrain.tryAddTree(tree_one, tree_position, 1, 0, 0.2f, &tree_data);
		require(firstAdmitted,
			"original tree first owner was not published");
		CopyGameClientRandomState(clientAfter);
		require(std::memcmp(freshEquivalentWords, clientAfter, sizeof(clientAfter)) == 0 &&
			terrain.treeAtlasWidth() == 512 && terrain.peekTreeVertexSource() &&
			terrain.peekTreeIndexSource() && terrain.peekTreeAtlasSource(),
			"original tree successful resource or client RNG publication missing");
		const auto afterFirstTree = device.resource_counts();
		const auto *acceptedVertexSource = terrain.peekTreeVertexSource();
		const auto *acceptedIndexSource = terrain.peekTreeIndexSource();
		const auto *acceptedAtlasSource = terrain.peekTreeAtlasSource();
		require(afterFirstTree.buffers == emptyTreeResources.buffers + 2 &&
			afterFirstTree.textures == emptyTreeResources.textures + 1,
			"original tree source resource counts differ from vertex/index/atlas");
		const auto *sourceVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer());
		const auto *sourceIndices = terrain.peekTreeIndexSource()->Get_CPU_Index_Buffer();
		require(terrain.peekTreeVertexSource()->Get_Vertex_Count() == 3 &&
			terrain.peekTreeIndexSource()->Get_Index_Count() == 3 &&
			std::fabs(sourceVertices[0].x - tree_position.x) < 0.001f &&
			std::fabs(sourceVertices[1].x - (tree_position.x + expectedScale)) < 0.001f &&
			std::fabs(sourceVertices[2].y - (tree_position.y + expectedScale)) < 0.001f &&
			sourceVertices[0].nx == expectedSway && sourceVertices[0].ny == 1.0f &&
			sourceVertices[0].nz == tree_position.z &&
			sourceVertices[0].u1 == 0 && sourceVertices[0].v1 == 0.125f &&
			sourceVertices[1].u1 == 0.125f && sourceVertices[1].v1 == 0.125f &&
			sourceVertices[2].v1 == 0 &&
			sourceIndices[0] == 0 && sourceIndices[1] == 1 && sourceIndices[2] == 2,
			"original tree source vertex/index/UV bytes differ from generated mesh");
		const auto vertexHandle = edge.bind_vertex(terrain.peekTreeVertexSource());
		const auto indexHandle = edge.bind_index(terrain.peekTreeIndexSource());
		const auto uploadedVertices = device.buffer_bytes(vertexHandle);
		const auto uploadedIndices = device.buffer_bytes(indexHandle);
		const auto atlasHandle = edge.texture_handle(terrain.peekTreeAtlasSource());
		const auto uploadedAtlas = device.texture_bytes(atlasHandle);
		require(uploadedVertices.size() == 3 * sizeof(VertexFormatXYZNDUV1) &&
			std::memcmp(uploadedVertices.data(), sourceVertices, uploadedVertices.size()) == 0 &&
			uploadedIndices.size() == 3 * sizeof(UnsignedShort) &&
			std::memcmp(uploadedIndices.data(), sourceIndices, uploadedIndices.size()) == 0 &&
			uploadedAtlas.size() == 512u * 512u * 4u &&
			std::memcmp(uploadedAtlas.data(), "\x17\x16\x15\xff", 4) == 0,
			"original tree Recording upload bytes differ from source wrappers/atlas");
		for (const char *fault : {"texture-create", "texture-upload",
			"texture-publish", "registry"}) {
			CopyGameClientRandomState(clientBefore);
			setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", fault, 1);
			require(!terrain.tryAddTree(tree_three, tree_position, 2, 1, 0.2f,
				&alternate_tree_data), "original tree later type fault admitted owner");
			unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
			CopyGameClientRandomState(clientAfter);
			require(terrain.treeTypeCount() == 1 && terrain.treeInstanceCount() == 1 &&
				device.resource_counts() == afterFirstTree &&
				terrain.peekTreeVertexSource() == acceptedVertexSource &&
				terrain.peekTreeIndexSource() == acceptedIndexSource &&
				terrain.peekTreeAtlasSource() == acceptedAtlasSource &&
				std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
				"original tree later fault damaged accepted type/resource/RNG");
		}
		FileSystem *publishedFiles = TheFileSystem;
		TheFileSystem = NULL;
		require(!terrain.tryAddTree(tree_three, tree_position, 2, 1, 0,
			&alternate_tree_data) && device.resource_counts() == afterFirstTree,
			"original tree accepted detached file-system provider");
		TheFileSystem = publishedFiles;
		W3DAssetManager *publishedAssets = W3DDisplay::m_assetManager;
		W3DDisplay::m_assetManager = NULL;
		require(!terrain.tryAddTree(tree_three, tree_position, 2, 1, 0,
			&alternate_tree_data) && device.resource_counts() == afterFirstTree,
			"original tree accepted detached asset provider");
		W3DDisplay::m_assetManager = publishedAssets;
		setenv("ZH_M22_TREE_REGISTRY_FAIL_AT", "instance", 1);
		require(!terrain.tryAddTree(tree_two, tree_position, 1, 0, 0, &tree_data) &&
			terrain.treeTypeCount() == 1 && terrain.treeInstanceCount() == 1,
			"original tree partial add damaged accepted type or instance");
		unsetenv("ZH_M22_TREE_REGISTRY_FAIL_AT");
		require(terrain.tryAddTree(tree_two, tree_position, 1, 0, 0, &tree_data) &&
			terrain.tryAddTree(tree_three, tree_position, 2, 1, 0, &alternate_tree_data) &&
			terrain.treeTypeCount() == 2 && terrain.treeInstanceCount() == 3 &&
			!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data),
			"original tree type/instance admission lost identity");
		const Int first_bucket = terrain.treePartitionBucket(tree_two);
		require(first_bucket >= 0 && terrain.treePartitionBucket(tree_three) == -1,
			"original tree partition admission changed source type policy");
		tree_position.set(24, 30, 3);
		require(terrain.updateTreePosition(tree_two, tree_position, 2) &&
			!terrain.updateTreePosition(static_cast<DrawableID>(999), tree_position, 2) &&
			terrain.treePartitionBucket(tree_two) != first_bucket,
			"original tree relocation accepted a stale ID");
		require(!terrain.peekTreeVertexSource() && !terrain.peekTreeIndexSource() &&
			device.resource_counts().buffers == emptyTreeResources.buffers,
			"original tree relocation retained obsolete geometry buffers");
		const auto tree_four = static_cast<DrawableID>(104);
		require(terrain.tryAddTree(tree_four, tree_position, 1, 0, 0, &tree_data) &&
			terrain.treeTypeCount() == 2 && terrain.treeInstanceCount() == 4 &&
			terrain.peekTreeVertexSource(),
			"original tree relocation retry lost accepted type or owner identity");
		const auto *relocatedVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer());
		require(std::fabs(relocatedVertices[3].x - tree_position.x) < 0.001f &&
			std::fabs(relocatedVertices[3].y - tree_position.y) < 0.001f,
			"original tree relocation retry used stale vertex bytes");
		terrain.removeTree(tree_four);
		terrain.removeTree(tree_one);
		terrain.removeTree(tree_two);
		require(terrain.treeTypeCount() == 1 && terrain.treeInstanceCount() == 1,
			"original tree removal retained source type");
		require(!terrain.peekTreeAtlasSource() && !terrain.peekTreeVertexSource() &&
			terrain.tryAddTree(tree_four, tree_position, 1, 0, 0, &alternate_tree_data) &&
			terrain.treeTypeCount() == 1 && terrain.treeInstanceCount() == 2 &&
			terrain.peekTreeAtlasSource() && terrain.peekTreeVertexSource(),
			"original tree remaining-type retry lost atlas or geometry identity");
		terrain.removeTree(tree_four);
		terrain.removeTree(tree_three);
		require(terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
			"original tree reverse removal retained owner");
		std::array<W3DTreeDrawModuleData, 65> tree_types;
		for (Int index = 0; index < 64; ++index) {
			tree_types[index].m_modelName = "TEST.LITONE01";
			tree_types[index].m_textureName = AsciiString(("Tree" + std::to_string(index) + ".tga").c_str());
			require(terrain.tryAddTree(static_cast<DrawableID>(200 + index), tree_position,
				1, 0, 0, &tree_types[index]), "original tree type budget rejected valid type");
		}
		tree_types[64].m_modelName = "TEST.LITONE01";
		tree_types[64].m_textureName = "Tree64.tga";
		require(!terrain.tryAddTree(static_cast<DrawableID>(264), tree_position, 1, 0, 0,
			&tree_types[64]) && terrain.treeTypeCount() == 64 &&
			terrain.treeInstanceCount() == 64,
			"original tree type budget changed published registry");
		terrain.removeAllTrees();
		for (Int index = 0; index < 4000; ++index)
			require(terrain.tryAddTree(static_cast<DrawableID>(1000 + index), tree_position,
				1, 0, 0, &tree_data), "original tree instance budget rejected valid tree");
		require(!terrain.tryAddTree(static_cast<DrawableID>(5000), tree_position,
			1, 0, 0, &tree_data) && terrain.treeTypeCount() == 1 &&
			terrain.treeInstanceCount() == 4000,
			"original tree instance budget changed published registry");
		terrain.removeAllTrees();
		require(terrain.treeTypeCount() == 0 && terrain.treeInstanceCount() == 0,
			"original tree clear retained capacity owner");
		require(device.resource_counts() == emptyTreeResources &&
			!terrain.peekTreeVertexSource() && !terrain.peekTreeAtlasSource(),
			"original tree clear retained pre-teardown Recording resources");
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
		require(!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data) &&
			terrain.treeInstanceCount() == 0,
			"original tree registry accepted detached terrain");
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
		W3DDisplay::m_3DScene->Add_Render_Object(&terrain);
		require(terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data),
			"original tree reset witness could not admit owner");
		const UnsignedInt old_tree_epoch = terrain.treeOwnerEpoch();
		terrain.reset();
		require(terrain.treeInstanceCount() == 0 && terrain.treeTypeCount() == 0 &&
			terrain.treeOwnerEpoch() != old_tree_epoch &&
			!terrain.tryAddTree(tree_one, tree_position, 1, 0, 0, &tree_data) &&
			device.resource_counts() == baseline,
			"original tree reset retained owner or admitted unready shroud");
		W3DDisplay::m_3DScene->Remove_Render_Object(&terrain);
		terrain.freeMapResources();
		require(rejected([&]() { terrain.notifyShroudChanged(); }),
			"original terrain shroud notification accepted a released map");
		require(rejected([&]() { display->clearShroud(); }) &&
			rejected([&]() { display->setShroudLevel(0, 0, CELLSHROUD_CLEAR); }),
			"original display shroud accepted a released map");
		edge.release_source_buffers();
		map->Release_Ref();
		map = NULL;
		TheDisplay = saved_display;
		display.reset();
		require(device.snapshot().find("draw pipeline=") == std::string::npos &&
			device.resource_counts().total() == 0,
			"original terrain scene attachment submitted or retained Recording resources");
	}
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	TheWritableGlobalData->m_shroudAlpha = saved_shrouded;
	TheWritableGlobalData->m_fogAlpha = saved_fogged;
	TheWritableGlobalData->m_clearAlpha = saved_clear;
	std::puts("original terrain scene attachment: bounds=8x8 registrations=2 shroud=2 display-cells=3 generations=2 resources=0");
}
