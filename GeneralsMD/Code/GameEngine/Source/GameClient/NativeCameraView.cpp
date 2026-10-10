// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeCameraView.h"
#include "GameClient/Display.h"
#include "GameClient/TerrainVisual.h"
#include "GameLogic/TerrainLogic.h"
#include "Common/GlobalData.h"
#include "WWMath/vector3.h"
#include <cmath>

void NativeCameraView::init(){ScriptedView::init();synchronizeCamera();}
NativeCamera::State NativeCameraView::cameraState()const{
  if(!TheGlobalData)throw ERROR_BAD_ARG;
  return {m_pos,m_cameraOffset,m_groundLevel,m_zoom,m_angle,m_pitchAngle,getFXPitch(),
    m_FOV,m_width,m_height,m_originX,m_originY,TheGlobalData->m_drawEntireTerrain};
}
void NativeCameraView::synchronizeCamera() const {
  if(!m_cameraDirty)return;
  m_spatialCamera.configure(cameraState());m_cameraDirty=FALSE;
}
void NativeCameraView::setWidth(Int width){
  if(width<1 || !TheDisplay || !TheDisplay->getWidth())throw ERROR_BAD_ARG;
  const Real fov=(Real(width)/TheDisplay->getWidth())*(50*PI/180);
  if(!std::isfinite(fov) || fov<=0 || fov>=PI)throw ERROR_BAD_ARG;
  View::setWidth(width);m_FOV=fov;cameraStateChanged();
}
void NativeCameraView::setHeight(Int height){if(height<1)throw ERROR_BAD_ARG;View::setHeight(height);cameraStateChanged();}
void NativeCameraView::setOrigin(Int x,Int y){View::setOrigin(x,y);cameraStateChanged();}
void NativeCameraView::setFieldOfView(Real fov){
  if(!std::isfinite(fov) || fov<=0 || fov>=PI)throw ERROR_BAD_ARG;
  View::setFieldOfView(fov);cameraStateChanged();
}
void NativeCameraView::forceRedraw(){cameraStateChanged();synchronizeCamera();}
const Coord3D& NativeCameraView::get3DCameraPosition()const{synchronizeCamera();return m_spatialCamera.eye();}
View::WorldToScreenReturn NativeCameraView::worldToScreenTriReturn(const Coord3D* world,ICoord2D* screen){
  if(!world || !screen)return WTS_INVALID;synchronizeCamera();return m_spatialCamera.project(world,screen);
}
void NativeCameraView::screenToWorld(const ICoord2D*,Coord3D*){throw ERROR_BAD_ARG;} // Original unimplemented, no callers.
void NativeCameraView::screenToWorldAtZ(const ICoord2D* screen,Coord3D* world,Real z){
  if(!screen || !world)throw ERROR_BAD_ARG;synchronizeCamera();const auto candidate=m_spatialCamera.atZ(*screen,z);*world=candidate;
}
void NativeCameraView::screenToTerrain(const ICoord2D* screen,Coord3D* world){
  if(!screen || !world)return;
  if(!TheTerrainVisual || !TheTerrainLogic)throw ERROR_BAD_ARG;
  synchronizeCamera();auto ray=m_spatialCamera.ray(*screen);Coord3D candidate{};
  TheTerrainVisual->intersectTerrain(&ray[0],&ray[1],&candidate);
  Vector3 bridge;
  if(TheTerrainLogic->pickBridge(Vector3(ray[0].x,ray[0].y,ray[0].z),Vector3(ray[1].x,ray[1].y,ray[1].z),&bridge)
    && bridge.Z>candidate.z)candidate={bridge.X,bridge.Y,bridge.Z};
  *world=candidate; // Source miss publishes zero; bridge remains a separate query.
}
Coord3D NativeCameraView::projectElevatedLookAt(const Coord3D& point){
  if(!TheTerrainVisual)throw ERROR_BAD_ARG;synchronizeCamera();
  const auto ray=m_spatialCamera.centerRay(); // Source logical (0,0), including odd viewport dimensions.
  Coord3D start=point,end{point.x+(ray[1].x-ray[0].x),point.y+(ray[1].y-ray[0].y),point.z+(ray[1].z-ray[0].z)},contact;
  if(!std::isfinite(end.x) || !std::isfinite(end.y) || !std::isfinite(end.z))throw ERROR_BAD_ARG;
  Coord3D result=point;
  if(TheTerrainVisual->intersectTerrain(&start,&end,&contact)){result.x=contact.x;result.y=contact.y;}
  return result;
}
void NativeCameraView::setGuardBandBias(const Coord2D* value){
  if(!value || !std::isfinite(value->x) || !std::isfinite(value->y))throw ERROR_BAD_ARG;m_guardBandBias=*value;
}
void NativeCameraView::scrollBy(Coord2D* delta){
  if(!delta)return;
  if(!std::isfinite(delta->x) || !std::isfinite(delta->y))throw ERROR_BAD_ARG;
  if(delta->x==0 && delta->y==0)return;
  synchronizeCamera();
  // Preserve the source integer division; do not silently change non-square
  // viewport motion while porting. Full frame settlement consumes this delta.
  const Coord2D pixels{delta->x*250.0f,delta->y*250.0f*(m_width/m_height)};
  const auto displacement=m_spatialCamera.viewPlaneDisplacement(pixels);
  Coord3D position=m_pos;position.x+=displacement.x;position.y+=displacement.y;
  const NativeCamera::State state{position,m_cameraOffset,m_groundLevel,m_zoom,m_angle,m_pitchAngle,getFXPitch(),
    m_FOV,m_width,m_height,m_originX,m_originY,TheGlobalData->m_drawEntireTerrain};
  NativeCamera candidate;candidate.configure(state);
  // All fallible calculations precede adoption and rotation cancellation.
  setPosition(&position);m_spatialCamera=candidate;m_cameraDirty=FALSE;
  m_scrollAmount=*delta;cancelCameraRotation();
}
