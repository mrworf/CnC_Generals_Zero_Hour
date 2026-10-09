// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeTerrainVisual.h"
#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/TerrainTypes.h"
#include "GameLogic/GameLogic.h"

void NativeTerrainVisual::init() {
  if (!TheGlobalData) throw ERROR_BAD_ARG;
  m_water.init(*TheGlobalData);
  TerrainVisual::init();
}
void NativeTerrainVisual::reset() {
  TerrainVisual::reset();
  m_water.reset(); // Source retains the accepted height map and water backing.
}
void NativeTerrainVisual::update() {
  if (!TheGameLogic || !TheGlobalData) throw ERROR_BAD_ARG;
  m_water.update(TheGameLogic->getFrame(), TheGlobalData->m_gravity);
}
Bool NativeTerrainVisual::load(AsciiString filename) {
  if (filename.isEmpty()) return FALSE;
  if (!TheGlobalData) throw ERROR_BAD_ARG;
  CachedFileInputStream stream;
  if (!stream.open(filename)) return FALSE;
  loadStream(stream, filename, TheGlobalData->m_use3WayTerrainBlends != 0);
  return TRUE;
}
void NativeTerrainVisual::loadStream(ChunkInputStream& stream, AsciiString filename,
                                   Bool threeWayBlends) {
  if (!TheGlobalData || filename.isEmpty()) throw ERROR_BAD_ARG;
  m_height.loadVisualTerrain(stream, threeWayBlends, *TheWritableGlobalData);
  m_filenameString.swap(filename); // All remaining publication is nonfallible.
  terrainHeightsChanged();
}
void NativeTerrainVisual::exchangeCpuState(NativeTerrainVisual& other) noexcept {
  m_height.swap(other.m_height);
  m_water.swap(other.m_water);
  m_filenameString.swap(other.m_filenameString);
  terrainHeightsChanged(); other.terrainHeightsChanged();
}
void NativeTerrainVisual::exchangeTerrainState(TerrainState& other) noexcept {
  m_height.swap(other.height);
  m_filenameString.swap(other.filename);
}
TerrainType* NativeTerrainVisual::getTerrainTile(Real x, Real y) {
  if (!m_height.width()) return nullptr;
  if (!TheTerrainTypes) throw ERROR_BAD_ARG;
  const auto name=m_height.terrainName(x,y);
  return TheTerrainTypes->findTerrain(AsciiString(std::string(name).c_str()));
}
void NativeTerrainVisual::enableWaterGrid(Bool value) { m_water.enable(value); }
void NativeTerrainVisual::setWaterGridHeightClamps(const WaterHandle*, Real low, Real high) { m_water.setClamps(low,high); }
void NativeTerrainVisual::setWaterAttenuationFactors(const WaterHandle*, Real a, Real b, Real c, Real range) { m_water.setAttenuation(a,b,c,range); }
void NativeTerrainVisual::setWaterTransform(const WaterHandle*, Real angle, Real x, Real y, Real z) { m_water.setTransform(angle,x,y,z); }
void NativeTerrainVisual::setWaterTransform(const Matrix3D* value) { if(value)m_water.setTransform(*value); }
void NativeTerrainVisual::getWaterTransform(const WaterHandle*, Matrix3D* value) { if(value)*value=m_water.transform(); }
void NativeTerrainVisual::setWaterGridResolution(const WaterHandle*, Real x, Real y, Real spacing) { m_water.setResolution(x,y,spacing); }
void NativeTerrainVisual::getWaterGridResolution(const WaterHandle*, Real* x, Real* y, Real* spacing) {
  // Null outputs are individually optional, as in the source grid adapter.
  if(x)*x=m_water.cellsX(); if(y)*y=m_water.cellsY(); if(spacing)*spacing=m_water.spacing();
}
void NativeTerrainVisual::changeWaterHeight(Real x, Real y, Real delta) { m_water.changeHeight(x,y,delta); }
void NativeTerrainVisual::addWaterVelocity(Real x, Real y, Real velocity, Real preferred) { m_water.addVelocity(x,y,velocity,preferred); }
Bool NativeTerrainVisual::getWaterGridHeight(Real x, Real y, Real* output) { return m_water.height(x,y,output); }
void NativeTerrainVisual::setRawMapHeight(const ICoord2D* position, Int height) {
  if(!m_height.width())return;
  if(!position)throw ERROR_BAD_ARG;
  if(m_height.lowerRawHeight(*position,height))terrainHeightsChanged();
}
Int NativeTerrainVisual::getRawMapHeight(const ICoord2D* position) {
  if(!m_height.width())return 0;
  if(!position)throw ERROR_BAD_ARG;
  return m_height.constructionHeight(*position);
}
