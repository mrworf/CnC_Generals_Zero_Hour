#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Smudge.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
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
unsigned occurrences(const std::string &trace, const char *needle) { unsigned n=0; for (std::size_t p=trace.find(needle);p!=std::string::npos;p=trace.find(needle,p+1)) ++n; return n; }
}

extern "C" void zh_probe_particle_provider()
{
	const char *map=std::getenv("ZH_M22_PARTICLE_MAP"); require(map,"original particle map fixture missing");
	const Real saved_partition=TheGlobalData->m_partitionCellSize;
	// Generated visual-map packets have authored cell-space dimensions.  The
	// source height-map owner requires the matching world-space partition.
	TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
	W3DParticleSystemManager inactive; require(rejected([&]{inactive.queueParticleRender();}),"original particle queue accepted without edge");
	zh::renderer::RecordingGpuDevice device;
	for (Int generation=0; generation!=2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		W3DDisplay display; display.init(); Display *saved_display=TheDisplay; TheDisplay=&display;
		TerrainVisual *saved_visual=TheTerrainVisual; W3DTerrainVisual visual; TheTerrainVisual=&visual;
		View *saved_view=TheTacticalView; W3DView *view=NULL;
		try {
			require(dynamic_cast<W3DParticleSystemManager *>(TheParticleSystemManager),"original particle provider not selected");
			display.setWidth(32); display.setHeight(24); visual.init(); require(visual.load(AsciiString(map)),"original particle map load failed");
			view=new W3DView; TheTacticalView=view; view->init(); display.attachView(view); view->setWidth(32); view->setHeight(24); view->setDefaultView(0,0,1);
			zh::renderer::TextureDesc target; target.width=32; target.height=24; target.render_target=true; target.sampled=false; target.format=zh::renderer::TextureFormat::bgra8;
			auto color=device.create_texture(target,"original particle color"); target.format=zh::renderer::TextureFormat::depth24_stencil8; auto depth=device.create_texture(target,"original particle depth"); require(color&&depth,"original particle targets missing");
			auto frame=[&]{ edge.bind_frame_targets(color,depth,32,24); display.draw(); require(!WW3D::Is_Rendering()&&!device.pass_active(),"original particle frame retained state"); };
			auto *manager=static_cast<W3DParticleSystemManager *>(TheParticleSystemManager);
			const std::string before=device.snapshot(); frame(); const std::string active=device.snapshot().substr(before.size());
			const auto terrain=active.find("original RTS3DScene::Render map terrain"), queue=active.find("original W3DParticleSystemManager::queueParticleRender"), request=active.find("original W3DParticleSystemManager::doParticles smudge request");
			if (!(terrain<queue&&queue<request&&occurrences(active,"original W3DParticleSystemManager::queueParticleRender")==1))
				throw std::runtime_error("original particle queue/order did not coalesce: terrain=" + std::to_string(terrain) + " queue=" + std::to_string(queue) + " request=" + std::to_string(request) + " count=" + std::to_string(occurrences(active,"original W3DParticleSystemManager::queueParticleRender")));
			// A producer may queue twice before the scene consumes it; the original
			// one-shot bit preserves one source request rather than a duplicate draw.
			const std::string queued_before=device.snapshot(); manager->queueParticleRender(); manager->queueParticleRender(); frame();
			const std::string queued=device.snapshot().substr(queued_before.size());
			require(occurrences(queued,"original W3DParticleSystemManager::queueParticleRender")==1 &&
				queued.find("original W3DParticleSystemManager::queueParticleRender") < queued.find("original W3DParticleSystemManager::doParticles smudge request"),
				"original particle duplicate queue did not coalesce");
			// The provider closes its owner boundary before the original point,
			// streak and volume geometry port: any generated source system rejects
			// the frame, drains its request, and a clean retry remains possible.
			auto *generated_template=manager->newTemplate(AsciiString("M22ParticleProviderReject"));
			auto *generated=manager->createParticleSystem(generated_template,FALSE);
			require(generated && manager->getParticleSystemCount()==1,"original particle generated system missing");
			require(rejected(frame),"original particle geometry did not fail closed");
			generated->deleteInstance(); require(manager->getParticleSystemCount()==0,"original particle generated system teardown failed"); frame();
			SmudgeManager *saved_smudge=TheSmudgeManager;
			try { TheSmudgeManager=NULL; require(rejected(frame),"original particle missing-smudge failure did not reject"); }
			catch (...) { TheSmudgeManager=saved_smudge; throw; }
			TheSmudgeManager=saved_smudge; frame();
			device.destroy(depth); device.destroy(color);
		} catch (...) { TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display; throw; }
		TheTacticalView=saved_view; TheTerrainVisual=saved_visual; TheDisplay=saved_display; edge.release_source_buffers();
	}
	TheWritableGlobalData->m_partitionCellSize=saved_partition;
	require(device.resource_counts().total()==0,"original particle provider retained Recording resources");
	std::puts("original particle provider: owner=1 queue=1 smudge=1 retry=1 generations=2 resources=0");
}
