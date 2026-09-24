#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "GameClient/Smudge.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename F> bool rejected(F f) { try { f(); } catch (const std::runtime_error &) { return true; } return false; }
unsigned draws(const std::string &trace) { unsigned n=0; for (std::size_t p=trace.find("draw pipeline="); p!=std::string::npos; p=trace.find("draw pipeline=",p+1)) ++n; return n; }
unsigned before(const std::string &trace, const char *needle, std::size_t end) { unsigned n=0; for (std::size_t p=trace.find(needle);p!=std::string::npos&&p<end;p=trace.find(needle,p+1)) ++n; return n; }
bool near(Real left, Real right) { return std::fabs(left-right)<0.0001f; }
class ScalarProbeWater final : public WaterRenderObjClass {
public:
	bool scalar_state_is(Real low, Real high, Real a, Real b, Real c, Real range) const {
		return near(m_minGridHeight,low) && near(m_maxGridHeight,high) &&
			near(m_gridChangeAtt0,a) && near(m_gridChangeAtt1,b) && near(m_gridChangeAtt2,c) &&
			near(m_gridChangeMaxRange,range/m_gridCellSize) && near(m_gridCellsX,4) &&
			near(m_gridCellsY,5) && near(m_gridCellSize,2);
	}
};
class GridProbeVisual final : public W3DTerrainVisual {
public:
	void forceGridState(Bool enabled) { m_isWaterGridRenderingEnabled = enabled; }
};
}

extern "C" void zh_probe_terrain_water()
{
	const char *map = std::getenv("ZH_M22_TERRAIN_WATER_MAP");
	require(map, "original terrain-water map fixture missing");
	const Real saved_partition = TheGlobalData->m_partitionCellSize;
	const Int saved_tracks = TheGlobalData->m_maxTerrainTracks;
	const Bool saved_marks = TheGlobalData->m_makeTrackMarks;
	const Bool saved_water = TheGlobalData->m_useWaterPlane;
	const Bool saved_cloud = TheGlobalData->m_useCloudPlane;
	const Real saved_x = TheGlobalData->m_waterExtentX, saved_y = TheGlobalData->m_waterExtentY;
	const Int saved_type = TheGlobalData->m_waterType;
	TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
	TheWritableGlobalData->m_maxTerrainTracks=1; TheWritableGlobalData->m_makeTrackMarks=TRUE;
	TheWritableGlobalData->m_useWaterPlane=TRUE;
	TheWritableGlobalData->m_waterExtentX=32; TheWritableGlobalData->m_waterExtentY=24;
	TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation=0; generation!=3; ++generation) {
		const Bool cloud = generation != 0;
		TheWritableGlobalData->m_useCloudPlane=cloud;
		zh::original_runtime::OriginalGpuEdge edge(device);
		{
			W3DDisplay display; display.init(); Display *saved_display=TheDisplay; TheDisplay=&display;
			TerrainVisual *saved_visual=TheTerrainVisual; GridProbeVisual visual; TheTerrainVisual=&visual;
			View *saved_view=TheTacticalView; W3DView *view=NULL;
			try {
				Real grid_height=31337;
				require(rejected([&]{visual.getWaterGridHeight(0,0,&grid_height);}),
					"original pre-init water grid query accepted");
				display.setWidth(32); display.setHeight(24); visual.init();
				require(!visual.getWaterGridHeight(0,0,&grid_height) && grid_height==31337 &&
					!visual.getWaterGridHeight(0,0,NULL),
					"original disabled water grid query changed height");
				visual.enableWaterGrid(FALSE);
				require(!visual.getWaterGridHeight(0,0,&grid_height) && grid_height==31337 &&
					rejected([&]{visual.enableWaterGrid(TRUE);}) &&
					!visual.getWaterGridHeight(0,0,&grid_height),
					"original disabled grid state changed after active request");
				visual.forceGridState(TRUE);
				require(rejected([&]{visual.getWaterGridHeight(0,0,&grid_height);}) &&
					grid_height==31337, "original active grid query accepted");
				visual.forceGridState(FALSE);
				WaterRenderObjClass *published_water=TheWaterRenderObj; TheWaterRenderObj=NULL;
				require(rejected([&]{visual.getWaterGridHeight(0,0,&grid_height);}),
					"original missing water provider accepted");
				TheWaterRenderObj=published_water;
				TheTerrainVisual=saved_visual;
				require(rejected([&]{visual.getWaterGridHeight(0,0,&grid_height);}),
					"original foreign terrain provider accepted");
				TheTerrainVisual=&visual;
				require(TheWaterRenderObj && TheWaterRenderObj->hasPendingGpuResources()==FALSE,
					"original active water did not publish source resources");
				require(visual.load(AsciiString(map)) && TheHeightMap && TheHeightMap->getMap(),
					"original water map did not load terrain owner");
				auto *water=TheWaterRenderObj; auto *tracks=TheTerrainTracksRenderObjClassSystem;
				require(water && tracks && static_cast<void *>(water->Peek_Scene())==static_cast<void *>(W3DDisplay::m_3DScene),
					"original water primary-scene membership failed");
				const std::string allocation=device.snapshot();
				require(allocation.find("size=12 usage=1 dynamic=true")!=std::string::npos &&
					allocation.find("size=96 usage=0 dynamic=true")!=std::string::npos,
					"original water source buffer descriptors failed");
				water->ReleaseResources(); device.fail_next_buffer_create();
				require(rejected([&]{water->ReAcquireResources();}) && water->hasPendingGpuResources(),
					"original water create rollback was not pending");
				require(rejected([&]{visual.getWaterGridHeight(0,0,&grid_height);}),
					"original pending water provider accepted");
				water->ReAcquireResources(); require(!water->hasPendingGpuResources(), "original water create retry failed");
				require(!visual.getWaterGridHeight(0,0,&grid_height) && grid_height==31337,
					"original water grid query did not recover after provider retry");
				view=new W3DView; TheTacticalView=view; view->init(); display.attachView(view);
				view->setWidth(32); view->setHeight(24); view->setDefaultView(0,0,1);
				auto *track=tracks->bindTrack(TheHeightMap,MAP_XY_FACTOR,""); require(track,"original water compatibility track missing");
				track->addEdgeToTrack(0,0); track->addEdgeToTrack(24,0); track->addEdgeToTrack(48,0);
				zh::renderer::TextureDesc target; target.width=32; target.height=24; target.render_target=true; target.sampled=false; target.format=zh::renderer::TextureFormat::bgra8;
				auto color=device.create_texture(target,"original terrain-water color"); target.format=zh::renderer::TextureFormat::depth24_stencil8;
				auto depth=device.create_texture(target,"original terrain-water depth"); require(color&&depth,"original water targets missing");
				auto frame=[&]{ edge.bind_frame_targets(color,depth,32,24); display.draw(); require(!WW3D::Is_Rendering()&&!device.pass_active(),"original water frame retained state"); };
				const std::string start=device.snapshot(); frame(); const std::string active=device.snapshot().substr(start.size());
				const char *water_name = cloud ? "original WaterRenderObjClass::Render cloud plane" : "original WaterRenderObjClass::Render translucent plane";
				const std::size_t terrain=active.find("original RTS3DScene::Render map terrain"), track_mark=active.find("original TerrainTracksRenderObjClassSystem::flush"), water_mark=active.find(water_name);
				require(draws(active)==4 && terrain<track_mark && track_mark<water_mark &&
					active.find("DX8Wrapper::Draw indexed first=0 count=6 base=0",water_mark)!=std::string::npos,
					"original terrain-track-water source ordering/range failed");
				auto *smudges=dynamic_cast<W3DSmudgeManager *>(TheSmudgeManager);
				require(smudges,"original terrain-water particle smudge owner missing");
				auto *set=smudges->addSmudgeSet(); auto *smudge=set->addSmudgeToSet();
				smudge->m_pos=Vector3(16,16,0); smudge->m_offset=Vector2(0,0); smudge->m_size=4; smudge->m_opacity=.5f;
				const Vector3 smudge_points[5]={Vector3(14,18,0),Vector3(14,14,0),Vector3(18,14,0),Vector3(18,18,0),Vector3(16,16,0)};
				for (Int i=0;i!=5;++i) { smudge->m_verts[i].pos=smudge_points[i]; smudge->m_verts[i].uv.Set((i==2||i==3)?1:0,(i==0||i==3)?0:1); }
				const std::string effect_start=device.snapshot(); frame(); const std::string effect=device.snapshot().substr(effect_start.size());
				const auto particle=effect.find("original W3DParticleSystemManager::doParticles smudge request"), smudge_draw=effect.find("original W3DSmudgeManager::render bounded batch");
				require(terrain!=std::string::npos && effect.find("original RTS3DScene::Render map terrain") < effect.find("original TerrainTracksRenderObjClassSystem::flush") &&
					effect.find("original TerrainTracksRenderObjClassSystem::flush") < effect.find(water_name) &&
					effect.find(water_name) < particle && particle < smudge_draw &&
					effect.find("DX8Wrapper::Draw indexed first=0 count=12 base=0",smudge_draw)!=std::string::npos,
					"original terrain-track-water-particle-smudge ordering failed");
				device.fail_buffer_upload_after(before(active,"upload ",water_mark));
				require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active()&&!water->hasPendingGpuResources(),"original water upload rollback failed");
				device.fail_draw_after(draws(active.substr(0,water_mark)));
				require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active(),"original water draw rollback failed"); frame();
				TheWritableGlobalData->m_useCloudPlane=FALSE; TheWritableGlobalData->m_useWaterPlane=FALSE; const std::string off=device.snapshot(); frame();
				require(draws(device.snapshot().substr(off.size()))==3,"original disabled water emitted a draw"); TheWritableGlobalData->m_useWaterPlane=TRUE;
				TheWritableGlobalData->m_useCloudPlane=!cloud; require(rejected(frame),"original cloud owner mutation was accepted"); TheWritableGlobalData->m_useCloudPlane=cloud;
				TheWritableGlobalData->m_waterExtentX=0; require(rejected(frame),"original zero-extent water activated"); TheWritableGlobalData->m_waterExtentX=32;
				TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_1_FB_REFLECTION; require(rejected(frame),"original reflection water activated"); TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
				W3DDisplay::m_3DScene->Remove_Render_Object(water); require(rejected(frame),"original water accepted mismatched scene"); W3DDisplay::m_3DScene->Add_Render_Object(water); frame();
				auto *foreign=NEW_REF(WaterRenderObjClass, ()); W3DDisplay::m_3DScene->Add_Render_Object(foreign);
				require(rejected(frame),"original arbitrary water object entered map traversal");
				W3DDisplay::m_3DScene->Remove_Render_Object(foreign); foreign->Release_Ref(); frame();
				device.destroy(depth); device.destroy(color);
			} catch (...) { TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display; throw; }
			TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display;
		}
		edge.release_source_buffers();
	}
	// Retail reaches a real translucent water plane, not the generated fixed
	// cloud-only control below.  Source GameClient reset detaches display objects
	// before terrain reaches that owner.
	TheWritableGlobalData->m_useWaterPlane=TRUE; TheWritableGlobalData->m_useCloudPlane=TRUE;
	TheWritableGlobalData->m_waterExtentX=32; TheWritableGlobalData->m_waterExtentY=24;
	TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
	for (Int generation=0; generation!=2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device); W3DDisplay display; display.init();
		Display *saved_display=TheDisplay; TerrainVisual *saved_visual=TheTerrainVisual; TheDisplay=&display;
		try {
			W3DTerrainVisual visual; TheTerrainVisual=&visual;
			setenv("ZH_M22_RETAIL_CONFIG_ROUTE","1",1); setenv("ZH_M22_RETAIL_CONFIG_RESET_PROFILE","1",1);
			visual.init(); WaterRenderObjClass *water=TheWaterRenderObj;
			require(water && !water->hasPendingGpuResources() && water->Peek_Scene()==W3DDisplay::m_3DScene,
				"original active water reset bootstrap failed");
			require(rejected([&]{visual.reset();}), "original active water reset accepted before display detach");
			display.reset();
			require(water->Peek_Scene()!=W3DDisplay::m_3DScene, "original display did not detach active water");
			Real grid_height=31337;
			require(!visual.getWaterGridHeight(0,0,&grid_height) && grid_height==31337,
				"original reset-detached water grid query failed");
			visual.reset();
			unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE");
			require(rejected([&]{visual.reset();}), "original detached active water accepted without selector");
			setenv("ZH_M22_RETAIL_CONFIG_ROUTE","1",1);
			WaterRenderObjClass *saved_owner=TheWaterRenderObj; TheWaterRenderObj=NULL;
			require(rejected([&]{visual.reset();}), "original detached active water accepted removed provider");
			TheWaterRenderObj=saved_owner; visual.reset();
			unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE"); unsetenv("ZH_M22_RETAIL_CONFIG_RESET_PROFILE");
			TheTerrainVisual=saved_visual;
		} catch (...) { unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE"); unsetenv("ZH_M22_RETAIL_CONFIG_RESET_PROFILE"); TheTerrainVisual=saved_visual; TheDisplay=saved_display; throw; }
		TheDisplay=saved_display; edge.release_source_buffers();
		require(!TheWaterRenderObj && !TheTerrainTracksRenderObjClassSystem && !TheW3DShadowManager && !TheSmudgeManager,
			"original detached active water retained source providers");
	}
	// The aggregate retail route is the sole cloud-only exception: without its
	// selector a zero-extent cloud owner stays rejected, while with it the
	// source owner is a fixed internal 1x1 lifecycle object (not a water draw).
	TheWritableGlobalData->m_useWaterPlane=FALSE; TheWritableGlobalData->m_useCloudPlane=TRUE;
	TheWritableGlobalData->m_waterExtentX=0; TheWritableGlobalData->m_waterExtentY=0;
	TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
	for (Int generation=0; generation!=2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device); W3DDisplay display; display.init();
		{
			ScalarProbeWater cloud;
			unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE");
			require(rejected([&]{cloud.init(0,0,0,W3DDisplay::m_3DScene,WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT);}),
				"original default zero-extent cloud owner was accepted");
			setenv("ZH_M22_RETAIL_CONFIG_ROUTE","1",1);
			require(cloud.init(0,0,0,W3DDisplay::m_3DScene,WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT)==0 &&
				TheWaterRenderObj==&cloud && cloud.hasPendingGpuResources(),
				"original selected fixed cloud owner failed");
			unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE");
			require(rejected([&]{cloud.setGridHeightClamps(-3,7);}), "original cloud clamps accepted without selector");
			setenv("ZH_M22_RETAIL_CONFIG_ROUTE","1",1);
			cloud.setGridHeightClamps(-3,7);
			cloud.setGridTransform(.25f,4,5,6);
			cloud.setGridResolution(4,5,2);
			cloud.setGridChangeAttenuationFactors(.1f,.2f,.3f,8);
			const Matrix3D &transform=cloud.Get_Transform();
			require(near(transform.Get_Translation().X,4) && near(transform.Get_Translation().Y,5) && near(transform.Get_Translation().Z,6) &&
				cloud.scalar_state_is(-3,7,.1f,.2f,.3f,8), "original cloud scalar order/state changed");
			require(rejected([&]{cloud.setGridResolution(-1,1,1);}) &&
				rejected([&]{cloud.setGridTransform(std::numeric_limits<Real>::infinity(),0,0,0);}) &&
				rejected([&]{cloud.setGridChangeAttenuationFactors(0,0,0,-1);}),
				"original cloud scalar bounds changed");
			ScalarProbeWater foreign;
			require(rejected([&]{foreign.setGridHeightClamps(0,0);}), "original foreign cloud owner was accepted");
			cloud.reset(); cloud.load(); cloud.update();
			unsetenv("ZH_M22_RETAIL_CONFIG_ROUTE");
		}
		edge.release_source_buffers();
	}
	TheWritableGlobalData->m_partitionCellSize=saved_partition; TheWritableGlobalData->m_maxTerrainTracks=saved_tracks; TheWritableGlobalData->m_makeTrackMarks=saved_marks;
	TheWritableGlobalData->m_useWaterPlane=saved_water; TheWritableGlobalData->m_useCloudPlane=saved_cloud; TheWritableGlobalData->m_waterExtentX=saved_x; TheWritableGlobalData->m_waterExtentY=saved_y; TheWritableGlobalData->m_waterType=saved_type;
	require(device.resource_counts().total()==0,"original water teardown retained Recording resources");
	std::puts("original terrain water: plane=1 cloud=1 retail-cloud-selector=1 scalar-transaction=1 active-reset=1 ordering=1 retry=2 tracks=1 siblings=0 grid-query=1 generations=7 resources=0");
}
