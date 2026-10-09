// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GameCommon.h"
#include "Common/Snapshot.h"
#include "Common/Terrain.h"
#include "WWMath/matrix3d.h"
#include <vector>
class GlobalData;

// Source WaterRenderObjClass CPU gameplay state. No renderer ownership or
// callbacks into simulation: the caller supplies the source logic frame/gravity.
class NativeWaterGrid final : public Snapshot {
public:
  struct Cell { Real height=0, velocity=0; UnsignedByte status=0, preferredHeight=0;
    bool operator==(const Cell&) const = default; };
  void init(const GlobalData& data);
  void enable(Bool enabled);
  void reset() noexcept;
  void setResolution(Real x,Real y,Real spacing);
  void setClamps(Real low,Real high);
  void setAttenuation(Real a,Real b,Real c,Real range);
  void setTransform(Real angle,Real x,Real y,Real z);
  // Source matrix overload intentionally does not update query origin/axes.
  void setTransform(const Matrix3D& transform);
  const Matrix3D& transform() const noexcept { return m_transform; }
  Bool worldToGrid(Real x,Real y,Real& gx,Real& gy) const;
  Bool height(Real x,Real y,Real* output) const;
  void changeHeight(Real x,Real y,Real delta);
  void addVelocity(Real x,Real y,Real velocity,Real preferred);
  void update(UnsignedInt logicFrame,Real gravity);
  Int cellsX() const noexcept { return m_x; }
  Int cellsY() const noexcept { return m_y; }
  Real spacing() const noexcept { return m_spacing; }
  Bool enabled() const noexcept { return m_enabled; }
  Bool inMotion() const noexcept { return m_motion; }
  Bool riverDisabled() const noexcept { return m_disableRiver; }
  const std::vector<Cell>& backing() const noexcept { return m_cells; }
  const Cell& vertex(Int x,Int y) const;
  void swap(NativeWaterGrid& other) noexcept;
private:
  Int m_x=128,m_y=128;
  Real m_spacing=MAP_XY_FACTOR;
  Real m_low=0,m_high=0,m_a=0,m_b=0,m_c=0,m_range=0;
  Real m_originX=0,m_originY=0,m_axisXX=1,m_axisXY=0,m_axisYX=1,m_axisYY=0;
  Matrix3D m_transform{1};
  Bool m_enabled=false,m_motion=false,m_disableRiver=false;
  UnsignedInt m_lastFrame=0;
  std::vector<Cell> m_cells;
  std::size_t index(Int x,Int y) const noexcept;
  void crc(Xfer*) override {} // Source water light CRC is empty.
  void xfer(Xfer*) override;
  void loadPostProcess() override {}
};
