// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GameCommon.h"
#include <cstddef>
#include <vector>

// Root-owned original C3 camera effects. All Views in a root borrow this owner;
// each explicit transform rebuild advances it, never a spatial query. Client
// thread only. Original libc visual randomness is separate from logic RNG.
class NativeCameraShaker {
public:
  NativeCameraShaker()=default;
  NativeCameraShaker(const NativeCameraShaker&)=delete;
  NativeCameraShaker& operator=(const NativeCameraShaker&)=delete;
  void add(const Coord3D&,Real radius,Real duration,Real powerDegrees);
  void timestep(Real seconds);
  Coord3D angles(const Coord3D& cameraPosition);
  Bool active()const noexcept{return !m_records.empty();}
  std::size_t count()const noexcept{return m_records.size();}
private:
  friend class NativeFrameCameraView;
  Real m_followFactor=-1; // Original update function-static, now root lifetime.
  struct Record {
    Coord3D position{},omega{},phase{};
    Real radius=0,duration=0,intensity=0,elapsed=0;
  };
  std::vector<Record> m_records;
};
