// SPDX-License-Identifier: GPL-3.0-or-later
// Original W3DView camera frame/rebuild phase, using game-owned CPU backing.
#include "PreRTS.h"
#include "GameClient/NativeFrameCameraView.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ClientRandomValue.h"
#include "Common/DrawModule.h"
#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/TerrainLogic.h"
#include "WWMath/matrix3d.h"
#include <algorithm>
#include <cmath>

namespace {
void finiteValue(Real value){if(!std::isfinite(value))throw ERROR_BAD_ARG;}
void point(const Coord3D& value){finiteValue(value.x);finiteValue(value.y);finiteValue(value.z);}
Coord3D coordinate(const Vector3& value){return {value.X,value.Y,value.Z};}
void normalizeCameraAngle(Real& angle){
  finiteValue(angle);if(angle<-10*PI || angle>10*PI)angle=0;
  while(angle<-PI)angle+=2*PI;while(angle>PI)angle-=2*PI;
}
Real heightAround(Real x,Real y){
  Real height=TheTerrainLogic->getGroundHeight(x,y);
  for(const Coord2D delta:{Coord2D{40,-40},Coord2D{-40,-40},Coord2D{40,40},Coord2D{-40,40}})
    height=std::max(height,TheTerrainLogic->getGroundHeight(x+delta.x,y+delta.y));
  finiteValue(height);return height;
}
NativeCamera::Pose pose(const Matrix3D& transform){
  return {coordinate(transform.Get_Translation()),coordinate(transform.Get_X_Vector()),
    coordinate(transform.Get_Y_Vector()),coordinate(transform.Get_Z_Vector())};
}
}
void NativeFrameCameraView::requireTransformParents()const{
  if(!TheGlobalData || (m_isSlaved && !TheScriptEngine))throw ERROR_BAD_ARG;
}
void NativeFrameCameraView::synchronizeCamera()const{
  // Source queries read the accepted CameraClass transform; they never rebuild
  // and therefore cannot consume RNG, advance C3 time or resolve a slave bone.
  if(!m_spatialCamera.ready())throw ERROR_BAD_ARG;
}
void NativeFrameCameraView::init(){
  requireTransformParents();ScriptedView::init();
  const Coord3D initial{870,770,0};setPosition(&initial);
  settleCameraTransform();m_cameraConstraintValid=FALSE;
  finiteValue(TheGlobalData->m_scrollAmountCutoff);m_scrollCutoff=TheGlobalData->m_scrollAmountCutoff;
}
NativeCamera NativeFrameCameraView::buildCameraTransform(){
  auto state=cameraState();state.lookAt.x+=m_shakeOffset.x;state.lookAt.y+=m_shakeOffset.y;
  if(m_cameraConstraintValid)state.lookAt=NativeCamera::clipPosition(state.lookAt,m_cameraConstraint);
  NativeCamera candidate;candidate.configure(state);
  auto transform=candidate.pose();
  m_context.timestep(1.0f/30);++m_buildCount;
  transform=NativeCamera::rotateLocal(transform,m_context.angles(transform.eye));
  if(m_isSlaved){
    auto* actor=TheScriptEngine->getUnitNamed(m_slaveObject);
    auto* drawable=actor?actor->getDrawable():nullptr;
    if(!drawable)m_isSlaved=FALSE;
    else {auto** modules=drawable->getDrawModules();
      for(;modules && *modules;++modules){const auto* draw=(*modules)->getObjectDrawInterface();if(!draw)continue;
        Matrix3D bone(1);
        // Original ignores false and adopts uninitialized data when no render
        // object produced a bone. Fix that UB by retaining the normal pose.
        if(draw->clientOnly_getRenderObjBoneTransform(m_slaveBone,&bone)){
          const auto slaved=pose(bone);NativeCamera validated;validated.configurePose(state,slaved);
          transform=slaved;m_pos=slaved.eye;
        }
        break; // Exactly the first ObjectDrawInterface, even on missing bone.
      }
    }
  }
  candidate.configurePose(state,transform);return candidate;
}
void NativeFrameCameraView::forceCameraConstraintRecalc(){
  requireTransformParents();synchronizeCamera();
  // Original GameClient creates its View before GameLogic creates terrain.
  // Preserve the invalid cache until a real map/terrain parent exists.
  if(!TheTerrainLogic)return;
  Region3D extent;TheTerrainLogic->getExtent(&extent);
  const auto bounds=m_spatialCamera.constraintRegion(extent,m_groundLevel,TheGlobalData->m_debugAI);
  m_cameraConstraint=bounds;m_cameraConstraintValid=TRUE;
}
void NativeFrameCameraView::settleCameraTransform(){
  requireTransformParents();
#if defined(_DEBUG) || defined(_INTERNAL)
  if(TheGlobalData->m_useCameraConstraints)
#endif
  {
    if(!m_cameraConstraintValid){m_spatialCamera=buildCameraTransform();forceCameraConstraintRecalc();}
    if(m_cameraConstraintValid)m_pos=NativeCamera::clipPosition(m_pos,m_cameraConstraint);
  }
  m_spatialCamera=buildCameraTransform();m_acceptedState=cameraState();
}
void NativeFrameCameraView::refreshProjection(){
  synchronizeCamera();auto state=m_acceptedState;
  state.width=m_width;state.height=m_height;state.originX=m_originX;state.originY=m_originY;state.fieldOfView=m_FOV;
  NativeCamera candidate;candidate.configurePose(state,m_spatialCamera.pose());
  m_spatialCamera=candidate;m_acceptedState=state;
}
void NativeFrameCameraView::forceRedraw(){settleCameraTransform();}
void NativeFrameCameraView::lookAt(const Coord3D* value){requireTransformParents();ScriptedView::lookAt(value);settleCameraTransform();}
void NativeFrameCameraView::setAngle(Real value){requireTransformParents();ScriptedView::setAngle(value);settleCameraTransform();}
void NativeFrameCameraView::setPitch(Real value){requireTransformParents();ScriptedView::setPitch(value);settleCameraTransform();}
void NativeFrameCameraView::setZoom(Real value){requireTransformParents();ScriptedView::setZoom(value);settleCameraTransform();}
void NativeFrameCameraView::setHeightAboveGround(Real value){requireTransformParents();ScriptedView::setHeightAboveGround(value);settleCameraTransform();}
void NativeFrameCameraView::setZoomToDefault(){requireTransformParents();ScriptedView::setZoomToDefault();settleCameraTransform();}
void NativeFrameCameraView::setAngleAndPitchToDefault(){requireTransformParents();ScriptedView::setAngleAndPitchToDefault();settleCameraTransform();}
void NativeFrameCameraView::initHeightForMap(){requireTransformParents();ScriptedView::initHeightForMap();settleCameraTransform();}
void NativeFrameCameraView::scrollBy(Coord2D* value){
  if(!value || (value->x==0 && value->y==0))return;
  requireTransformParents();NativeCameraView::scrollBy(value);settleCameraTransform();
}
void NativeFrameCameraView::setWidth(Int value){NativeCameraView::setWidth(value);refreshProjection();}
void NativeFrameCameraView::setHeight(Int value){NativeCameraView::setHeight(value);refreshProjection();}
void NativeFrameCameraView::setOrigin(Int x,Int y){NativeCameraView::setOrigin(x,y);refreshProjection();}
void NativeFrameCameraView::setFieldOfView(Real value){NativeCameraView::setFieldOfView(value);refreshProjection();}
void NativeFrameCameraView::Add_Camera_Shake(const Coord3D& point,float radius,float duration,float power){m_context.add(point,radius,duration,power);}
void NativeFrameCameraView::cameraEnableSlaveMode(const AsciiString& object,const AsciiString& bone){
  // Copy the fallible strings before publishing mode or replacing either name.
  AsciiString nextObject(object),nextBone(bone);m_slaveObject.swap(nextObject);m_slaveBone.swap(nextBone);m_isSlaved=TRUE;
}
void NativeFrameCameraView::shake(const Coord3D* epicenter,CameraShakeType type){
  if(!TheGlobalData || !epicenter)throw ERROR_BAD_ARG;point(*epicenter);
  Real intensity=0;
  switch(type){
    case SHAKE_SUBTLE:intensity=TheGlobalData->m_shakeSubtleIntensity;break;
    case SHAKE_NORMAL:intensity=TheGlobalData->m_shakeNormalIntensity;break;
    case SHAKE_STRONG:intensity=TheGlobalData->m_shakeStrongIntensity;break;
    case SHAKE_SEVERE:intensity=TheGlobalData->m_shakeSevereIntensity;break;
    case SHAKE_CINE_EXTREME:intensity=TheGlobalData->m_shakeCineExtremeIntensity;break;
    case SHAKE_CINE_INSANE:intensity=TheGlobalData->m_shakeCineInsaneIntensity;break;
    default:throw ERROR_BAD_ARG;
  }
  finiteValue(intensity);finiteValue(TheGlobalData->m_maxShakeRange);finiteValue(TheGlobalData->m_maxShakeIntensity);
  if(TheGlobalData->m_maxShakeRange<=0)throw ERROR_BAD_ARG;
  const double dx=double(epicenter->x)-m_pos.x,dy=double(epicenter->y)-m_pos.y;
  const double distance=std::hypot(dx,dy);
  const Real angle=GameClientRandomValueReal(0,2*PI);
  m_shakeCos=std::cos(angle);m_shakeSin=std::sin(angle);
  if(distance>TheGlobalData->m_maxShakeRange)return;
  intensity*=1.0f-Real(distance/TheGlobalData->m_maxShakeRange);
  m_shakeIntensity+=intensity;
  if(m_shakeIntensity>TheGlobalData->m_maxShakeIntensity)m_shakeIntensity=3.0f;
}
void NativeFrameCameraView::updateView(){
  if(!TheGlobalData || !TheTerrainLogic || !TheScriptEngine || !TheGameLogic || !TheInGameUI)throw ERROR_BAD_ARG;
  if(getCameraLock()!=INVALID_ID && m_lockType==LOCK_TETHER && !m_snapImmediate &&
    (!std::isfinite(TheGlobalData->m_partitionCellSize) || TheGlobalData->m_partitionCellSize<=0))throw ERROR_BAD_ARG;
  synchronizeCamera();Bool recalc=FALSE,didScriptedMovement=FALSE;
  const auto lock=getCameraLock();
  if(lock==INVALID_ID)m_context.m_followFactor=-1;
  else {
    cancelWaypointTracking();auto* actor=TheGameLogic->findObjectByID(lock);
    if(!actor){setCameraLock(INVALID_ID);setCameraLockDrawable(nullptr);m_context.m_followFactor=-1;}
    else {
      auto& factor=m_context.m_followFactor;factor=factor<0?.05f:std::min(1.0f,factor+.05f);
      if(auto* drawable=getCameraLockDrawable()){
        // Terminal paths withdraw borrows. Also require the surviving actor's
        // reciprocal child identity before querying its first draw module.
        if(actor->getDrawable()!=drawable)setCameraLockDrawable(nullptr);
        else {Coord3D location{};Real radius=0;Matrix3D model(1);
          if(drawable->clientOnly_getFirstRenderObjInfo(&location,&radius,&model)){
            point(location);finiteValue(radius);if(radius<0)throw ERROR_BAD_ARG;
            location.z+=radius;const auto direction=coordinate(model.Get_X_Vector());point(direction);
            const Coord3D desired{location.x-direction.x*radius*4.5f,location.y-direction.y*radius*4.5f,location.z-direction.z*radius*4.5f};
            const auto& previous=m_spatialCamera.eye();
            const Coord3D eye{previous.x+(desired.x-previous.x)*.1f,previous.y+(desired.y-previous.y)*.1f,previous.z+(desired.z-previous.z)*.1f};
            NativeCamera candidate;candidate.configurePose(m_acceptedState,NativeCamera::lookAtPose(eye,location));m_spatialCamera=candidate;
          }
        }
      }else {
        const auto actorPosition=*actor->getPosition();auto current=m_pos;
        const Real dx=actorPosition.x-current.x,dy=actorPosition.y-current.y;
        if(m_snapImmediate){current.x=actorPosition.x;current.y=actorPosition.y;}
        else if(m_lockType==LOCK_TETHER){const Real threshold=TheGlobalData->m_partitionCellSize*TheGlobalData->m_partitionCellSize;
          const Real distance=dx*dx+dy*dy;
          const Real ratio=distance>=threshold?(1-threshold/distance)*.5f:.01f*m_lockDist;
          current.x+=dx*ratio;current.y+=dy*ratio;
        }else {current.x+=dx*factor;current.y+=dy*factor;}
        if(!(TheScriptEngine->isTimeFrozenDebug() || TheScriptEngine->isTimeFrozenScript()) && !TheGameLogic->isGamePaused())m_previousLookAt=m_pos;
        setPosition(&current);
        if(m_lockType==LOCK_FOLLOW && actor->isUsingAirborneLocomotor() && actor->isAboveTerrainOrWater()){
          Real old=m_angle,ideal=actor->getOrientation()-PI/2;normalizeCameraAngle(old);normalizeCameraAngle(ideal);
          Real difference=ideal-old;normalizeCameraAngle(difference);m_angle=m_snapImmediate?ideal:m_angle+difference*.1f;normalizeCameraAngle(m_angle);
        }
        m_snapImmediate=FALSE;m_groundLevel=actorPosition.z;didScriptedMovement=TRUE;recalc=TRUE;
      }
    }
  }
  if(!TheScriptEngine->isTimeFrozenDebug() && !TheGameLogic->isGamePaused()){
    if(updateCameraMovements(frameLengthMilliseconds())){didScriptedMovement=TRUE;recalc=TRUE;}
  }else if(hasScriptedCameraWork())didScriptedMovement=TRUE;
  if(m_shakeIntensity>.01f){m_shakeOffset={m_shakeIntensity*m_shakeCos,m_shakeIntensity*m_shakeSin};
    m_shakeIntensity*=.75f;m_shakeCos=-m_shakeCos;m_shakeSin=-m_shakeSin;recalc=TRUE;
  }else {m_shakeIntensity=0;m_shakeOffset={};}
  if(m_context.active())recalc=TRUE;
  m_terrainHeightUnderCamera=heightAround(m_pos.x,m_pos.y);
  m_currentHeightAboveGround=m_cameraOffset.z*m_zoom-m_terrainHeightUnderCamera;
  if(m_okToAdjustHeight && !TheGameLogic->isGamePaused()){
    const Real desiredZoom=(m_terrainHeightUnderCamera+m_heightAboveGround)/m_cameraOffset.z;
    if(didScriptedMovement || (TheGameLogic->isInReplayGame() && TheGlobalData->m_useCameraInReplay))m_heightAboveGround=m_currentHeightAboveGround;
    if(TheInGameUI->isScrolling()){
      if(m_scrollAmount.length()<m_scrollCutoff || m_currentHeightAboveGround<m_minHeightAboveGround ||
        (TheGlobalData->m_enforceMaxCameraHeight && m_currentHeightAboveGround>m_maxHeightAboveGround)){
        const Real adjustment=(desiredZoom-m_zoom)*TheGlobalData->m_cameraAdjustSpeed;
        if(std::abs(adjustment)>=.0001f){m_zoom+=adjustment;recalc=TRUE;}
      }
    }else {const Real adjustment=(m_zoom-desiredZoom)*TheGlobalData->m_cameraAdjustSpeed;
      if(std::abs(adjustment)>=.0001f && !didScriptedMovement){m_zoom-=adjustment;recalc=TRUE;}}
  }
  if(TheScriptEngine->isTimeFast())return;
  if(recalc || m_isSlaved)settleCameraTransform();
  // N3 performs scene visibility/draw submission after this camera phase.
}
