// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/ScriptedView.h"
#include "GameClient/NativeCamera.h"

// Real shared script/spatial View behavior. Scene picking, physical drawing and
// the full source frame/lock/shake lifecycle still require a concrete adapter.
class NativeCameraView : public ScriptedView {
public:
  void init() override;
  void setWidth(Int) override;
  void setHeight(Int) override;
  void setOrigin(Int,Int) override;
  void setFieldOfView(Real) override;
  void forceRedraw() override;
  const Coord3D& get3DCameraPosition() const override;
  WorldToScreenReturn worldToScreenTriReturn(const Coord3D*,ICoord2D*) override;
  void screenToWorld(const ICoord2D*,Coord3D*) override;
  void screenToTerrain(const ICoord2D*,Coord3D*) override;
  void screenToWorldAtZ(const ICoord2D*,Coord3D*,Real) override;
  void setGuardBandBias(const Coord2D*) override;
  // Do not inherit View's old 2D scroll or empty reachable effect callbacks.
  void scrollBy(Coord2D*) override = 0;
  void shake(const Coord3D*,CameraShakeType) override = 0;
  void Add_Camera_Shake(const Coord3D&,float,float,float) override = 0;
  void cameraEnableSlaveMode(const AsciiString&,const AsciiString&) override = 0;
  void cameraDisableSlaveMode() override = 0;
  void forceCameraConstraintRecalc() override = 0;
protected:
  void cameraStateChanged() noexcept override {m_cameraDirty=TRUE;}
  Coord3D projectElevatedLookAt(const Coord3D&) override;
  void synchronizeCamera() const;
  mutable NativeCamera m_spatialCamera;
private:
  mutable Bool m_cameraDirty=TRUE;
};
