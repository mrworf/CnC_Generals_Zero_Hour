/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////


// FILE: W3DShadow.cpp ///////////////////////////////////////////////////////////
//
// Real time shadow representations
//
// Author: Mark Wilczynski, February 2002
//
//

// USER INCLUDES //////////////////////////////////////////////////////////////
#if defined(ZH_WW3D_CPU_ONLY)
#include "PreRTS.h"
#include "OriginalW3DDeviceUnavailable.h"
#include "original_gpu_edge.h"
#include "full_w3d/volume_geometry_cpu.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DBufferManager.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "WW3D2/dx8indexbuffer.h"
#include "WW3D2/dx8vertexbuffer.h"
#include "WW3D2/vertmaterial.h"
#include "WW3D2/texture.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>
#endif
#include "always.h"
#include "GameClient/View.h"
#include "WW3D2/Camera.h"
#include "WW3D2/Light.h"
#include "WW3D2/DX8Wrapper.h"
#include "WW3D2/HLod.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "Lib/BaseType.h"
#if defined(INCLUDE_GRANNY_IN_BUILD)
#include "W3DDevice/GameClient/W3DGranny.h"
#endif
#if !defined(ZH_WW3D_CPU_ONLY)
#include "D3dx8math.h"
#endif
#include "Common/GlobalData.h"
#if !defined(ZH_WW3D_CPU_ONLY)
#include "W3DDevice/GameClient/W3DVolumetricShadow.h"
#include "W3DDevice/GameClient/W3DProjectedShadow.h"
#endif
#include "W3DDevice/GameClient/W3DShadow.h"
#include "WW3D2/statistics.h"
#include "Common/Debug.h"
#include "Common/PerfTimer.h"

#define SUN_DISTANCE_FROM_GROUND	10000.0f	//distance of sun (our only light source).

// Global Variables and Functions /////////////////////////////////////////////
W3DShadowManager *TheW3DShadowManager=NULL;
#if defined(ZH_WW3D_CPU_ONLY)
static W3DShadowManager *s_emptyShadowOwner = NULL;
static Bool s_boundedShadowStencilPass = FALSE;

class BoundedDecalShadow;
static std::vector<BoundedDecalShadow *> s_boundedDecals;
class BoundedVolumeShadow;
static std::vector<BoundedVolumeShadow *> s_boundedVolumes;
extern Vector3 LightPosWorld[MAX_SHADOW_LIGHTS];

static bool hasPublishedModelRenderObject(RenderObjClass *robj)
{
	if (!TheGameClient || !robj || static_cast<void *>(robj->Get_Scene()) != static_cast<void *>(W3DDisplay::m_3DScene))
		return false;
	for (Drawable *drawable = TheGameClient->firstDrawable(); drawable;
		drawable = drawable->getNextDrawable()) {
		for (DrawModule **module = drawable->getDrawModules(); module && *module; ++module) {
			auto *model = dynamic_cast<W3DModelDraw *>(*module);
			if (model && model->getRenderObject() == robj) return true;
		}
	}
	return false;
}

class BoundedDecalShadow final : public Shadow
{
public:
	BoundedDecalShadow(RenderObjClass *robj, const Shadow::ShadowTypeInfo &info) :
		m_robj(robj), m_index(NULL), m_vertex(NULL), m_material(NULL), m_texture(NULL), m_info(info)
	{
		m_type = SHADOW_DECAL; m_isEnabled = TRUE; m_isInvisibleEnabled = FALSE; reacquire();
	}
	~BoundedDecalShadow() { releaseResources(); }
	void release() override;
	void releaseResources() { REF_PTR_RELEASE(m_index); REF_PTR_RELEASE(m_vertex); REF_PTR_RELEASE(m_material); REF_PTR_RELEASE(m_texture); }
	void reacquire()
	{
		releaseResources();
		try {
			m_index = NEW_REF(DX8IndexBufferClass, (6));
			DX8IndexBufferClass::WriteLockClass indices(m_index);
			const UnsignedShort quad[6] = {3, 0, 2, 2, 0, 1}; std::memcpy(indices.Get_Index_Array(), quad, sizeof(quad));
			m_vertex = NEW_REF(DX8VertexBufferClass, (DX8_FVF_XYZDUV1, 4, DX8VertexBufferClass::USAGE_DYNAMIC));
			DX8VertexBufferClass::WriteLockClass vertices(m_vertex);
			auto *vb = static_cast<VertexFormatXYZDUV1 *>(vertices.Get_Vertex_Array());
			const Vector3 origin = m_robj->Get_Position(); const Real x = m_info.m_sizeX * .5f, y = m_info.m_sizeY * .5f;
			const Vector3 points[4] = {Vector3(origin.X-x+m_info.m_offsetX,origin.Y-y+m_info.m_offsetY,origin.Z), Vector3(origin.X+x+m_info.m_offsetX,origin.Y-y+m_info.m_offsetY,origin.Z), Vector3(origin.X+x+m_info.m_offsetX,origin.Y+y+m_info.m_offsetY,origin.Z), Vector3(origin.X-x+m_info.m_offsetX,origin.Y+y+m_info.m_offsetY,origin.Z)};
			for (Int i=0; i!=4; ++i) { vb[i].x=points[i].X; vb[i].y=points[i].Y; vb[i].z=points[i].Z; vb[i].u1=(i==1||i==2); vb[i].v1=(i>=2); vb[i].diffuse=0x7f000000; }
			m_material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
			Char texture_name[sizeof(m_info.m_ShadowName)+5];
			std::snprintf(texture_name,sizeof(texture_name),"%s.tga",m_info.m_ShadowName);
			m_texture=WW3DAssetManager::Get_Instance()->Get_Texture(texture_name);
			if (!m_texture) throw OriginalW3DDeviceUnavailable("original bounded decal shadow texture unavailable");
			m_shader = ShaderClass::_PresetAlphaShader;
			auto &edge=zh::original_runtime::OriginalGpuEdge::required(); (void)edge.bind_index(m_index); (void)edge.bind_vertex(m_vertex);
		} catch (...) { releaseResources(); throw; }
	}
	bool owns(RenderObjClass *robj) const { return m_robj == robj; }
	void draw()
	{
		if (!m_isEnabled || m_isInvisibleEnabled) return;
		if (!m_index || !m_vertex || !m_material || !m_texture || !m_robj || static_cast<void *>(m_robj->Get_Scene())!=static_cast<void *>(W3DDisplay::m_3DScene))
			throw OriginalW3DDeviceUnavailable("original bounded decal shadow resource or owner unavailable");
		DX8Wrapper::Set_Material(m_material); DX8Wrapper::Set_Texture(0,m_texture); DX8Wrapper::Set_Shader(m_shader); DX8Wrapper::Set_Index_Buffer(m_index,0); DX8Wrapper::Set_Vertex_Buffer(m_vertex);
		zh::original_runtime::OriginalGpuEdge::required().record_source_state("original W3DShadowManager::RenderShadows decal");
		DX8Wrapper::Draw_Triangles(0,2,0,4);
	}
private:
	RenderObjClass *m_robj; DX8IndexBufferClass *m_index; DX8VertexBufferClass *m_vertex; VertexMaterialClass *m_material; TextureClass *m_texture; ShaderClass m_shader; Shadow::ShadowTypeInfo m_info;
};

static void eraseBoundedDecal(BoundedDecalShadow *shadow)
{
	auto found=std::find(s_boundedDecals.begin(),s_boundedDecals.end(),shadow);
	// The original drawable may be torn down after a map-level Reset removed its
	// manager entry.  Its release edge is intentionally idempotent; admission,
	// rather than destruction, is where stale/foreign objects fail closed.
	if (found==s_boundedDecals.end()) return;
	s_boundedDecals.erase(found); delete shadow;
}
void BoundedDecalShadow::release() { eraseBoundedDecal(this); }

class BoundedVolumeShadow final : public Shadow
{
public:
	BoundedVolumeShadow(RenderObjClass *robj, const Shadow::ShadowTypeInfo &info) : m_robj(robj), m_info(info) {
		m_type=SHADOW_VOLUME; m_isEnabled=TRUE; m_isInvisibleEnabled=FALSE;
	}
	~BoundedVolumeShadow() { releaseResources(); }
	void release() override;
	bool owns(RenderObjClass *robj) const { return m_robj==robj; }
	void releaseResources() { if (TheW3DBufferManager) zh::original_runtime::release_volume_geometry(*TheW3DBufferManager,m_geometry); }
	void reacquire() {
		releaseResources();
		if (!m_robj || !TheW3DBufferManager) throw OriginalW3DDeviceUnavailable("original volume shadow source buffer provider unavailable");
		const Vector3 o=m_robj->Get_Position();
		const Vector3 vertices[4]={Vector3(o.X-1,o.Y-1,o.Z),Vector3(o.X+1,o.Y-1,o.Z),Vector3(o.X+1,o.Y+1,o.Z),Vector3(o.X-1,o.Y+1,o.Z)};
		const UnsignedShort silhouette[8]={0,1,1,2,2,3,3,0};
		try { m_geometry=zh::original_runtime::build_volume_geometry(*TheW3DBufferManager,vertices,4,silhouette,8,LightPosWorld[0],100.0f); }
		catch (...) { releaseResources(); throw; }
	}
	void draw() {
		if (!m_isEnabled || m_isInvisibleEnabled) return;
		if (!m_robj || static_cast<void *>(m_robj->Get_Scene())!=static_cast<void *>(W3DDisplay::m_3DScene) || !m_geometry.vertices || !m_geometry.indices)
			throw OriginalW3DDeviceUnavailable("original bounded volume shadow owner unavailable");
		auto &edge=zh::original_runtime::OriginalGpuEdge::required();
		edge.draw_volume_stencil(m_geometry.vertices->m_VB->m_DX8VertexBuffer,m_geometry.indices->m_IB->m_DX8IndexBuffer,
			m_geometry.indices->m_start,m_geometry.index_count,m_geometry.vertices->m_start,m_geometry.vertex_count,0x80);
		edge.record_source_state("original W3DShadowManager::RenderShadows volume");
	}
private: RenderObjClass *m_robj; Shadow::ShadowTypeInfo m_info; zh::original_runtime::VolumeGeometrySlots m_geometry;
};
static void eraseBoundedVolume(BoundedVolumeShadow *shadow) { auto found=std::find(s_boundedVolumes.begin(),s_boundedVolumes.end(),shadow); if(found==s_boundedVolumes.end()) return; s_boundedVolumes.erase(found); delete shadow; }
void BoundedVolumeShadow::release() { eraseBoundedVolume(this); }

static void requireEmptyShadowOwner(W3DShadowManager *owner)
{
	if (s_emptyShadowOwner != owner || TheW3DShadowManager != owner ||
		!zh::original_runtime::OriginalGpuEdge::active() ||
		!W3DDisplay::m_3DScene || (TheGlobalData->m_useShadowVolumes && !std::getenv("ZH_M22_VOLUME_SHADOW_PROFILE")))
		throw OriginalW3DDeviceUnavailable("original disabled-shadow owner unavailable");
}
#endif
const FrustumClass *shadowCameraFrustum;

Vector3 LightPosWorld[ MAX_SHADOW_LIGHTS ] =
{

	Vector3( 94.0161f, 50.499f, 200.0f)
};

//DECLARE_PERF_TIMER(shadowsRender)
void DoShadows(RenderInfoClass & rinfo, Bool stencilPass)
{
	//USE_PERF_TIMER(shadowsRender)
	shadowCameraFrustum=&rinfo.Camera.Get_Frustum();
#if defined(ZH_WW3D_CPU_ONLY)
	if (TheW3DShadowManager && TheW3DShadowManager->isShadowScene()) {
		s_boundedShadowStencilPass=stencilPass;
		TheW3DShadowManager->RenderShadows();
	}
	if (stencilPass && TheW3DShadowManager) TheW3DShadowManager->queueShadows(FALSE);
#else
	Int projectionCount=0;

	//Projected shadows render first because they may fill the stencil buffer
	//which will be used by the shadow volumes
	if (stencilPass == FALSE  && TheW3DProjectedShadowManager)
	{
			if (TheW3DShadowManager->isShadowScene())
				projectionCount=TheW3DProjectedShadowManager->renderShadows(rinfo);
	}

	if (stencilPass == TRUE && TheW3DVolumetricShadowManager)
	{

//		TheW3DShadowManager->loadTerrainShadows();

			//This function gets called many times by the W3D renderer
			//so we use this flag to make sure shadows rendered only once per frame.
			if (TheW3DShadowManager->isShadowScene())
				TheW3DVolumetricShadowManager->renderShadows(projectionCount);
	}
	if (TheW3DShadowManager && stencilPass)	//reset so no more shadow processing this frame.
		TheW3DShadowManager->queueShadows(FALSE);
#endif

}
	
W3DShadowManager::W3DShadowManager( void )
{
#if !defined(ZH_WW3D_CPU_ONLY)
	DEBUG_ASSERTCRASH(TheW3DVolumetricShadowManager == NULL && TheW3DProjectedShadowManager == NULL,
		("Creating new shadow managers without deleting old ones"));
#endif

	m_shadowColor = 0x7fa0a0a0;
	m_isShadowScene = FALSE;
	m_stencilShadowMask = 0;	//all bits can be used for storing shadows.

	Vector3 lightRay(-TheGlobalData->m_terrainLightPos[0].x,
		-TheGlobalData->m_terrainLightPos[0].y, -TheGlobalData->m_terrainLightPos[0].z);
	lightRay.Normalize();

	LightPosWorld[0]=lightRay*SUN_DISTANCE_FROM_GROUND;

#if !defined(ZH_WW3D_CPU_ONLY)
	TheW3DVolumetricShadowManager = NEW W3DVolumetricShadowManager;
	TheProjectedShadowManager = TheW3DProjectedShadowManager = NEW W3DProjectedShadowManager;
#endif
}

W3DShadowManager::~W3DShadowManager( void )
{
#if defined(ZH_WW3D_CPU_ONLY)
	removeAllShadows();
	if (s_emptyShadowOwner == this) s_emptyShadowOwner = NULL;
	if (TheW3DShadowManager == this) TheW3DShadowManager = NULL;
#endif
#if !defined(ZH_WW3D_CPU_ONLY)
	delete TheW3DVolumetricShadowManager;
	TheW3DVolumetricShadowManager = NULL;
	delete TheW3DProjectedShadowManager;
	TheProjectedShadowManager = TheW3DProjectedShadowManager = NULL;
#endif
}

/** Do one-time initilalization of shadow systems that need to be
active for full duration of game*/
Bool W3DShadowManager::init( void )
{
#if defined(ZH_WW3D_CPU_ONLY)
	if (!zh::original_runtime::OriginalGpuEdge::active() ||
		!W3DDisplay::m_3DScene || (TheGlobalData->m_useShadowVolumes && !std::getenv("ZH_M22_VOLUME_SHADOW_PROFILE")) ||
		(TheGlobalData->m_useShadowDecals && !std::getenv("ZH_M22_SHADOW_DECAL_PROFILE") && !std::getenv("ZH_M22_FULL_FEATURE_PROFILE")) ||
		(TheW3DShadowManager && TheW3DShadowManager != this) ||
		(s_emptyShadowOwner && s_emptyShadowOwner != this)) {
		if (TheW3DShadowManager == this && s_emptyShadowOwner != this)
			TheW3DShadowManager = NULL;
		throw OriginalW3DDeviceUnavailable("original enabled-shadow or owner bootstrap pending");
	}
	TheW3DShadowManager = this;
	s_emptyShadowOwner = this;
	return TRUE;
#else
	Bool result=TRUE;

	if	(TheW3DVolumetricShadowManager && TheW3DVolumetricShadowManager->init())
	{
		if (TheW3DVolumetricShadowManager->ReAcquireResources())
			result = TRUE;
	}
	if ( TheW3DProjectedShadowManager && TheW3DProjectedShadowManager->init())
	{
		if (TheW3DProjectedShadowManager->ReAcquireResources())
			result = TRUE;
	}

	return result;
#endif
}

/** Do per-map reset.  This frees up shadows from all objects since
they may not exist on the next map*/
void W3DShadowManager::Reset( void )
{
#if defined(ZH_WW3D_CPU_ONLY)
	requireEmptyShadowOwner(this);
	// Source draw modules retain their Shadow pointer across a terrain reset.
	// Keep the admitted owner list intact and let the normal release/reacquire
	// lifecycle rebuild its resources; removing it here would strand that source
	// pointer and make its next allocateShadows call silently skip re-admission.
	for (auto *shadow : s_boundedDecals) shadow->releaseResources(); for (auto *shadow : s_boundedVolumes) shadow->releaseResources();
	m_isShadowScene = FALSE;
	m_stencilShadowMask = 0;
#else
	if (TheW3DVolumetricShadowManager)
		TheW3DVolumetricShadowManager->reset();
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->reset();
#endif
}

Bool W3DShadowManager::ReAcquireResources()
{
#if defined(ZH_WW3D_CPU_ONLY)
	requireEmptyShadowOwner(this);
	try {
		for (auto *shadow : s_boundedDecals) shadow->reacquire();
		for (auto *shadow : s_boundedVolumes) shadow->reacquire();
	} catch (...) {
		for (auto *shadow : s_boundedDecals) shadow->releaseResources();
		for (auto *shadow : s_boundedVolumes) shadow->releaseResources();
		throw;
	}
	return TRUE;
#else
	Bool result = TRUE;

	if (TheW3DVolumetricShadowManager && !TheW3DVolumetricShadowManager->ReAcquireResources())
		result = FALSE;
	if (TheW3DProjectedShadowManager && !TheW3DProjectedShadowManager->ReAcquireResources())
		result = FALSE;

	return result;
#endif
}

void W3DShadowManager::ReleaseResources(void)
{
#if defined(ZH_WW3D_CPU_ONLY)
	requireEmptyShadowOwner(this);
	for (auto *shadow : s_boundedDecals) shadow->releaseResources(); for (auto *shadow : s_boundedVolumes) shadow->releaseResources();
#else
	if (TheW3DVolumetricShadowManager)
		TheW3DVolumetricShadowManager->ReleaseResources();
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->ReleaseResources();
#endif
}

Shadow *W3DShadowManager::addShadow( RenderObjClass *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw)
{
	ShadowType type = SHADOW_VOLUME;

	if (shadowInfo)
		type = shadowInfo->m_type;

	switch(type)
	{
		case	SHADOW_VOLUME:
#if defined(ZH_WW3D_CPU_ONLY)
			if (!std::getenv("ZH_M22_VOLUME_SHADOW_PROFILE") || !TheGlobalData->m_useShadowVolumes)
				throw OriginalW3DDeviceUnavailable("original volumetric shadow derived-manager creation pending");
			if (TheW3DShadowManager!=this || !shadowInfo || !hasPublishedModelRenderObject(robj))
				throw OriginalW3DDeviceUnavailable("original bounded volume shadow owner or state unavailable");
			for (auto *shadow:s_boundedVolumes) if(shadow->owns(robj)) throw OriginalW3DDeviceUnavailable("original duplicate volume shadow owner");
			{ auto *shadow=NEW BoundedVolumeShadow(robj,*shadowInfo); s_boundedVolumes.push_back(shadow); return shadow; }
#else
			if (TheW3DVolumetricShadowManager)
				return (Shadow *)TheW3DVolumetricShadowManager->addShadow(robj, shadowInfo, draw);
#endif
			break;
		case	SHADOW_PROJECTION:
		case	SHADOW_DECAL:
#if defined(ZH_WW3D_CPU_ONLY)
			if (type != SHADOW_DECAL)
				throw OriginalW3DDeviceUnavailable("original projected shadow derived-manager creation pending");
			if (!TheGlobalData->m_useShadowDecals)
				throw OriginalW3DDeviceUnavailable("original decal shadow derived-manager creation pending");
			if (TheGlobalData->m_useShadowVolumes ||
				TheW3DShadowManager != this || !shadowInfo || shadowInfo->m_sizeX <= 0 ||
				shadowInfo->m_sizeY <= 0 || !hasPublishedModelRenderObject(robj))
				throw OriginalW3DDeviceUnavailable("original bounded decal shadow owner or state unavailable");
			for (auto *shadow : s_boundedDecals)
				if (shadow->owns(robj))
					throw OriginalW3DDeviceUnavailable("original duplicate decal shadow owner");
			{
				auto *shadow=NEW BoundedDecalShadow(robj,*shadowInfo);
				s_boundedDecals.push_back(shadow);
				return shadow;
			}
#else
			if (TheW3DProjectedShadowManager)
				return (Shadow *)TheW3DProjectedShadowManager->addShadow(robj, shadowInfo, draw);
#endif
			break;
		default:
			return NULL;
	}
		
	return NULL;
}

void W3DShadowManager::removeShadow(Shadow *shadow)
{
	if (shadow) shadow->release();
}

void W3DShadowManager::removeAllShadows(void)
{
#if defined(ZH_WW3D_CPU_ONLY)
	if (TheW3DShadowManager != this) return;
	while (!s_boundedDecals.empty()) {
		auto *shadow=s_boundedDecals.back(); s_boundedDecals.pop_back(); delete shadow;
	}
	while (!s_boundedVolumes.empty()) { auto *shadow=s_boundedVolumes.back(); s_boundedVolumes.pop_back(); delete shadow; }
#else
	if (TheW3DVolumetricShadowManager)
		TheW3DVolumetricShadowManager->removeAllShadows();
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->removeAllShadows();
#endif
}

/**Force update of all shadows even when light source and object have not moved*/
void W3DShadowManager::invalidateCachedLightPositions(void)
{
#if defined(ZH_WW3D_CPU_ONLY)
	requireEmptyShadowOwner(this);
#else
	if (TheW3DVolumetricShadowManager)
		TheW3DVolumetricShadowManager->invalidateCachedLightPositions();
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->invalidateCachedLightPositions();
#endif
}

Vector3 &W3DShadowManager::getLightPosWorld(Int lightIndex)
{
	return LightPosWorld[lightIndex];
}

void W3DShadowManager::setLightPosition(Int lightIndex, Real x, Real y, Real z)
{
	if (lightIndex != 0)
		return;	///@todo: Add support for multiple lights

	LightPosWorld[lightIndex]=Vector3(x,y,z);
}

void W3DShadowManager::setTimeOfDay(TimeOfDay tod)
{
	//Ray to light source
	const GlobalData::TerrainLighting *ol=&TheGlobalData->m_terrainObjectsLighting[tod][0];

	Vector3 lightRay(-ol->lightPos.x,-ol->lightPos.y,-ol->lightPos.z);

	lightRay.Normalize();
	lightRay *= SUN_DISTANCE_FROM_GROUND;

	setLightPosition(0, lightRay.X, lightRay.Y, lightRay.Z);
}

Bool W3DShadowManager::ownsBoundedDecalCaster(RenderObjClass *robj) const
{
#if defined(ZH_WW3D_CPU_ONLY)
	return TheW3DShadowManager == this && std::any_of(s_boundedDecals.begin(),s_boundedDecals.end(),
		[robj](const BoundedDecalShadow *shadow) { return shadow->owns(robj); });
#else
	return FALSE;
#endif
}

Bool W3DShadowManager::hasBoundedDecalCasters() const
{
#if defined(ZH_WW3D_CPU_ONLY)
	return TheW3DShadowManager == this && !s_boundedDecals.empty();
#else
	return FALSE;
#endif
}

Bool W3DShadowManager::ownsBoundedVolumeCaster(RenderObjClass *robj) const
{
#if defined(ZH_WW3D_CPU_ONLY)
	return TheW3DShadowManager == this && std::any_of(s_boundedVolumes.begin(),s_boundedVolumes.end(),
		[robj](const BoundedVolumeShadow *shadow) { return shadow->owns(robj); });
#else
	return FALSE;
#endif
}

Bool W3DShadowManager::hasBoundedVolumeCasters() const
{
#if defined(ZH_WW3D_CPU_ONLY)
	return TheW3DShadowManager == this && !s_boundedVolumes.empty();
#else
	return FALSE;
#endif
}

void W3DShadowManager::RenderShadows(void)
{
#if defined(ZH_WW3D_CPU_ONLY)
	if (TheW3DShadowManager != this || !m_isShadowScene) return;
	if ((!TheGlobalData->m_useShadowDecals && !TheGlobalData->m_useShadowVolumes) || !TheHeightMap || !TheHeightMap->getMap())
		throw OriginalW3DDeviceUnavailable("original bounded decal shadow scene unavailable");
	try {
		if (!s_boundedShadowStencilPass) {
			for (auto *shadow : s_boundedDecals) shadow->draw();
			// The source scene performs the volume operation on its later stencil
			// pass.  Keep this frame queued across the first call only for the
			// explicitly admitted volume profile; ordinary decals retain their
			// one-pass lifetime.
			if (!TheGlobalData->m_useShadowVolumes) m_isShadowScene=FALSE;
		} else {
			for (auto *shadow:s_boundedVolumes) shadow->draw();
			m_isShadowScene=FALSE;
		}
	} catch (...) { m_isShadowScene=FALSE; throw; }
#else
	throw OriginalW3DDeviceUnavailable("original shadow physical render pending");
#endif
}
