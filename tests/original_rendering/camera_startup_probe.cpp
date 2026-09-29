#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/TerrainLogic.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DPropBuffer.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/camerashakesystem.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/coltest.h"
#include "WW3D2/statistics.h"
#include "colmath.h"
#include "tri.h"
#include "WWLib/RAMFILE.H"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <vector>
#include <fstream>
#include <iterator>
#include <type_traits>

namespace {
void require(bool value,const char *message) { if (!value) throw std::runtime_error(message); }
template<typename F> bool rejected(F operation)
{
	try { operation(); } catch (...) { return true; }
	return false;
}
bool same(const Matrix3D &a,const Matrix3D &b)
{
	for (int row=0;row<3;++row) for (int column=0;column<4;++column)
		if (a[row][column]!=b[row][column]) return false;
	return true;
}
bool nativeRayReference(BaseHeightMapRenderObjClass &terrain,RayCollisionTestClass &raytest)
{
	auto *m_map=terrain.getMap();
	auto getClipHeight=[&](Int x,Int y) { return terrain.getClipHeight(x,y); };
#define ADJUST_FROM_INDEX_TO_REAL(k) (((k)-m_map->getBorderSizeInline())*MAP_XY_FACTOR)
#include "native_ray.inc"
#undef ADJUST_FROM_INDEX_TO_REAL
}
}

struct W3DCameraStartupGeneratedProbeAccess {
	struct Snapshot {
		std::vector<Real> values;
		std::vector<std::vector<unsigned char>> cpu,backup,native;
		std::vector<const void *> sources,backups;
		std::vector<zh::renderer::BufferHandle> handles;
		std::vector<Int> refs;
		zh::renderer::ResourceCounts resources;
		std::string trace;
		unsigned long bits=0;
		UnsignedInt rngSeed=0,rng[6]{},clientRng[6]{};
		std::vector<const void *> propOwners;
		std::vector<Int> propRefs;
	};
	template<class Device> static Snapshot snapshot(W3DView &view,zh::original_runtime::OriginalGpuEdge &edge,
		Device &device)
	{
		Snapshot state;auto &values=state.values;auto &camera=*view.m_3DCamera;
		auto vector=[&](const Vector3 &value) { values.insert(values.end(),{value.X,value.Y,value.Z}); };
		auto matrix=[&](const Matrix3D &value) {
			for (int r=0;r<3;++r) for (int c=0;c<4;++c) values.push_back(value[r][c]);
		};
		auto frustum=[&](const FrustumClass &value) {
			matrix(value.CameraTransform);vector(value.BoundMin);vector(value.BoundMax);
			for (const auto &corner:value.Corners) vector(corner);
			for (const auto &plane:value.Planes) { vector(plane.N);values.push_back(plane.D); }
		};
		values.insert(values.end(),{view.m_pos.x,view.m_pos.y,view.m_pos.z,view.m_cameraOffset.x,
			view.m_cameraOffset.y,view.m_cameraOffset.z,view.m_cameraConstraint.lo.x,view.m_cameraConstraint.lo.y,
			view.m_cameraConstraint.hi.x,view.m_cameraConstraint.hi.y,view.m_angle,view.m_pitchAngle,view.m_zoom,
			view.m_heightAboveGround,view.m_groundLevel,view.m_FXPitch,Real(view.m_cameraConstraintValid),
			Real(view.m_cameraHasMovedSinceRequest),Real(camera.FrustumValid),Real(camera.IsTransformIdentity),
			camera.AspectRatio,camera.ZNear,camera.ZFar,camera.ZBufferMin,camera.ZBufferMax,
			Real(camera.Projection),camera.Viewport.Min.X,camera.Viewport.Min.Y,camera.Viewport.Max.X,
			camera.Viewport.Max.Y,camera.ViewPlane.Min.X,camera.ViewPlane.Min.Y,camera.ViewPlane.Max.X,
			camera.ViewPlane.Max.Y,Real(TheHeightMap->m_needFullUpdate)});
		matrix(camera.Transform);matrix(camera.CameraInvTransform);
		for (int r=0;r<4;++r) for (int c=0;c<4;++c) values.push_back(camera.ProjectionTransform[r][c]);
		frustum(camera.Frustum);frustum(camera.ViewSpaceFrustum);
		vector(camera.NearClipBBox.Center);vector(camera.NearClipBBox.Extent);
		for (int r=0;r<3;++r) for (int c=0;c<3;++c) values.push_back(camera.NearClipBBox.Basis[r][c]);
		vector(camera.CachedBoundingSphere.Center);values.push_back(camera.CachedBoundingSphere.Radius);
		vector(camera.CachedBoundingBox.Center);vector(camera.CachedBoundingBox.Extent);state.bits=camera.Bits;
		auto *props=TheHeightMap->m_propBuffer;state.propOwners.push_back(props);
		if (props) {
			values.insert(values.end(),{Real(props->m_numProps),Real(props->m_numPropTypes),
				Real(props->m_doCull),Real(props->m_anythingChanged),Real(props->m_initialized)});
			for (Int i=0;i<props->m_numProps;++i) {
				const auto &prop=props->m_props[i];state.propOwners.push_back(prop.m_robj);
				state.propRefs.push_back(prop.m_robj->Num_Refs());
				values.insert(values.end(),{Real(prop.id),prop.location.x,prop.location.y,prop.location.z,
					Real(prop.propType),Real(prop.ss),Real(prop.visible),prop.bounds.Radius});
				vector(prop.bounds.Center);matrix(prop.m_robj->Get_Transform_No_Validity_Check());
			}
			for (Int i=0;i<props->m_numPropTypes;++i) {
				const auto &type=props->m_propTypes[i];state.propOwners.push_back(type.m_robj);
				state.propRefs.push_back(type.m_robj->Num_Refs());
				vector(type.m_bounds.Center);values.push_back(type.m_bounds.Radius);
			}
		}
		for (Int i=0;i<TheHeightMap->m_numVertexBufferTiles;++i) {
			auto *source=TheHeightMap->m_vertexBufferTiles[i];
			const auto count=source->Get_Vertex_Count()*source->FVF_Info().Get_FVF_Size();
			auto *bytes=reinterpret_cast<unsigned char *>(TheHeightMap->m_vertexBufferBackup[i]);
			const auto handle=edge.vertices_.at(source);
			state.sources.push_back(source);state.backups.push_back(bytes);state.handles.push_back(handle);
			state.refs.push_back(source->Num_Refs());
			state.cpu.emplace_back(source->Get_CPU_Vertex_Buffer(),source->Get_CPU_Vertex_Buffer()+count);
			state.backup.emplace_back(bytes,bytes+count);
			if constexpr (std::is_same_v<Device,zh::renderer::RecordingGpuDevice>)
				state.native.push_back(device.buffer_bytes(handle));
		}
		if constexpr (std::is_same_v<Device,zh::renderer::RecordingGpuDevice>) {
			state.resources=device.resource_counts();state.trace=device.snapshot();
		} else state.resources.buffers=device.live_resource_count();
		CopyGameLogicRandomState(&state.rngSeed,state.rng);CopyGameClientRandomState(state.clientRng);return state;
	}
	static bool unchanged(const Snapshot &a,const Snapshot &b)
	{
		return a.values==b.values && a.cpu==b.cpu && a.backup==b.backup && a.native==b.native &&
			a.sources==b.sources && a.backups==b.backups && a.handles==b.handles && a.refs==b.refs &&
			a.resources==b.resources && a.trace==b.trace && a.bits==b.bits && a.rngSeed==b.rngSeed &&
			a.propOwners==b.propOwners && a.propRefs==b.propRefs &&
			std::equal(std::begin(a.rng),std::end(a.rng),std::begin(b.rng)) &&
			std::equal(std::begin(a.clientRng),std::end(a.clientRng),std::begin(b.clientRng));
	}
	static void providers(W3DView &view,zh::original_runtime::OriginalGpuEdge &edge,
		zh::renderer::RecordingGpuDevice &device)
	{
		const auto before=snapshot(view,edge,device);Coord3D target{30,40,0};
		auto substitute=[&](auto &slot,auto replacement) {
			const auto saved=slot;slot=replacement;
			const bool failed=rejected([&]{view.lookAt(&target);});slot=saved;
			require(failed && unchanged(before,snapshot(view,edge,device)),
				"camera foreign/removed provider changed exact source/device/RNG baseline");
		};
		substitute(TheTerrainLogic,static_cast<TerrainLogic *>(nullptr));
		substitute(TheTerrainLogic,reinterpret_cast<TerrainLogic *>(1));
		substitute(TheDisplay,reinterpret_cast<Display *>(1));
		substitute(TheTerrainVisual,reinterpret_cast<TerrainVisual *>(1));
		substitute(TheHeightMap,reinterpret_cast<HeightMapRenderObjClass *>(1));
		substitute(TheTerrainRenderObject,reinterpret_cast<BaseHeightMapRenderObjClass *>(1));
		substitute(W3DDisplay::m_3DScene,reinterpret_cast<RTS3DScene *>(1));
		substitute(TheHeightMap->m_map,reinterpret_cast<WorldHeightMap *>(1));
		substitute(TheHeightMap->m_propBuffer,reinterpret_cast<W3DPropBuffer *>(1));
		substitute(view.m_3DCamera,reinterpret_cast<CameraClass *>(1));
		substitute(edge.generation_,edge.generation_+1);
		view.lookAt(&target);
	}
	static unsigned faults(W3DView &view,zh::original_runtime::OriginalGpuEdge &edge,
		zh::renderer::RecordingGpuDevice &device)
	{
		unsigned count=0;Coord3D elevated{30,40,400};
		auto apply=[&](int operation) {
			if (operation==0) view.setAngleAndPitchToDefault();
			if (operation==1) view.lookAt(&elevated);
			if (operation==2) view.initHeightForMap();
			if (operation==3) view.setZoomToDefault();
		};
		struct Clear { ~Clear() { W3DView::setCameraStartupFaultOrdinal(-1); } } clear;
		for (int operation=0;operation<4;++operation) {
			const auto old=TheHeightMap->getClipHeight(1,1);
			TheHeightMap->getMap()->setRawHeight(1,1,old==255 ? 254 : old+1);
			TheHeightMap->m_needFullUpdate=TRUE;view.m_cameraConstraintValid=FALSE;
			auto before=snapshot(view,edge,device);
			device.fail_next_buffer_upload();
			require(rejected([&]{apply(operation);}) && unchanged(before,snapshot(view,edge,device)),
				"camera terrain upload failed exact typed rollback");
			apply(operation);++count;
			require(before.native!=snapshot(view,edge,device).native,"camera upload fault did not reach changed terrain bytes");
			for (int ordinal=0;ordinal<64;++ordinal) {
				TheHeightMap->m_needFullUpdate=TRUE;view.m_cameraConstraintValid=FALSE;
				before=snapshot(view,edge,device);view.setCameraStartupFaultOrdinal(ordinal);
				const bool failed=rejected([&]{apply(operation);});
				view.setCameraStartupFaultOrdinal(-1);
				if (!failed) break;
				require(unchanged(before,snapshot(view,edge,device)),"camera query/allocation/commit failed exact typed rollback");
				apply(operation);++count;
				require(ordinal<63,"camera fault sweep did not reach bounded success");
			}
			TheHeightMap->getMap()->setRawHeight(1,1,old);TheHeightMap->m_needFullUpdate=TRUE;apply(operation);
		}
		return count;
	}
	static void invalidateTerrain() { TheHeightMap->m_needFullUpdate=TRUE; }
	static void fault(Int ordinal) { W3DView::setCameraStartupFaultOrdinal(ordinal); }
	static void modes(W3DView &view,zh::original_runtime::OriginalGpuEdge &edge,
		zh::renderer::RecordingGpuDevice &device)
	{
		const auto before=snapshot(view,edge,device);Coord3D target{30,40,0};
		auto substitute=[&](auto &slot,auto replacement) {
			const auto saved=slot;slot=replacement;
			const bool failed=rejected([&]{view.lookAt(&target);});slot=saved;
			require(failed && unchanged(before,snapshot(view,edge,device)),
				"camera excluded mode/capacity changed typed source/device/RNG baseline");
		};
		substitute(view.m_cameraLock,ObjectID(1));
		substitute(view.m_cameraLockDrawable,reinterpret_cast<Drawable *>(1));
		substitute(view.m_doingMoveCameraOnWaypointPath,TRUE);
		substitute(view.m_doingRotateCamera,TRUE);substitute(view.m_doingPitchCamera,TRUE);
		substitute(view.m_doingZoomCamera,TRUE);substitute(view.m_doingScriptedCameraLock,TRUE);
		substitute(view.m_isCameraSlaved,TRUE);substitute(view.m_useRealZoomCam,TRUE);
		substitute(view.m_freezeTimeForCameraMovement,TRUE);
		substitute(view.m_isWireFrameEnabled,TRUE);substitute(view.m_nextWireFrameEnabled,TRUE);
		substitute(view.m_shakeIntensity,Real(1));substitute(view.m_width,Int(0));
		substitute(TheHeightMap->m_numVertexBufferTiles,Int(4097));
		for (Real value:{std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),
			-std::numeric_limits<Real>::infinity()}) {
			Coord3D invalid{value,40,0};
			require(rejected([&]{view.lookAt(&invalid);}) && unchanged(before,snapshot(view,edge,device)),
				"camera nonfinite lookAt changed typed baseline");
			const auto saved=TheGlobalData->m_cameraPitch;TheWritableGlobalData->m_cameraPitch=value;
			const bool failed=rejected([&]{view.initHeightForMap();});TheWritableGlobalData->m_cameraPitch=saved;
			require(failed && unchanged(before,snapshot(view,edge,device)),"camera nonfinite divisor changed baseline");
		}
		zh::renderer::TextureDesc desc;desc.width=32;desc.height=24;desc.render_target=true;
		desc.format=zh::renderer::TextureFormat::rgba8;
		const auto color=device.create_texture(desc,"camera rejected-frame color");
		desc.format=zh::renderer::TextureFormat::depth24_stencil8;desc.sampled=false;
		const auto depth=device.create_texture(desc,"camera rejected-frame depth");
		require(color && depth,"camera generated frame targets rejected");edge.bind_frame_targets(color,depth,32,24);
		for (bool active:{false,true}) {
			require(edge.begin_tree_source_frame(),"camera generated pending journal rejected");
			if (active) edge.begin_source_frame(true,true,0,0,0,1);
			const auto pending=snapshot(view,edge,device);
			require(rejected([&]{view.lookAt(&target);}) && rejected([&]{view.reset();}) &&
				unchanged(pending,snapshot(view,edge,device)),"camera pending/active frame changed baseline");
			if (active) edge.abort_source_frame();
			require(edge.abort_tree_source_frame(),"camera rejected journal abort failed");
		}
		device.destroy(depth);device.destroy(color);
		auto *shroud=TheHeightMap->getShroud();shroud->render(view.get3DCamera());
		zh::original_runtime::OriginalGpuEdge::SourceStageSelection selection{1,shroud->getShroudTexture()};
		zh::original_runtime::OriginalGpuEdge::SourceStageDesc stage;
		stage.generation=edge.generation();stage.selections=&selection;stage.count=1;
		zh::original_runtime::OriginalGpuEdge::SourceStageToken token;
		require(bool(edge.begin_source_stages(stage,token)),"camera selected-stage fixture rejected");
		const auto selected=snapshot(view,edge,device);
		require(rejected([&]{view.lookAt(&target);}) && unchanged(selected,snapshot(view,edge,device)) &&
			!edge.commit_source_stages(token),"camera source-stage guard did not poison exact owner");
		require(edge.abort_source_stages(token),"camera poisoned source-stage abort failed");
		require(edge.queued_source_reference_count()==1 && edge.drain_source_references(edge.generation()) &&
			!edge.queued_source_reference_count(),"camera rejected stage pin idle drain failed");
		view.reset();view.setAngleAndPitchToDefault();view.setZoomToDefault();view.lookAt(&target);
		view.initHeightForMap();view.updateView();formula(view);
	}
	static void freshCaches()
	{
		CameraClass fresh;
		require(!fresh.FrustumValid,"fresh camera initialization eagerly validates cache");
		for (int r=0;r<3;++r) for (int c=0;c<4;++c)
			require(std::isfinite(fresh.CameraInvTransform[r][c]),"fresh inverse cache backing undefined");
		for (int r=0;r<4;++r) for (int c=0;c<4;++c)
			require(std::isfinite(fresh.ProjectionTransform[r][c]),"fresh projection cache backing undefined");
		(void)fresh.Get_Frustum();require(fresh.FrustumValid,"camera lazy validation missing");
		CameraClass copy(fresh);require(!copy.FrustumValid,"camera copy changed lazy validity");
		(void)copy.Get_Frustum();
		require(copy.FrustumValid && same(copy.CameraInvTransform,fresh.CameraInvTransform),
			"camera copied cache changed lazy success semantics");
	}
	static void shaker(W3DView &view,zh::original_runtime::OriginalGpuEdge &edge,
		zh::renderer::RecordingGpuDevice &device)
	{
		require(!CameraShakerSystem.IsCameraShaking(),"camera fixture has foreign shake owner");
		CameraShakerSystem.Add_Camera_Shake(Vector3(0,0,0),50,1,1);
		struct Clear { ~Clear() { CameraShakerSystem.Timestep(2); } } clear;
		const auto before=snapshot(view,edge,device);
		require(CameraShakerSystem.IsCameraShaking() && rejected([&]{view.setZoomToDefault();}) &&
			unchanged(before,snapshot(view,edge,device)),"active shaker camera admission changed source/device/RNG");
	}
	static Real ground(const W3DView &view) { return view.m_groundLevel; }
	static void rays(W3DView &view)
	{
		const std::pair<Vector3,Vector3> cases[]={
			{Vector3(30,40,400),Vector3(30,40,-100)},
			{Vector3(-20,-10,400),Vector3(-20,-10,-100)},
			{Vector3(70,70,400),Vector3(100,100,-100)},
			{Vector3(2000,2000,2000),Vector3(2200,2100,2100)},
			{Vector3(0,0,500),Vector3(50,500,-100)},
			{Vector3(10,10,5),Vector3(10,10,-5)}};
		unsigned hits=0,misses=0;
		for (const auto &entry:cases) {
			CastResultStruct result;RayCollisionTestClass reference(LineSegClass(entry.first,entry.second),&result);
			const bool expected=nativeRayReference(*TheHeightMap,reference);
			Vector3 actual(7,8,9);const bool hit=view.castCameraStartupRay(entry.first,entry.second,&actual);
			require(hit==expected && (hit ? actual==result.ContactPoint : actual==Vector3(7,8,9)),
				"camera bounded ray changed native sample/intersection order");
			if (hit) ++hits;else ++misses;
		}
		require(hits && misses,"camera ray witness omitted hit/miss");
		Vector3 unchanged(7,8,9);
		require(rejected([&]{view.castCameraStartupRay(Vector3(1,2,3),Vector3(1,2,3),&unchanged);}) &&
			unchanged==Vector3(7,8,9),"camera degenerate ray changed output");
		for (Real value:{std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),
			-std::numeric_limits<Real>::infinity()})
			require(rejected([&]{view.castCameraStartupRay(Vector3(value,2,3),Vector3(4,5,6),&unchanged);}) &&
				unchanged==Vector3(7,8,9),"camera nonfinite ray changed output");
		require(view.cameraStartupRayWorkAllowed(4194304) && !view.cameraStartupRayWorkAllowed(4194305),
			"camera ray work maximum/bound+1 mismatch");
	}
	static void formula(W3DView &view)
	{
		const Coord3D position=*view.getPosition();
		const Real z=view.m_cameraOffset.z*view.getZoom();
		Vector3 source(view.m_cameraOffset.x*view.getZoom(),view.m_cameraOffset.y*view.getZoom(),z);
		Matrix3D pitch(Vector3(1,0,0),view.getPitch()),angle(Vector3(0,0,1),view.getAngle());
		pitch.mulVector3(source);angle.mulVector3(source);
		source*=1-view.m_groundLevel/z;
		source+=Vector3(position.x,position.y,view.m_groundLevel);
		Matrix3D expected(1);expected.Look_At(source,Vector3(position.x,position.y,view.m_groundLevel),0);
		require(same(expected,view.get3DCamera()->Get_Transform()),"camera native stationary formula mismatch");
	}
};

namespace {
void physicalCamera(const char *map_path)
{
	const auto height=TheGlobalData->m_cameraHeight,minimum=TheGlobalData->m_minCameraHeight,
		maximum=TheGlobalData->m_maxCameraHeight;
	struct RestoreSettings {
		Real height,minimum,maximum;
		~RestoreSettings() { TheWritableGlobalData->m_cameraHeight=height;
			TheWritableGlobalData->m_minCameraHeight=minimum;TheWritableGlobalData->m_maxCameraHeight=maximum; }
	} settings{height,minimum,maximum};
	TheWritableGlobalData->m_cameraHeight=60;TheWritableGlobalData->m_minCameraHeight=50;
	TheWritableGlobalData->m_maxCameraHeight=100;
	require(SDL_Init(SDL_INIT_VIDEO),"camera physical video services rejected");
	auto *window=SDL_CreateWindow("generated native camera",32,24,SDL_WINDOW_HIDDEN);
	require(window,"camera physical window rejected");
	struct Window { SDL_Window *value;~Window() { SDL_DestroyWindow(value);SDL_Quit(); } } owned_window{window};
	unsigned boundaries=0;
	for (unsigned generation=0;generation<2;++generation) {
		zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
		zh::renderer::BgfxGpuDevice device(options);require(device.claim_window(window),"camera physical claim rejected");
		{
			zh::original_runtime::OriginalGpuEdge edge(device);
			{
				W3DDisplay display;display.init();
				struct Restore {
					Display *display;TerrainVisual *visual;View *view;
					~Restore() { TheTacticalView=view;TheTerrainVisual=visual;TheDisplay=display; }
				} restore{TheDisplay,TheTerrainVisual,TheTacticalView};
				TheDisplay=&display;display.setWidth(32);display.setHeight(24);
				W3DTerrainVisual visual;TheTerrainVisual=&visual;visual.init();
				require(visual.load(AsciiString(map_path)),"camera physical map rejected");
				auto *view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
				view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
				view->setAngleAndPitchToDefault();view->setZoomToDefault();
				Coord3D target{30,40,0};view->lookAt(&target);view->initHeightForMap();
				view->setAngleAndPitchToDefault();view->setZoomToDefault();
				W3DCameraStartupGeneratedProbeAccess::formula(*view);
				zh::renderer::TextureDesc desc;desc.width=32;desc.height=24;desc.render_target=true;
				desc.format=zh::renderer::TextureFormat::rgba8;desc.sampled=true;
				const auto color=device.create_texture(desc,"camera physical color");
				desc.format=zh::renderer::TextureFormat::depth24_stencil8;desc.sampled=false;
				const auto depth=device.create_texture(desc,"camera physical depth");
				require(color && depth,"camera physical targets rejected");edge.bind_frame_targets(color,depth,32,24);
				TheHeightMap->getShroud()->fillShroudData(255);
				display.draw();require(device.present(color),"camera physical source present rejected");
				const auto pixels=device.readback_rgba(color);unsigned nonblack=0;
				for (std::size_t i=0;i<pixels.size();i+=4) if(pixels[i]||pixels[i+1]||pixels[i+2]) ++nonblack;
				require(pixels.size()==32*24*4 && nonblack && nonblack<=32*24,
					"camera physical native-camera terrain pixels absent");
				for (Int ordinal=0;ordinal<64;++ordinal) {
					W3DCameraStartupGeneratedProbeAccess::invalidateTerrain();
					const auto before=W3DCameraStartupGeneratedProbeAccess::snapshot(*view,edge,device);
					W3DCameraStartupGeneratedProbeAccess::fault(ordinal);
					const bool failed=rejected([&]{view->lookAt(&target);});W3DCameraStartupGeneratedProbeAccess::fault(-1);
					if (!failed) break;
					require(W3DCameraStartupGeneratedProbeAccess::unchanged(before,
						W3DCameraStartupGeneratedProbeAccess::snapshot(*view,edge,device)) &&
						device.readback_rgba(color)==pixels,"camera physical typed/pixel rollback changed accepted identity");
					view->lookAt(&target);display.draw();require(device.present(color),"camera physical retry present rejected");
					require(device.readback_rgba(color)==pixels,"camera physical same-owner retry pixels differ");++boundaries;
					require(ordinal<63,"camera physical fault sweep exceeded bound");
				}
				Coord3D elevated{30,40,400};view->lookAt(&elevated);
				W3DCameraStartupGeneratedProbeAccess::formula(*view);view->updateView();
				view->lookAt(&target);display.draw();require(device.present(color),"camera physical ground regression present rejected");
				require(device.readback_rgba(color)==pixels,"camera physical elevated/ground regression pixels differ");
				device.destroy(depth);device.destroy(color);
			}
			edge.release_source_buffers();
		}
		device.release_window();require(device.wait_idle() && !device.live_resource_count(),"camera physical resources retained");
	}
	Debug_Statistics::Shutdown_Statistics();
	std::printf("original camera startup physical: boundaries=%u retry=1 generations=2 resources=0\n",boundaries);
}
}

extern "C" void zh_probe_camera_startup()
{
	const char *map_path=std::getenv("ZH_M22_CAMERA_STARTUP_MAP");
	const char *asset_path=std::getenv("ZH_M22_CAMERA_STARTUP_ASSET");
	require(map_path && asset_path && TheTerrainLogic,"camera generated provider missing");
	struct Globals {
		Real partition=TheGlobalData->m_partitionCellSize,height=TheGlobalData->m_cameraHeight,
			pitch=TheGlobalData->m_cameraPitch,yaw=TheGlobalData->m_cameraYaw,
			minimum=TheGlobalData->m_minCameraHeight,maximum=TheGlobalData->m_maxCameraHeight;
		~Globals() {
			TheWritableGlobalData->m_partitionCellSize=partition;TheWritableGlobalData->m_cameraHeight=height;
			TheWritableGlobalData->m_cameraPitch=pitch;TheWritableGlobalData->m_cameraYaw=yaw;
			TheWritableGlobalData->m_minCameraHeight=minimum;TheWritableGlobalData->m_maxCameraHeight=maximum;
		}
	} globals;
	TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
	TheWritableGlobalData->m_cameraHeight=180;TheWritableGlobalData->m_cameraPitch=37.5f;
	TheWritableGlobalData->m_cameraYaw=0;TheWritableGlobalData->m_minCameraHeight=100;
	TheWritableGlobalData->m_maxCameraHeight=300;
	require(TheTerrainLogic->loadMap(AsciiString(map_path),TRUE),"camera generated logical map rejected");
	zh::renderer::RecordingGpuDevice device;
	W3DCameraStartupGeneratedProbeAccess::freshCaches();
	unsigned typed_faults=0;
	for (Int generation=0;generation!=2;++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		{
			W3DDisplay display;display.init();
			Display *saved_display=TheDisplay;TerrainVisual *saved_visual=TheTerrainVisual;
			View *saved_view=TheTacticalView;
			struct Restore {
				Display *display;TerrainVisual *visual;View *view;
				~Restore() { TheTacticalView=view;TheTerrainVisual=visual;TheDisplay=display; }
			} restore{saved_display,saved_visual,saved_view};
			TheDisplay=&display;display.setWidth(32);display.setHeight(24);
			W3DTerrainVisual visual;TheTerrainVisual=&visual;
			visual.init();require(visual.load(AsciiString(map_path)),"camera generated visual map rejected");
			auto *view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
			view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
			view->setAngleAndPitchToDefault();
			require(view->getAngle()==0 && view->getPitch()==0,"camera defaults changed authored values");
			Coord3D previous;view->getPosition(&previous);
			Real maximum=TheTerrainLogic->getGroundHeight(previous.x,previous.y);
			for (int x:{-40,40}) for (int y:{-40,40}) maximum=std::max(maximum,
				TheTerrainLogic->getGroundHeight(previous.x+x,previous.y+y));
			view->setZoomToDefault();
			require(view->getZoom()==(maximum+300)/180,"camera default zoom changed five-sample source rule");
			Coord3D target{30,40,0};view->lookAt(&target);view->initHeightForMap();
			Coord3D ground_position;view->getPosition(&ground_position);
			require(W3DCameraStartupGeneratedProbeAccess::ground(*view)==std::min(
				TheTerrainLogic->getGroundHeight(ground_position.x,ground_position.y),120.0f),
				"camera map height changed native cap");
			view->setAngleAndPitchToDefault();view->setZoomToDefault();
			W3DCameraStartupGeneratedProbeAccess::formula(*view);
			const Matrix3D stationary=view->get3DCamera()->Get_Transform();
			Coord3D stationary_position;view->getPosition(&stationary_position);
			require(stationary_position.x!=0 || stationary_position.y!=0,"camera stationary witness remained zero");
			view->updateView();view->updateView();
			require(same(stationary,view->get3DCamera()->Get_Transform()),"camera stationary update advanced transform");
			const auto counts=device.resource_counts();const auto before=device.snapshot();
			device.fail_next_transaction_checkpoint();
			require(rejected([&]{view->lookAt(&target);}) && same(stationary,view->get3DCamera()->Get_Transform()) &&
				counts==device.resource_counts() && before==device.snapshot(),"camera checkpoint rejection changed baseline");
			view->lookAt(&target);W3DCameraStartupGeneratedProbeAccess::formula(*view);
			W3DCameraStartupGeneratedProbeAccess::rays(*view);
			const Matrix3D elevated_before=view->get3DCamera()->Get_Transform();
			Coord3D elevated{30,40,400};
			device.fail_next_transaction_checkpoint();
			require(rejected([&]{view->lookAt(&elevated);}) &&
				same(elevated_before,view->get3DCamera()->Get_Transform()),"camera elevated rejection changed transform");
			view->lookAt(&elevated);W3DCameraStartupGeneratedProbeAccess::formula(*view);
			view->updateView();
			typed_faults+=W3DCameraStartupGeneratedProbeAccess::faults(*view,edge,device);
			W3DCameraStartupGeneratedProbeAccess::providers(*view,edge,device);
			W3DCameraStartupGeneratedProbeAccess::formula(*view);
			W3DCameraStartupGeneratedProbeAccess::shaker(*view,edge,device);
			view->setAngleAndPitchToDefault();view->setZoomToDefault();view->updateView();
			std::ifstream input(asset_path,std::ios::binary);
			std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
			RAMFileClass packet(bytes.data(),static_cast<int>(bytes.size()));
			require(!bytes.empty() && static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
				"camera generated prop asset rejected");
			const auto *model=TheThingFactory->findTemplate(AsciiString("CameraPropFixture"),FALSE);
			require(model,"camera generated prop template missing");
			const Coord3D prop_position{30,40,TheHeightMap->getMap()->getDisplayHeight(3,4)*MAP_HEIGHT_SCALE+4};
			visual.addProp(model,&prop_position,0);
			require(TheHeightMap->hasLiveProps(),"camera prop-installed owner absent");
			typed_faults+=W3DCameraStartupGeneratedProbeAccess::faults(*view,edge,device);
			W3DCameraStartupGeneratedProbeAccess::providers(*view,edge,device);
			W3DCameraStartupGeneratedProbeAccess::rays(*view);
			W3DCameraStartupGeneratedProbeAccess::shaker(*view,edge,device);
			view->setAngleAndPitchToDefault();view->setZoomToDefault();view->updateView();
			W3DCameraStartupGeneratedProbeAccess::formula(*view);
			W3DCameraStartupGeneratedProbeAccess::modes(*view,edge,device);
		}
		require(!TheHeightMap && !TheTerrainRenderObject,"camera teardown retained terrain owner");
		edge.release_source_buffers();
	}
	require(device.resource_counts().total()==0,"camera teardown retained device resources");
	std::printf("original camera startup typed faults: count=%u\n",typed_faults);
	std::puts("original camera startup: ground-default=1 stationary=1 retry=1 generations=2 resources=0");
	if (std::getenv("ZH_M22_CAMERA_STARTUP_PHYSICAL")) physicalCamera(map_path);
}
