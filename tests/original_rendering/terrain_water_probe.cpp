#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template <typename F> bool rejected(F f) { try { f(); } catch (const std::runtime_error &) { return true; } return false; }
unsigned draws(const std::string &trace) { unsigned n=0; for (std::size_t p=trace.find("draw pipeline="); p!=std::string::npos; p=trace.find("draw pipeline=",p+1)) ++n; return n; }
unsigned before(const std::string &trace, const char *needle, std::size_t end) { unsigned n=0; for (std::size_t p=trace.find(needle);p!=std::string::npos&&p<end;p=trace.find(needle,p+1)) ++n; return n; }
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
	TheWritableGlobalData->m_useWaterPlane=TRUE; TheWritableGlobalData->m_useCloudPlane=FALSE;
	TheWritableGlobalData->m_waterExtentX=32; TheWritableGlobalData->m_waterExtentY=24;
	TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation=0; generation!=2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		{
			W3DDisplay display; display.init(); Display *saved_display=TheDisplay; TheDisplay=&display;
			TerrainVisual *saved_visual=TheTerrainVisual; W3DTerrainVisual visual; TheTerrainVisual=&visual;
			View *saved_view=TheTacticalView; W3DView *view=NULL;
			try {
				display.setWidth(32); display.setHeight(24); visual.init();
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
				water->ReAcquireResources(); require(!water->hasPendingGpuResources(), "original water create retry failed");
				view=new W3DView; TheTacticalView=view; view->init(); display.attachView(view);
				view->setWidth(32); view->setHeight(24); view->setDefaultView(0,0,1);
				auto *track=tracks->bindTrack(TheHeightMap,MAP_XY_FACTOR,""); require(track,"original water compatibility track missing");
				track->addEdgeToTrack(0,0); track->addEdgeToTrack(24,0); track->addEdgeToTrack(48,0);
				zh::renderer::TextureDesc target; target.width=32; target.height=24; target.render_target=true; target.sampled=false; target.format=zh::renderer::TextureFormat::bgra8;
				auto color=device.create_texture(target,"original terrain-water color"); target.format=zh::renderer::TextureFormat::depth24_stencil8;
				auto depth=device.create_texture(target,"original terrain-water depth"); require(color&&depth,"original water targets missing");
				auto frame=[&]{ edge.bind_frame_targets(color,depth,32,24); display.draw(); require(!WW3D::Is_Rendering()&&!device.pass_active(),"original water frame retained state"); };
				const std::string start=device.snapshot(); frame(); const std::string active=device.snapshot().substr(start.size());
				const std::size_t terrain=active.find("original RTS3DScene::Render map terrain"), track_mark=active.find("original TerrainTracksRenderObjClassSystem::flush"), water_mark=active.find("original WaterRenderObjClass::Render translucent plane");
				require(draws(active)==4 && terrain<track_mark && track_mark<water_mark &&
					active.find("DX8Wrapper::Draw indexed first=0 count=6 base=0",water_mark)!=std::string::npos,
					"original terrain-track-water source ordering/range failed");
				device.fail_buffer_upload_after(before(active,"upload ",water_mark));
				require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active()&&!water->hasPendingGpuResources(),"original water upload rollback failed");
				device.fail_draw_after(draws(active.substr(0,water_mark)));
				require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active(),"original water draw rollback failed"); frame();
				TheWritableGlobalData->m_useWaterPlane=FALSE; const std::string off=device.snapshot(); frame();
				require(draws(device.snapshot().substr(off.size()))==3,"original disabled water emitted a draw"); TheWritableGlobalData->m_useWaterPlane=TRUE;
				TheWritableGlobalData->m_useCloudPlane=TRUE; require(rejected(frame),"original cloud sibling activated water"); TheWritableGlobalData->m_useCloudPlane=FALSE;
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
	TheWritableGlobalData->m_partitionCellSize=saved_partition; TheWritableGlobalData->m_maxTerrainTracks=saved_tracks; TheWritableGlobalData->m_makeTrackMarks=saved_marks;
	TheWritableGlobalData->m_useWaterPlane=saved_water; TheWritableGlobalData->m_useCloudPlane=saved_cloud; TheWritableGlobalData->m_waterExtentX=saved_x; TheWritableGlobalData->m_waterExtentY=saved_y; TheWritableGlobalData->m_waterType=saved_type;
	require(device.resource_counts().total()==0,"original water teardown retained Recording resources");
	std::puts("original terrain water: plane=1 ordering=1 retry=2 tracks=1 siblings=0 generations=2 resources=0");
}
