// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeW3DAssets.h"

// Scaled source prototype Clone bounds, not renderer or collision ownership.
class NativeW3DModelBounds final {
public:
  struct Box {std::array<float,3> center{},extent{};};
  static NativeW3DModelBounds decode(const NativeW3DAssets::Model&,float geometryScale);
  Box objectBox(const NativeW3DPose&,const NativeW3DPose::Matrix& world)const;
  float worldSphereRadius(const NativeW3DPose&,const NativeW3DPose::Matrix& world,float objectScale)const;
private:
  Box m_base;
  std::array<float,3> m_sphereCenter{};
  float m_radius=0;
  bool m_hlod=false;
  struct Override {std::uint32_t bone;Box local;};
  std::optional<Override> m_override;
  static Box transform(const Box&,const NativeW3DPose::Matrix&);
};
