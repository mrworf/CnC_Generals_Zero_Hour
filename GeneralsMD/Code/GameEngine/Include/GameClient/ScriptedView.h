// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/View.h"
#include "GameClient/ParabolicEase.h"
#include "Common/GameCommon.h"
#include <cstdint>

// Game-owned camera choreography from original W3DView. Physical projection,
// picking and presentation remain the responsibility of a concrete View.
// No renderer/vendor implementation or serialized camera block is imported.
class ScriptedView : public View
{
public:
  ScriptedView();
  void init() override;
  void reset() override;
  void setFrameLengthMilliseconds(Int milliseconds);
  Bool updateCameraMovements(Int elapsedMilliseconds);
  void lookAt(const Coord3D*) override;
  void initHeightForMap() override;
  void setAngle(Real) override;
  void setPitch(Real) override;
  void setZoom(Real) override;
  void setHeightAboveGround(Real) override;
  void setZoomToDefault() override;
  void setAngleAndPitchToDefault() override;
  void setDefaultView(Real,Real,Real) override;
  void setCameraLock(ObjectID) override;
  void setSnapMode(CameraLockType,Real) override;
  void moveCameraTo(const Coord3D*,Int,Int,Bool,Real,Real) override;
  void moveCameraAlongWaypointPath(Waypoint*,Int,Int,Bool,Real,Real) override;
  void rotateCamera(Real,Int,Real,Real) override;
  void rotateCameraTowardObject(ObjectID,Int,Int,Real,Real) override;
  void rotateCameraTowardPosition(const Coord3D*,Int,Real,Real,Bool) override;
  void zoomCamera(Real,Int,Real,Real) override;
  void pitchCamera(Real,Int,Real,Real) override;
  void cameraModFinalZoom(Real,Real,Real) override;
  void cameraModFinalPitch(Real,Real,Real) override;
  void cameraModFinalTimeMultiplier(Int) override;
  void cameraModRollingAverage(Int) override;
  void cameraModFreezeTime() override { m_freezeTimeForCameraMovement=TRUE; }
  void cameraModFreezeAngle() override;
  void cameraModLookToward(Coord3D*) override;
  void cameraModFinalLookToward(Coord3D*) override;
  void cameraModFinalMoveTo(Coord3D*) override;
  void resetCamera(const Coord3D*,Int,Real,Real) override;
  Bool isCameraMovementFinished() override;
  Bool isCameraMovementAtWaypointAlongPath();
  Bool isTimeFrozen() override { return m_freezeTimeForCameraMovement; }
  Int getTimeMultiplier() override { return m_timeMultiplier; }
  void setTimeMultiplier(Int value) override { m_timeMultiplier=value; }
  Real getFXPitch() const override { return m_FXPitch; }
  FilterModes getViewFilterMode() override { return m_viewFilterMode; }
  FilterTypes getViewFilterType() override { return m_viewFilter; }

protected:
  // A concrete adapter must provide actual spatial projection and transformed
  // camera backing. No false success for missing output or elevated look-at.
  virtual void cameraStateChanged() noexcept = 0;
  virtual Coord3D projectElevatedLookAt(const Coord3D&) = 0;
  virtual void beforeCameraPositionStep() noexcept {}
  void cancelCameraRotation() noexcept { m_doingRotateCamera=FALSE; }
  void cancelWaypointTracking() noexcept {m_doingMoveCameraOnWaypointPath=FALSE;m_CameraArrivedAtWaypointOnPathFlag=FALSE;}
  Bool hasScriptedCameraWork() const noexcept {
    return m_doingRotateCamera || m_doingMoveCameraOnWaypointPath || m_doingPitchCamera || m_doingZoomCamera || m_doingScriptedCameraLock;
  }
  Int frameLengthMilliseconds() const noexcept {return m_frameLengthMilliseconds;}
  Coord3D m_cameraOffset{};
  Real m_groundLevel=10.0f;
  Region2D m_cameraConstraint{};
  Bool m_cameraConstraintValid=FALSE;
  FilterModes m_viewFilterMode=FM_VIEW_DEFAULT;
  FilterTypes m_viewFilter=FT_VIEW_DEFAULT;

private:
  enum { MAX_WAYPOINTS=25 };
  struct Path {
    Int numWaypoints=0;
    Coord3D waypoints[MAX_WAYPOINTS+2]{};
    Real waySegLength[MAX_WAYPOINTS+2]{};
    Real cameraAngle[MAX_WAYPOINTS+2]{};
    Int timeMultiplier[MAX_WAYPOINTS+2]{};
    Real groundHeight[MAX_WAYPOINTS+2]{};
    Int totalTimeMilliseconds=0;
    std::int64_t elapsedTimeMilliseconds=0;
    Real totalDistance=0,curSegDistance=0;
    Int shutter=1,curSegment=1,curShutter=1,rollingAverageFrames=1;
    ParabolicEase ease{0,0};
  } m_mcwpInfo;
  struct Rotation {
    Int numFrames=1,numHoldFrames=0;
    std::int64_t curFrame=0;
    Int startTimeMultiplier=1,endTimeMultiplier=1;
    ParabolicEase ease;
    Bool trackObject=FALSE;
    struct { ObjectID targetObjectID=INVALID_ID; Coord3D targetObjectPos{}; } target;
    struct { Real startAngle=0,endAngle=0; } angle;
  } m_rcInfo;
  struct Zoom {
    Int numFrames=1;
    std::int64_t curFrame=0;
    Real startZoom=0,endZoom=0;
    Int startTimeMultiplier=1,endTimeMultiplier=1;
    ParabolicEase ease;
  } m_zcInfo;
  struct Pitch {
    Int numFrames=1;
    std::int64_t curFrame=0;
    Real startPitch=1,endPitch=1;
    Int startTimeMultiplier=1,endTimeMultiplier=1;
    ParabolicEase ease;
  } m_pcInfo;
  Int m_frameLengthMilliseconds=1000/LOGICFRAMES_PER_SECOND;
  Int m_timeMultiplier=1;
  Real m_FXPitch=1;
  Bool m_doingMoveCameraOnWaypointPath=FALSE,m_doingRotateCamera=FALSE;
  Bool m_doingPitchCamera=FALSE,m_doingZoomCamera=FALSE;
  Bool m_doingScriptedCameraLock=FALSE;
  Bool m_freezeTimeForCameraMovement=FALSE;
  Bool m_CameraArrivedAtWaypointOnPathFlag=FALSE;
  void setupWaypointPath(Bool);
  void moveAlongWaypointPath(Int);
  void rotateCameraOneFrame();
  void zoomCameraOneFrame();
  void pitchCameraOneFrame();
};
