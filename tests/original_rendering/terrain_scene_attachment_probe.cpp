#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/Geometry.h"
#include "Common/GameAudio.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/RandomValue.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/GameClient.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"
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
#include "camera.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
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
	W3DShroud *takeShroudForTest()
	{
		W3DShroud *shroud = m_shroud;
		m_shroud = NULL;
		return shroud;
	}
	void restoreShroudForTest(W3DShroud *shroud) { m_shroud = shroud; }
	Int updates = 0;
};

// The generated logical partition has more cells than its tiny visual map.
// Preserve real PartitionManager shroud transitions while isolating the
// unrelated display-cell notification surface during this focused witness.
class ShroudNotificationSink final : public Display
{
public:
	struct Notice { Int x, y; CellShroudStatus status; };
	std::vector<Notice> notices;
	void doSmartAssetPurgeAndPreload(const char *) override {}
#if defined(_DEBUG) || defined(_INTERNAL)
	void dumpAssetUsage(const char *) override {}
	void dumpModelAssets(const char *) override {}
#endif
	VideoBuffer *createVideoBuffer() override { return NULL; }
	void setClipRegion(IRegion2D *) override {}
	Bool isClippingEnabled() override { return FALSE; }
	void enableClipping(Bool) override {}
	void setTimeOfDay(TimeOfDay) override {}
	void createLightPulse(const Coord3D *, const RGBColor *, Real, Real,
		UnsignedInt, UnsignedInt) override {}
	void drawLine(Int, Int, Int, Int, Real, UnsignedInt) override {}
	void drawLine(Int, Int, Int, Int, Real, UnsignedInt, UnsignedInt) override {}
	void drawOpenRect(Int, Int, Int, Int, Real, UnsignedInt) override {}
	void drawFillRect(Int, Int, Int, Int, UnsignedInt) override {}
	void drawRectClock(Int, Int, Int, Int, Int, UnsignedInt) override {}
	void drawRemainingRectClock(Int, Int, Int, Int, Int, UnsignedInt) override {}
	void drawImage(const Image *, Int, Int, Int, Int, Color, DrawImageMode) override {}
	void drawVideoBuffer(VideoBuffer *, Int, Int, Int, Int) override {}
	void setShroudLevel(Int x, Int y, CellShroudStatus status) override
	{
		notices.push_back({x, y, status});
	}
	void clearShroud() override {}
	void setBorderShroudLevel(UnsignedByte) override {}
	void preloadModelAssets(AsciiString) override {}
	void preloadTextureAssets(AsciiString) override {}
	void takeScreenShot() override {}
	void toggleMovieCapture() override {}
	void toggleLetterBox() override {}
	void enableLetterBox(Bool) override {}
	Real getAverageFPS() override { return 0; }
	Int getLastFrameDrawCalls() override { return 0; }
};

class DisplayOverride final
{
public:
	explicit DisplayOverride(Display *replacement) : m_saved(TheDisplay)
	{
		TheDisplay = replacement;
	}
	~DisplayOverride() { TheDisplay = m_saved; }
private:
	Display *m_saved;
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

// Each fixture FXList owns its distinct pooled edge nugget; child links are
// non-owning, like FXListAtBonePos, so a shared child is never double-freed.
class ProbeFXNugget final : public FXNugget
{
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(ProbeFXNugget, "ProbeFXNugget", 8, 8)
public:
	ProbeFXNugget(const FXList *child, Int *dispatches) : m_child(child), m_dispatches(dispatches) {}
	void doFXPos(const Coord3D *, const Matrix3D *, Real, const Coord3D *, Real) const override
	{ ++*m_dispatches; }
	FXPositionNuggetReadiness cpuPositionReady(const Coord3D *, const Coord3D *) const override
	{ return FXPositionNuggetReadiness::Ready; }
	const FXList *cpuPositionChild() const override { return m_child; }
private:
	const FXList *m_child;
	Int *m_dispatches;
};
EMPTY_DTOR(ProbeFXNugget)

class UnknownFXNugget final : public FXNugget
{
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(UnknownFXNugget, "UnknownFXNugget", 1, 1)
public:
	explicit UnknownFXNugget(Int *dispatches) : m_dispatches(dispatches) {}
	void doFXPos(const Coord3D *, const Matrix3D *, Real, const Coord3D *, Real) const override
	{ ++*m_dispatches; }
private:
	Int *m_dispatches;
};
EMPTY_DTOR(UnknownFXNugget)

struct PositionFXTrace {
	Int token;
	Coord3D position;
};
class DispatchFXNugget final : public FXNugget
{
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(DispatchFXNugget, "DispatchFXNugget", 8, 8)
public:
	DispatchFXNugget(std::vector<PositionFXTrace> *trace, Int token,
		BaseHeightMapRenderObjClass *terrain, DrawableID id, bool bounce,
		bool fail = false, bool remove = false) : m_trace(trace), m_token(token),
		m_terrain(terrain), m_id(id), m_bounce(bounce), m_fail(fail), m_remove(remove) {}
	void doFXPos(const Coord3D *position, const Matrix3D *, Real,
		const Coord3D *, Real) const override
	{
		if (m_terrain) {
			require(m_terrain->peekTreeVertexSource() &&
				(m_bounce ? m_terrain->treeConsumedBounceEvents(m_id) :
					m_terrain->treeConsumedToppleStartEvents(m_id)) == 1,
				"tree FX dispatched before accepted geometry/consumed marker");
		}
		m_trace->push_back({m_token, *position});
		(void)GameClientRandomValue(0, 100); // Explicit external-effect randomness.
		if (m_remove) m_terrain->removeTree(m_id);
		if (m_fail) throw std::runtime_error("generated position FX dispatch failure");
	}
	FXPositionNuggetReadiness cpuPositionReady(const Coord3D *, const Coord3D *) const override
	{ return FXPositionNuggetReadiness::Ready; }
private:
	std::vector<PositionFXTrace> *m_trace;
	Int m_token;
	BaseHeightMapRenderObjClass *m_terrain;
	DrawableID m_id;
	bool m_bounce, m_fail, m_remove;
};
EMPTY_DTOR(DispatchFXNugget)

void checkPositionFXAdmission(const Coord3D &position,
	const zh::renderer::RecordingGpuDevice &device)
{
	using Result = FXPositionAdmission;
	UnsignedInt clientBefore[6]{}, clientAfter[6]{};
	UnsignedInt logicBefore[6]{}, logicAfter[6]{};
	UnsignedInt logicSeedBefore = 0, logicSeedAfter = 0;
	CopyGameClientRandomState(clientBefore);
	CopyGameLogicRandomState(&logicSeedBefore, logicBefore);
	const UnsignedInt frameBefore = TheGameLogic->getFrame();
	const auto gpuBefore = device.resource_counts();
	AudioManager *audioBefore = TheAudio;
	require(FXList::preflightPositionDispatch(NULL, NULL) == Result::Ready,
		"null position FX list rejected");
	require(TheFXListStore && ThePlayerList && ThePartitionManager &&
		ThePlayerList->peekLocalPlayerForPositionFX(),
		"initialized local-player position FX provider missing");
	const FXList *empty = TheFXListStore->findFXList("FixtureEmpty");
	const FXList *view = TheFXListStore->findFXList("FixtureView");
	const FXList *viewTwo = TheFXListStore->findFXList("FixtureViewTwo");
	const FXList *lateLight = TheFXListStore->findFXList("FixtureLateLight");
	const FXList *scorch = TheFXListStore->findFXList("FixtureScorch");
	const FXList *particle = TheFXListStore->findFXList("FixtureParticle");
	const FXList *tracer = TheFXListStore->findFXList("FixtureTracer");
	const FXList *ray = TheFXListStore->findFXList("FixtureRay");
	const FXList *missingTracer = TheFXListStore->findFXList("FixtureMissingTracer");
	const FXList *missingRay = TheFXListStore->findFXList("FixtureMissingRay");
	const FXList *bone = TheFXListStore->findFXList("FixtureBone");
	const FXList *boneSelf = TheFXListStore->findFXList("FixtureBoneSelf");
	require(empty && view && viewTwo && lateLight && scorch && particle &&
		tracer && ray && missingTracer && missingRay && bone && boneSelf,
		"generated position FX list definitions missing");
	require(FXList::preflightPositionDispatch(empty, &position) == Result::Ready &&
		FXList::preflightPositionDispatch(view, &position) == Result::Ready &&
		FXList::preflightPositionDispatch(viewTwo, &position) == Result::Ready,
		"empty or supported position FX list rejected");
	require(FXList::preflightPositionDispatch(view, NULL) == Result::InvalidInput,
		"null position FX coordinate accepted");
	Coord3D invalid = position;
	invalid.x = std::numeric_limits<Real>::infinity();
	require(FXList::preflightPositionDispatch(view, &invalid) == Result::InvalidInput,
		"nonfinite position FX coordinate accepted");
	require(FXList::preflightPositionDispatch(view, &position, &invalid) == Result::InvalidInput,
		"nonfinite secondary position FX coordinate accepted");
	FXList foreign;
	require(FXList::preflightPositionDispatch(&foreign, &position) == Result::ForeignList,
		"non-store position FX root accepted");
	PlayerList *players = ThePlayerList;
	ThePlayerList = NULL;
	const Result noPlayers = FXList::preflightPositionDispatch(view, &position);
	ThePlayerList = players;
	require(noPlayers == Result::MissingProvider,
		"fresh-null player-list position FX provider accepted");
	require(FXList::preflightPositionDispatch(view, &position) == Result::Ready,
		"initialized local-player position FX retry rejected");
	require(FXList::preflightPositionDispatch(
		TheFXListStore->findFXList("FixtureSound"), &position) == Result::Ready &&
		FXList::preflightPositionDispatch(scorch, &position) == Result::Ready,
		"available positional audio or scorch provider rejected");
	require(FXList::preflightPositionDispatch(lateLight, &position) == Result::UnsupportedNugget &&
		FXList::preflightPositionDispatch(bone, &position) == Result::UnsupportedNugget &&
		FXList::preflightPositionDispatch(boneSelf, &position) == Result::Cycle &&
		FXList::preflightPositionDispatch(tracer, &position) == Result::UnsupportedNugget &&
		FXList::preflightPositionDispatch(ray, &position) == Result::UnsupportedNugget,
		"late light pulse or recursive object-only FX admission changed");
	require(TheThingFactory, "position FX template factory missing");
	const size_t templateCount = TheThingFactory->existingTemplateCountForPositionFX();
	const ThingTemplate *existingTemplate = TheThingFactory->findTemplate(AsciiString("EnemyFixture"), FALSE);
	require(existingTemplate, "generated existing position FX template missing");
	require(FXList::preflightPositionDispatch(tracer, &position, &position) == Result::Ready &&
		FXList::preflightPositionDispatch(ray, &position, &position) == Result::Ready &&
		FXList::preflightPositionDispatch(missingTracer, &position, &position) == Result::MissingProvider &&
		FXList::preflightPositionDispatch(missingRay, &position, &position) == Result::MissingProvider &&
		FXList::preflightPositionDispatch(particle, &position) == Result::MissingProvider,
		"positional FX template admission changed");
	require(TheThingFactory->existingTemplateCountForPositionFX() == templateCount &&
		TheThingFactory->findTemplate(AsciiString("EnemyFixture"), FALSE) == existingTemplate &&
		!TheThingFactory->hasExistingTemplateForPositionFX(AsciiString("GenericTracer")),
		"position FX preflight mutated template factory state");
	FXListStore *store = TheFXListStore;
	TheFXListStore = NULL;
	const Result noStore = FXList::preflightPositionDispatch(view, &position);
	TheFXListStore = store;
	require(noStore == Result::MissingProvider, "missing FX store accepted");
	PartitionManager *partition = ThePartitionManager;
	ThePartitionManager = NULL;
	const Result noPartition = FXList::preflightPositionDispatch(view, &position);
	ThePartitionManager = partition;
	require(noPartition == Result::MissingProvider, "missing partition accepted");
	AudioManager *audio = TheAudio;
	TheAudio = NULL;
	const Result noAudio = FXList::preflightPositionDispatch(
		TheFXListStore->findFXList("FixtureSound"), &position);
	TheAudio = audio;
	require(noAudio == Result::MissingProvider, "missing audio accepted");
	GameClient *client = TheGameClient;
	TheGameClient = NULL;
	const Result noClient = FXList::preflightPositionDispatch(scorch, &position);
	TheGameClient = client;
	require(noClient == Result::MissingProvider, "missing game client accepted");
	Int dispatches = 0;
	FXList *graph[66]{};
	for (Int index = 0; index != 66; ++index) {
		const std::string name = "FixtureGraph" + std::to_string(index);
		graph[index] = const_cast<FXList *>(TheFXListStore->findFXList(name.c_str()));
		require(graph[index], "generated position FX graph node missing");
	}
	graph[0]->addFXNugget(NULL);
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::UnsupportedNugget,
		"null position FX nugget admitted");
	graph[0]->clear();
	graph[0]->addFXNugget(newInstance(UnknownFXNugget)(&dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::UnsupportedNugget,
		"unknown position FX nugget admitted");
	graph[0]->clear();
	graph[0]->addFXNugget(newInstance(ProbeFXNugget)(graph[1], &dispatches));
	graph[0]->addFXNugget(newInstance(ProbeFXNugget)(graph[1], &dispatches));
	const Result dag = FXList::preflightPositionDispatch(graph[0], &position);
	require(dag == Result::Ready &&
		dispatches == 0, "shared position FX DAG dispatched or rejected");
	graph[1]->addFXNugget(newInstance(ProbeFXNugget)(graph[0], &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::Cycle &&
		dispatches == 0, "mutual position FX cycle dispatched or admitted");
	graph[1]->clear();
	graph[0]->clear();
	graph[0]->addFXNugget(newInstance(ProbeFXNugget)(
		reinterpret_cast<const FXList *>(static_cast<uintptr_t>(1)), &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::ForeignList,
		"stale position FX child dereferenced or admitted");
	graph[0]->clear();
	graph[0]->addFXNugget(newInstance(ProbeFXNugget)(graph[0], &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::Cycle,
		"self position FX cycle admitted");
	graph[0]->clear();
	graph[0]->addFXNugget(newInstance(ProbeFXNugget)(&foreign, &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::ForeignList,
		"non-store position FX child admitted");
	graph[0]->clear();
	for (Int index = 0; index != 16; ++index)
		graph[index]->addFXNugget(newInstance(ProbeFXNugget)(graph[index + 1], &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::Bounds,
		"position FX depth bound not enforced");
	for (Int index = 0; index != 16; ++index) graph[index]->clear();
	for (Int index = 1; index != 66; ++index)
		graph[0]->addFXNugget(newInstance(ProbeFXNugget)(graph[index], &dispatches));
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::Bounds &&
		dispatches == 0, "position FX node bound dispatched or not enforced");
	graph[0]->clear();
	require(FXList::preflightPositionDispatch(graph[0], &position) == Result::Ready,
		"position FX graph retry rejected");
	CopyGameClientRandomState(clientAfter);
	CopyGameLogicRandomState(&logicSeedAfter, logicAfter);
	require(dispatches == 0 &&
		std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0 &&
		logicSeedBefore == logicSeedAfter &&
		std::memcmp(logicBefore, logicAfter, sizeof(logicBefore)) == 0 &&
		TheGameLogic->getFrame() == frameBefore &&
		device.resource_counts() == gpuBefore && TheAudio == audioBefore,
		"position FX preflight consumed dispatch, RNG, frame, GPU or audio owner");
}
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
		checkPositionFXAdmission(tree_position, device);
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
		// C1A: the source's breeze/cull phase publishes only complete visible
		// geometry. Its failed candidate must leave the accepted B3 registry,
		// Recording wrappers and GameClient stream untouched.
		const auto frame_one = static_cast<DrawableID>(6010);
		const auto frame_two = static_cast<DrawableID>(6011);
		Coord3D second_position;
		second_position.set(24, 30, 5);
		require(terrain.tryAddTree(frame_one, tree_position, 1, 0, 0, &tree_data) &&
			terrain.tryAddTree(frame_two, second_position, 1, 0, 0, &alternate_tree_data),
			"original tree visible-frame fixture admission failed");
		const auto acceptedFrameResources = device.resource_counts();
		const auto acceptedFrameVertex = terrain.peekTreeVertexSource();
		const auto acceptedFrameIndex = terrain.peekTreeIndexSource();
		const auto acceptedFrameEpoch = terrain.treeOwnerEpoch();
		CameraClass frameCamera;
		frameCamera.Set_Position(Vector3(18, 24, 50));
		frameCamera.Set_View_Plane(Vector2(-1, -1), Vector2(1, 1));
		frameCamera.Set_Clip_Planes(1, 100);
		BreezeInfo breeze{};
		breeze.m_directionVec.x = 1;
		breeze.m_directionVec.y = 0;
		breeze.m_intensity = 0.2f;
		breeze.m_lean = 0.1f;
		breeze.m_randomness = 0.4f;
		breeze.m_breezePeriod = 60;
		breeze.m_breezeVersion = 1;
		const auto frameUnchanged = [&]() {
			CopyGameClientRandomState(clientAfter);
			return terrain.treeOwnerEpoch() == acceptedFrameEpoch &&
				terrain.treeInstanceCount() == 2 && terrain.treeTypeCount() == 2 &&
				terrain.treeVisibleCount() == 0 && terrain.treeSwayVersion() == -1 &&
				terrain.peekTreeVertexSource() == acceptedFrameVertex &&
				terrain.peekTreeIndexSource() == acceptedFrameIndex &&
				device.resource_counts() == acceptedFrameResources &&
				std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0;
		};
		CopyGameClientRandomState(clientBefore);
		UnsignedInt expectedFrameWords[6]{};
		std::memcpy(expectedFrameWords, clientBefore, sizeof(expectedFrameWords));
		Int expectedSwaySlots[2]{};
		for (Int index = 0; index != 2; ++index)
			expectedSwaySlots[index] = 1 +
				PreviewGameClientRandomValue(expectedFrameWords, 0, 9);
		for (Int index = 0; index != 10; ++index) {
			(void)PreviewGameClientRandomValueReal(expectedFrameWords, 0.8f, 1.2f);
			(void)PreviewGameClientRandomValueReal(expectedFrameWords, 0.8f, 1.2f);
		}
		require(!terrain.updateTreeVisibleFrame(NULL, breeze, FALSE) &&
			frameUnchanged(), "original tree frame accepted null camera");
		BreezeInfo invalidBreeze = breeze;
		invalidBreeze.m_breezePeriod = 0;
		require(!terrain.updateTreeVisibleFrame(&frameCamera, invalidBreeze, FALSE) &&
			frameUnchanged(), "original tree frame accepted zero breeze period");
		for (const char *fault : {"edge-mismatch", "cull",
			"sort-key-nonfinite", "publish"}) {
			setenv("ZH_M22_TREE_FRAME_FAIL_AT", fault, 1);
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE),
				"original tree frame injected fault published candidate");
			unsetenv("ZH_M22_TREE_FRAME_FAIL_AT");
			require(frameUnchanged(),
				"original tree frame injected fault changed accepted owner or RNG");
		}
		for (const char *fault : {"geometry", "vertex", "index"}) {
			setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", fault, 1);
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE),
				"original tree visible geometry fault published candidate");
			unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
			require(frameUnchanged(),
				"original tree visible geometry fault retained candidate");
		}
		for (Int failure = 0; failure != 2; ++failure) {
			if (failure == 0) device.fail_next_buffer_create();
			if (failure == 1) device.fail_next_buffer_upload();
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				frameUnchanged(),
				"original tree visible Recording fault retained candidate");
		}
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeVisibleCount() == 2 &&
			terrain.treeIsVisible(frame_one) && terrain.treeIsVisible(frame_two) &&
			terrain.treeSwayVersion() == 1 &&
			terrain.peekTreeVertexSource()->Get_Vertex_Count() == 6 &&
			terrain.peekTreeIndexSource()->Get_Index_Count() == 6 &&
			terrain.treeOwnerEpoch() == acceptedFrameEpoch,
			"original tree visible-frame retry lost source geometry or identity");
		const auto *frameVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer());
		CopyGameClientRandomState(clientAfter);
		require(frameVertices[0].nx == expectedSwaySlots[0] &&
			frameVertices[3].nx == expectedSwaySlots[1] &&
			std::memcmp(expectedFrameWords, clientAfter, sizeof(clientAfter)) == 0 &&
			std::isfinite(terrain.treeSwayVector(0).X) &&
			terrain.treeSwayVector(0).X > 0,
			"original tree breeze version changed source sway or client RNG sequence");
		const auto readyFrameResources = device.resource_counts();
		W3DAssetManager *frameAssets = W3DDisplay::m_assetManager;
		W3DDisplay::m_assetManager = NULL;
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			device.resource_counts() == readyFrameResources &&
			terrain.treeVisibleCount() == 2 &&
			terrain.treeSwayVersion() == 1,
			"original tree visible frame accepted missing asset provider");
		W3DDisplay::m_assetManager = frameAssets;
		const Vector3 sampledBeforePause = terrain.treeSwayVector(0);
		CopyGameClientRandomState(clientBefore);
		breeze.m_breezeVersion = 2;
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeSwayVersion() == 1 &&
			terrain.treeSwayVector(0).X == sampledBeforePause.X,
			"original tree paused frame changed breeze version");
		CopyGameClientRandomState(clientAfter);
		require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0 &&
			terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeSwayVersion() == 2,
			"original tree resumed frame failed breeze-version update");
		frameCamera.Set_Position(Vector3(1000, 1000, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeVisibleCount() == 0 && !terrain.treeIsVisible(frame_one) &&
			!terrain.peekTreeVertexSource() && !terrain.peekTreeIndexSource(),
			"original tree camera cull retained offscreen frame geometry");
		frameCamera.Set_Position(Vector3(18, 24, 50));
		const bool frameReentered = terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE);
		require(frameReentered &&
			terrain.treeVisibleCount() == 2 &&
			terrain.treeSortKey(frame_one) != terrain.treeSortKey(frame_two),
			"original tree camera re-entry lost visible identity or sort keys");
		Coord3D moved_frame_position;
		moved_frame_position.set(30, 20, 7);
		require(terrain.updateTreePosition(frame_one, moved_frame_position, 0) &&
			terrain.treeVisibleCount() == 0 &&
			!terrain.peekTreeVertexSource() &&
			terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeVisibleCount() == 2 &&
			terrain.treeSortKey(frame_one) != terrain.treeSortKey(frame_two),
			"original tree moved frame did not rebuild visible source geometry");
		terrain.removeTree(frame_two);
		require(terrain.treeInstanceCount() == 1 &&
			terrain.treeTypeCount() == 1 && !terrain.peekTreeAtlasSource() &&
			!terrain.peekTreeVertexSource(),
			"original tree type removal retained obsolete frame resources");
		const auto oneTreeResources = device.resource_counts();
		setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", "texture-create", 1);
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE),
			"original tree removal-frame atlas fault published candidate");
		unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
		require(device.resource_counts() == oneTreeResources &&
			terrain.treeInstanceCount() == 1 &&
			terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeVisibleCount() == 1 && terrain.peekTreeAtlasSource() &&
			terrain.peekTreeVertexSource(),
			"original tree removal-frame atlas retry lost accepted survivor");
		terrain.removeTree(frame_one);
		require(terrain.treeInstanceCount() == 0 &&
			terrain.treeVisibleCount() == 0 && device.resource_counts() == baseline,
			"original tree frame owner removal retained Recording resources");
		require(terrain.tryAddTree(frame_one, tree_position, 1, 0, 0, &tree_data),
			"original tree paused-first fixture admission failed");
		CopyGameClientRandomState(clientBefore);
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeVisibleCount() == 1 &&
			terrain.treeSwayVersion() == -1 &&
			terrain.treeSwayVector(0).X == 0 &&
			terrain.treeSwayVector(0).Y == 0 &&
			terrain.treeSwayVector(0).Z == 0,
			"original tree paused first frame sampled uninitialized sway state");
		CopyGameClientRandomState(clientAfter);
		require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
			"original tree paused first frame consumed client RNG");
		terrain.removeTree(frame_one);
		require(device.resource_counts() == baseline,
			"original tree paused first-frame removal retained resources");
		Object *pushUnit = NULL;
		Object *immobileUnit = NULL;
		for (Object *object = TheGameLogic->getFirstObject(); object;
			object = object->getNextObject()) {
			if (object->getTemplate() &&
				object->getTemplate()->getName() == "LogicFixture") pushUnit = object;
			if (object->getTemplate() &&
				object->getTemplate()->getName() == "EnemyFixture") immobileUnit = object;
		}
		require(pushUnit && immobileUnit && TheGameClient &&
			pushUnit->isKindOf(KINDOF_VEHICLE) &&
			immobileUnit->isKindOf(KINDOF_IMMOBILE),
			"original tree push fixture missing generated mobile/immobile units");
		const GeometryInfo originalPushGeometry = pushUnit->getGeometryInfo();
		pushUnit->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER, FALSE, 2, 4, 4));
		W3DTreeDrawModuleData pushData;
		pushData.m_modelName = "TEST.PUSHTREE";
		pushData.m_textureName = "Tree0.tga";
		pushData.m_framesToMoveOutward = 4;
		pushData.m_framesToMoveInward = 2;
		pushData.m_maxOutwardMovement = 2;
		pushData.m_darkening = 0.25f;
		const DrawableID pushTree = static_cast<DrawableID>(6020);
		const Coord3D originalPushPosition = *pushUnit->getPosition();
		Coord3D pushPosition = originalPushPosition;
		pushPosition.x += 2;
		require(terrain.tryAddTree(pushTree, pushPosition, 1, 0, 0, &pushData) &&
			terrain.treePartitionBucket(pushTree) >= 0,
			"original tree push fixture admission or partition missing");
		frameCamera.Set_Position(Vector3(pushPosition.x, pushPosition.y, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeVisibleCount() == 1 &&
			terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0 &&
			terrain.treePushSource(pushTree) == INVALID_ID,
			"original tree push fixture did not start upright");
		const auto pushReadyResources = device.resource_counts();
		const auto *pushReadyVertex = terrain.peekTreeVertexSource();
		require(pushReadyVertex && rejected([&] { terrain.unitMoved(NULL); }) &&
			terrain.treePushDelta(pushTree) == 0 &&
			device.resource_counts() == pushReadyResources,
			"original tree push accepted missing unit or changed accepted resources");
		pushUnit->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER, FALSE, 2, 0, 0));
		require(rejected([&] { terrain.unitMoved(pushUnit); }) &&
			terrain.treePushDelta(pushTree) == 0 &&
			device.resource_counts() == pushReadyResources,
			"original tree push accepted zero collision radius or changed resources");
		pushUnit->setGeometryInfo(GeometryInfo(GEOMETRY_CYLINDER, FALSE, 2, 4, 4));
		const Coord3D originalImmobilePosition = *immobileUnit->getPosition();
		immobileUnit->setPosition(&pushPosition);
		terrain.unitMoved(immobileUnit);
		immobileUnit->setPosition(&originalImmobilePosition);
		require(terrain.treePushDelta(pushTree) == 0 &&
			device.resource_counts() == pushReadyResources,
			"original tree immobile collision changed push state or resources");
		const auto *pushBaseVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			pushReadyVertex->Get_CPU_Vertex_Buffer());
		const Real pushBaseX = pushBaseVertices[2].x;
		const Real pushBaseY = pushBaseVertices[2].y;
		CopyGameClientRandomState(clientBefore);
		setenv("ZH_M22_TREE_PUSH_FAIL_AT", "publish", 1);
		require(rejected([&] { terrain.unitMoved(pushUnit); }),
			"original tree push injected publication accepted candidate");
		unsetenv("ZH_M22_TREE_PUSH_FAIL_AT");
		CopyGameClientRandomState(clientAfter);
		require(terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0 &&
			terrain.treePushSource(pushTree) == INVALID_ID &&
			terrain.peekTreeVertexSource() == pushReadyVertex &&
			device.resource_counts() == pushReadyResources &&
			std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
			"original tree push rejected candidate changed state, GPU or RNG");
		pushUnit->setPosition(&pushPosition);
		require(terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0.25f &&
			terrain.treePushSource(pushTree) == pushUnit->getID(),
			"original tree mobile-unit route did not publish push candidate");
		TheGameClient->notifyTerrainObjectMoved(pushUnit);
		require(terrain.treePushDelta(pushTree) == 0.25f,
			"original tree repeated pusher changed outward step");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0.25f,
			"original tree paused push frame advanced motion");
		setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", "geometry", 1);
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0.25f &&
			device.resource_counts() == pushReadyResources,
			"original tree pushed geometry fault consumed candidate state");
		unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
		device.fail_next_buffer_upload();
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0.25f &&
			terrain.peekTreeVertexSource() == pushReadyVertex &&
			device.resource_counts() == pushReadyResources,
			"original tree pushed frame upload fault consumed candidate state");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treePushAside(pushTree) == 0.25f &&
			terrain.treePushDelta(pushTree) == 0.25f,
			"original tree pushed frame retry lost outward step");
		const auto *pushedVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer());
		const Coord3D *pushDirection = pushUnit->getUnitDirectionVector2D();
		const Real relativeX = pushPosition.x - originalPushPosition.x;
		const Real relativeY = pushPosition.y - originalPushPosition.y;
		const bool leftSide = pushDirection->x * relativeY -
			pushDirection->y * relativeX > 0;
		const Real pushCos = leftSide ? -pushDirection->y : pushDirection->y;
		const Real pushSin = leftSide ? pushDirection->x : -pushDirection->x;
		require(pushedVertices[0].ny == 0.9375f &&
			std::fabs(pushedVertices[2].x - (pushBaseX + 0.5f * pushCos)) < 0.001f &&
			std::fabs(pushedVertices[2].y - (pushBaseY + 0.5f * pushSin)) < 0.001f,
			"original tree push displacement/darkening differs from source vertex slots");
		for (Int step = 0; step != 5; ++step)
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE),
				"original tree push outward/inward advance rejected valid frame");
		require(terrain.treePushAside(pushTree) == 0 &&
			terrain.treePushDelta(pushTree) == 0 &&
			terrain.treePushSource(pushTree) == pushUnit->getID(),
			"original tree push failed to return inward to upright");
		Coord3D edgePushPosition = pushPosition;
		edgePushPosition.x = 0.5f;
		edgePushPosition.y = 0.5f;
		require(terrain.updateTreePosition(pushTree, edgePushPosition, 0) &&
			terrain.treePartitionBucket(pushTree) == 0,
			"original tree push relocation left stale partition bucket");
		terrain.removeTree(pushTree);
		require(device.resource_counts() == baseline,
			"original tree relocated push removal retained resources");
		const DrawableID edgePushTree = static_cast<DrawableID>(6021);
		require(terrain.tryAddTree(edgePushTree, edgePushPosition, 1, 0, 0,
			&pushData) && terrain.treePartitionBucket(edgePushTree) == 0,
			"original tree edge-bucket fixture admission failed");
		pushUnit->setGeometryInfo(GeometryInfo(GEOMETRY_BOX, FALSE, 2, 4, 1));
		pushUnit->setPosition(&edgePushPosition);
		require(terrain.treePushDelta(edgePushTree) == 0.25f &&
			terrain.treePushSource(edgePushTree) == pushUnit->getID(),
			"original tree edge-bucket collision missed moved unit");
		terrain.removeTree(edgePushTree);
		pushUnit->setPosition(&originalPushPosition);
		pushUnit->setGeometryInfo(originalPushGeometry);
		require(device.resource_counts() == baseline &&
			terrain.treePartitionBucket(edgePushTree) == -1,
			"original tree push removal retained resources or partition entry");
		W3DTreeDrawModuleData invalidPushData;
		invalidPushData.m_modelName = "TEST.PUSHTREE";
		invalidPushData.m_textureName = "Tree0.tga";
		invalidPushData.m_framesToMoveOutward = 4;
		invalidPushData.m_framesToMoveInward = 0;
		require(terrain.tryAddTree(pushTree, originalPushPosition, 1, 0, 0,
			&invalidPushData),
			"original tree invalid-push fixture was not staged for collision test");
		const auto invalidPushResources = device.resource_counts();
		require(rejected([&] { terrain.unitMoved(pushUnit); }) &&
			terrain.treePushDelta(pushTree) == 0 &&
			terrain.treePushSource(pushTree) == INVALID_ID &&
			device.resource_counts() == invalidPushResources,
			"original tree accepted zero inward frames or changed accepted owner");
		terrain.removeTree(pushTree);
		require(device.resource_counts() == baseline,
			"original tree invalid-push removal retained resources");
		require(pushUnit->getCrusherLevel() > 1 && ThePartitionManager &&
			ThePlayerList && ThePlayerList->getLocalPlayer(),
			"original tree crusher fixture missing generated gameplay owner");
		W3DTreeDrawModuleData toppleData;
		toppleData.m_modelName = "TEST.PUSHTREE";
		toppleData.m_textureName = "Tree0.tga";
		toppleData.m_doTopple = TRUE;
		toppleData.m_killWhenToppled = FALSE; // Sink belongs to B2B.
		const DrawableID toppleTree = static_cast<DrawableID>(6022);
		Coord3D toppleUnitPosition = originalPushPosition;
		toppleUnitPosition.x += 3;
		Coord3D toppleTreePosition = toppleUnitPosition;
		toppleTreePosition.x += 1;
		require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0,
			&toppleData) && terrain.treePartitionBucket(toppleTree) >= 0 &&
			terrain.treeToppleState(toppleTree) == 0,
			"original tree crusher candidate fixture admission failed");
		frameCamera.Set_Position(Vector3(toppleTreePosition.x,
			toppleTreePosition.y, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeVisibleCount() == 1,
			"original tree crusher fixture initial visible frame failed");
		const auto toppleReadyResources = device.resource_counts();
		const auto *toppleReadyVertex = terrain.peekTreeVertexSource();
		require(toppleReadyVertex, "original tree crusher fixture vertex owner missing");
		const auto *toppleBaseVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			toppleReadyVertex->Get_CPU_Vertex_Buffer());
		const Real toppleBaseX = toppleBaseVertices[2].x;
		const Real toppleBaseZ = toppleBaseVertices[2].z;
		PartitionManager *savedPartitionManager = ThePartitionManager;
		ThePartitionManager = NULL;
		const bool missingTopplePartition = rejected([&] { terrain.unitMoved(pushUnit); });
		ThePartitionManager = savedPartitionManager;
		PlayerList *savedPlayerList = ThePlayerList;
		ThePlayerList = NULL;
		const bool missingTopplePlayer = rejected([&] { terrain.unitMoved(pushUnit); });
		ThePlayerList = savedPlayerList;
		W3DShroud *savedTreeShroud = terrain.takeShroudForTest();
		const bool missingToppleShroud = rejected([&] { terrain.unitMoved(pushUnit); });
		terrain.restoreShroudForTest(savedTreeShroud);
		terrain.unitMoved(immobileUnit);
		require(missingTopplePartition && missingTopplePlayer &&
			missingToppleShroud &&
			terrain.treeToppleState(toppleTree) == 0 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == toppleReadyVertex &&
			device.resource_counts() == toppleReadyResources,
			"original tree crusher missing provider changed accepted owner");
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.treeToppleState(toppleTree) == 1 &&
			terrain.treeToppleStartEvents(toppleTree) == 1 &&
			terrain.treeToppleAngle(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == toppleReadyVertex,
			"original tree crusher public movement did not stage falling state");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeToppleAngle(toppleTree) == 0,
			"original tree paused crusher frame advanced angle");
		setenv("ZH_M22_TREE_TOPPLE_FAIL_AT", "state", 1);
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleAngle(toppleTree) == 0 &&
			device.resource_counts() == toppleReadyResources,
			"original tree crusher injected state fault consumed angle/resources");
		unsetenv("ZH_M22_TREE_TOPPLE_FAIL_AT");
		setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", "geometry", 1);
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleAngle(toppleTree) == 0 &&
			device.resource_counts() == toppleReadyResources,
			"original tree crusher geometry fault consumed angle/resources");
		unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
		device.fail_next_buffer_upload();
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleAngle(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == toppleReadyVertex &&
			device.resource_counts() == toppleReadyResources,
			"original tree crusher Recording fault consumed accepted frame");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleState(toppleTree) == 1 &&
			std::fabs(terrain.treeToppleAngle(toppleTree) - 0.1f) < 0.0001f,
			"original tree crusher retry lost minimum-speed angular step");
		const auto *toppleVertices = reinterpret_cast<const VertexFormatXYZNDUV1 *>(
			terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer());
		require(std::fabs(toppleVertices[2].x -
			(toppleBaseX + std::sin(0.1f))) < 0.001f &&
			std::fabs(toppleVertices[2].z -
			(toppleBaseZ + std::cos(0.1f) - 1.0f)) < 0.001f,
			"original tree crusher transformed raised vertex differs from native matrix");
		frameCamera.Set_Position(Vector3(1000, 1000, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeVisibleCount() == 0 && !terrain.treeIsVisible(toppleTree) &&
			!terrain.peekTreeVertexSource() && !terrain.peekTreeIndexSource() &&
			std::fabs(terrain.treeToppleAngle(toppleTree) - 0.205f) < 0.0001f &&
			std::fabs(terrain.treeToppleVelocity(toppleTree) - 0.11f) < 0.0001f,
			"original tree hidden frame lost accelerated topple state or retained geometry");
		frameCamera.Set_Position(Vector3(toppleTreePosition.x,
			toppleTreePosition.y, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeVisibleCount() == 1 && terrain.treeIsVisible(toppleTree) &&
			terrain.peekTreeVertexSource() &&
			std::fabs(terrain.treeToppleAngle(toppleTree) - 0.205f) < 0.0001f,
			"original tree hidden-frame reentry lost topple identity or paused angle");
		Player *topplePlayer = ThePlayerList->getLocalPlayer();
		const Int topplePlayerIndex = topplePlayer->getPlayerIndex();
		const PlayerMaskType topplePlayerMask = topplePlayer->getPlayerMask();
		const Real toppleRevealRadius = ThePartitionManager->getCellSize() * 2;
		const Int shroudCellsX = ThePartitionManager->getCellCountX();
		const Int shroudCellsY = ThePartitionManager->getCellCountY();
		std::vector<CellShroudStatus> originalShroudCells;
		originalShroudCells.reserve(shroudCellsX * shroudCellsY);
		for (Int y = 0; y != shroudCellsY; ++y)
			for (Int x = 0; x != shroudCellsX; ++x)
				originalShroudCells.push_back(ThePartitionManager->getShroudStatusForPlayer(
					topplePlayerIndex, x, y));
		ShroudNotificationSink shroudNotices;
		std::size_t firstRevealCount = 0;
		{
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->doShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
			firstRevealCount = shroudNotices.notices.size();
			ThePartitionManager->undoShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
		}
		bool matchedShroudNotices = firstRevealCount > 0 &&
			shroudNotices.notices.size() == firstRevealCount * 2;
		for (std::size_t index = 0; matchedShroudNotices && index < firstRevealCount;
			++index) {
			const auto &reveal = shroudNotices.notices[index];
			const auto &undo = shroudNotices.notices[index + firstRevealCount];
			matchedShroudNotices = reveal.status == CELLSHROUD_CLEAR &&
				undo.status == CELLSHROUD_FOGGED &&
				reveal.x == undo.x && reveal.y == undo.y;
		}
		require(matchedShroudNotices && TheDisplay == display.get(),
			"original tree crusher shroud adapter lost reveal/undo notifications");
		require(ThePartitionManager->getPropShroudStatusForPlayer(topplePlayerIndex,
			&toppleTreePosition) == OBJECTSHROUD_FOGGED,
			"original tree crusher fixture failed to establish real fog status");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleState(toppleTree) == 2 &&
			std::fabs(terrain.treeToppleAngle(toppleTree) - 0.205f) < 0.0001f &&
			terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleState(toppleTree) == 2,
			"original tree crusher fogged frames advanced angular state");
		{
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->doShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
		}
		bool secondRevealMatched = TheDisplay == display.get() &&
			shroudNotices.notices.size() == firstRevealCount * 3;
		for (std::size_t index = 0; secondRevealMatched && index < firstRevealCount;
			++index) {
			const auto &first = shroudNotices.notices[index];
			const auto &second = shroudNotices.notices[index + firstRevealCount * 2];
			secondRevealMatched = second.status == CELLSHROUD_CLEAR &&
				second.x == first.x && second.y == first.y;
		}
		require(secondRevealMatched,
			"original tree crusher reveal notification/owner restoration changed");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeToppleState(toppleTree) == 4 &&
			terrain.treeToppleVelocity(toppleTree) == 0,
			"original tree crusher reveal did not enter source down branch");
		{
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->undoShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
		}
		bool finalUndoMatched = TheDisplay == display.get() &&
			shroudNotices.notices.size() == firstRevealCount * 4;
		for (std::size_t index = 0; finalUndoMatched && index < firstRevealCount;
			++index) {
			const auto &first = shroudNotices.notices[index];
			const auto &undo = shroudNotices.notices[index + firstRevealCount * 3];
			finalUndoMatched = undo.status == CELLSHROUD_FOGGED &&
				undo.x == first.x && undo.y == first.y;
		}
		require(finalUndoMatched,
			"original tree crusher final undo notification/owner restoration changed");
		{
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->doShroudCover(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
			ThePartitionManager->undoShroudCover(toppleTreePosition.x,
				toppleTreePosition.y, toppleRevealRadius, topplePlayerMask);
		}
		bool shroudRestored = TheDisplay == display.get() &&
			shroudNotices.notices.size() == firstRevealCount * 5;
		for (std::size_t index = 0; shroudRestored && index < firstRevealCount;
			++index) {
			const auto &first = shroudNotices.notices[index];
			const auto &cover = shroudNotices.notices[index + firstRevealCount * 4];
			shroudRestored = cover.status == CELLSHROUD_SHROUDED &&
				cover.x == first.x && cover.y == first.y;
		}
		for (Int y = 0; shroudRestored && y != shroudCellsY; ++y)
			for (Int x = 0; shroudRestored && x != shroudCellsX; ++x)
				shroudRestored = ThePartitionManager->getShroudStatusForPlayer(
					topplePlayerIndex, x, y) == originalShroudCells[y * shroudCellsX + x];
		require(shroudRestored,
			"original tree crusher shroud fixture baseline was not restored");
		terrain.removeTree(toppleTree);
		pushUnit->setPosition(&originalPushPosition);
		require(terrain.treeToppleState(toppleTree) == -1 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.treeBounceEvents(toppleTree) == 0 &&
			device.resource_counts() == baseline,
			"original tree crusher removal retained resources");
		// A full-speed fall bounces; a zero-percent fall enters DOWN without
		// an effect intent. Both use the shipping movement callback and leave
		// the accepted registry/Recording owner reusable after removal.
		toppleData.m_minimumToppleSpeed = 1.0f;
		toppleData.m_initialVelocityPercent = 1.0f;
		toppleData.m_initialAccelPercent = 0.0f;
		for (const bool shouldBounce : {true, false}) {
			toppleData.m_bounceVelocityPercent = shouldBounce ? 0.5f : 0.0f;
			require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0,
				&toppleData) && terrain.treeToppleState(toppleTree) == 0 &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
				"original tree bounce fixture admission failed");
			CopyGameClientRandomState(clientBefore);
			pushUnit->setPosition(&toppleUnitPosition);
			CopyGameClientRandomState(clientAfter);
			require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
				"original tree crusher initiation consumed GameClient random state");
			require(terrain.treeToppleState(toppleTree) == 1 &&
				terrain.treeToppleStartEvents(toppleTree) == 1 &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE),
				"original tree bounce movement or frame admission failed");
			const Real angleAtLimit = terrain.treeToppleAngle(toppleTree);
			require(angleAtLimit > 1.4f && angleAtLimit < 1.6f &&
				terrain.treeToppleState(toppleTree) == (shouldBounce ? 1 : 4) &&
				terrain.treeBounceEvents(toppleTree) == (shouldBounce ? 1U : 0U) &&
				(shouldBounce ? terrain.treeToppleVelocity(toppleTree) < 0 :
					terrain.treeToppleVelocity(toppleTree) == 0),
				"original tree bounce/down state or event intent changed");
			if (shouldBounce)
				require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeToppleAngle(toppleTree) < angleAtLimit &&
					terrain.treeBounceEvents(toppleTree) == 1,
					"original tree bounce did not reverse the angular step");
			else {
				const Real noKillZ = terrain.treeSinkLocationZ(toppleTree);
				require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeToppleState(toppleTree) == 4 &&
					terrain.treeSinkLocationZ(toppleTree) == noKillZ &&
					terrain.treeSinkFramesLeft(toppleTree) == 0,
					"original non-kill DOWN tree consumed sink or retired owner");
			}
			terrain.removeTree(toppleTree);
			pushUnit->setPosition(&originalPushPosition);
			require(terrain.treeToppleState(toppleTree) == -1 &&
				terrain.treeToppleStartEvents(toppleTree) == 0 &&
				terrain.treeBounceEvents(toppleTree) == 0 &&
				device.resource_counts() == baseline,
				"original tree bounce removal retained state or resources");
		}
		// Position FX are registry-owned intents until the Recording frame commits.
		// Keep the real logical shroud query; the notification adapter only bridges
		// this generated fixture's logical/visual map-size mismatch.
		std::vector<PositionFXTrace> fxTrace;
		FXList *startFX = const_cast<FXList *>(TheFXListStore->findFXList("FixtureGraph60"));
		FXList *bounceFX = const_cast<FXList *>(TheFXListStore->findFXList("FixtureGraph61"));
		require(startFX && bounceFX, "tree FX fixture lists missing");
		W3DTreeDrawModuleData fxData = toppleData;
		fxData.m_toppleFX = startFX;
		fxData.m_bounceFX = bounceFX;
		fxData.m_bounceVelocityPercent = 0.5f;
		// The bounce is gated at its transformed tip, not the tree's base.
		const Real fxRevealRadius = toppleRevealRadius + 32;
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 1, &terrain, toppleTree, false));
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 2, &terrain, toppleTree, false));
		bounceFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 3, &terrain, toppleTree, true));
		const auto revealFX = [&] {
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->doShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, fxRevealRadius, topplePlayerMask);
		};
		const auto undoFX = [&] {
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->undoShroudReveal(toppleTreePosition.x,
				toppleTreePosition.y, fxRevealRadius, topplePlayerMask);
		};
		const auto admitFXTree = [&] {
			pushUnit->setPosition(&originalPushPosition);
			frameCamera.Set_Position(Vector3(toppleTreePosition.x,
				toppleTreePosition.y, 50));
			require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0,
				&fxData) && terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
				"tree FX fixture admission failed");
		};
		const auto retireFXTree = [&] {
			terrain.removeTree(toppleTree);
			pushUnit->setPosition(&originalPushPosition);
			require(terrain.treeToppleState(toppleTree) == -1 &&
				device.resource_counts() == baseline,
				"tree FX retirement retained state/resources");
		};
		const std::size_t fxRevealNotice = shroudNotices.notices.size();
		revealFX();
		const std::size_t fxRevealCount = shroudNotices.notices.size() - fxRevealNotice;
		require(fxRevealCount > 0 && TheDisplay == display.get() &&
			ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
				&toppleTreePosition) == CELLSHROUD_CLEAR,
			"tree FX clear-shroud fixture/notification restoration failed");
		admitFXTree();
		const auto fxReadyResources = device.resource_counts();
		const auto *fxReadyVertex = terrain.peekTreeVertexSource();
		UnsignedInt collisionClientBefore[6]{}, collisionClientAfter[6]{};
		UnsignedInt collisionLogicBefore[6]{}, collisionLogicAfter[6]{};
		UnsignedInt collisionSeedBefore = 0, collisionSeedAfter = 0;
		CopyGameClientRandomState(collisionClientBefore);
		CopyGameLogicRandomState(&collisionSeedBefore, collisionLogicBefore);
		FXListStore *collisionStore = TheFXListStore;
		TheFXListStore = NULL;
		const bool collisionNoStore = rejected([&] { pushUnit->setPosition(&toppleUnitPosition); });
		TheFXListStore = collisionStore;
		require(collisionNoStore && terrain.treeToppleStartEvents(toppleTree) == 0 &&
			fxTrace.empty(), "tree FX missing store changed collision intent");
		pushUnit->setPosition(&originalPushPosition);
		// The entire list is checked before public collision publication.
		fxData.m_toppleFX = TheFXListStore->findFXList("FixtureLateLight");
		require(rejected([&] { pushUnit->setPosition(&toppleUnitPosition); }) &&
			terrain.treeToppleStartEvents(toppleTree) == 0 && fxTrace.empty(),
			"tree FX late unsupported nugget changed collision state/effects");
		pushUnit->setPosition(&originalPushPosition);
		fxData.m_toppleFX = reinterpret_cast<const FXList *>(static_cast<uintptr_t>(1));
		require(rejected([&] { pushUnit->setPosition(&toppleUnitPosition); }) &&
			terrain.treeToppleStartEvents(toppleTree) == 0 && fxTrace.empty(),
			"tree FX stale list changed collision state/effects");
		pushUnit->setPosition(&originalPushPosition);
		fxData.m_toppleFX = startFX;
		for (const char *owner : {"ZH_M22_TREE_PUSH_FAIL_AT", "ZH_M22_TREE_FX_FAIL_AT"}) {
			setenv(owner, std::strcmp(owner, "ZH_M22_TREE_PUSH_FAIL_AT") == 0 ?
				"publish" : "order-overflow", 1);
			const bool failed = rejected([&] { pushUnit->setPosition(&toppleUnitPosition); });
			unsetenv(owner);
			require(failed && terrain.treeToppleStartOrder(toppleTree) == 0 &&
				terrain.treeToppleStartEvents(toppleTree) == 0 && fxTrace.empty() &&
				terrain.peekTreeVertexSource() == fxReadyVertex &&
				device.resource_counts() == fxReadyResources,
				"tree FX collision publication/overflow consumed order or intent");
			pushUnit->setPosition(&originalPushPosition);
		}
		CopyGameClientRandomState(collisionClientAfter);
		CopyGameLogicRandomState(&collisionSeedAfter, collisionLogicAfter);
		require(std::memcmp(collisionClientBefore, collisionClientAfter, sizeof(collisionClientBefore)) == 0 &&
			collisionSeedBefore == collisionSeedAfter &&
			std::memcmp(collisionLogicBefore, collisionLogicAfter, sizeof(collisionLogicBefore)) == 0 &&
			terrain.peekTreeVertexSource() == fxReadyVertex &&
			device.resource_counts() == fxReadyResources,
			"tree FX rejected collision admission consumed RNG/GPU state");
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.treeToppleStartEvents(toppleTree) == 1 &&
			terrain.treeToppleStartOrder(toppleTree) == 1 &&
			terrain.treeConsumedToppleStartEvents(toppleTree) == 0 && fxTrace.empty(),
			"tree FX collision intent was consumed before frame commit");
		UnsignedInt fxClientBefore[6]{}, fxClientAfter[6]{};
		UnsignedInt fxLogicBefore[6]{}, fxLogicAfter[6]{};
		UnsignedInt fxLogicSeedBefore = 0, fxLogicSeedAfter = 0;
		CopyGameClientRandomState(fxClientBefore);
		CopyGameLogicRandomState(&fxLogicSeedBefore, fxLogicBefore);
		const auto pendingFXUnchanged = [&] {
			CopyGameClientRandomState(fxClientAfter);
			CopyGameLogicRandomState(&fxLogicSeedAfter, fxLogicAfter);
			return terrain.treeToppleStartEvents(toppleTree) == 1 &&
				terrain.treeConsumedToppleStartEvents(toppleTree) == 0 &&
				terrain.treeToppleAngle(toppleTree) == 0 && fxTrace.empty() &&
				terrain.peekTreeVertexSource() == fxReadyVertex &&
				device.resource_counts() == fxReadyResources &&
				std::memcmp(fxClientBefore, fxClientAfter, sizeof(fxClientBefore)) == 0 &&
				fxLogicSeedBefore == fxLogicSeedAfter &&
				std::memcmp(fxLogicBefore, fxLogicAfter, sizeof(fxLogicBefore)) == 0;
		};
		FXListStore *fxStore = TheFXListStore;
		TheFXListStore = NULL;
		const bool missingFXStore = !terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE);
		TheFXListStore = fxStore;
		require(missingFXStore && pendingFXUnchanged(),
			"tree FX failed provider preflight consumed pending intent");
		for (const char *boundary : {"sort-bound", "sort", "queue", "publish"}) {
			setenv("ZH_M22_TREE_FX_FAIL_AT", boundary, 1);
			const bool failed = !terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE);
			unsetenv("ZH_M22_TREE_FX_FAIL_AT");
			require(failed && pendingFXUnchanged(),
				"tree FX queue/publication failure consumed pending intent");
		}
		for (const char *owner : {"ZH_M22_TREE_TOPPLE_FAIL_AT", "ZH_M22_TREE_RESOURCE_FAIL_AT"}) {
			setenv(owner, std::strcmp(owner, "ZH_M22_TREE_TOPPLE_FAIL_AT") == 0 ?
				"state" : "geometry", 1);
			const bool failed = !terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE);
			unsetenv(owner);
			require(failed && pendingFXUnchanged(),
				"tree FX state/geometry failure consumed pending intent");
		}
		device.fail_next_buffer_upload();
		require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			pendingFXUnchanged(), "tree FX Recording failure consumed pending intent");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeConsumedToppleStartEvents(toppleTree) == 1 &&
			fxTrace.size() == 2 && fxTrace[0].token == 1 && fxTrace[1].token == 2 &&
			fxTrace[0].position.x == toppleTreePosition.x &&
			fxTrace[0].position.y == toppleTreePosition.y &&
			fxTrace[0].position.z == toppleTreePosition.z,
			"tree FX clean retry lost start ordering/position/commit marker");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == 2, "tree FX paused frame replayed start");
		const Real bounceAngle = PI / 2 - PI / 64;
		const Coord3D expectedBouncePosition{
			toppleTreePosition.x + 21 * std::sin(bounceAngle), toppleTreePosition.y,
			toppleTreePosition.z + 21 * std::cos(bounceAngle)};
		require(ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
			&expectedBouncePosition) == CELLSHROUD_CLEAR &&
			terrain.treeConsumedBounceEvents(toppleTree) == 0,
			"tree FX bounce position not admitted clear before frame dispatch");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			terrain.treeConsumedBounceEvents(toppleTree) == 1 && fxTrace.size() == 3 &&
			fxTrace[2].token == 3, "tree FX bounce intent was not consumed exactly once");
		require(std::fabs(fxTrace[2].position.x -
			(toppleTreePosition.x + 21 * std::sin(bounceAngle))) < 0.001f &&
			std::fabs(fxTrace[2].position.y - toppleTreePosition.y) < 0.001f &&
			std::fabs(fxTrace[2].position.z -
			(toppleTreePosition.z + 21 * std::cos(bounceAngle))) < 0.001f &&
			ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
				&fxTrace[2].position) == CELLSHROUD_CLEAR,
			"tree FX bounce position differs from exact native transformed point");
		frameCamera.Set_Position(Vector3(10000, 10000, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == 3, "tree FX hidden frame replayed an event");
		frameCamera.Set_Position(Vector3(toppleTreePosition.x, toppleTreePosition.y, 50));
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == 3, "tree FX reentry replayed an event");
		retireFXTree();
		fxTrace.clear();
		// Separate collision callbacks keep chronology, not insertion order.
		const DrawableID otherFXTree = static_cast<DrawableID>(6023);
		Coord3D otherFXPosition = toppleTreePosition;
		otherFXPosition.y += 30;
		Coord3D otherFXUnitPosition = toppleUnitPosition;
		otherFXUnitPosition.y += 30;
		FXList *otherFX = const_cast<FXList *>(TheFXListStore->findFXList("FixtureGraph62"));
		require(otherFX, "tree FX second collision list missing");
		W3DTreeDrawModuleData otherFXData = fxData;
		otherFXData.m_textureName = "Tree1.tga";
		otherFXData.m_toppleFX = otherFX;
		otherFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 11, &terrain, otherFXTree, false));
		startFX->clear();
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 12, &terrain, toppleTree, false));
		for (const bool cancelFirst : {false, true}) {
			admitFXTree();
			require(terrain.tryAddTree(otherFXTree, otherFXPosition, 1, 0, 0, &otherFXData) &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
				"tree FX callback ordering fixture admission failed");
			if (cancelFirst) pushUnit->setPosition(&toppleUnitPosition);
			pushUnit->setPosition(&otherFXUnitPosition);
			if (cancelFirst) {
				terrain.removeTree(toppleTree);
				require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0, &fxData),
					"tree FX canceled intent replacement admission failed");
			}
			pushUnit->setPosition(&toppleUnitPosition);
			require(terrain.treeToppleStartOrder(otherFXTree) == (cancelFirst ? 2U : 1U) &&
				terrain.treeToppleStartOrder(toppleTree) == (cancelFirst ? 3U : 2U) &&
				fxTrace.empty() && terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
				fxTrace.size() == 2 && fxTrace[0].token == 11 && fxTrace[1].token == 12,
				"tree FX callback chronology/cancellation reordered surviving intent");
			terrain.removeTree(otherFXTree);
			retireFXTree();
			fxTrace.clear();
		}
		otherFX->clear();
		// Four reverse callbacks exercise the explicit bounded in-place sort.
		// The sort-fault rejection must leave all registry/RNG/GPU state untouched.
		startFX->clear();
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 13, NULL, toppleTree, false));
		bounceFX->clear();
		bounceFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 14, NULL, toppleTree, true));
		Coord3D orderedPositions[4]{};
		pushUnit->setPosition(&originalPushPosition);
		for (Int index = 0; index < 4; ++index) {
			orderedPositions[index] = toppleTreePosition;
			orderedPositions[index].x += (index % 2) * 20;
			orderedPositions[index].y += (index / 2) * 20;
			require(ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
				&orderedPositions[index]) == CELLSHROUD_CLEAR,
				"tree FX reverse-event source position is not admitted clear shroud");
			require(terrain.tryAddTree(static_cast<DrawableID>(6040 + index),
				orderedPositions[index], 1, 0, 0, &fxData),
				"tree FX reverse-event fixture admission failed");
		}
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
			"tree FX reverse-event initial geometry failed");
		for (Int index = 3; index >= 0; --index) {
			Coord3D position = orderedPositions[index];
			position.x -= 1;
			pushUnit->setPosition(&position);
			require(terrain.treeToppleStartOrder(static_cast<DrawableID>(6040 + index)) ==
				static_cast<UnsignedInt64>(4 - index),
				"tree FX reverse-event collision order changed");
		}
		for (const Coord3D &position : orderedPositions)
			require(ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
				&position) == CELLSHROUD_CLEAR,
				"tree FX reverse-event dispatch position lost real clear shroud");
		const auto reverseResources = device.resource_counts();
		const auto *reverseVertex = terrain.peekTreeVertexSource();
		CopyGameClientRandomState(fxClientBefore);
		CopyGameLogicRandomState(&fxLogicSeedBefore, fxLogicBefore);
		setenv("ZH_M22_TREE_FX_FAIL_AT", "sort", 1);
		const bool reverseSortFailed = !terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE);
		unsetenv("ZH_M22_TREE_FX_FAIL_AT");
		CopyGameClientRandomState(fxClientAfter);
		CopyGameLogicRandomState(&fxLogicSeedAfter, fxLogicAfter);
		require(reverseSortFailed && fxTrace.empty() &&
			terrain.peekTreeVertexSource() == reverseVertex &&
			device.resource_counts() == reverseResources &&
			std::memcmp(fxClientBefore, fxClientAfter, sizeof(fxClientBefore)) == 0 &&
			fxLogicSeedBefore == fxLogicSeedAfter &&
			std::memcmp(fxLogicBefore, fxLogicAfter, sizeof(fxLogicBefore)) == 0,
			"tree FX reverse-event sorting consumed accepted state/resources/RNG");
		for (Int index = 0; index < 4; ++index)
			require(terrain.treeConsumedToppleStartEvents(static_cast<DrawableID>(6040 + index)) == 0,
				"tree FX reverse-event sort fault consumed a marker");
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) && fxTrace.size() == 4,
			"tree FX reverse-event retry lost an admitted event");
		for (Int index = 0; index < 4; ++index) {
			require(fxTrace[index].token == 13 &&
				fxTrace[index].position.x == orderedPositions[3 - index].x &&
				fxTrace[index].position.y == orderedPositions[3 - index].y &&
				terrain.treeConsumedToppleStartEvents(static_cast<DrawableID>(6040 + index)) == 1,
				"tree FX reverse-event retry changed chronology/identity");
		}
		// Bounce order follows source registry order, not reversed start chronology.
		// Keep source gating at each transformed tip, including suppressed tips.
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) && fxTrace.size() == 4,
			"tree FX reverse-event first angular step emitted premature bounce");
		std::vector<Coord3D> admittedBouncePositions;
		Int suppressedBounces = 0;
		for (const Coord3D &base : orderedPositions) {
			Coord3D tip{base.x + 21 * std::sin(bounceAngle), base.y,
				base.z + 21 * std::cos(bounceAngle)};
			if (ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex, &tip) ==
				CELLSHROUD_CLEAR) admittedBouncePositions.push_back(tip);
			else ++suppressedBounces;
		}
		require(!admittedBouncePositions.empty() && suppressedBounces > 0 &&
			terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
			fxTrace.size() == 4 + admittedBouncePositions.size(),
			"tree FX source-ordered bounce/shroud batch admission failed");
		for (std::size_t index = 0; index < admittedBouncePositions.size(); ++index) {
			const Coord3D &actual = fxTrace[4 + index].position;
			const Coord3D &expected = admittedBouncePositions[index];
			require(fxTrace[4 + index].token == 14 &&
				std::fabs(actual.x - expected.x) < 0.001f &&
				std::fabs(actual.y - expected.y) < 0.001f &&
				std::fabs(actual.z - expected.z) < 0.001f,
				"tree FX bounce source ordinal was replaced by start chronology");
		}
		const std::size_t dispatchedBounceCount = fxTrace.size();
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == dispatchedBounceCount,
			"tree FX source-ordered bounce retry replayed suppressed or accepted event");
		for (Int index = 0; index < 4; ++index) {
			require(terrain.treeBounceEvents(static_cast<DrawableID>(6040 + index)) == 1 &&
				terrain.treeConsumedBounceEvents(static_cast<DrawableID>(6040 + index)) == 1,
				"tree FX shroud-suppressed bounce retained a replay marker");
			terrain.removeTree(static_cast<DrawableID>(6040 + index));
		}
		pushUnit->setPosition(&originalPushPosition);
		require(device.resource_counts() == baseline,
			"tree FX reverse-event retirement retained resources");
		fxTrace.clear();
		// Removing an unconsumed collision cancels it; a new owner epoch is clean.
		admitFXTree();
		pushUnit->setPosition(&toppleUnitPosition);
		const UnsignedInt removedFXEpoch = terrain.treeOwnerEpoch();
		retireFXTree();
		admitFXTree();
		require(terrain.treeOwnerEpoch() != removedFXEpoch &&
			terrain.treeConsumedToppleStartEvents(toppleTree) == 0 && fxTrace.empty(),
			"tree FX removal/recreation inherited pending intent");
		retireFXTree();
		// A failed nugget after an earlier effect is accepted/consumed, never replayed.
		startFX->clear();
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 4, &terrain, toppleTree, false));
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 5, &terrain, toppleTree, false, true));
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 9, &terrain, toppleTree, false));
		admitFXTree();
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == 2 && fxTrace[0].token == 4 && fxTrace[1].token == 5 &&
			terrain.treeConsumedToppleStartEvents(toppleTree) == 1,
			"tree FX partial dispatch failure was not accepted/consumed");
		CopyGameClientRandomState(fxClientBefore);
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) && fxTrace.size() == 2,
			"tree FX partial dispatch failure replayed an event");
		CopyGameClientRandomState(fxClientAfter);
		require(std::memcmp(fxClientBefore, fxClientAfter, sizeof(fxClientBefore)) == 0,
			"tree FX partial dispatch retry consumed extra RNG");
		retireFXTree();
		fxTrace.clear();
		// The first nugget destroys the sole registry. The second still dispatches
		// from the immutable batch: production must not touch the erased registry.
		startFX->clear();
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 6, &terrain, toppleTree, false, false, true));
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 7, NULL, toppleTree, false));
		admitFXTree();
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			fxTrace.size() == 2 && fxTrace[0].token == 6 && fxTrace[1].token == 7 &&
			terrain.treeInstanceCount() == 0 && device.resource_counts() == baseline,
			"tree FX dispatch accessed removed registry or lost later nugget");
		pushUnit->setPosition(&originalPushPosition);
		fxTrace.clear();
		startFX->clear();
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 8, &terrain, toppleTree, false));
		undoFX();
		require(TheDisplay == display.get() &&
			ThePartitionManager->getShroudStatusForPlayer(topplePlayerIndex,
				&toppleTreePosition) == CELLSHROUD_FOGGED,
			"tree FX fog-shroud fixture/notification restoration failed");
		admitFXTree();
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
			terrain.treeConsumedToppleStartEvents(toppleTree) == 1 && fxTrace.empty(),
			"tree FX native fog gate dispatched or retained replay intent");
		revealFX();
		require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) && fxTrace.empty(),
			"tree FX visibility return replayed fog-gated event");
		retireFXTree();
		undoFX();
		{
			DisplayOverride notificationOnly(&shroudNotices);
			ThePartitionManager->doShroudCover(toppleTreePosition.x,
				toppleTreePosition.y, fxRevealRadius, topplePlayerMask);
			ThePartitionManager->undoShroudCover(toppleTreePosition.x,
				toppleTreePosition.y, fxRevealRadius, topplePlayerMask);
		}
		bool fxShroudRestored = TheDisplay == display.get() &&
			shroudNotices.notices.size() == fxRevealNotice + fxRevealCount * 5;
		for (std::size_t index = 0; fxShroudRestored && index < fxRevealCount; ++index)
			for (std::size_t transition = 0; fxShroudRestored && transition < 5; ++transition) {
				const auto &first = shroudNotices.notices[fxRevealNotice + index];
				const auto &notice = shroudNotices.notices[
					fxRevealNotice + transition * fxRevealCount + index];
				fxShroudRestored = first.x == notice.x && first.y == notice.y &&
					notice.status == (transition == 4 ? CELLSHROUD_SHROUDED :
						(transition % 2 == 0 ? CELLSHROUD_CLEAR : CELLSHROUD_FOGGED));
			}
		for (Int y = 0; fxShroudRestored && y != shroudCellsY; ++y)
			for (Int x = 0; fxShroudRestored && x != shroudCellsX; ++x)
				fxShroudRestored = ThePartitionManager->getShroudStatusForPlayer(
					topplePlayerIndex, x, y) == originalShroudCells[y * shroudCellsX + x];
		require(fxShroudRestored, "tree FX notification sequence/baseline restoration failed");
		startFX->clear();
		bounceFX->clear();
		pushUnit->setPosition(&toppleUnitPosition);
		toppleData.m_minimumToppleSpeed = 0.0f;
		require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0,
			&toppleData), "original tree invalid-parameter fixture admission failed");
		const auto *invalidToppleVertex = terrain.peekTreeVertexSource();
		const auto invalidToppleResources = device.resource_counts();
		CopyGameClientRandomState(clientBefore);
		require(rejected([&] { terrain.unitMoved(pushUnit); }) &&
			terrain.treeToppleState(toppleTree) == 0 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == invalidToppleVertex &&
			device.resource_counts() == invalidToppleResources,
			"original tree invalid topple parameters changed accepted owner");
		CopyGameClientRandomState(clientAfter);
		require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
			"original tree invalid topple parameters consumed random state");
		toppleData.m_minimumToppleSpeed = 1.0f;
		toppleData.m_initialVelocityPercent =
			std::numeric_limits<Real>::quiet_NaN();
		require(rejected([&] { terrain.unitMoved(pushUnit); }) &&
			terrain.treeToppleState(toppleTree) == 0 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == invalidToppleVertex &&
			device.resource_counts() == invalidToppleResources,
			"original tree nonfinite topple velocity changed accepted owner");
		toppleData.m_initialVelocityPercent = 1.0f;
		terrain.removeTree(toppleTree);
		require(terrain.tryAddTree(toppleTree, toppleUnitPosition, 1, 0, 0,
			&toppleData), "original tree degenerate-direction fixture admission failed");
		const auto *degenerateVertex = terrain.peekTreeVertexSource();
		const auto degenerateResources = device.resource_counts();
		require(rejected([&] { terrain.unitMoved(pushUnit); }) &&
			terrain.treeToppleState(toppleTree) == 0 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.peekTreeVertexSource() == degenerateVertex &&
			device.resource_counts() == degenerateResources,
			"original tree zero-length topple direction changed accepted owner");
		terrain.removeTree(toppleTree);
		pushUnit->setPosition(&originalPushPosition);
		require(device.resource_counts() == baseline,
			"original tree negative-topple removal retained resources");
		// Native DOWN consumes exactly the configured positive sink frames,
		// then retires the tree on the following unpaused frame. Deletion must
		// publish type, atlas, partition, and Recording ownership atomically.
		toppleData.m_killWhenToppled = TRUE;
		toppleData.m_bounceVelocityPercent = 0;
		toppleData.m_sinkDistance = 4;
		toppleData.m_sinkFrames = 2;
		for (const bool sharedType : {true, false}) {
			const DrawableID sinkTree = static_cast<DrawableID>(6026);
			const DrawableID survivorTree = static_cast<DrawableID>(6027);
			Coord3D survivorPosition = toppleTreePosition;
			survivorPosition.x += 30;
			W3DTreeDrawModuleData survivorData = toppleData;
			if (!sharedType) survivorData.m_textureName = "Tree1.tga";
			require(terrain.tryAddTree(sinkTree, toppleTreePosition, 1, 0, 0,
				&toppleData) && terrain.tryAddTree(survivorTree, survivorPosition,
				1, 0, 0, sharedType ? &toppleData : &survivorData) &&
				terrain.treeInstanceCount() == 2 &&
				terrain.treeTypeCount() == (sharedType ? 1 : 2) &&
				terrain.treePartitionBucket(sinkTree) >= 0 &&
				terrain.treePartitionBucket(survivorTree) >= 0,
				"original tree sink/survivor fixture admission failed");
			frameCamera.Set_Position(Vector3(toppleTreePosition.x,
				toppleTreePosition.y, 50));
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
				"original tree sink fixture initial frame failed");
			pushUnit->setPosition(&toppleUnitPosition);
			require(terrain.treeToppleState(sinkTree) == 1 &&
				terrain.treeToppleState(survivorTree) == 0 &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeToppleState(sinkTree) == 4 &&
				terrain.treeSinkFramesLeft(sinkTree) == 2,
				"original tree sink fixture did not enter DOWN through public movement");
			const Real sinkStartZ = terrain.treeSinkLocationZ(sinkTree);
			const auto *sinkVertex = terrain.peekTreeVertexSource();
			const auto sinkResources = device.resource_counts();
			const Int sinkBucket = terrain.treePartitionBucket(sinkTree);
			const Int survivorBucket = terrain.treePartitionBucket(survivorTree);
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
				terrain.treeSinkFramesLeft(sinkTree) == 2 &&
				terrain.treeSinkLocationZ(sinkTree) == sinkStartZ,
				"original tree paused sink consumed countdown or position");
			setenv("ZH_M22_TREE_SINK_FAIL_AT", "state", 1);
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(sinkTree) == 2 &&
				terrain.treeSinkLocationZ(sinkTree) == sinkStartZ &&
				terrain.peekTreeVertexSource() == sinkVertex &&
				device.resource_counts() == sinkResources,
				"original tree sink state fault consumed countdown or Recording owner");
			unsetenv("ZH_M22_TREE_SINK_FAIL_AT");
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(sinkTree) == 1 &&
				std::fabs(terrain.treeSinkLocationZ(sinkTree) - (sinkStartZ - 2)) < 0.001f &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(sinkTree) == 0 &&
				std::fabs(terrain.treeSinkLocationZ(sinkTree) - (sinkStartZ - 4)) < 0.001f,
				"original tree sink countdown or bounded displacement changed");
			const auto *readySinkAtlas = terrain.peekTreeAtlasSource();
			const auto *readySinkVertex = terrain.peekTreeVertexSource();
			const auto readySinkResources = device.resource_counts();
			CopyGameClientRandomState(clientBefore);
			for (const char *fault : {"type", "publish"}) {
				setenv("ZH_M22_TREE_SINK_FAIL_AT", fault, 1);
				require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeInstanceCount() == 2 &&
					terrain.treeTypeCount() == (sharedType ? 1 : 2) &&
					terrain.treeSinkFramesLeft(sinkTree) == 0 &&
					terrain.treePartitionBucket(sinkTree) == sinkBucket &&
					terrain.treePartitionBucket(survivorTree) == survivorBucket &&
					terrain.peekTreeAtlasSource() == readySinkAtlas &&
					terrain.peekTreeVertexSource() == readySinkVertex &&
					device.resource_counts() == readySinkResources,
					"original tree sink deletion fault changed accepted owner");
				unsetenv("ZH_M22_TREE_SINK_FAIL_AT");
				CopyGameClientRandomState(clientAfter);
				require(std::memcmp(clientBefore, clientAfter, sizeof(clientBefore)) == 0,
					"original tree sink deletion fault consumed GameClient RNG");
			}
			if (!sharedType) {
				setenv("ZH_M22_TREE_ATLAS_FAIL_AT", "read", 1);
				require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeInstanceCount() == 2 &&
					terrain.treeTypeCount() == 2 &&
					terrain.peekTreeAtlasSource() == readySinkAtlas &&
					device.resource_counts() == readySinkResources,
					"original tree sink atlas fault retired accepted type");
				unsetenv("ZH_M22_TREE_ATLAS_FAIL_AT");
				setenv("ZH_M22_TREE_RESOURCE_FAIL_AT", "geometry", 1);
				require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeInstanceCount() == 2 &&
					terrain.peekTreeAtlasSource() == readySinkAtlas &&
					device.resource_counts() == readySinkResources,
					"original tree sink geometry fault retired accepted type");
				unsetenv("ZH_M22_TREE_RESOURCE_FAIL_AT");
				device.fail_next_buffer_upload();
				require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
					terrain.treeInstanceCount() == 2 &&
					terrain.peekTreeAtlasSource() == readySinkAtlas &&
					device.resource_counts() == readySinkResources,
					"original tree sink Recording fault retired accepted type");
			}
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeInstanceCount() == 1 &&
				terrain.treeTypeCount() == 1 &&
				terrain.treeToppleState(sinkTree) == -1 &&
				terrain.treePartitionBucket(sinkTree) == -1 &&
				terrain.treePartitionBucket(survivorTree) == survivorBucket &&
				terrain.treeToppleState(survivorTree) == 0 &&
				terrain.peekTreeAtlasSource() &&
				(sharedType ? terrain.peekTreeAtlasSource() == readySinkAtlas :
					terrain.peekTreeAtlasSource() != readySinkAtlas),
				"original tree sink retry lost survivor or retained obsolete atlas");
			frameCamera.Set_Position(Vector3(survivorPosition.x,
				survivorPosition.y, 50));
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE) &&
				terrain.treeIsVisible(survivorTree) &&
				terrain.treeVisibleCount() == 1 &&
				terrain.peekTreeVertexSource() &&
				terrain.peekTreeVertexSource()->Get_Vertex_Count() == 3 &&
				std::fabs(reinterpret_cast<const VertexFormatXYZNDUV1 *>(
					terrain.peekTreeVertexSource()->Get_CPU_Vertex_Buffer())[0].x -
						survivorPosition.x) < 0.001f,
				"original tree sink survivor type remap lost reentry geometry");
			const auto retiredResources = device.resource_counts();
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeInstanceCount() == 1 &&
				terrain.treeTypeCount() == 1 &&
				terrain.treeIsVisible(survivorTree) &&
				terrain.treePartitionBucket(sinkTree) == -1 &&
				device.resource_counts() == retiredResources,
				"original tree sink retired identity or resources twice");
			terrain.removeTree(survivorTree);
			pushUnit->setPosition(&originalPushPosition);
			frameCamera.Set_Position(Vector3(toppleTreePosition.x,
				toppleTreePosition.y, 50));
			require(terrain.treeInstanceCount() == 0 &&
				terrain.treeTypeCount() == 0 &&
				!terrain.peekTreeAtlasSource() &&
				!terrain.peekTreeVertexSource() &&
				device.resource_counts() == baseline,
				"original tree sink survivor removal retained type or Recording owner");
		}
		// A sole hidden tree must retain its accepted registry through failed
		// sink/deletion frames, then release the registry without rebuilding an
		// atlas for an empty type list. Removal during a pending sink is safe.
		const DrawableID soleSinkTree = static_cast<DrawableID>(6028);
		for (const bool resetPending : {true, false}) {
			require(terrain.tryAddTree(soleSinkTree, toppleTreePosition, 1, 0, 0,
				&toppleData) &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, TRUE),
				"original sole-tree sink fixture admission failed");
			const UnsignedInt sinkEpoch = terrain.treeOwnerEpoch();
			pushUnit->setPosition(&toppleUnitPosition);
			require(terrain.treeToppleState(soleSinkTree) == 1 &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeToppleState(soleSinkTree) == 4 &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 2,
				"original sole-tree sink did not enter DOWN");
			const Real soleStartZ = terrain.treeSinkLocationZ(soleSinkTree);
			const auto soleResources = device.resource_counts();
			toppleData.m_sinkFrames = 0;
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 2 &&
				terrain.treeSinkLocationZ(soleSinkTree) == soleStartZ &&
				device.resource_counts() == soleResources,
				"original sole-tree zero sink frames changed accepted owner");
			toppleData.m_sinkFrames = 2;
			toppleData.m_sinkDistance = std::numeric_limits<Real>::infinity();
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 2 &&
				terrain.treeSinkLocationZ(soleSinkTree) == soleStartZ &&
				device.resource_counts() == soleResources,
				"original sole-tree nonfinite sink distance changed accepted owner");
			toppleData.m_sinkDistance = 4;
			frameCamera.Set_Position(Vector3(1000, 1000, 50));
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 1 &&
				terrain.treeVisibleCount() == 0 &&
				!terrain.peekTreeVertexSource() &&
				terrain.treeOwnerEpoch() == sinkEpoch,
				"original sole-tree hidden sink lost countdown or registry");
			if (resetPending) {
				terrain.removeAllTrees();
				pushUnit->setPosition(&originalPushPosition);
				frameCamera.Set_Position(Vector3(toppleTreePosition.x,
					toppleTreePosition.y, 50));
				require(terrain.treeOwnerEpoch() == 0 &&
					terrain.treePartitionBucket(soleSinkTree) == -1 &&
					!terrain.peekTreeAtlasSource() &&
					device.resource_counts() == baseline,
					"original sole-tree pending sink removal retained owner");
				continue;
			}
			setenv("ZH_M22_TREE_SINK_FAIL_AT", "state", 1);
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 1 &&
				terrain.treeOwnerEpoch() == sinkEpoch,
				"original sole-tree hidden sink fault consumed countdown");
			unsetenv("ZH_M22_TREE_SINK_FAIL_AT");
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeSinkFramesLeft(soleSinkTree) == 0 &&
				std::fabs(terrain.treeSinkLocationZ(soleSinkTree) -
					(soleStartZ - 4)) < 0.001f,
				"original sole-tree hidden sink retry lost final displacement");
			const auto *soleAtlas = terrain.peekTreeAtlasSource();
			const auto beforeDeleteResources = device.resource_counts();
			setenv("ZH_M22_TREE_SINK_FAIL_AT", "publish", 1);
			require(!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeInstanceCount() == 1 &&
				terrain.treeTypeCount() == 1 &&
				terrain.treeOwnerEpoch() == sinkEpoch &&
				terrain.peekTreeAtlasSource() == soleAtlas &&
				device.resource_counts() == beforeDeleteResources,
				"original sole-tree deletion fault retired accepted registry");
			unsetenv("ZH_M22_TREE_SINK_FAIL_AT");
			require(terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				terrain.treeOwnerEpoch() == 0 &&
				terrain.treeInstanceCount() == 0 &&
				terrain.treeTypeCount() == 0 &&
				terrain.treePartitionBucket(soleSinkTree) == -1 &&
				!terrain.peekTreeAtlasSource() &&
				!terrain.peekTreeVertexSource() &&
				device.resource_counts() == baseline &&
				!terrain.updateTreeVisibleFrame(&frameCamera, breeze, FALSE) &&
				device.resource_counts() == baseline,
				"original sole-tree deletion retry retained or recreated owner");
			pushUnit->setPosition(&originalPushPosition);
			frameCamera.Set_Position(Vector3(toppleTreePosition.x,
				toppleTreePosition.y, 50));
		}
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
		require(terrain.tryAddTree(toppleTree, toppleTreePosition, 1, 0, 0,
			&fxData) && terrain.treeToppleState(toppleTree) == 0,
			"original tree reset witness could not admit live topple owner");
		startFX->addFXNugget(newInstance(DispatchFXNugget)(
			&fxTrace, 10, &terrain, toppleTree, false));
		pushUnit->setPosition(&toppleUnitPosition);
		require(terrain.treeToppleState(toppleTree) == 1 &&
			terrain.treeToppleStartEvents(toppleTree) == 1,
			"original tree reset witness did not publish live topple state");
		const UnsignedInt old_tree_epoch = terrain.treeOwnerEpoch();
		terrain.reset();
		require(fxTrace.empty(), "tree FX reset dispatched pending collision intent");
		startFX->clear();
		pushUnit->setPosition(&originalPushPosition);
		require(terrain.treeInstanceCount() == 0 && terrain.treeTypeCount() == 0 &&
			terrain.treeOwnerEpoch() != old_tree_epoch &&
			terrain.treeToppleState(toppleTree) == -1 &&
			terrain.treeToppleStartEvents(toppleTree) == 0 &&
			terrain.treeBounceEvents(toppleTree) == 0 &&
			!terrain.peekTreeVertexSource() && !terrain.peekTreeIndexSource() &&
			!terrain.peekTreeAtlasSource() &&
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
