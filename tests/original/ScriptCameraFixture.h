// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/ScriptedView.h"
// Observe real shared choreography; all absent physical operations reject.
struct ScriptCameraFixture:ScriptedView {
  unsigned changes=0;
  void cameraStateChanged() noexcept override {++changes;}
  Coord3D projectElevatedLookAt(const Coord3D&) override {throw ERROR_BAD_ARG;}
  Drawable* pickDrawable(const ICoord2D*,Bool,PickType) override {throw ERROR_BAD_ARG;}
  Int iterateDrawablesInRegion(IRegion2D*,Bool(*)(Drawable*,void*),void*) override {throw ERROR_BAD_ARG;}
  void forceRedraw() override {throw ERROR_BAD_ARG;}
  const Coord3D& get3DCameraPosition() const override {throw ERROR_BAD_ARG;}
  WorldToScreenReturn worldToScreenTriReturn(const Coord3D*,ICoord2D*) override {throw ERROR_BAD_ARG;}
  void screenToWorld(const ICoord2D*,Coord3D*) override {throw ERROR_BAD_ARG;}
  void screenToTerrain(const ICoord2D*,Coord3D*) override {throw ERROR_BAD_ARG;}
  void screenToWorldAtZ(const ICoord2D*,Coord3D*,Real) override {throw ERROR_BAD_ARG;}
  void drawView() override {throw ERROR_BAD_ARG;}
  void updateView() override {updateCameraMovements(33);}
  void setGuardBandBias(const Coord2D* value) override {m_guardBandBias=*value;}
  void filter(FilterTypes type,FilterModes mode){m_viewFilter=type;m_viewFilterMode=mode;}
  Coord3D position(){Coord3D value;View::getPosition(&value);return value;}
};
