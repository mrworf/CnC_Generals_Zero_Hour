// SPDX-License-Identifier: GPL-3.0-or-later
// Camera choreography adapted from original Electronic Arts W3DView.cpp.
#include "PreRTS.h"
#include "GameClient/ScriptedView.h"
#include "Common/GlobalData.h"
#include "Common/Terrain.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/AIPathfind.h"
#include "WWMath/vector2.h"
#include "WWMath/wwmath.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace {
void finite(Real value) { if(!std::isfinite(value))throw ERROR_BAD_ARG; }
void point(const Coord3D* value) {
  if(!value)throw ERROR_BAD_ARG;
  finite(value->x);finite(value->y);finite(value->z);
  // Vector2's original squared-length arithmetic must remain finite too.
  const Real limit=std::sqrt(std::numeric_limits<Real>::max())/4;
  if(std::abs(value->x)>limit || std::abs(value->y)>limit)throw ERROR_BAD_ARG;
}
void timing(Real easeIn,Real easeOut) { finite(easeIn);finite(easeOut); }
void parents() { if(!TheGlobalData || !TheTerrainLogic)throw ERROR_BAD_ARG; }
Int duration(std::int64_t value) {
  if(value>std::numeric_limits<Int>::max())throw ERROR_BAD_ARG;
  return value<1?1:static_cast<Int>(value);
}
Int roundedMultiplier(Real value) {
  finite(value);
  const double result=std::floor(static_cast<double>(value));
  return static_cast<Int>(std::clamp(result,double(std::numeric_limits<Int>::min()),double(std::numeric_limits<Int>::max())));
}
Int interpolateMultiplier(Int start,Int end,Real factor) {
  const Real delta=static_cast<Real>(std::int64_t(end)-start);
  const auto result=std::int64_t(start)+static_cast<std::int64_t>(std::floor(double(0.5f+delta*factor)));
  return static_cast<Int>(std::clamp(result,std::int64_t(std::numeric_limits<Int>::min()),std::int64_t(std::numeric_limits<Int>::max())));
}
void validatePath(Waypoint* node,const Coord3D& origin) {
  Coord3D previous=origin;
  Int count=1;
  // Brent's constant-storage cycle detection resets whenever an accepted point
  // makes progress toward the source 25-point limit. A cycle consisting solely
  // of discarded near-coincident points otherwise never terminates.
  Waypoint* checkpoint=node;
  std::uint64_t power=1,length=0;
  while(node && count<25){
    point(node->getLocation());
    const Coord3D position=*node->getLocation();
    Waypoint* next=node->getNumLinks()>0?node->getLink(0):nullptr;
    const Vector2 direction(position.x-previous.x,position.y-previous.y);
    if(direction.Length()>=MAP_XY_FACTOR){
      ++count;previous=position;checkpoint=next;power=1;length=0;
    }else if(next){
      ++length;if(next==checkpoint)throw ERROR_BAD_ARG;
      if(length==power){checkpoint=next;power*=2;length=0;}
    }
    node=next;
  }
}
}

ScriptedView::ScriptedView() { m_lockType=LOCK_FOLLOW;m_pos.z=0; }
void ScriptedView::init() {
  if(!TheGlobalData)throw ERROR_BAD_ARG;
  finite(TheGlobalData->m_cameraHeight);finite(TheGlobalData->m_cameraPitch);finite(TheGlobalData->m_cameraYaw);
  const Real y=-(TheGlobalData->m_cameraHeight / tan(TheGlobalData->m_cameraPitch*(PI/180.0)));
  const Real x=-(y*tan(TheGlobalData->m_cameraYaw*(PI/180.0)));
  if(TheGlobalData->m_cameraHeight<=0 || !std::isfinite(x) || !std::isfinite(y))throw ERROR_BAD_ARG;
  View::init();
  m_cameraOffset={x,y,TheGlobalData->m_cameraHeight};
  cameraStateChanged();
}
void ScriptedView::setFrameLengthMilliseconds(Int value) {
  if(value<1)throw ERROR_BAD_ARG;
  m_frameLengthMilliseconds=value;
}
void ScriptedView::reset() {
  parents();View::reset();setTimeMultiplier(1);
  const Coord3D origin{};resetCamera(&origin,1,0,0);
  m_viewFilter=FT_VIEW_DEFAULT;m_viewFilterMode=FM_VIEW_DEFAULT;
  const Coord2D guard{};setGuardBandBias(&guard);
}
void ScriptedView::lookAt(const Coord3D* value) {
  parents();point(value);
  Coord3D position=*value;
  if(position.z>PATHFIND_CELL_SIZE_F+TheTerrainLogic->getGroundHeight(position.x,position.y)) {
    position=projectElevatedLookAt(position);point(&position);
  }
  position.z=0;setPosition(&position);
  m_doingRotateCamera=FALSE;m_doingMoveCameraOnWaypointPath=FALSE;
  m_CameraArrivedAtWaypointOnPathFlag=FALSE;m_doingScriptedCameraLock=FALSE;
  cameraStateChanged();
}
Bool ScriptedView::updateCameraMovements(Int elapsedMilliseconds) {
  parents();if(elapsedMilliseconds<0)throw ERROR_BAD_ARG;
  Bool updated=FALSE;
  if(m_doingZoomCamera){zoomCameraOneFrame();updated=TRUE;}
  if(m_doingPitchCamera){pitchCameraOneFrame();updated=TRUE;}
  if(m_doingRotateCamera){rotateCameraOneFrame();updated=TRUE;}
  else if(m_doingMoveCameraOnWaypointPath){moveAlongWaypointPath(elapsedMilliseconds);updated=TRUE;}
  if(m_doingScriptedCameraLock)updated=TRUE;
  if(updated)cameraStateChanged();
  return updated;
}

static void normAngle(Real &angle)
{
	if (angle < -10*PI) {
		angle = 0;
	}
	if (angle > 10*PI) {
		angle = 0;
	}
	while (angle < -PI) {
		angle += 2*PI;
	}
	while (angle > PI) {
		angle -= 2*PI;
	}
}

#define TERRAIN_SAMPLE_SIZE 40.0f
static Real getHeightAroundPos(Real x, Real y)
{
	// terrain height + desired height offset == cameraOffset * actual zoom
	Real terrainHeight = TheTerrainLogic->getGroundHeight(x, y);

	// find best approximation of max terrain height we can see
	Real terrainHeightMax = terrainHeight;
	terrainHeightMax = std::max(terrainHeightMax, TheTerrainLogic->getGroundHeight(x+TERRAIN_SAMPLE_SIZE, y-TERRAIN_SAMPLE_SIZE));
	terrainHeightMax = std::max(terrainHeightMax, TheTerrainLogic->getGroundHeight(x-TERRAIN_SAMPLE_SIZE, y-TERRAIN_SAMPLE_SIZE));
	terrainHeightMax = std::max(terrainHeightMax, TheTerrainLogic->getGroundHeight(x+TERRAIN_SAMPLE_SIZE, y+TERRAIN_SAMPLE_SIZE));
	terrainHeightMax = std::max(terrainHeightMax, TheTerrainLogic->getGroundHeight(x-TERRAIN_SAMPLE_SIZE, y+TERRAIN_SAMPLE_SIZE));

	return terrainHeightMax;
}



void ScriptedView::setAngle( Real angle )
{
  finite(angle);
	// Normalize to +-PI.
	normAngle(angle);
	// call our base class, we are adding functionality
	View::setAngle( angle );


	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;

	m_doingRotateCamera = false;
	m_doingPitchCamera = false;
	m_doingZoomCamera = false;
	m_doingScriptedCameraLock = false;
	// set the camera
	cameraStateChanged();
}

void ScriptedView::setPitch( Real angle )
{
  finite(angle);
	// call our base class, we are extending functionality
	View::setPitch( angle );


	m_doingMoveCameraOnWaypointPath = false;
	m_doingRotateCamera = false;
	m_doingPitchCamera = false;
	m_doingZoomCamera = false;
	m_doingScriptedCameraLock = false;
	// set the camera
	cameraStateChanged();
}

void ScriptedView::setZoom(Real z)
{
  finite(z);
	m_zoom = z;

	if (m_zoom < m_minZoom)
		m_zoom = m_minZoom;

	if (m_zoom > m_maxZoom)
		m_zoom = m_maxZoom;

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
	m_doingRotateCamera = false;
	m_doingPitchCamera = false;
	m_doingZoomCamera = false;
	m_doingScriptedCameraLock = false;
	m_cameraConstraintValid = false; // recalc it.
	cameraStateChanged();
}

void ScriptedView::setHeightAboveGround(Real z)
{
  finite(z);
	m_heightAboveGround = z;

  // if our zoom is limited, we will stay within a predefined distance from the terrain
	if( m_zoomLimited )
	{

		if (m_heightAboveGround < m_minHeightAboveGround)
			m_heightAboveGround = m_minHeightAboveGround;

		if (m_heightAboveGround > m_maxHeightAboveGround)
			m_heightAboveGround = m_maxHeightAboveGround;

	}  // end if

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
	m_doingRotateCamera = false;
	m_doingPitchCamera = false;
	m_doingZoomCamera = false;
	m_doingScriptedCameraLock = false;
	m_cameraConstraintValid = false; // recalc it.
	cameraStateChanged();
}

void ScriptedView::setZoomToDefault( void )
{
  parents();
	// default zoom has to be max, otherwise players will just zoom to max always

	// terrain height + desired height offset == cameraOffset * actual zoom
	// find best approximation of max terrain height we can see
	Real terrainHeightMax = getHeightAroundPos(m_pos.x, m_pos.y);

	Real desiredHeight = (terrainHeightMax + m_maxHeightAboveGround);
	Real desiredZoom = desiredHeight / m_cameraOffset.z;

	//DEBUG_LOG(("ScriptedView::setZoomToDefault() Current zoom: %g  Desired zoom: %g\n", m_zoom, desiredZoom));

	m_zoom = desiredZoom;
	m_heightAboveGround = m_maxHeightAboveGround;

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
	m_doingRotateCamera = false;
	m_doingPitchCamera = false;
	m_doingZoomCamera = false;
	m_doingScriptedCameraLock = false;
	m_cameraConstraintValid = false; // recalc it.
	cameraStateChanged();
}

void ScriptedView::setAngleAndPitchToDefault( void )
{ 
	// call our base class, we are adding functionality
	View::setAngleAndPitchToDefault();

	this->m_FXPitch = 1.0;

	// set the camera
	cameraStateChanged();
}

void ScriptedView::setDefaultView(Real pitch, Real angle, Real maxHeight)
{
  parents();finite(pitch);finite(angle);finite(maxHeight);finite(TheGlobalData->m_maxCameraHeight*maxHeight);
	// MDC - we no longer want to rotate maps (design made all of them right to begin with)
	//	m_defaultAngle = angle * M_PI/180.0f;
	m_defaultPitchAngle = pitch;
	m_maxHeightAboveGround = TheGlobalData->m_maxCameraHeight*maxHeight;
	if (m_minHeightAboveGround > m_maxHeightAboveGround)
		m_maxHeightAboveGround = m_minHeightAboveGround;
}

void ScriptedView::initHeightForMap( void ) 
{
  parents();
	m_groundLevel = TheTerrainLogic->getGroundHeight(m_pos.x, m_pos.y);
	const Real MAX_GROUND_LEVEL = 120.0; // jba - starting ground level can't exceed this height.
	if (m_groundLevel>MAX_GROUND_LEVEL) {
		m_groundLevel = MAX_GROUND_LEVEL;
	}

	m_cameraOffset.z = m_groundLevel+TheGlobalData->m_cameraHeight;
	m_cameraOffset.y = -(m_cameraOffset.z / tan(TheGlobalData->m_cameraPitch * (PI / 180.0)));
	m_cameraOffset.x = -(m_cameraOffset.y * tan(TheGlobalData->m_cameraYaw * (PI / 180.0)));
	m_cameraConstraintValid = false;	// possible ground level change invalidates cam constraints
	cameraStateChanged();

}

void ScriptedView::moveCameraTo(const Coord3D *o, Int milliseconds, Int shutter, Bool orient, Real easeIn, Real easeOut)
{
  parents();point(o);timing(easeIn,easeOut);m_mcwpInfo=Path{};
	m_mcwpInfo.waypoints[0] = *getPosition();	
	m_mcwpInfo.cameraAngle[0] = getAngle();	
	m_mcwpInfo.waySegLength[0] = 0;	

	m_mcwpInfo.waypoints[1] = *getPosition();	
	m_mcwpInfo.waySegLength[1] = 0;	

	m_mcwpInfo.waypoints[2] = *o;	
	m_mcwpInfo.waySegLength[2] = 0;	

	m_mcwpInfo.numWaypoints = 2;
	if (milliseconds<1) milliseconds = 1;
	m_mcwpInfo.totalTimeMilliseconds = milliseconds;
	m_mcwpInfo.shutter = 1;
	m_mcwpInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);
	m_mcwpInfo.curSegment = 1;
	m_mcwpInfo.curSegDistance = 0;
	m_mcwpInfo.totalDistance = 0;

	setupWaypointPath(orient);
	if (m_mcwpInfo.totalTimeMilliseconds==1) {
		// do it instantly.
		moveAlongWaypointPath(1);
		m_doingMoveCameraOnWaypointPath = true;
		m_CameraArrivedAtWaypointOnPathFlag = false;
	}
}

//-------------------------------------------------------------------------------------------------
/** Rotate the camera */
//-------------------------------------------------------------------------------------------------
void ScriptedView::rotateCamera(Real rotations, Int milliseconds, Real easeIn, Real easeOut)
{
  finite(rotations);finite(m_angle+2*PI*rotations);timing(easeIn,easeOut);
	m_rcInfo.numHoldFrames = 0;
	m_rcInfo.trackObject = FALSE;

	if (milliseconds<1) milliseconds = 1;
	m_rcInfo.numFrames = milliseconds/m_frameLengthMilliseconds;
	if (m_rcInfo.numFrames < 1) {
		m_rcInfo.numFrames = 1;
	}
	m_rcInfo.curFrame = 0;
	m_doingRotateCamera = true;
	m_rcInfo.angle.startAngle = m_angle;
	m_rcInfo.angle.endAngle = m_angle + 2*PI*rotations;
	m_rcInfo.startTimeMultiplier = m_timeMultiplier;
	m_rcInfo.endTimeMultiplier = m_timeMultiplier;
	m_rcInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
}

//-------------------------------------------------------------------------------------------------
/** Rotate the camera to follow a unit */
//-------------------------------------------------------------------------------------------------
void ScriptedView::rotateCameraTowardObject(ObjectID id, Int milliseconds, Int holdMilliseconds, Real easeIn, Real easeOut)
{
  timing(easeIn,easeOut);if(!TheGameLogic)throw ERROR_BAD_ARG;const Object* initial=TheGameLogic->findObjectByID(id);if(!initial)throw ERROR_BAD_ARG;point(initial->getPosition());m_rcInfo.target.targetObjectPos=*initial->getPosition();
	m_rcInfo.trackObject = TRUE;
	if (holdMilliseconds<1) holdMilliseconds = 0;
	m_rcInfo.numHoldFrames = holdMilliseconds/m_frameLengthMilliseconds;
	if (m_rcInfo.numHoldFrames < 1) {
		m_rcInfo.numHoldFrames = 0;
	}

	if (milliseconds<1) milliseconds = 1;
	m_rcInfo.numFrames = milliseconds/m_frameLengthMilliseconds;
	if (m_rcInfo.numFrames < 1) {
		m_rcInfo.numFrames = 1;
	}
	m_rcInfo.curFrame = 0;
	m_doingRotateCamera = true;
	m_rcInfo.target.targetObjectID = id;
	m_rcInfo.startTimeMultiplier = m_timeMultiplier;
	m_rcInfo.endTimeMultiplier = m_timeMultiplier;
	m_rcInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
}

//-------------------------------------------------------------------------------------------------
/** Rotate camera to face a location */
//-------------------------------------------------------------------------------------------------
void ScriptedView::rotateCameraTowardPosition(const Coord3D *pLoc, Int milliseconds, Real easeIn, Real easeOut, Bool reverseRotation)
{
  point(pLoc);timing(easeIn,easeOut);
	Coord3D curPos = *getPosition();
	Vector2 dir(pLoc->x-curPos.x, pLoc->y-curPos.y);
	const Real dirLength = dir.Length();
	if (dirLength<0.1f) return;
	m_rcInfo.numHoldFrames = 0;
	m_rcInfo.trackObject = FALSE;

	if (milliseconds<1) milliseconds = 1;
	m_rcInfo.numFrames = milliseconds/m_frameLengthMilliseconds;
	if (m_rcInfo.numFrames < 1) {
		m_rcInfo.numFrames = 1;
	}
	Real angle = WWMath::Acos(dir.X/dirLength);
	if (dir.Y<0.0f) {
		angle = -angle;
	}
	// Default camera is rotated 90 degrees, so match.
	angle -= PI/2;
	normAngle(angle);

	if (reverseRotation) {
		if (m_angle < angle) {
			angle -= 2.0f*WWMATH_PI;
		} else {
			angle += 2.0f*WWMATH_PI;
		}
	}

	m_rcInfo.curFrame = 0;
	m_doingRotateCamera = true;
	m_rcInfo.angle.startAngle = m_angle;
	m_rcInfo.angle.endAngle = angle;
	m_rcInfo.startTimeMultiplier = m_timeMultiplier;
	m_rcInfo.endTimeMultiplier = m_timeMultiplier;
	m_rcInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);

	m_doingMoveCameraOnWaypointPath = false;
	m_CameraArrivedAtWaypointOnPathFlag = false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ScriptedView::zoomCamera( Real finalZoom, Int milliseconds, Real easeIn, Real easeOut )
{
  finite(finalZoom);timing(easeIn,easeOut);
	if (milliseconds<1) milliseconds = 1;
	m_zcInfo.numFrames = milliseconds/m_frameLengthMilliseconds;
	if (m_zcInfo.numFrames < 1) {
		m_zcInfo.numFrames = 1;
	}
	m_zcInfo.curFrame = 0;
	m_doingZoomCamera = TRUE;
	m_zcInfo.startZoom = m_zoom;
	m_zcInfo.endZoom = finalZoom;
	m_zcInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ScriptedView::pitchCamera( Real finalPitch, Int milliseconds, Real easeIn, Real easeOut )
{
  finite(finalPitch);timing(easeIn,easeOut);
	if (milliseconds<1) milliseconds = 1;
	m_pcInfo.numFrames = milliseconds/m_frameLengthMilliseconds;
	if (m_pcInfo.numFrames < 1) {
		m_pcInfo.numFrames = 1;
	}
	m_pcInfo.curFrame = 0;
	m_doingPitchCamera = TRUE;
	m_pcInfo.startPitch = m_FXPitch;
	m_pcInfo.endPitch = finalPitch;
	m_pcInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);
}

//-------------------------------------------------------------------------------------------------
/** Sets the final zoom for a camera movement. */
//-------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFinalZoom( Real finalZoom, Real easeIn, Real easeOut ) 
{
  parents();finite(finalZoom);timing(easeIn,easeOut);

	if (m_doingRotateCamera) 
	{
		Real terrainHeightMax = getHeightAroundPos(m_pos.x, m_pos.y);
		Real maxHeight = (terrainHeightMax + m_maxHeightAboveGround);
		Real maxZoom = maxHeight / m_cameraOffset.z;

		Int time = duration((std::int64_t(m_rcInfo.numFrames) + m_rcInfo.numHoldFrames - m_rcInfo.curFrame)*m_frameLengthMilliseconds);
		zoomCamera( finalZoom*maxZoom, time, time*easeIn, time*easeOut );
	}
	if (m_doingMoveCameraOnWaypointPath) 
	{
		Coord3D pos = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
		Real terrainHeightMax = getHeightAroundPos(pos.x, pos.y);
		Real maxHeight = (terrainHeightMax + m_maxHeightAboveGround);
		Real maxZoom = maxHeight / m_cameraOffset.z;

		Int time = duration(m_mcwpInfo.totalTimeMilliseconds - m_mcwpInfo.elapsedTimeMilliseconds);
		zoomCamera( finalZoom*maxZoom, time, time*easeIn, time*easeOut );
	}
}

//-------------------------------------------------------------------------------------------------
/** Sets the final zoom for a camera movement. */
//-------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFreezeAngle(void) 
{
	if (m_doingRotateCamera) {
		if (m_rcInfo.trackObject) {
			m_rcInfo.target.targetObjectID = INVALID_ID;
		} else {
			m_rcInfo.angle.startAngle = m_rcInfo.angle.endAngle = m_angle; // Silly, but consistent.
		}
	}
	if (m_doingMoveCameraOnWaypointPath) {
		Int i;
//		Real curDistance = 0;
		for (i=0; i<m_mcwpInfo.numWaypoints; i++) {
			m_mcwpInfo.cameraAngle[i+1] = m_mcwpInfo.cameraAngle[0];
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Sets the look toward point for a camera movement. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModLookToward(Coord3D *pLoc) 
{
  point(pLoc);
	if (m_doingRotateCamera) {
		return; // Doesn't apply to rotate about a point.
	}
	if (m_doingMoveCameraOnWaypointPath) {
		Int i;
//		Real curDistance = 0;
		for (i=2; i<=m_mcwpInfo.numWaypoints; i++) {
			Coord3D start, mid, end;
			Real factor = 0.5;
			start = m_mcwpInfo.waypoints[i-1];
			start.x += m_mcwpInfo.waypoints[i].x;
			start.y += m_mcwpInfo.waypoints[i].y;
			start.x /= 2;
			start.y /= 2;
			mid = m_mcwpInfo.waypoints[i];
			end = m_mcwpInfo.waypoints[i];
			end.x += m_mcwpInfo.waypoints[i+1].x;
			end.y += m_mcwpInfo.waypoints[i+1].y;
			end.x /= 2;
			end.y /= 2;
			Coord3D result = start;
			result.x += factor*(end.x-start.x);
			result.y += factor*(end.y-start.y);
			result.x += (1-factor)*factor*(mid.x-end.x + mid.x-start.x);
			result.y += (1-factor)*factor*(mid.y-end.y + mid.y-start.y);
			result.z = 0;
			Vector2 dir(pLoc->x-result.x, pLoc->y-result.y);
			const Real dirLength = dir.Length();
			if (dirLength<0.1f) continue;
			Real angle = WWMath::Acos(dir.X/dirLength);
			if (dir.Y<0.0f) {
				angle = -angle;
			}
			// Default camera is rotated 90 degrees, so match.
			angle -= PI/2;
			normAngle(angle);
			m_mcwpInfo.cameraAngle[i] = angle;
		}
		if (m_mcwpInfo.totalTimeMilliseconds==1) {
			// do it instantly.
			moveAlongWaypointPath(1);
			m_doingMoveCameraOnWaypointPath = true;
			m_CameraArrivedAtWaypointOnPathFlag = false;
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Sets the look toward point for the end of a camera movement. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFinalMoveTo(Coord3D *pLoc) 
{
  point(pLoc);
	if (m_doingRotateCamera) {
		return; // Doesn't apply to rotate about a point.
	}
	if (m_doingMoveCameraOnWaypointPath) {
		Int i;
		Coord3D start, delta;
		start = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
		delta.x = pLoc->x - start.x;
		delta.y = pLoc->y - start.y;
		for (i=2; i<=m_mcwpInfo.numWaypoints; i++) {
			Coord3D result = m_mcwpInfo.waypoints[i];
			result.x += delta.x;
			result.y += delta.y;
			m_mcwpInfo.waypoints[i] = result;
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Sets the look toward point for the end of a camera movement. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFinalLookToward(Coord3D *pLoc) 
{
  point(pLoc);
	if (m_doingRotateCamera) {
		return; // Doesn't apply to rotate about a point.
	}
	if (m_doingMoveCameraOnWaypointPath) {
		Int i;
		Int min = m_mcwpInfo.numWaypoints-1;
		if (min<2) min=2;
//		Real curDistance = 0;
		for (i=min; i<=m_mcwpInfo.numWaypoints; i++) {
			Coord3D start, mid, end;
			Real factor = 0.5;
			start = m_mcwpInfo.waypoints[i-1];
			start.x += m_mcwpInfo.waypoints[i].x;
			start.y += m_mcwpInfo.waypoints[i].y;
			start.x /= 2;
			start.y /= 2;
			mid = m_mcwpInfo.waypoints[i];
			end = m_mcwpInfo.waypoints[i];
			end.x += m_mcwpInfo.waypoints[i+1].x;
			end.y += m_mcwpInfo.waypoints[i+1].y;
			end.x /= 2;
			end.y /= 2;
			Coord3D result = start;
			result.x += factor*(end.x-start.x);
			result.y += factor*(end.y-start.y);
			result.x += (1-factor)*factor*(mid.x-end.x + mid.x-start.x);
			result.y += (1-factor)*factor*(mid.y-end.y + mid.y-start.y);
			result.z = 0;
			Vector2 dir(pLoc->x-result.x, pLoc->y-result.y);
			const Real dirLength = dir.Length();
			if (dirLength<0.1f) continue;
			Real angle = WWMath::Acos(dir.X/dirLength);
			if (dir.Y<0.0f) {
				angle = -angle;
			}
			// Default camera is rotated 90 degrees, so match.
			angle -= PI/2;
			normAngle(angle);
			if (i==m_mcwpInfo.numWaypoints) { 
				m_mcwpInfo.cameraAngle[i] = angle;
			} else {
				Real deltaAngle = angle - m_mcwpInfo.cameraAngle[i];
				normAngle(deltaAngle);
				angle = m_mcwpInfo.cameraAngle[i] + deltaAngle/2;
				normAngle(angle);
				m_mcwpInfo.cameraAngle[i] = angle;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Sets the final time multiplier for a camera movement. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFinalTimeMultiplier(Int finalMultiplier) 
{
	if (m_doingZoomCamera)
		m_zcInfo.endTimeMultiplier = finalMultiplier;
	if (m_doingPitchCamera)
		m_pcInfo.endTimeMultiplier = finalMultiplier;
	if (m_doingRotateCamera) {
		m_rcInfo.endTimeMultiplier = finalMultiplier;
	} else if (m_doingMoveCameraOnWaypointPath) {
		Int i;
		Real curDistance = 0;
		for (i=0; i<m_mcwpInfo.numWaypoints; i++) {
			curDistance += m_mcwpInfo.waySegLength[i];
			Real factor2 = curDistance / m_mcwpInfo.totalDistance;
			Real factor1 = 1.0-factor2;
			m_mcwpInfo.timeMultiplier[i+1] = roundedMultiplier(0.5+m_mcwpInfo.timeMultiplier[i+1]*factor1 + finalMultiplier*factor2);
		}
	} else {
		// If we aren't doing a camera movement, just set the time.
		m_timeMultiplier = finalMultiplier;
	}
}

// ------------------------------------------------------------------------------------------------
/** Sets the number of frames to average motion for a camera movement */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModRollingAverage(Int framesToAverage) 
{
	if (framesToAverage < 1) framesToAverage = 1;
	m_mcwpInfo.rollingAverageFrames = framesToAverage;
}
 
// ------------------------------------------------------------------------------------------------
/** Sets the final pitch for a camera movement. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::cameraModFinalPitch(Real finalPitch, Real easeIn, Real easeOut) {
  finite(finalPitch);timing(easeIn,easeOut);
	if (m_doingRotateCamera) {
		Int time = duration((std::int64_t(m_rcInfo.numFrames) + m_rcInfo.numHoldFrames - m_rcInfo.curFrame)*m_frameLengthMilliseconds);
		pitchCamera( finalPitch, time, time*easeIn, time*easeOut );
	}
	if (m_doingMoveCameraOnWaypointPath) {
		Int time = duration(m_mcwpInfo.totalTimeMilliseconds - m_mcwpInfo.elapsedTimeMilliseconds);
		pitchCamera( finalPitch, time, time*easeIn, time*easeOut );
	}
}

// ------------------------------------------------------------------------------------------------
/** Move camera to a waypoint, resetting the default angle, pitch & zoom along the way.. */
// ------------------------------------------------------------------------------------------------
void ScriptedView::resetCamera(const Coord3D *location, Int milliseconds, Real easeIn, Real easeOut)
{
  parents();point(location);timing(easeIn,easeOut);
	moveCameraTo(location, milliseconds, 0, false, easeIn, easeOut);
	m_mcwpInfo.cameraAngle[2] = 0.0; // default angle.
	// m_mcwpInfo.cameraAngle[2] = m_defaultAngle;
	m_angle = m_mcwpInfo.cameraAngle[0];

	// terrain height + desired height offset == cameraOffset * actual zoom
	// find best approximation of max terrain height we can see
	//Real terrainHeightMax = getHeightAroundPos(m_pos.x, m_pos.y);
	Real terrainHeightMax = getHeightAroundPos(location->x, location->y);
	Real desiredHeight = (terrainHeightMax + m_maxHeightAboveGround);
	Real desiredZoom = desiredHeight / m_cameraOffset.z;

	zoomCamera( desiredZoom, milliseconds, easeIn, easeOut );	// this isn't right... or is it?

	pitchCamera( 1.0, milliseconds, easeIn, easeOut );
	// pitchCamera( m_defaultPitchAngle, milliseconds, easeIn, easeOut );
	//DEBUG_LOG(("ScriptedView::resetCamera() Current zoom: %g  Desired zoom: %g  Current pitch: %g  Desired pitch: %g\n",
	//	m_zoom, desiredZoom, m_pitchAngle, m_defaultPitchAngle));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool ScriptedView::isCameraMovementFinished(void)
{
	if (m_viewFilter == FT_VIEW_MOTION_BLUR_FILTER) {
		// Several of the motion blur effects are similar to camera movements.
		if (m_viewFilterMode == FM_VIEW_MB_IN_AND_OUT_ALPHA ||
				m_viewFilterMode == FM_VIEW_MB_IN_AND_OUT_SATURATE ||
				m_viewFilterMode == FM_VIEW_MB_IN_ALPHA ||
				m_viewFilterMode == FM_VIEW_MB_OUT_ALPHA ||
				m_viewFilterMode == FM_VIEW_MB_IN_SATURATE ||
				m_viewFilterMode == FM_VIEW_MB_OUT_SATURATE ) {
			return true;
		}
	}
	return !m_doingMoveCameraOnWaypointPath && !m_doingRotateCamera && !m_doingPitchCamera && !m_doingZoomCamera;
}


Bool ScriptedView::isCameraMovementAtWaypointAlongPath(void)
{
	// WWDEBUG_SAY((( "MBL: Polling ScriptedView::isCameraMovementAtWaypointAlongPath\n" )));
	
	Bool return_value = m_CameraArrivedAtWaypointOnPathFlag;
	m_CameraArrivedAtWaypointOnPathFlag = false;
	return( return_value );
}

// ------------------------------------------------------------------------------------------------
/** Move camera along a waypoint path in an interesting fashion.  Sets up parameters that get
 * evaluated in draw(). */
 // ------------------------------------------------------------------------------------------------
void ScriptedView::moveCameraAlongWaypointPath(Waypoint *pWay, Int milliseconds, Int shutter, Bool orient, Real easeIn, Real easeOut)
{
  parents();timing(easeIn,easeOut);validatePath(pWay,m_pos);m_mcwpInfo=Path{};
	const Real MIN_DELTA = MAP_XY_FACTOR;

	m_mcwpInfo.waypoints[0] = *getPosition();	
	m_mcwpInfo.cameraAngle[0] = getAngle();	
	m_mcwpInfo.waySegLength[0] = 0;	
	m_mcwpInfo.waypoints[1] = *getPosition();	
	m_mcwpInfo.numWaypoints = 1;
	if (milliseconds<1) milliseconds = 1;
	m_mcwpInfo.totalTimeMilliseconds = milliseconds;
	m_mcwpInfo.shutter = shutter/m_frameLengthMilliseconds;
	if (m_mcwpInfo.shutter<1) m_mcwpInfo.shutter = 1;
	m_mcwpInfo.ease.setEaseTimes(easeIn/milliseconds, easeOut/milliseconds);

	while (pWay && m_mcwpInfo.numWaypoints <MAX_WAYPOINTS) {
		m_mcwpInfo.numWaypoints++;
		m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints] = *pWay->getLocation();
		if (pWay->getNumLinks()>0) {
			pWay = pWay->getLink(0);
		} else {
			pWay = NULL;
		}	
		Vector2 dir(m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints].x-m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints-1].x, m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints].y-m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints-1].y);
		if (dir.Length()<MIN_DELTA) {
			if (pWay) {
				m_mcwpInfo.numWaypoints--; // drop this one.
			} else {
				m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints-1] = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
				m_mcwpInfo.numWaypoints--; // Push this one back.
			}
		}
	}
	setupWaypointPath(orient);
}

// ------------------------------------------------------------------------------------------------
/** Calculates angles and distances for moving along a waypoint path.  Sets up parameters that get
 * evaluated in draw(). */
// ------------------------------------------------------------------------------------------------
void ScriptedView::setupWaypointPath(Bool orient)
{
	m_mcwpInfo.curSegment = 1;
	m_mcwpInfo.curSegDistance = 0;
	m_mcwpInfo.totalDistance = 0;
	m_mcwpInfo.rollingAverageFrames = 1;
	Int i;
	Real angle = getAngle();
	for (i=1; i<m_mcwpInfo.numWaypoints; i++) {
		Vector2 dir(m_mcwpInfo.waypoints[i+1].x-m_mcwpInfo.waypoints[i].x, m_mcwpInfo.waypoints[i+1].y-m_mcwpInfo.waypoints[i].y);
		m_mcwpInfo.waySegLength[i] = dir.Length();
		m_mcwpInfo.totalDistance += m_mcwpInfo.waySegLength[i];
		if (orient && m_mcwpInfo.waySegLength[i]>0) {
			angle = WWMath::Acos(dir.X/m_mcwpInfo.waySegLength[i]);
			if (dir.Y<0.0f) {
				angle = -angle;
			}

			// Default camera is rotated 90 degrees, so match.
			angle -= PI/2;
			normAngle(angle);
		}
		//DEBUG_LOG(("Original Index %d, angle %.2f\n", i, angle*180/PI));
		m_mcwpInfo.cameraAngle[i] = angle;
	}
	m_mcwpInfo.cameraAngle[1] = getAngle();
  if(m_mcwpInfo.numWaypoints==1){m_mcwpInfo.waypoints[2]=m_mcwpInfo.waypoints[1];m_mcwpInfo.cameraAngle[0]=getAngle();}	
	m_mcwpInfo.cameraAngle[m_mcwpInfo.numWaypoints] = m_mcwpInfo.cameraAngle[m_mcwpInfo.numWaypoints-1];	
	for (i=m_mcwpInfo.numWaypoints-1; i>1; i--) {
		m_mcwpInfo.cameraAngle[i] = (m_mcwpInfo.cameraAngle[i] + m_mcwpInfo.cameraAngle[i-1]) / 2;  
	}
	m_mcwpInfo.waySegLength[m_mcwpInfo.numWaypoints+1] = m_mcwpInfo.waySegLength[m_mcwpInfo.numWaypoints];	

	// Prevent a possible divide by zero.
	if (m_mcwpInfo.totalDistance<1.0) {
		m_mcwpInfo.waySegLength[m_mcwpInfo.numWaypoints-1] += 1.0-m_mcwpInfo.totalDistance;
		m_mcwpInfo.totalDistance = 1.0;
	}

	Real curDistance = 0;
	Coord3D finalPos = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
	Real newGround = TheTerrainLogic->getGroundHeight(finalPos.x, finalPos.y);
	for (i=0; i<=m_mcwpInfo.numWaypoints+1; i++) {
		Real factor2 = curDistance / m_mcwpInfo.totalDistance;
		Real factor1 = 1.0-factor2;
		m_mcwpInfo.timeMultiplier[i] = m_timeMultiplier;
		m_mcwpInfo.groundHeight[i] = m_groundLevel*factor1 + newGround*factor2;
		curDistance += m_mcwpInfo.waySegLength[i];
		//DEBUG_LOG(("New Index %d, angle %.2f\n", i, m_mcwpInfo.cameraAngle[i]*180/PI));
	}

	// Pad the end.
	m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints+1] = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
	Coord3D cur = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
	Coord3D prev = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints-1];
	m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints+1].x += cur.x-prev.x;
	m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints+1].y += cur.y-prev.y;
	m_mcwpInfo.cameraAngle[m_mcwpInfo.numWaypoints+1] = m_mcwpInfo.cameraAngle[m_mcwpInfo.numWaypoints];	
	m_mcwpInfo.groundHeight[m_mcwpInfo.numWaypoints+1] = newGround;	


	cur = m_mcwpInfo.waypoints[2];
	prev = m_mcwpInfo.waypoints[1];
	m_mcwpInfo.waypoints[0].x -= cur.x-prev.x;
	m_mcwpInfo.waypoints[0].y -= cur.y-prev.y;

	m_doingMoveCameraOnWaypointPath = m_mcwpInfo.numWaypoints>1;
	m_CameraArrivedAtWaypointOnPathFlag = false;
	m_doingRotateCamera = false;

	m_mcwpInfo.elapsedTimeMilliseconds = 0;
	m_mcwpInfo.curShutter = m_mcwpInfo.shutter;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static Real makeQuadraticS(Real t) 
{
	// for t = linear 0-1, convert to quadratic s where 0==0, 0.5==0.5 && 1.0 == 1.0.
	Real tPrime = t;
	if (t<0.5) {
		tPrime = 0.5 * (2*t*2*t);
	} else {
		tPrime = (t-0.5)*2;
		tPrime = WWMath::Sqrt(tPrime);
		tPrime = 0.5 + 0.5*(tPrime);
	}
	return tPrime*0.5 + t*0.5;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ScriptedView::rotateCameraOneFrame(void)
{
	m_rcInfo.curFrame++;
	if (TheGlobalData->m_disableCameraMovement) {
		if (m_rcInfo.curFrame >= std::int64_t(m_rcInfo.numFrames) + m_rcInfo.numHoldFrames) {
			m_doingRotateCamera = false;
			m_freezeTimeForCameraMovement = false;
		}
		return;
	}

	if (m_rcInfo.trackObject)
	{
		if (m_rcInfo.curFrame <= std::int64_t(m_rcInfo.numFrames) + m_rcInfo.numHoldFrames)
		{
			const Object *obj = TheGameLogic->findObjectByID(m_rcInfo.target.targetObjectID);
			if (obj)
			{
				// object has not been destroyed
				m_rcInfo.target.targetObjectPos = *obj->getPosition();
			}

			const Vector2 dir(m_rcInfo.target.targetObjectPos.x - m_pos.x, m_rcInfo.target.targetObjectPos.y - m_pos.y);
			const Real dirLength = dir.Length();
			if (dirLength>=0.1f)
			{
				Real angle = WWMath::Acos(dir.X/dirLength);
				if (dir.Y<0.0f) {
					angle = -angle;
				}
				// Default camera is rotated 90 degrees, so match.
				angle -= PI/2;
				normAngle(angle);

				if (m_rcInfo.curFrame <= m_rcInfo.numFrames)
				{
					Real factor = m_rcInfo.ease(((Real)m_rcInfo.curFrame)/m_rcInfo.numFrames);
					Real angleDiff = angle - m_angle;
					normAngle(angleDiff);
					angleDiff *= factor;
					m_angle += angleDiff;
					normAngle(m_angle);
					m_timeMultiplier = interpolateMultiplier(m_rcInfo.startTimeMultiplier,m_rcInfo.endTimeMultiplier,factor);
				}
				else
				{
					m_angle = angle;
				}
			}
		}
	}
	else if (m_rcInfo.curFrame <= m_rcInfo.numFrames)
	{
		Real factor = m_rcInfo.ease(((Real)m_rcInfo.curFrame)/m_rcInfo.numFrames);
		m_angle = WWMath::Lerp(m_rcInfo.angle.startAngle, m_rcInfo.angle.endAngle, factor);
		normAngle(m_angle);
		m_timeMultiplier = interpolateMultiplier(m_rcInfo.startTimeMultiplier,m_rcInfo.endTimeMultiplier,factor);
	}


	if (m_rcInfo.curFrame >= std::int64_t(m_rcInfo.numFrames) + m_rcInfo.numHoldFrames) {
		m_doingRotateCamera = false;
		m_freezeTimeForCameraMovement = false;
		if (! m_rcInfo.trackObject)
		{
			m_angle = m_rcInfo.angle.endAngle;
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ScriptedView::zoomCameraOneFrame(void)
{
	m_zcInfo.curFrame++;
	if (TheGlobalData->m_disableCameraMovement) {
		if (m_zcInfo.curFrame >= m_zcInfo.numFrames) {
			m_doingZoomCamera = false;
		}
		return;
	}
	if (m_zcInfo.curFrame <= m_zcInfo.numFrames)
	{
		// not just holding; do the camera adjustment
		Real factor = m_zcInfo.ease(((Real)m_zcInfo.curFrame)/m_zcInfo.numFrames);
		m_zoom = WWMath::Lerp(m_zcInfo.startZoom, m_zcInfo.endZoom, factor);
	}

	if (m_zcInfo.curFrame >= m_zcInfo.numFrames) {
		m_doingZoomCamera = false;
		m_zoom = m_zcInfo.endZoom;
	}

	//DEBUG_LOG(("ScriptedView::zoomCameraOneFrame() - m_zoom = %g\n", m_zoom));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ScriptedView::pitchCameraOneFrame(void)
{
	m_pcInfo.curFrame++;
	if (TheGlobalData->m_disableCameraMovement) {
		if (m_pcInfo.curFrame >= m_pcInfo.numFrames) {
			m_doingPitchCamera = false;
		}
		return;
	}
	if (m_pcInfo.curFrame <= m_pcInfo.numFrames)
	{
		// not just holding; do the camera adjustment
		Real factor = m_pcInfo.ease(((Real)m_pcInfo.curFrame)/m_pcInfo.numFrames);
		m_FXPitch = WWMath::Lerp(m_pcInfo.startPitch, m_pcInfo.endPitch, factor);
	}

	if (m_pcInfo.curFrame >= m_pcInfo.numFrames) {
		m_doingPitchCamera = false;
		m_FXPitch = m_pcInfo.endPitch;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ScriptedView::moveAlongWaypointPath(Int milliseconds)
{
	m_mcwpInfo.elapsedTimeMilliseconds += milliseconds;
	if (TheGlobalData->m_disableCameraMovement) {
		if (m_mcwpInfo.elapsedTimeMilliseconds>m_mcwpInfo.totalTimeMilliseconds) {
			m_doingMoveCameraOnWaypointPath = false;
			m_freezeTimeForCameraMovement = false;
		}
		return;
	}
	if (m_mcwpInfo.elapsedTimeMilliseconds>m_mcwpInfo.totalTimeMilliseconds) {
		m_doingMoveCameraOnWaypointPath = false;
		m_CameraArrivedAtWaypointOnPathFlag = false;

		m_freezeTimeForCameraMovement = false;
		m_angle = m_mcwpInfo.cameraAngle[m_mcwpInfo.numWaypoints];

		m_groundLevel = m_mcwpInfo.groundHeight[m_mcwpInfo.numWaypoints];
		/////////////////////m_cameraOffset.z = m_groundLevel+TheGlobalData->m_cameraHeight;
		m_cameraOffset.y = -(m_cameraOffset.z / tan(TheGlobalData->m_cameraPitch * (PI / 180.0)));
		m_cameraOffset.x = -(m_cameraOffset.y * tan(TheGlobalData->m_cameraYaw * (PI / 180.0)));

		Coord3D pos = m_mcwpInfo.waypoints[m_mcwpInfo.numWaypoints];
		pos.z = 0;
		setPosition(&pos);
		// Note - assuming that the scripter knows what he is doing, we adjust the constraints so that
		// the scripted action can occur.
		m_cameraConstraint.lo.x = std::min(m_cameraConstraint.lo.x, pos.x);
		m_cameraConstraint.hi.x = std::max(m_cameraConstraint.hi.x, pos.x);
		m_cameraConstraint.lo.y = std::min(m_cameraConstraint.lo.y, pos.y);
		m_cameraConstraint.hi.y = std::max(m_cameraConstraint.hi.y, pos.y);
		return;
	}

	const Real totalTime = m_mcwpInfo.totalTimeMilliseconds;
	const Real deltaTime = m_mcwpInfo.ease(m_mcwpInfo.elapsedTimeMilliseconds/totalTime) -
		m_mcwpInfo.ease((m_mcwpInfo.elapsedTimeMilliseconds - milliseconds)/totalTime);
	m_mcwpInfo.curSegDistance += deltaTime*m_mcwpInfo.totalDistance;
	while (m_mcwpInfo.curSegDistance >= m_mcwpInfo.waySegLength[m_mcwpInfo.curSegment]) {

		if ( m_doingMoveCameraOnWaypointPath )
		{
			//WWDEBUG_SAY(( "MBL TEST: Camera waypoint along path reached!\n" ));
			m_CameraArrivedAtWaypointOnPathFlag = true;
		}

		m_mcwpInfo.curSegDistance -= m_mcwpInfo.waySegLength[m_mcwpInfo.curSegment];
		m_mcwpInfo.curSegment++;
		if (m_mcwpInfo.curSegment >= m_mcwpInfo.numWaypoints) { 
			m_mcwpInfo.totalTimeMilliseconds = 0; // Will end following next frame.
			return;
		}
	}
	Real avgFactor = 1.0/m_mcwpInfo.rollingAverageFrames;
	m_mcwpInfo.curShutter--;
	if (m_mcwpInfo.curShutter>0) {
		return;
	}
	m_mcwpInfo.curShutter = m_mcwpInfo.shutter;
	Real factor = m_mcwpInfo.curSegDistance / m_mcwpInfo.waySegLength[m_mcwpInfo.curSegment];
	if (m_mcwpInfo.curSegment == m_mcwpInfo.numWaypoints-1) {
		avgFactor = avgFactor + (1.0-avgFactor)*factor;
	}
	Real factor1;
	Real factor2;
	factor1 = 1.0-factor;
	//factor1 = makeQuadraticS(factor1);
	factor2 = 1.0-factor1;
	Real angle1 = m_mcwpInfo.cameraAngle[m_mcwpInfo.curSegment];
	Real angle2 = m_mcwpInfo.cameraAngle[m_mcwpInfo.curSegment+1];
	if (angle2-angle1 > PI) angle1 += 2*PI;
	if (angle2-angle1 < -PI) angle1 -= 2*PI;
	Real angle = angle1 * (factor1) + angle2 * (factor2); 

	normAngle(angle);
	Real deltaAngle = angle-m_angle;
	normAngle(deltaAngle);
	if (fabs(deltaAngle) > PI/10) {
		DEBUG_LOG(("Huh.\n"));
	}
	m_angle += avgFactor*(deltaAngle);
	normAngle(m_angle);

	Real timeMultiplier = m_mcwpInfo.timeMultiplier[m_mcwpInfo.curSegment]*factor1 + 
			m_mcwpInfo.timeMultiplier[m_mcwpInfo.curSegment+1]*factor2;
	m_timeMultiplier = roundedMultiplier(0.5 + timeMultiplier);

	m_groundLevel = m_mcwpInfo.groundHeight[m_mcwpInfo.curSegment]*factor1 + 
			m_mcwpInfo.groundHeight[m_mcwpInfo.curSegment+1]*factor2;
	//////////////m_cameraOffset.z = m_groundLevel+TheGlobalData->m_cameraHeight;
	m_cameraOffset.y = -(m_cameraOffset.z / tan(TheGlobalData->m_cameraPitch * (PI / 180.0)));
	m_cameraOffset.x = -(m_cameraOffset.y * tan(TheGlobalData->m_cameraYaw * (PI / 180.0)));

	Coord3D start, mid, end;
	if (factor<0.5) {
		start = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment-1];
		start.x += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment].x;
		start.y += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment].y;
		start.x /= 2;
		start.y /= 2;
		mid = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment];
		end = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment];
		end.x += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1].x;
		end.y += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1].y;
		end.x /= 2;
		end.y /= 2;
		factor += 0.5;
	} else {
		start = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment];
		start.x += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1].x;
		start.y += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1].y;
		start.x /= 2;
		start.y /= 2;
		mid = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1];
		end = m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+1];
		end.x += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+2].x;
		end.y += m_mcwpInfo.waypoints[m_mcwpInfo.curSegment+2].y;
		end.x /= 2;
		end.y /= 2;
		factor -= 0.5;
	}

	Coord3D result = start;
	result.x += factor*(end.x-start.x);
	result.y += factor*(end.y-start.y);
	result.x += (1-factor)*factor*(mid.x-end.x + mid.x-start.x);
	result.y += (1-factor)*factor*(mid.y-end.y + mid.y-start.y);
	result.z = 0;
/*
	static Real prevGround = 0;
	DEBUG_LOG(("Dx %.2f, dy %.2f, DeltaANgle = %.2f, %.2f DeltaGround %.2f\n", m_pos.x-result.x, m_pos.y-result.y, deltaAngle, m_groundLevel, m_groundLevel-prevGround));
	prevGround = m_groundLevel;
*/
	setPosition(&result);
	// Note - assuming that the scripter knows what he is doing, we adjust the constraints so that
	// the scripted action can occur.
	m_cameraConstraint.lo.x = std::min(m_cameraConstraint.lo.x, result.x);
	m_cameraConstraint.hi.x = std::max(m_cameraConstraint.hi.x, result.x);
	m_cameraConstraint.lo.y = std::min(m_cameraConstraint.lo.y, result.y);
	m_cameraConstraint.hi.y = std::max(m_cameraConstraint.hi.y, result.y);

}


// ------------------------------------------------------------------------------------------------
/** Add an impulse force to shake the camera.
 * The camera shake is a simple simulation of an oscillating spring/damper.
 * The idea is that some sort of shock has "pushed" the camera once, as an
 * impluse, after which the camera vibrates back to its rest position.
 * @todo This should be part of "View", not "W3DView". */
// ------------------------------------------------------------------------------------------------
