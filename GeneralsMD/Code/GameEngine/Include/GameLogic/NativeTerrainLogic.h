// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/NativeLogicalWorld.h"
class NativeTerrainVisual;

// Shipped terrain CPU semantics without the old device renderer. Concrete
// devices must supply their full map and road/bridge admission, not silent stubs.
class NativeTerrainLogic : public TerrainLogic {
public:
  explicit NativeTerrainLogic(NativeTerrainVisual& visual):m_visual(visual){}
  ~NativeTerrainLogic() override;
  using Admission=void (*)(NativeTerrainLogic&,void*);
  void init() override;
  void reset() override;
  Bool loadMap(AsciiString filename,Bool query) override;
  void loadStream(ChunkInputStream&,AsciiString filename,Bool query,
                  Admission admission=nullptr,void* context=nullptr);
  void newMap(Bool saveGame) override;
  Real getGroundHeight(Real x,Real y,Coord3D* normal=nullptr)const override;
  Real getLayerHeight(Real x,Real y,PathfindLayerEnum,Coord3D* normal=nullptr,Bool clip=TRUE)const override;
  Bool isCliffCell(Real x,Real y)const override;
  Bool isClearLineOfSight(const Coord3D&,const Coord3D&)const override;
  void getExtent(Region3D*)const override;
  void getMaximumPathfindExtent(Region3D*)const override;
  void getExtentIncludingBorder(Region3D*)const override;
  const NativeTerrainHeightMap& logicalMetadata()const noexcept { return m_world.height(); }
protected:
  // Device admission must stage/guard its own output mutations; the common CPU
  // scope cannot restore arbitrary effects in an external renderer or client.
  virtual void admitDeviceMap()=0;
  virtual void loadRoadsAndBridges(Bool saveGame)=0;
  void xfer(Xfer*) override;
private:
  NativeTerrainVisual& m_visual;
  NativeLogicalWorld m_world;
  Real m_minZ=0,m_maxZ=1;
  struct Load;
  static void admitWorld(const NativeTerrainHeightMap&,void*);
};
