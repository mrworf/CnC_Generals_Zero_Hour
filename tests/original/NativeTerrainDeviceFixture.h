// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeTerrainVisual.h"
// Deliberately absent test output, never a production headless device.
struct DeviceFixture final:NativeTerrainVisual {
  unsigned dirty=0;
  void terrainHeightsChanged()noexcept override{++dirty;}
  void getTerrainColorAt(Real,Real,RGBColor*)override{throw ERROR_BAD_ARG;}
  Bool intersectTerrain(Coord3D*,Coord3D*,Coord3D*)override{throw ERROR_BAD_ARG;}
  void setTerrainTracksDetail()override{}
  void setShoreLineDetail()override{}
  void addFactionBib(Object*,Bool,Real)override{}
  void removeFactionBib(Object*)override{}
  void addFactionBibDrawable(Drawable*,Bool,Real)override{}
  void removeFactionBibDrawable(Drawable*)override{}
  void removeAllBibs()override{}
  void removeBibHighlighting()override{}
  void removeTreesAndPropsForConstruction(const Coord3D*,const GeometryInfo&,Real)override{}
  void addProp(const ThingTemplate*,const Coord3D*,Real)override{}
  void replaceSkyboxTextures(const AsciiString* [5],const AsciiString* [5])override{}
  void xfer(Xfer*)override{throw ERROR_BAD_ARG;}
};
