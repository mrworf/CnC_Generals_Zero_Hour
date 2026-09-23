#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
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
unsigned draws(const std::string &trace) { unsigned n=0; for (std::size_t p=trace.find("draw pipeline=");p!=std::string::npos;p=trace.find("draw pipeline=",p+1)) ++n; return n; }
W3DModelDraw *sourceModel()
{
	W3DModelDraw *result=NULL;
	for (Drawable *drawable=TheGameClient->firstDrawable(); drawable; drawable=drawable->getNextDrawable()) {
		if (!drawable->getTemplate() || drawable->getTemplate()->getName()!="LogicFixture") continue;
		for (DrawModule **module=drawable->getDrawModules(); *module; ++module)
			if (auto *model=dynamic_cast<W3DModelDraw *>(*module)) { require(!result,"original decal fixture has duplicate source owner"); result=model; }
	}
	require(result && result->getRenderObject(),"original decal fixture source owner missing"); return result;
}
}

extern "C" void zh_probe_shadow_decal_route()
{
	const char *map=std::getenv("ZH_M22_SHADOW_DECAL_MAP"); require(map,"original decal map fixture missing");
	const bool volume=std::getenv("ZH_M22_VOLUME_SHADOW_PROFILE")!=NULL;
	const Real saved_partition=TheGlobalData->m_partitionCellSize;
	const Bool saved_marks=TheGlobalData->m_makeTrackMarks;
	const Bool saved_decals=TheGlobalData->m_useShadowDecals, saved_volumes=TheGlobalData->m_useShadowVolumes;
	const Int saved_tracks=TheGlobalData->m_maxTerrainTracks; const Bool saved_water=TheGlobalData->m_useWaterPlane, saved_cloud=TheGlobalData->m_useCloudPlane;
	const Real saved_x=TheGlobalData->m_waterExtentX, saved_y=TheGlobalData->m_waterExtentY; const Int saved_type=TheGlobalData->m_waterType;
	TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR; TheWritableGlobalData->m_makeTrackMarks=TRUE; TheWritableGlobalData->m_useShadowDecals=!volume; TheWritableGlobalData->m_useShadowVolumes=volume;
	TheWritableGlobalData->m_maxTerrainTracks=1; TheWritableGlobalData->m_useWaterPlane=TRUE; TheWritableGlobalData->m_useCloudPlane=FALSE;
	TheWritableGlobalData->m_waterExtentX=32; TheWritableGlobalData->m_waterExtentY=24; TheWritableGlobalData->m_waterType=WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT;
	zh::renderer::RecordingGpuDevice device;
	try {
		for (Int generation=0;generation!=2;++generation) {
			zh::original_runtime::OriginalGpuEdge edge(device);
			{
				W3DDisplay display; display.init(); Display *saved_display=TheDisplay; TheDisplay=&display;
				TerrainVisual *saved_visual=TheTerrainVisual; W3DTerrainVisual visual; TheTerrainVisual=&visual;
				View *saved_view=TheTacticalView; W3DView *view=NULL;
				try {
					display.setWidth(32); display.setHeight(24); visual.init();
					auto *model=sourceModel(); auto *render=model->getRenderObject();
					require(visual.load(AsciiString(map)) && TheHeightMap && TheHeightMap->getMap(),"original decal map did not load");
					SceneClass *source_scene=render->Get_Scene(); require(source_scene,"original decal source owner had no source scene");
					source_scene->Remove_Render_Object(render); W3DDisplay::m_3DScene->Add_Render_Object(render);
					Shadow::ShadowTypeInfo negative{}; negative.m_type=SHADOW_VOLUME;
					if (!volume) require(rejected([&]{TheW3DShadowManager->addShadow(render,&negative);}),"original volume shadow entered decal route");
					negative.m_type=SHADOW_PROJECTION; require(rejected([&]{TheW3DShadowManager->addShadow(render,&negative);}),"original projection shadow entered decal route");
					negative.m_type=SHADOW_NONE; require(!TheW3DShadowManager->addShadow(render,&negative),"original none shadow entered decal route");
					if (!volume) {
						device.fail_next_buffer_create();
						require(rejected([&]{model->allocateShadows();}),"original decal create rollback failed");
					}
					model->allocateShadows();
					if (volume) require(TheW3DShadowManager->hasBoundedVolumeCasters()&&TheW3DShadowManager->ownsBoundedVolumeCaster(render),"original source volume owner was not admitted");
					else require(TheW3DShadowManager->hasBoundedDecalCasters()&&TheW3DShadowManager->ownsBoundedDecalCaster(render),"original source decal owner was not admitted");
					// W3DModelDraw admits the owner while the source asset scene is being
					// assembled.  Slots become available only after map visual loading,
					// so resource acquisition is deliberately deferred to this source
					// lifecycle boundary.
					TheW3DShadowManager->ReAcquireResources();
					if (volume) {
						Shadow::ShadowTypeInfo duplicate{}; duplicate.m_type=SHADOW_VOLUME;
						require(rejected([&]{TheW3DShadowManager->addShadow(render,&duplicate);}),"original duplicate volume owner accepted");
					} else require(rejected([&]{model->allocateShadows(); TheW3DShadowManager->addShadow(render,NULL);}),"original duplicate decal owner accepted");
					W3DDisplay::m_3DScene->Remove_Render_Object(render); require(rejected([&]{TheW3DShadowManager->addShadow(render,NULL);}),"original detached decal owner accepted"); W3DDisplay::m_3DScene->Add_Render_Object(render);
					view=new W3DView; TheTacticalView=view; view->init(); display.attachView(view); view->setWidth(32); view->setHeight(24); view->setDefaultView(0,0,1);
					auto *track=TheTerrainTracksRenderObjClassSystem->bindTrack(TheHeightMap,MAP_XY_FACTOR,""); require(track,"original decal tracks compatibility owner missing");
					track->addEdgeToTrack(0,0); track->addEdgeToTrack(24,0); track->addEdgeToTrack(48,0);
					zh::renderer::TextureDesc target; target.width=32; target.height=24; target.render_target=true; target.sampled=false; target.format=zh::renderer::TextureFormat::bgra8;
					auto color=device.create_texture(target,"original decal color"); target.format=zh::renderer::TextureFormat::depth24_stencil8; auto depth=device.create_texture(target,"original decal depth"); require(color&&depth,"original decal targets missing");
					auto frame=[&]{edge.bind_frame_targets(color,depth,32,24); display.draw(); require(!WW3D::Is_Rendering()&&!device.pass_active(),"original decal frame retained state");};
					if (volume) {
						device.fail_next_buffer_create();
						require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active(),"original volume source create rollback failed");
					}
					const std::string start=device.snapshot(); frame(); const std::string active=device.snapshot().substr(start.size());
					const char *shadow_marker=volume ? "original W3DShadowManager::RenderShadows volume" : "original W3DShadowManager::RenderShadows decal";
					const std::size_t terrain=active.find("original RTS3DScene::Render map terrain"), track_mark=active.find("original TerrainTracksRenderObjClassSystem::flush"), shadow=active.find(shadow_marker), water=active.find("original WaterRenderObjClass::Render translucent plane");
					require(terrain<track_mark&&track_mark<shadow&&shadow<water&&draws(active)>=4&&active.find("DX8Wrapper::Draw indexed first=0 count=6 base=0",shadow)!=std::string::npos,"original terrain-shadow-water order or range failed");
					if (std::getenv("ZH_M22_FULL_FEATURE_PROFILE")) {
						auto *smudges=dynamic_cast<W3DSmudgeManager *>(TheSmudgeManager);
						require(smudges && dynamic_cast<W3DParticleSystemManager *>(TheParticleSystemManager), "original full feature providers missing");
						auto *set=smudges->addSmudgeSet(); auto *smudge=set->addSmudgeToSet();
						smudge->m_size=4; smudge->m_opacity=.5f;
						const Vector3 points[5]={Vector3(14,18,0),Vector3(14,14,0),Vector3(18,14,0),Vector3(18,18,0),Vector3(16,16,0)};
						for (Int i=0;i!=5;++i) { smudge->m_verts[i].pos=points[i]; smudge->m_verts[i].uv.Set((i==2||i==3)?1:0,(i==0||i==3)?0:1); }
						const std::string full_start=device.snapshot(); frame(); const std::string full=device.snapshot().substr(full_start.size());
						require(full.find("original RTS3DScene::Render map terrain") < full.find("original TerrainTracksRenderObjClassSystem::flush") &&
							full.find("original TerrainTracksRenderObjClassSystem::flush") < full.find("original W3DShadowManager::RenderShadows decal") &&
							full.find("original W3DShadowManager::RenderShadows decal") < full.find("original WaterRenderObjClass::Render translucent plane") &&
							full.find("original WaterRenderObjClass::Render translucent plane") < full.find("original W3DParticleSystemManager::doParticles smudge request") &&
							full.find("original W3DSmudgeManager::render bounded batch") != std::string::npos,
							"original full feature source ordering failed");
					}
					TheW3DShadowManager->ReleaseResources();
					if (!volume) { device.fail_next_buffer_create(); require(rejected([&]{TheW3DShadowManager->ReAcquireResources();}),"original decal reacquire create rollback failed"); }
					TheW3DShadowManager->ReAcquireResources();
					device.fail_draw_after(draws(active.substr(0,shadow))); require(rejected(frame)&&!WW3D::Is_Rendering()&&!device.pass_active(),"original decal draw rollback failed"); frame();
					if (volume) {
						TheWritableGlobalData->m_useShadowVolumes=FALSE;
						const std::string off=device.snapshot(); frame();
						require(device.snapshot().substr(off.size()).find("original W3DShadowManager::RenderShadows volume")==std::string::npos,"original disabled volumes emitted a draw");
						TheWritableGlobalData->m_useShadowVolumes=TRUE;
					} else {
						TheWritableGlobalData->m_useShadowDecals=FALSE; const std::string off=device.snapshot(); frame();
						require(device.snapshot().substr(off.size()).find("original W3DShadowManager::RenderShadows decal")==std::string::npos,"original disabled decals emitted a draw"); TheWritableGlobalData->m_useShadowDecals=TRUE;
					}
					model->releaseShadows();
					require(volume ? !TheW3DShadowManager->hasBoundedVolumeCasters() : !TheW3DShadowManager->hasBoundedDecalCasters(),"original source shadow removal retained manager owner");
					W3DDisplay::m_3DScene->Remove_Render_Object(render); source_scene->Add_Render_Object(render);
					device.destroy(depth); device.destroy(color);
				} catch (...) { TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display; throw; }
				TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display;
			}
			edge.release_source_buffers();
		}
	} catch (...) { TheWritableGlobalData->m_partitionCellSize=saved_partition; TheWritableGlobalData->m_makeTrackMarks=saved_marks; TheWritableGlobalData->m_useShadowDecals=saved_decals; TheWritableGlobalData->m_useShadowVolumes=saved_volumes; TheWritableGlobalData->m_maxTerrainTracks=saved_tracks; TheWritableGlobalData->m_useWaterPlane=saved_water; TheWritableGlobalData->m_useCloudPlane=saved_cloud; TheWritableGlobalData->m_waterExtentX=saved_x; TheWritableGlobalData->m_waterExtentY=saved_y; TheWritableGlobalData->m_waterType=saved_type; throw; }
	TheWritableGlobalData->m_partitionCellSize=saved_partition; TheWritableGlobalData->m_makeTrackMarks=saved_marks; TheWritableGlobalData->m_useShadowDecals=saved_decals; TheWritableGlobalData->m_useShadowVolumes=saved_volumes; TheWritableGlobalData->m_maxTerrainTracks=saved_tracks; TheWritableGlobalData->m_useWaterPlane=saved_water; TheWritableGlobalData->m_useCloudPlane=saved_cloud; TheWritableGlobalData->m_waterExtentX=saved_x; TheWritableGlobalData->m_waterExtentY=saved_y; TheWritableGlobalData->m_waterType=saved_type;
	require(device.resource_counts().total()==0,"original decal teardown retained Recording resources");
	if (volume)
		std::puts("original volume shadow: source=1 ordering=1 retry=2 tracks-water=1 negatives=1 removal=1 generations=2 resources=0");
	if (volume && std::getenv("ZH_M22_VOLUME_SHADOW_AGGREGATE_PROFILE"))
		std::puts("original volume aggregate: slots-geometry-edge-owner=1 source-order=1 rollback=1 default-guard=1 generations=2 resources=0");
	else if (std::getenv("ZH_M22_FULL_FEATURE_PROFILE"))
		std::puts("original full feature map: terrain-tracks-shadow-water-particle-smudge=1 failures=2 generations=2 resources=0");
	else
		std::puts("original decal shadow: source=1 ordering=1 retry=2 tracks-water=1 negatives=1 removal=1 generations=2 resources=0");
}
