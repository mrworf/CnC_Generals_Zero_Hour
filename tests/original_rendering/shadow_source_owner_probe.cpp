#include "PreRTS.h"
#include "OriginalW3DDeviceUnavailable.h"
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/Object.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "WW3D2/RendObj.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

Drawable *findSourceOwner(W3DModelDraw **model)
{
	Drawable *owner = NULL;
	*model = NULL;
	for (Drawable *drawable = TheGameClient->firstDrawable(); drawable;
		drawable = drawable->getNextDrawable()) {
		if (!drawable->getTemplate() || drawable->getTemplate()->getName() != "LogicFixture")
			continue;
		for (DrawModule **module = drawable->getDrawModules(); *module; ++module) {
			if (W3DModelDraw *candidate = dynamic_cast<W3DModelDraw *>(*module)) {
				require(!*model, "generated shadow fixture constructed duplicate W3DModelDraw owners");
				*model = candidate;
			}
		}
		owner = drawable;
	}
	require(owner && *model, "generated shadow fixture did not construct W3DModelDraw source owner");
	return owner;
}

void requireTypedRejection(W3DShadowManager *manager, RenderObjClass *render,
	Drawable *owner, ShadowType type, const char *diagnostic)
{
	Shadow::ShadowTypeInfo info{};
	info.m_type = type;
	try {
		(void)manager->addShadow(render, &info, owner);
		throw std::runtime_error("unsupported shadow type was accepted by the pending manager");
	} catch (const OriginalW3DDeviceUnavailable &error) {
		require(std::string(error.what()).find(diagnostic) != std::string::npos,
			"pending shadow manager selected the wrong typed rejection");
	}
}
}

extern "C" void zh_probe_shadow_source_owner()
{
	require(std::getenv("ZH_M22_SHADOW_SOURCE_PROFILE"),
		"original source-shadow profile missing");
	require(TheGameClient && W3DDisplay::m_assetManager && W3DDisplay::m_3DScene,
		"original source-shadow fixture did not publish GameClient display owners");

	W3DModelDraw *model = NULL;
	Drawable *owner = findSourceOwner(&model);
	const ThingTemplate *tmplate = owner->getTemplate();
	RenderObjClass *render = model->getRenderObject();
	require(render && render->Get_Scene() == W3DDisplay::m_3DScene,
		"generated source-shadow model was not attached to the original primary scene");
	require(tmplate->getShadowType() == SHADOW_DECAL &&
		tmplate->getShadowTextureName() == "M22SourceShadow" &&
		tmplate->getShadowSizeX() == 16.0f && tmplate->getShadowSizeY() == 8.0f &&
		tmplate->getShadowOffsetX() == 1.0f && tmplate->getShadowOffsetY() == -2.0f,
		"generated source-shadow template did not preserve exact decal metadata");

	const Bool saved_volumes = TheGlobalData->m_useShadowVolumes;
	const Bool saved_decals = TheGlobalData->m_useShadowDecals;
	const Int saved_tracks = TheGlobalData->m_maxTerrainTracks;
	const Bool saved_water = TheGlobalData->m_useWaterPlane;
	const Bool saved_cloud = TheGlobalData->m_useCloudPlane;
	const Real saved_extent_x = TheGlobalData->m_waterExtentX;
	const Real saved_extent_y = TheGlobalData->m_waterExtentY;
	const Int saved_water_type = TheGlobalData->m_waterType;
	TheWritableGlobalData->m_useShadowVolumes = FALSE;
	TheWritableGlobalData->m_useShadowDecals = FALSE;
	TheWritableGlobalData->m_maxTerrainTracks = 0;
	TheWritableGlobalData->m_useWaterPlane = FALSE;
	TheWritableGlobalData->m_useCloudPlane = FALSE;
	TheWritableGlobalData->m_waterExtentX = 0;
	TheWritableGlobalData->m_waterExtentY = 0;
	TheWritableGlobalData->m_waterType = 0;

	zh::renderer::RecordingGpuDevice device;
	try {
		for (Int generation = 0; generation != 2; ++generation) {
			zh::original_runtime::OriginalGpuEdge edge(device);
			TerrainVisual *saved_visual = TheTerrainVisual;
			try {
				W3DTerrainVisual visual;
				TheTerrainVisual = &visual;
				visual.init();
				require(TheW3DShadowManager && !TheW3DShadowManager->isShadowScene(),
					"generated source-shadow fixture did not publish the disabled original manager");

				// These are negative controls only. They mirror the default source
				// volume route and unsupported projection route; no direct manager
				// request is used as the accepting decal witness.
				requireTypedRejection(TheW3DShadowManager, render, owner, SHADOW_VOLUME,
					"volumetric shadow");
				requireTypedRejection(TheW3DShadowManager, render, owner, SHADOW_PROJECTION,
					"projected shadow");
				Shadow::ShadowTypeInfo none{};
				none.m_type = SHADOW_NONE;
				require(!TheW3DShadowManager->addShadow(render, &none, owner),
					"pending shadow manager accepted SHADOW_NONE");

				try {
					// This is the actual original W3DModelDraw source call. Its
					// manager-side typed failure is the FD0 witness, not a substitute
					// direct manager request.
					model->allocateShadows();
					throw std::runtime_error("source W3DModelDraw decal request silently succeeded");
				} catch (const OriginalW3DDeviceUnavailable &error) {
					require(std::string(error.what()).find("original decal shadow derived-manager creation pending") != std::string::npos,
						"source W3DModelDraw did not reach the pending decal manager route");
				}
				require(!TheW3DShadowManager->isShadowScene() && device.resource_counts().total() == 0,
					"rejected source decal request published a shadow pass or Recording resource");
				TheTerrainVisual = saved_visual;
			} catch (...) {
				TheTerrainVisual = saved_visual;
				throw;
			}
			require(!TheW3DShadowManager && device.resource_counts().total() == 0,
				"source-shadow manager generation leaked publication or Recording resources");
		}

		Object *object = owner->getObject();
		render->Add_Ref();
		TheGameClient->destroyDrawable(owner);
		require(object && object->getDrawable() == NULL && render->Get_Scene() == NULL,
			"original source-shadow owner removal retained object or primary-scene ownership");
		render->Release_Ref();
	} catch (...) {
		TheWritableGlobalData->m_useShadowVolumes = saved_volumes;
		TheWritableGlobalData->m_useShadowDecals = saved_decals;
		TheWritableGlobalData->m_maxTerrainTracks = saved_tracks;
		TheWritableGlobalData->m_useWaterPlane = saved_water;
		TheWritableGlobalData->m_useCloudPlane = saved_cloud;
		TheWritableGlobalData->m_waterExtentX = saved_extent_x;
		TheWritableGlobalData->m_waterExtentY = saved_extent_y;
		TheWritableGlobalData->m_waterType = saved_water_type;
		throw;
	}
	TheWritableGlobalData->m_useShadowVolumes = saved_volumes;
	TheWritableGlobalData->m_useShadowDecals = saved_decals;
	TheWritableGlobalData->m_maxTerrainTracks = saved_tracks;
	TheWritableGlobalData->m_useWaterPlane = saved_water;
	TheWritableGlobalData->m_useCloudPlane = saved_cloud;
	TheWritableGlobalData->m_waterExtentX = saved_extent_x;
	TheWritableGlobalData->m_waterExtentY = saved_extent_y;
	TheWritableGlobalData->m_waterType = saved_water_type;
	std::puts("original source shadow: owner=1 decal-request=2 typed-negatives=3 removal=1 generations=2 resources=0");
}
