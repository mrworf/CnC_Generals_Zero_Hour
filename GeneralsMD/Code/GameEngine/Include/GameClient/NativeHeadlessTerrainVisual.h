// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeTerrainVisual.h"

// CPU terrain and water state for the drawless original runtime. Physical
// terrain output is absent; requests for it fail at the provider boundary.
class NativeHeadlessTerrainVisual final : public NativeTerrainVisual {
public:
  void getTerrainColorAt(Real,Real,RGBColor*) override;
  void setTerrainTracksDetail() override;
  void setShoreLineDetail() override;
  void addFactionBib(Object*,Bool,Real) override;
  void removeFactionBib(Object*) override;
  void addFactionBibDrawable(Drawable*,Bool,Real) override;
  void removeFactionBibDrawable(Drawable*) override;
  void removeAllBibs() override;
  void removeBibHighlighting() override;
  void removeTreesAndPropsForConstruction(const Coord3D*,const GeometryInfo&,Real) override;
  void addProp(const ThingTemplate*,const Coord3D*,Real) override;
  void replaceSkyboxTextures(const AsciiString* [5],const AsciiString* [5]) override;
protected:
  void terrainHeightsChanged() noexcept override;
  void xfer(Xfer*) override;
};
