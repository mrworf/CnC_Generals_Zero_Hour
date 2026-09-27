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

// FILE: W3DPropBuffer.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: W3DPropBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the props in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#if defined(ZH_WW3D_CPU_ONLY)
#include "PreRTS.h"
#include "original_gpu_edge.h"
#endif
#include "W3DDevice/GameClient/W3DPropBuffer.h"

#include <stdio.h>
#include <string.h>
#include <assetmgr.h>
#include "Common/Geometry.h"
#include "Common/PerfTimer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "WW3D2/camera.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/light.h"
#include "WW3D2/lightenvironment.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8renderer.h"
#include "W3DDevice/GameClient/Module/W3DPropDraw.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "GameLogic/PartitionManager.h"
#include "Common/Xfer.h"
#if defined(__linux__)
#include "clone_graph.h"
#include "prop_graph.h"
#include "original_gpu_edge.h"
#include <exception>
#include <cmath>
#include <limits>
#endif

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif




//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------

//=============================================================================
// W3DPropBuffer::cull
//=============================================================================
/** Culls the props, marking the visible flag.  If a prop becomes visible, it sets
it's sortKey */
//=============================================================================
void W3DPropBuffer::cull(CameraClass * camera)
{
	Int curProp;

	for (curProp=0; curProp<m_numProps; curProp++) {
		Bool visible = !camera->Cull_Sphere(m_props[curProp].bounds);
		m_props[curProp].visible=visible;
	}
}



//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// W3DPropBuffer::~W3DPropBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DPropBuffer::~W3DPropBuffer(void)
{
#if defined(__linux__)
	// Every published teardown route must admit idle retirement before deleting
	// the owner. Constructor rollback is private and never enters a source frame.
	if (auto* edge=zh::original_runtime::OriginalGpuEdge::active()) {
		if (!edge->source_buffers_retirable() || edge->source_stages_active()) std::terminate();
	}
	releaseAllProps();
	REF_PTR_RELEASE(m_propShroudMaterialPass);
	REF_PTR_RELEASE(m_light);
#else
	Int i;
	for (i=0; i<MAX_TYPES; i++) {
		REF_PTR_RELEASE(m_propTypes[i].m_robj);
	}
	REF_PTR_RELEASE(m_light);
	REF_PTR_RELEASE(m_propShroudMaterialPass);
#endif
}

//=============================================================================
// W3DPropBuffer::W3DPropBuffer
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the props. */
//=============================================================================
W3DPropBuffer::W3DPropBuffer(void)
{
#if defined(__linux__)
	m_numProps=0;m_numPropTypes=0;m_anythingChanged=false;
	m_initialized=false;m_doCull=true;m_light=NULL;m_propShroudMaterialPass=NULL;
	for (int i=0;i<MAX_PROPS;++i) {
		m_props[i].m_robj=NULL;m_props[i].id=0;m_props[i].location.set(0,0,0);
		m_props[i].propType=-1;m_props[i].ss=OBJECTSHROUD_INVALID;m_props[i].visible=false;
		m_props[i].bounds.Init(Vector3(0,0,0),1.0f);
	}
	for (int i=0;i<MAX_TYPES;++i) {
		m_propTypes[i].m_robj=NULL;
		m_propTypes[i].m_bounds.Init(Vector3(0,0,0),0.0f);
	}
	preflightRemoval();
	ww3d_clone::Attempt attempt;
	try {
		ww3d_clone::Attempt::fault();
		ww3d_clone::Ref<LightClass> light(NEW_REF(LightClass,(LightClass::DIRECTIONAL)));
		ww3d_clone::Attempt::fault();
		ww3d_clone::Ref<W3DShroudMaterialPassClass> pass(NEW_REF(W3DShroudMaterialPassClass,()));
		m_light=light.release();m_propShroudMaterialPass=pass.release();
		m_initialized=true;attempt.commit();
	} catch (...) { releaseAllProps();throw; }
#else
	memset(this, sizeof(W3DPropBuffer), 0);
	m_initialized = false;
	clearAllProps();
	m_light = NEW_REF( LightClass, (LightClass::DIRECTIONAL) );
	m_propShroudMaterialPass = NEW_REF(W3DShroudMaterialPassClass,());
	m_initialized = true;
#endif
}





//=============================================================================
// W3DPropBuffer::clearAllProps
//=============================================================================
/** Removes all props. */
//=============================================================================
void W3DPropBuffer::clearAllProps(void)
{
#if defined(__linux__)
	preflightRemoval();
	releaseAllProps();
#else
	m_numProps=0;
	Int i;
	for (i=0; i<MAX_TYPES; i++) {
		REF_PTR_RELEASE(m_propTypes[i].m_robj);
		m_propTypes[i].m_robjName.clear();
	}
	m_numPropTypes = 0;
#endif
}

#if defined(__linux__)
void W3DPropBuffer::preflightRemoval() const
{
	if (auto* edge=zh::original_runtime::OriginalGpuEdge::active()) {
		if (edge->source_stages_active()) {
			edge->poison_source_stages();
			throw std::runtime_error("original prop source attempt is active");
		}
		if (!edge->source_buffers_retirable())
			throw std::runtime_error("original prop frame is active");
	}
}

void W3DPropBuffer::releaseAllProps() noexcept
{
	for (int i=m_numProps;i>0;--i) {
		REF_PTR_RELEASE(m_props[i-1].m_robj);
		m_props[i-1].propType=-1;m_props[i-1].visible=false;
		m_props[i-1].ss=OBJECTSHROUD_INVALID;
		m_props[i-1].id=0;m_props[i-1].location.set(0,0,0);
		m_props[i-1].bounds.Init(Vector3(0,0,0),1.0f);
	}
	m_numProps=0;
	for (int i=m_numPropTypes;i>0;--i) {
		REF_PTR_RELEASE(m_propTypes[i-1].m_robj);
		m_propTypes[i-1].m_robjName.clear();
		m_propTypes[i-1].m_bounds.Init(Vector3(0,0,0),0.0f);
	}
	m_numPropTypes=0;m_anythingChanged=false;m_doCull=true;
}
#endif

//=============================================================================
// W3DPropBuffer::addPropTypes
//=============================================================================
/** Adds a type of prop (model & texture). */
//=============================================================================
Int W3DPropBuffer::addPropType(const AsciiString &modelName)
{
#if defined(__linux__)
	preflightRemoval();
	if (!m_initialized || modelName.isEmpty() || m_numPropTypes>=MAX_TYPES)
		throw std::runtime_error("original prop type admission is unavailable");
	for (int i=0;i<m_numPropTypes;++i)
		if (!m_propTypes[i].m_robjName.compareNoCase(modelName))
			throw std::runtime_error("original prop duplicate type is not admitted");
	ww3d_prop::Attempt attempt(WW3DAssetManager::Get_Instance());
	ww3d_clone::Attempt::fault();
	ww3d_clone::Ref<RenderObjClass> candidate(WW3DAssetManager::Get_Instance()->Create_Render_Obj(modelName.str()));
	if (!candidate.get()) { attempt.commit();return -1; }
	ww3d_clone::Attempt::fault();
	AsciiString name(modelName);
	SphereClass bounds=candidate.get()->Get_Bounding_Sphere();
	if (!std::isfinite(bounds.Center.X) || !std::isfinite(bounds.Center.Y) ||
		!std::isfinite(bounds.Center.Z) || !std::isfinite(bounds.Radius) || bounds.Radius<0)
		throw std::runtime_error("original prop type bounds are not admitted");
	ww3d_clone::Attempt::fault();
	// AsciiString shares an already-owned buffer; this final assignment cannot
	// allocate. All fallible name/model/graph work precedes publication.
	m_propTypes[m_numPropTypes].m_robjName=name;
	m_propTypes[m_numPropTypes].m_bounds=bounds;
	m_propTypes[m_numPropTypes].m_robj=candidate.release();
	const int index=m_numPropTypes++;
	attempt.commit();return index;
#else
	if (m_numPropTypes>=MAX_TYPES) {
		DEBUG_CRASH(("Too many kinds of props in map.  Reduce kinds of props, or raise prop limit. jba.")); 
		return 0;
	}

	m_propTypes[m_numPropTypes].m_robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(modelName.str());
	if (m_propTypes[m_numPropTypes].m_robj==NULL) {
		DEBUG_CRASH(("Unable to find model for prop %s\n", modelName.str()));
		return -1;
	}
	m_propTypes[m_numPropTypes].m_robjName = modelName;
	
	SphereClass bounds = m_propTypes[m_numPropTypes].m_robj->Get_Bounding_Sphere();
	m_propTypes[m_numPropTypes].m_bounds = bounds;
	m_numPropTypes++;
	return m_numPropTypes-1;
#endif
}

//=============================================================================
// W3DPropBuffer::addProp
//=============================================================================
/** Adds a prop.  Name is the W3D model name, supported models are
ALPINE, DECIDUOUS and SHRUB. */
//=============================================================================
void W3DPropBuffer::addProp(Int id, Coord3D location, Real angle,Real scale, const AsciiString &modelName)
{
#if defined(__linux__)
	preflightRemoval();
	if (!m_initialized || m_numProps>=MAX_PROPS || modelName.isEmpty() ||
		!std::isfinite(location.x) || !std::isfinite(location.y) || !std::isfinite(location.z) ||
		!std::isfinite(angle) || !std::isfinite(scale))
		throw std::runtime_error("original prop instance admission is unavailable");
	int type=-1;
	for (int i=0;i<m_numPropTypes;++i)
		if (!m_propTypes[i].m_robjName.compareNoCase(modelName)) { type=i;break; }
	const bool new_type=type<0;
	if (new_type && m_numPropTypes>=MAX_TYPES)
		throw std::runtime_error("original prop type capacity is exhausted");
	ww3d_prop::Attempt attempt(WW3DAssetManager::Get_Instance());
	std::optional<ww3d_clone::Ref<RenderObjClass>> candidate_type;
	RenderObjClass* source=nullptr;
	SphereClass bounds;
	AsciiString name;
	if (new_type) {
		ww3d_clone::Attempt::fault();
		ww3d_clone::Ref<RenderObjClass> acquired(WW3DAssetManager::Get_Instance()->Create_Render_Obj(modelName.str()));
		if (!acquired.get()) { attempt.commit();return; }
		source=acquired.get();
		bounds=source->Get_Bounding_Sphere();
		ww3d_clone::Attempt::fault();
		name=modelName;
		candidate_type.emplace(acquired.release());
		type=m_numPropTypes;
	} else {
		source=m_propTypes[type].m_robj;bounds=m_propTypes[type].m_bounds;
		ww3d_prop::Audit audit(*WW3DAssetManager::Get_Instance());
		audit.render(source);
	}
	if (!source || !std::isfinite(bounds.Center.X) || !std::isfinite(bounds.Center.Y) ||
		!std::isfinite(bounds.Center.Z) || !std::isfinite(bounds.Radius) || bounds.Radius<0)
		throw std::runtime_error("original prop type provider is not admitted");
	ww3d_clone::Attempt::fault();
	ww3d_clone::Ref<RenderObjClass> instance(source->Clone());
	if (!instance.get()) throw std::runtime_error("original prop instance is absent");
	Matrix3D transform(true);transform.Rotate_Z(angle);transform.Scale(scale);
	transform.Set_Translation(Vector3(location.x,location.y,location.z));
	ww3d_clone::Attempt::fault();
	instance.get()->Set_Transform(transform);instance.get()->Set_ObjectScale(scale);
	SphereClass translated=bounds;translated.Center+=Vector3(location.x,location.y,location.z);
	if (!std::isfinite(translated.Center.X) || !std::isfinite(translated.Center.Y) ||
		!std::isfinite(translated.Center.Z))
		throw std::runtime_error("original prop translated bounds are not admitted");
	ww3d_clone::Attempt::fault(); // type publication boundary, before any owner mutation
	ww3d_clone::Attempt::fault(); // instance publication boundary, also before any mutation
	if (new_type) {
		m_propTypes[type].m_robjName=name;m_propTypes[type].m_bounds=bounds;
		m_propTypes[type].m_robj=candidate_type->release();++m_numPropTypes;
	}
	TProp& published=m_props[m_numProps];
	published.location=location;published.id=id;published.ss=OBJECTSHROUD_INVALID;
	published.m_robj=instance.release();published.propType=type;
	published.bounds=translated;published.visible=false;++m_numProps;
	attempt.commit();
#else
	if (m_numProps >= MAX_PROPS) {
		return;  
	}
	if (!m_initialized) {
		return;  
	}
	Int propType = -1;
	Int i;
	for (i=0; i<m_numPropTypes; i++) {
		if (m_propTypes[i].m_robjName.compareNoCase(modelName)==0) {
			propType = i;
			break;
		}
	}
	if (propType<0) {
		propType = addPropType(modelName);
		if (propType<0) {
			return;
		}
	}

	Matrix3D mtx(true);
	mtx.Rotate_Z(angle);
	mtx.Scale(scale);
	mtx.Set_Translation(Vector3(location.x, location.y, location.z));

	m_props[m_numProps].location = location;
	m_props[m_numProps].id = id;
	m_props[m_numProps].ss = OBJECTSHROUD_INVALID;
	m_props[m_numProps].m_robj = m_propTypes[propType].m_robj->Clone();
	m_props[m_numProps].m_robj->Set_Transform(mtx);
	m_props[m_numProps].m_robj->Set_ObjectScale(scale);
	m_props[m_numProps].propType = propType;
	// Translate the bounding sphere of the model.
	m_props[m_numProps].bounds = m_propTypes[propType].m_bounds;
	m_props[m_numProps].bounds.Center += Vector3(location.x, location.y, location.z);
	// Initially set it invisible.  cull will update it's visiblity flag.
	m_props[m_numProps].visible = false;

	m_numProps++;
#endif
}

//=============================================================================
// W3DPropBuffer::updatePropPosition
//=============================================================================
/** Updates a prop's position */
//=============================================================================
Bool W3DPropBuffer::updatePropPosition(Int id, const Coord3D &location, Real angle, Real scale)
{
#if defined(__linux__)
	preflightRemoval();
	if (!std::isfinite(location.x) || !std::isfinite(location.y) || !std::isfinite(location.z) ||
		!std::isfinite(angle) || !std::isfinite(scale))
		throw std::runtime_error("original prop transform is not admitted");
#endif
	Int i;
	for (i=0; i<m_numProps; i++) {
		if (m_props[i].id == id) {
#if defined(__linux__)
			// Native removal keeps tombstones/IDs. Never dereference a removed
			// instance when a later live instance reuses the same source ID.
			if (!m_props[i].m_robj) continue;
			if (m_props[i].propType<0 || m_props[i].propType>=m_numPropTypes ||
				!m_propTypes[m_props[i].propType].m_robj)
				throw std::runtime_error("original prop transform owner is not admitted");
			SphereClass translated=m_propTypes[m_props[i].propType].m_bounds;
			translated.Center+=Vector3(location.x,location.y,location.z);
			if (!std::isfinite(translated.Center.X) || !std::isfinite(translated.Center.Y) ||
				!std::isfinite(translated.Center.Z))
				throw std::runtime_error("original prop translated bounds are not admitted");
#endif
			Matrix3D mtx(true);
			mtx.Rotate_Z(angle);
			mtx.Scale(scale);
			mtx.Set_Translation(Vector3(location.x, location.y, location.z));
#if !defined(__linux__)
			m_props[i].location = location;
#endif
			m_props[i].m_robj->Set_Transform(mtx);
			m_props[i].m_robj->Set_ObjectScale(scale);
#if defined(__linux__)
			m_props[i].location = location;
#endif
			// Translate the bounding sphere of the model.
			m_props[i].bounds = m_propTypes[m_props[i].propType].m_bounds;
			m_props[i].bounds.Center += Vector3(location.x, location.y, location.z);
			m_anythingChanged = true;
			return true;
		}
	}
	return false;
}

//=============================================================================
// W3DPropBuffer::removeProp
//=============================================================================
/** Removes a prop.  */
//=============================================================================
void W3DPropBuffer::removeProp(Int id)
{
#if defined(__linux__)
	preflightRemoval();
#endif
	Int i;
	for (i=0; i<m_numProps; i++) {
		if (m_props[i].id == id) {
			m_props[i].location.set(0,0,0);
			m_props[i].propType = -1;
			REF_PTR_RELEASE(m_props[i].m_robj);
			// Translate the bounding sphere of the model.
			m_props[i].bounds.Center = Vector3(0,0,0);
			m_props[i].bounds.Radius = 1;
			m_anythingChanged = true;
		}
	}
}

//=============================================================================
// W3DPropBuffer::removePropsForConstruction
//=============================================================================
/** Removes any props that would be under a building.  */
//=============================================================================
void W3DPropBuffer::removePropsForConstruction(const Coord3D* pos, const GeometryInfo& geom, Real angle )
{
#if defined(__linux__)
	preflightRemoval();
	if (!pos || !ThePartitionManager || !std::isfinite(pos->x) || !std::isfinite(pos->y) ||
		geom.getGeomType()<GEOMETRY_FIRST || geom.getGeomType()>=GEOMETRY_NUM_TYPES ||
		!std::isfinite(pos->z) || !std::isfinite(angle) || !std::isfinite(geom.getMajorRadius()) ||
		!std::isfinite(geom.getMinorRadius()) || !std::isfinite(geom.getMaxHeightAbovePosition()) ||
		!std::isfinite(geom.getMaxHeightBelowPosition()) || !std::isfinite(geom.getBoundingCircleRadius()))
		throw std::runtime_error("original prop construction geometry is not admitted");
	for (int p=0;p<m_numProps;++p) if (m_props[p].m_robj) {
		const Real radius=m_props[p].bounds.Radius;
		if (!std::isfinite(radius) || radius<0 || radius>(std::numeric_limits<Real>::max)()/5 ||
			!std::isfinite(m_props[p].location.x) || !std::isfinite(m_props[p].location.y) ||
			!std::isfinite(m_props[p].location.z))
			throw std::runtime_error("original prop construction radius is not admitted");
	}
#endif
	// Just iterate all trees, as even non-collidable ones get removed. jba. [7/11/2003]
	Int i;
	for (i=0; i<m_numProps; i++) {				
		if (m_props[i].m_robj == NULL) {
			continue; // already deleted.
		}
		Real radius = m_props[i].bounds.Radius;
		GeometryInfo info(GEOMETRY_CYLINDER, false, 5*radius, 2*radius, 2*radius);
		if (ThePartitionManager->geomCollidesWithGeom( pos, geom, angle, &m_props[i].location, info, 0.0f)) {
			// remove it [7/11/2003]
			m_props[i].location.set(0,0,0);
			m_props[i].propType = -1;
			REF_PTR_RELEASE(m_props[i].m_robj);
			// Translate the bounding sphere of the model.
			m_props[i].bounds.Center = Vector3(0,0,0);
			m_props[i].bounds.Radius = 1;
			m_anythingChanged = true;
		} 
	}
}



//=============================================================================
// W3DPropBuffer::notifyShroudChanged
//=============================================================================
/** Sets the shroud to status, so it is recomputed.  */
//=============================================================================
void W3DPropBuffer::notifyShroudChanged()
{
	Int i;
	for (i=0; i<m_numProps; i++) {
		m_props[i].ss = ThePartitionManager?OBJECTSHROUD_INVALID:OBJECTSHROUD_CLEAR;
	}
}


DECLARE_PERF_TIMER(Prop_Render)

//=============================================================================
// W3DPropBuffer::drawProps
//=============================================================================
/** Draws the props.  Uses camera to cull. */
//=============================================================================
void W3DPropBuffer::drawProps(RenderInfoClass &rinfo)
{
#if defined(ZH_WW3D_CPU_ONLY)
	// Logical admission is separate from the R0B pass and R0C frame owner.
	throw std::runtime_error("original prop draw owner is not admitted");
#else
	USE_PERF_TIMER(Prop_Render)

	Int i;
	if (m_doCull) {
		cull(&rinfo.Camera);
	}
	const GlobalData::TerrainLighting *objectLighting = TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay];

	LightEnvironmentClass lightEnv;
	Vector3 center(0,0,0); // arbitrary center point. [6/6/2003]
	Vector3 ambient(objectLighting[0].ambient.red, objectLighting[0].ambient.green, objectLighting[0].ambient.blue);
	lightEnv.Reset(center, ambient);

	Matrix3D mtx;
	const Vector3 zeroVector(0.0f, 0.0f, 0.0f);
	const Vector3 xVector(1.0f, 0.0f, 0.0f);
	const Vector3 yVector(0.0f, 1.0f, 0.0f);

	for (i = 0; i < MAX_GLOBAL_LIGHTS; ++i)
	{
			m_light->Set_Ambient(zeroVector);
			m_light->Set_Diffuse(Vector3(objectLighting[i].diffuse.red,
																		 objectLighting[i].diffuse.green,
																		 objectLighting[i].diffuse.blue));
			m_light->Set_Specular(zeroVector);
			mtx.Set(xVector, yVector, Vector3(objectLighting[i].lightPos.x, objectLighting[i].lightPos.y, objectLighting[i].lightPos.z), zeroVector);
			m_light->Set_Transform(mtx);
			lightEnv.Add_Light(*m_light);
	}
	
	rinfo.light_environment = &lightEnv;
	for	(i=0; i<m_numProps; i++) {
		if (!m_props[i].visible) {
			continue;
		}
		if (m_props[i].m_robj==NULL) {
			continue;
		}
		if (!ThePlayerList || !ThePartitionManager) {
			//  in Worldbuilder. No shroud either. jba. [6/9/2003]
			m_props[i].ss = OBJECTSHROUD_CLEAR;
		}
		if (m_props[i].ss == OBJECTSHROUD_INVALID) {
			Int localPlayerIndex = ThePlayerList ? ThePlayerList->getLocalPlayer()->getPlayerIndex() : 0;
			m_props[i].ss = ThePartitionManager->getPropShroudStatusForPlayer(localPlayerIndex, &m_props[i].location);
		}
		if (m_props[i].ss >= OBJECTSHROUD_SHROUDED) {
			continue;
		}
		if (m_props[i].ss <= OBJECTSHROUD_INVALID) {
			continue;
		}
		if (TheTerrainRenderObject->getShroud() && m_props[i].ss != CELLSHROUD_CLEAR) {
			rinfo.Push_Material_Pass(m_propShroudMaterialPass);
			m_props[i].m_robj->Render(rinfo);
			rinfo.Pop_Material_Pass();
		} else {
			m_props[i].m_robj->Render(rinfo);
		}
	}
	rinfo.light_environment = NULL;
#endif
}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DPropBuffer::crc( Xfer *xfer )
{
	// empty. jba [8/11/2003]	
}  // end CRC

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DPropBuffer::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DPropBuffer::loadPostProcess( void )
{
	// empty. jba [8/11/2003]	
}  // end loadPostProcess
