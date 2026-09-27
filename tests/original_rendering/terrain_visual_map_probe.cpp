#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/GameEngine.h"
#include "Common/SubsystemInterface.h"
#include "Common/Geometry.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "nullrobj.h"
#include "proto.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DPropBuffer.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "WW3D2/scene.h"
#include "WW3D2/camera.h"
#include "WW3D2/rinfo.h"
#include "dx8vertexbuffer.h"
#include "dx8fvf.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

struct W3DTerrainPropLifecycleProbeAccess {
	static void malformed(HeightMapRenderObjClass& owner,zh::original_runtime::OriginalGpuEdge& edge)
	{
		auto* const buffer=owner.m_propBuffer;
		auto* const map=owner.m_map;
		auto* const assets=W3DDisplay::m_assetManager;
		const auto generation=edge.generation_;
		const auto before=image(owner);
		const auto mapping=mappings(edge);
		const auto count=buffer->m_numProps;
		const auto types=buffer->m_numPropTypes;
		const auto restore=[&] {
			owner.m_propBuffer=buffer;owner.m_map=map;
			W3DDisplay::m_assetManager=assets;edge.generation_=generation;
		};
		try {
			for(unsigned fault=0;fault<5;++fault) {
				restore();
				if(fault==0) owner.m_propBuffer=reinterpret_cast<W3DPropBuffer*>(&owner);
				if(fault==1) owner.m_propBuffer=NULL;
				if(fault==2) owner.m_map=reinterpret_cast<WorldHeightMap*>(&owner);
				if(fault==3) ++edge.generation_;
				if(fault==4) W3DDisplay::m_assetManager=reinterpret_cast<W3DAssetManager*>(&owner);
				const auto rejects=[](auto operation) { try { operation(); } catch(...) { return true; } return false; };
				if(owner.canNotifyShroudChanged() ||
					!rejects([&] { owner.notifyShroudChanged(); }) ||
					!rejects([&] { owner.hasLiveProps(); }) ||
					!rejects([&] { owner.removeAllProps(); }) ||
					!rejects([&] { owner.freeMapResources(); }) ||
					!rejects([&] { preflightClientTerrainRemovalAdmission(); }))
					throw std::runtime_error("generated malformed prop identity escaped admission");
				restore();
				if(image(owner)!=before || mappings(edge)!=mapping ||
					buffer->m_numProps!=count || buffer->m_numPropTypes!=types ||
					!owner.canNotifyShroudChanged() || !owner.hasLiveProps())
					throw std::runtime_error("generated malformed prop identity changed accepted owner");
				preflightClientTerrainRemovalAdmission();
				owner.notifyShroudChanged();
			}
		} catch(...) { restore();throw; }
	}
	static void construction(HeightMapRenderObjClass& owner,const Coord3D& near,const ThingTemplate* model,W3DTerrainVisual& visual)
	{
		auto& buffer=*owner.m_propBuffer;
		Coord3D far=near;far.x+=1000;
		visual.addProp(model,&far,0);
		const int survivor=buffer.m_numProps-1;
		auto* identity=buffer.m_props[survivor].m_robj;
		GeometryInfo geometry(GEOMETRY_BOX,false,10,10,10);
		owner.removeTreesAndPropsForConstruction(&near,geometry,0);
		for(int i=0;i<survivor;++i)
			if(buffer.m_props[i].m_robj) throw std::runtime_error("generated native construction clearing missed overlap");
		if(buffer.m_props[survivor].m_robj!=identity || buffer.m_props[survivor].location.x!=far.x)
			throw std::runtime_error("generated construction clearing changed distant sibling");
		buffer.m_doCull=false;
		CameraClass camera;
		owner.updateCenter(&camera,NULL);
		if(!buffer.m_doCull || buffer.m_props[survivor].m_robj!=identity)
			throw std::runtime_error("generated native prop cull invalidation differs");
	}
	static void last(const HeightMapRenderObjClass& owner,const char* model,const Coord3D& position,Real angle,Real scale)
	{
		const auto& buffer=*owner.m_propBuffer;
		if(!buffer.m_numProps) throw std::runtime_error("generated weather prop is absent");
		const auto& prop=buffer.m_props[buffer.m_numProps-1];
		if(buffer.m_propTypes[prop.propType].m_robjName.compareNoCase(AsciiString(model))!=0 ||
			prop.id!=1 || prop.visible || prop.ss!=OBJECTSHROUD_INVALID)
			throw std::runtime_error("generated first module/weather/type state differs");
		Matrix3D expected(true);expected.Rotate_Z(angle);expected.Scale(scale);
		expected.Set_Translation(Vector3(position.x,position.y,position.z));
		for(int r=0;r<3;++r) for(int c=0;c<4;++c)
			if(prop.m_robj->Get_Transform()[r][c]!=expected[r][c])
				throw std::runtime_error("generated native prop transform order differs");
		if(prop.bounds.Radius!=buffer.m_propTypes[prop.propType].m_bounds.Radius)
			throw std::runtime_error("generated prop invented scale/radius correction");
	}
	using Mapping = std::vector<std::pair<const void*,zh::renderer::BufferHandle>>;
	static std::pair<Mapping,Mapping> mappings(const zh::original_runtime::OriginalGpuEdge& edge)
	{
		Mapping vertices,indices;
		for(const auto& value:edge.vertices_) vertices.emplace_back(value.first,value.second);
		for(const auto& value:edge.indices_) indices.emplace_back(value.first,value.second);
		return {vertices,indices};
	}
	static std::vector<unsigned char> image(const HeightMapRenderObjClass& owner)
	{
		std::vector<unsigned char> bytes;
		const auto append=[&](const auto& value) {
			const auto* p=reinterpret_cast<const unsigned char*>(&value);
			bytes.insert(bytes.end(),p,p+sizeof(value));
		};
		append(owner.m_indexBuffer);append(owner.m_vertexBufferTiles);append(owner.m_vertexBufferBackup);
		append(owner.m_numVBTilesX);append(owner.m_numVBTilesY);append(owner.m_numVertexBufferTiles);
		append(owner.m_numBlockColumnsInLastVB);append(owner.m_numBlockRowsInLastVB);
		for (Int i=0;i<owner.m_numVertexBufferTiles;++i) {
			append(owner.m_vertexBufferTiles[i]);append(owner.m_vertexBufferBackup[i]);
			const std::size_t extent=owner.m_vertexBufferTiles[i]->Get_Vertex_Count()*owner.m_vertexBufferTiles[i]->FVF_Info().Get_FVF_Size();
			const auto* p=reinterpret_cast<const unsigned char*>(owner.m_vertexBufferBackup[i]);
			bytes.insert(bytes.end(),p,p+extent);
		}
		return bytes;
	}
};

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
template <typename Operation>
bool rejected_any(Operation operation)
{ try { operation(); } catch (...) { return true; } return false; }

unsigned draw_count(const std::string &trace)
{
	unsigned count = 0;
	for (std::size_t at = trace.find("draw pipeline="); at != std::string::npos;
		at = trace.find("draw pipeline=", at + 1)) ++count;
	return count;
}

void require_rolled_back(W3DTerrainVisual *visual, const char *message)
{
	require(!visual->getLogicHeightMap() && TheHeightMap && !TheHeightMap->getMap() &&
		!TheHeightMap->Peek_Scene(), message);
}
}

extern "C" void zh_probe_terrain_visual_map()
{
	const char *map_path = std::getenv("ZH_M22_TERRAIN_VISUAL_MAP");
	const char *bad_map_path = std::getenv("ZH_M22_TERRAIN_VISUAL_BAD_MAP");
	require(map_path && bad_map_path, "original terrain visual map fixture missing");
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	const ThingTemplate *plain_prop = TheThingFactory->findTemplate(AsciiString("FixtureProp"), FALSE);
	const ThingTemplate *modeled_prop = TheThingFactory->findTemplate(AsciiString("ModeledProp"), FALSE);
	const ThingTemplate *weather_prop = TheThingFactory->findTemplate(AsciiString("WeatherProp"), FALSE);
	require(plain_prop && modeled_prop && plain_prop->getDrawModuleInfo().getCount() == 0 &&
		modeled_prop->getDrawModuleInfo().getCount() > 0 && weather_prop,
		"original terrain prop generated templates unavailable");
	Coord3D prop_pos;
	prop_pos.set(20.0f, 20.0f, 0.0f);
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		struct RestoreProviders {
			Display* display=TheDisplay;TerrainVisual* visual=TheTerrainVisual;
			~RestoreProviders() { TheDisplay=display;TheTerrainVisual=visual; }
		} restore_providers;
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		Display *saved_display = TheDisplay;
		TheDisplay = display.get();
		TerrainVisual *saved_visual = TheTerrainVisual;
		auto visual = std::make_unique<W3DTerrainVisual>();
		struct AbortFrameBeforeOwners {
			zh::original_runtime::OriginalGpuEdge& edge;
			~AbortFrameBeforeOwners() { edge.abort_source_frame(); }
		} abort_frame_before_owners{edge};
		TheTerrainVisual = visual.get();
		visual->init();
		require(TheHeightMap && !TheHeightMap->getMap() && !TheHeightMap->Peek_Scene(),
			"original terrain visual init published a map");
		require(rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); }),
			"original terrain prop accepted a missing map");
		const auto owner_resources = device.resource_counts().total();

		require(!visual->load(AsciiString("missing-generated-terrain.map")),
			"original terrain visual missing map succeeded");
		require_rolled_back(visual.get(), "original terrain visual missing map retained owner");

		bool malformed_rejected = false;
		try { visual->load(AsciiString(bad_map_path)); }
		catch (const std::runtime_error &) { malformed_rejected = true; }
		require(malformed_rejected, "original terrain visual malformed map accepted");
		require_rolled_back(visual.get(), "original terrain visual malformed map retained owner");

		device.fail_next_buffer_create();
		bool buffer_rejected = false;
		try { visual->load(AsciiString(map_path)); }
		catch (const std::runtime_error &) { buffer_rejected = true; }
		require(buffer_rejected, "original terrain visual buffer failure accepted");
		require_rolled_back(visual.get(), "original terrain visual buffer failure retained owner");
		require(device.resource_counts().total() == owner_resources,
			"original terrain visual failed transaction retained bounded smudge resource");

		require(visual->load(AsciiString(map_path)) && visual->getLogicHeightMap() &&
			TheHeightMap && TheHeightMap->getMap() == visual->getLogicHeightMap() &&
			TheHeightMap->Peek_Scene() == W3DDisplay::m_3DScene,
			"original terrain visual map transaction did not attach primary terrain");
		require(!TheHeightMap->hasBibBuffer(),
			"original CPU-only map unexpectedly published active bib storage");
		const auto bib_resources = device.resource_counts();
		const auto bib_trace = device.snapshot();
		const auto bib_refs = TheHeightMap->Num_Refs();
		visual->removeAllBibs();
		visual->removeAllBibs();
		require(TheHeightMap->getMap() == visual->getLogicHeightMap() &&
			TheHeightMap->Num_Refs() == bib_refs &&
			device.resource_counts() == bib_resources && device.snapshot() == bib_trace,
			"original map-loaded empty bib cleanup changed owners or Recording");
		TerrainVisual *bib_visual = TheTerrainVisual;
		TheTerrainVisual = NULL;
		const bool missing_bib_visual = rejected([&]() { visual->removeAllBibs(); });
		TheTerrainVisual = bib_visual;
		HeightMapRenderObjClass *bib_height = TheHeightMap;
		TheHeightMap = NULL;
		const bool missing_bib_height = rejected([&]() { visual->removeAllBibs(); });
		TheHeightMap = bib_height;
		BaseHeightMapRenderObjClass *bib_terrain = TheTerrainRenderObject;
		TheTerrainRenderObject = NULL;
		const bool missing_bib_terrain = rejected([&]() { visual->removeAllBibs(); });
		TheTerrainRenderObject = bib_terrain;
		W3DDisplay::m_3DScene->Remove_Render_Object(TheHeightMap);
		visual->removeAllBibs();
		W3DDisplay::m_3DScene->Add_Render_Object(TheHeightMap);
		require(missing_bib_visual && missing_bib_height && missing_bib_terrain &&
			TheHeightMap->Num_Refs() == bib_refs &&
			device.resource_counts() == bib_resources && device.snapshot() == bib_trace,
			"original map-loaded bib cleanup accepted foreign owner or changed Recording");
		require(rejected([&]() { visual->addFactionBib(NULL, TRUE); }) &&
			rejected([&]() { visual->addFactionBibDrawable(NULL, TRUE); }),
			"original active bib producer unexpectedly admitted");
		const auto prop_resources = device.resource_counts();
		const auto prop_refs = TheHeightMap->Num_Refs();
		visual->addProp(plain_prop, &prop_pos, 0.0f);
		visual->addProp(plain_prop, &prop_pos, 1.0f);
		require(TheHeightMap->Num_Refs() == prop_refs && device.resource_counts() == prop_resources,
			"original no-model terrain prop changed source owners or Recording resources");
		// An authored missing model retains the native optional no-publication
		// behavior. A resident logical prop is admitted below, not rendered in A.
		visual->addProp(modeled_prop, &prop_pos, 0.0f);
		require(!TheHeightMap->hasPropBuffer(), "missing prop model published a buffer");
		auto* resident = new Null3DObjClass("TEST.HLOD");
		W3DDisplay::m_assetManager->Add_Prototype(new PrimitivePrototypeClass(resident));
		resident->Release_Ref();
		visual->addProp(modeled_prop, &prop_pos, 0.0f);
		require(TheHeightMap->hasLiveProps() && resident->Num_Refs()==1,
			"original logical prop publication or prototype ownership differs");
		W3DTerrainPropLifecycleProbeAccess::last(*TheHeightMap,"TEST.HLOD",prop_pos,0,modeled_prop->getAssetScale());
		for(const char* name:{"PROP.NORMAL","PROP.SNOW","PROP.NIGHT","PROP.SNOW_NIGHT"}) {
			auto* model=new Null3DObjClass(name);
			W3DDisplay::m_assetManager->Add_Prototype(new PrimitivePrototypeClass(model));model->Release_Ref();
		}
		const auto saved_weather=TheWritableGlobalData->m_weather;
		const auto saved_time=TheWritableGlobalData->m_timeOfDay;
		for(unsigned condition=0;condition<4;++condition) {
			TheWritableGlobalData->m_weather=condition&1?WEATHER_SNOWY:WEATHER_NORMAL;
			TheWritableGlobalData->m_timeOfDay=condition&2?TIME_OF_DAY_NIGHT:TIME_OF_DAY_AFTERNOON;
			visual->addProp(weather_prop,&prop_pos,0.375f);
			const char* names[]={"PROP.NORMAL","PROP.SNOW","PROP.NIGHT","PROP.SNOW_NIGHT"};
			W3DTerrainPropLifecycleProbeAccess::last(*TheHeightMap,names[condition],prop_pos,0.375f,weather_prop->getAssetScale());
		}
		TheWritableGlobalData->m_weather=saved_weather;TheWritableGlobalData->m_timeOfDay=saved_time;
		W3DTerrainPropLifecycleProbeAccess::construction(*TheHeightMap,prop_pos,modeled_prop,*visual);
		W3DTerrainPropLifecycleProbeAccess::malformed(*TheHeightMap,edge);
		Coord3D malformed_pos = prop_pos;
		malformed_pos.x = std::numeric_limits<Real>::quiet_NaN();
		require(rejected([&]() { visual->addProp(NULL, &prop_pos, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, NULL, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, &malformed_pos, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, &prop_pos,
				std::numeric_limits<Real>::infinity()); }),
			"original terrain prop accepted malformed input");
		TheDisplay = saved_display;
		const bool missing_display = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheDisplay = display.get();
		TheTerrainVisual = saved_visual;
		const bool missing_visual = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheTerrainVisual = visual.get();
		BaseHeightMapRenderObjClass *saved_terrain = TheTerrainRenderObject;
		TheTerrainRenderObject = NULL;
		const bool missing_terrain = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheTerrainRenderObject = saved_terrain;
		HeightMapRenderObjClass *saved_height = TheHeightMap;
		TheHeightMap = NULL;
		const bool missing_height = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheHeightMap = saved_height;
		W3DAssetManager *saved_assets = W3DDisplay::m_assetManager;
		W3DDisplay::m_assetManager = NULL;
		const bool missing_assets = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		require(rejected_any([&] { preflightClientTerrainRemovalAdmission(); }) &&
			rejected_any([&] { TheHeightMap->freeMapResources(); }),
			"prop provider removal bypassed whole-owner preflight");
		W3DDisplay::m_assetManager = saved_assets;
		GlobalData *saved_global_data = TheWritableGlobalData;
		TheWritableGlobalData = NULL;
		const bool missing_global_data = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheWritableGlobalData = saved_global_data;
		W3DDisplay::m_3DScene->Remove_Render_Object(TheHeightMap);
		const bool missing_scene = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		W3DDisplay::m_3DScene->Add_Render_Object(TheHeightMap);
		require(missing_display && missing_visual && missing_terrain && missing_height &&
			missing_assets && missing_global_data && missing_scene &&
			device.resource_counts() == prop_resources &&
			TheHeightMap->Num_Refs() == prop_refs,
			"original terrain prop accepted removed provider or changed owner");
		visual->addProp(plain_prop, &prop_pos, 0.0f);
		require(device.resource_counts() == prop_resources,
			"original terrain prop provider retry created Recording resources");

		ShaderClass baseline;
		baseline.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
		DX8Wrapper::Set_Shader(baseline);
		DX8Wrapper::Set_Material(NULL);
		DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_VIEW, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION, Matrix4x4(true));
		DX8Wrapper::Apply_Render_State_Changes();
		CameraClass camera;
		RenderInfoClass render_info(camera);
		const std::string before_no_frame = device.snapshot();
		bool no_frame_rejected = false;
		try { TheHeightMap->Render(render_info); }
		catch (const std::runtime_error &) { no_frame_rejected = true; }
		require(no_frame_rejected && draw_count(device.snapshot().substr(before_no_frame.size())) == 0,
			"original terrain visual accepted a draw without source frame");

		zh::renderer::TextureDesc target;
		target.width = target.height = 32;
		target.render_target = true;
		target.sampled = false;
		target.format = zh::renderer::TextureFormat::bgra8;
		const auto color = device.create_texture(target, "original terrain visual color");
		target.format = zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth = device.create_texture(target, "original terrain visual depth");
		const std::string before_draw = device.snapshot();
		edge.bind_frame_targets(color, depth, 32, 32);
		edge.begin_source_frame(true, true, 0, 0, 0, 1);
		const auto live_prop_resources=device.resource_counts();
		const auto live_prop_trace=device.snapshot();
		const auto live_prop_refs=TheHeightMap->Num_Refs();
		const auto live_prop_buffers=W3DTerrainPropLifecycleProbeAccess::image(*TheHeightMap);
		const auto live_prop_mappings=W3DTerrainPropLifecycleProbeAccess::mappings(edge);
		const auto rejects_any=[](auto action) { try { action(); } catch (...) { return true; } return false; };
		require(rejects_any([&] { preflightClientTerrainRemovalAdmission(); }) &&
			rejects_any([&] { TheGameClient->reset(); }) &&
			rejects_any([&] { TheGameLogic->reset(); }) &&
			rejects_any([&] { TheGameEngine->reset(); }) &&
			rejects_any([&] { TheSubsystemList->resetAll(); }) &&
			rejects_any([&] { TheSubsystemList->shutdownAll(); }) &&
			rejects_any([&] { visual->reset(); }) &&
			rejects_any([&] { TheHeightMap->freeMapResources(); }) &&
			rejects_any([&] { TheHeightMap->ReleaseResources(); }) &&
			rejects_any([&] { TheHeightMap->removeAllProps(); }),
			"props-only active source frame bypassed lifecycle preflight");
		require(TheHeightMap->hasLiveProps() && TheHeightMap->Num_Refs()==live_prop_refs &&
			W3DTerrainPropLifecycleProbeAccess::image(*TheHeightMap)==live_prop_buffers &&
			W3DTerrainPropLifecycleProbeAccess::mappings(edge)==live_prop_mappings &&
			device.resource_counts()==live_prop_resources && device.snapshot()==live_prop_trace &&
			TheHeightMap->getMap()==visual->getLogicHeightMap(),
			"props-only lifecycle rejection changed accepted owner/resources");
		edge.abort_source_frame();
		preflightClientTerrainRemovalAdmission();
		TheHeightMap->ReleaseResources();TheHeightMap->ReAcquireResources();
		require(TheHeightMap->hasLiveProps(), "prop device recreation changed logical owner");
		TheHeightMap->notifyShroudChanged();
		TheHeightMap->removeAllProps();
		require(!TheHeightMap->hasLiveProps(), "idle prop cleanup retry retained instances");
		edge.begin_source_frame(true, true, 0, 0, 0, 1);
		try {
			TheHeightMap->Render(render_info);
			edge.end_source_frame(false);
		} catch (...) {
			edge.abort_source_frame();
			throw;
		}
		const std::string draws = device.snapshot().substr(before_draw.size());
		const std::string range = "count=42 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		require(draw_count(draws) == 14 && draws.find(range) != std::string::npos &&
			draws.find(range, draws.find(range) + 1) != std::string::npos,
			"original terrain visual changed two-pass terrain submission");
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL, 0);
		TheHeightMap->freeMapResources();
		require(!TheHeightMap->getMap() && !TheHeightMap->hasPropBuffer(),
			"direct idle terrain free retained prop/map ownership");
		preflightClientTerrainRemovalAdmission();
		edge.begin_source_frame(true,true,0,0,0,1);
		require(rejects_any([&] { preflightClientTerrainRemovalAdmission(); }),
			"empty post-free terrain callback admitted active frame");
		edge.abort_source_frame();
		TheHeightMap->freeMapResources();
		preflightClientTerrainRemovalAdmission();
		device.destroy(depth);
		device.destroy(color);

		visual.reset();
		require(!TheTerrainVisual && !TheHeightMap && !TheTerrainRenderObject,
			"original terrain visual unload retained map owner");
		TheTerrainVisual = saved_visual;
		edge.release_source_buffers();
		TheDisplay = saved_display;
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original terrain visual re-entry retained Recording resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original terrain visual map: attach=1 rollback=3 draws=14 props=2 generations=2 resources=0");
}
