// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeCameraView.h"
// Production spatial/navigation backing. No absent frame/model/output provider
// is accepted by this adapter; use only for explicit component lifetime tests.
struct NativeCameraViewFixture:NativeCameraView {
  Drawable* pickDrawable(const ICoord2D*,Bool,PickType)override{throw ERROR_BAD_ARG;}
  Int iterateDrawablesInRegion(IRegion2D*,Bool(*)(Drawable*,void*),void*)override{throw ERROR_BAD_ARG;}
  void drawView()override{throw ERROR_BAD_ARG;}
  void updateView()override{throw ERROR_BAD_ARG;}
  void shake(const Coord3D*,CameraShakeType)override{throw ERROR_BAD_ARG;}
  void Add_Camera_Shake(const Coord3D&,float,float,float)override{throw ERROR_BAD_ARG;}
  void cameraEnableSlaveMode(const AsciiString&,const AsciiString&)override{throw ERROR_BAD_ARG;}
  void cameraDisableSlaveMode()override{throw ERROR_BAD_ARG;}
  void forceCameraConstraintRecalc()override{throw ERROR_BAD_ARG;}
};
