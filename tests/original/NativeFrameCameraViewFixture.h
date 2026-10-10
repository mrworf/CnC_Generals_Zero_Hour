// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeFrameCameraView.h"
// Actual production camera frame. Physical picking/drawing explicitly absent.
struct NativeFrameCameraViewFixture:NativeFrameCameraView {
  using NativeFrameCameraView::NativeFrameCameraView;
  Drawable* pickDrawable(const ICoord2D*,Bool,PickType)override{throw ERROR_BAD_ARG;}
  Int iterateDrawablesInRegion(IRegion2D*,Bool(*)(Drawable*,void*),void*)override{throw ERROR_BAD_ARG;}
  void drawView()override{throw ERROR_BAD_ARG;}
  Coord3D position()const{return m_pos;}
  Region2D constraints()const{return m_cameraConstraint;}
  Bool constraintsValid()const{return m_cameraConstraintValid;}
  Real ground()const{return m_groundLevel;}
  NativeCamera::State state()const{return cameraState();}
  const NativeCamera& spatial=m_spatialCamera;
};
