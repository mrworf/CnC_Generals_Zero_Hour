// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeCameraView.h"
#include "GameClient/NativeCameraShaker.h"

// Original client camera phase, independent of rendering/presentation. The root
// owns the shared effect/follow context and must keep it alive through all Views.
// Physical picking/drawing remain explicit adapter obligations, not empty output.
class NativeFrameCameraView:public NativeCameraView {
public:
  explicit NativeFrameCameraView(NativeCameraShaker& context):m_context(context){}
  void init()override;
  void updateView()override;
  void forceRedraw()override;
  void forceCameraConstraintRecalc()override;
  void lookAt(const Coord3D*)override;
  void setAngle(Real)override;
  void setPitch(Real)override;
  void setZoom(Real)override;
  void setHeightAboveGround(Real)override;
  void setZoomToDefault()override;
  void setAngleAndPitchToDefault()override;
  void initHeightForMap()override;
  void scrollBy(Coord2D*)override;
  void setWidth(Int)override;
  void setHeight(Int)override;
  void setOrigin(Int,Int)override;
  void setFieldOfView(Real)override;
  void shake(const Coord3D*,CameraShakeType)override;
  void Add_Camera_Shake(const Coord3D&,float,float,float)override;
  void cameraEnableSlaveMode(const AsciiString&,const AsciiString&)override;
  void cameraDisableSlaveMode()override {m_isSlaved=FALSE;}
  std::size_t transformBuildCount()const noexcept{return m_buildCount;}
  const Coord3D& previousLookAtPosition()const noexcept{return m_previousLookAt;}
protected:
  void synchronizeCamera()const override;
  void beforeCameraPositionStep()noexcept override {m_previousLookAt=m_pos;}
private:
  NativeCameraShaker& m_context;
  NativeCamera::State m_acceptedState{};
  Coord3D m_previousLookAt{};
  Coord2D m_shakeOffset{};
  Real m_shakeIntensity=0,m_shakeCos=0,m_shakeSin=0,m_scrollCutoff=0;
  Bool m_isSlaved=FALSE;
  AsciiString m_slaveObject,m_slaveBone;
  std::size_t m_buildCount=0;
  void requireTransformParents()const;
  void settleCameraTransform();
  NativeCamera buildCameraTransform();
  void refreshProjection();
};
