// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/TerrainVisual.h"
#include "GameClient/NativeWaterGrid.h"
#include "GameLogic/NativeTerrainHeightMap.h"

// Common native CPU provider. Device output remains abstract: this is not a
// no-op renderer, nor a complete replacement for the device's v3 snapshot.
class NativeTerrainVisual : public TerrainVisual {
public:
  void init() override;
  void reset() override;
  void update() override;
  Bool load(AsciiString filename) override;
  void loadStream(ChunkInputStream&, AsciiString filename, Bool threeWayBlends);
  void exchangeCpuState(NativeTerrainVisual&) noexcept;
  struct TerrainState {
    NativeTerrainHeightMap height;
    AsciiString filename;
    explicit TerrainState(std::optional<UnsignedInt> shadow = std::nullopt):height(shadow){}
  };
  // Root admission exchanges candidate CPU map state without output callbacks;
  // invalidate only after the enclosing world transaction commits successfully.
  void exchangeTerrainState(TerrainState&) noexcept;
  void invalidateTerrainHeights() noexcept { terrainHeightsChanged(); }
  const NativeTerrainHeightMap& heightMap() const noexcept { return m_height; }
  const NativeWaterGrid& waterGrid() const noexcept { return m_water; }
  const AsciiString& filename() const noexcept { return m_filenameString; }
  TerrainType* getTerrainTile(Real x, Real y) override;
  void oversizeTerrain(Int) override {} // Original BaseHeightMap implementation.
  void enableWaterGrid(Bool enable) override;
  void setWaterGridHeightClamps(const WaterHandle*, Real low, Real high) override;
  void setWaterAttenuationFactors(const WaterHandle*, Real a, Real b, Real c, Real range) override;
  void setWaterTransform(const WaterHandle*, Real angle, Real x, Real y, Real z) override;
  void setWaterTransform(const Matrix3D*) override;
  void getWaterTransform(const WaterHandle*, Matrix3D*) override;
  void setWaterGridResolution(const WaterHandle*, Real x, Real y, Real spacing) override;
  void getWaterGridResolution(const WaterHandle*, Real* x, Real* y, Real* spacing) override;
  void changeWaterHeight(Real x, Real y, Real delta) override;
  void addWaterVelocity(Real x, Real y, Real velocity, Real preferred) override;
  Bool getWaterGridHeight(Real x, Real y, Real* height) override;
  void setRawMapHeight(const ICoord2D*, Int height) override;
  Int getRawMapHeight(const ICoord2D*) override;
  Bool intersectTerrain(Coord3D*, Coord3D*, Coord3D*) override;
protected:
  // Devices mark dirty state here; fallible upload belongs in device admission.
  virtual void terrainHeightsChanged() noexcept = 0;
  void xfer(Xfer*) override = 0;
  NativeTerrainHeightMap m_height;
  NativeWaterGrid m_water;
};
