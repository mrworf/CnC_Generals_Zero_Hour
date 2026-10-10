// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeW3DAnimation.h"
#include <optional>
#include <string_view>

// CPU hierarchy owner, independent of renderer state. Borrowed bone views are
// invalidated by the next successful evaluation or replacement, not rejection.
class NativeW3DPose final {
public:
  using Matrix=std::array<float,12>; // Row-major affine 3x4, original math order.
  struct Bone {Matrix transform;bool visible=true;};
  NativeW3DPose()=default;
  NativeW3DPose(const NativeW3DPose&)=delete;
  NativeW3DPose& operator=(const NativeW3DPose&)=delete;
  NativeW3DPose(NativeW3DPose&&) noexcept=default;
  NativeW3DPose& operator=(NativeW3DPose&&) noexcept=default;
  static Matrix identity() noexcept;
  static NativeW3DPose decode(const NativeW3DDocument::Hierarchy&);
  void replace(const NativeW3DDocument::Hierarchy&);
  void swap(NativeW3DPose&) noexcept;
  NativeW3DPose clone() const;
  void evaluate(const Matrix& root,const NativeW3DAnimation* motion=nullptr,float frame=0,float scale=1);
  void evaluateRuntime(const Matrix&,const NativeW3DAnimation*,float frame,float scale=1);
  void evaluateBlend(const Matrix& root,const NativeW3DAnimation&,float frame0,
                     const NativeW3DAnimation&,float frame1,float percentage,float scale=1);
  std::optional<std::size_t> findBone(std::string_view) const noexcept;
  std::size_t sourceBoneIndex(std::string_view name) const noexcept{return findBone(name).value_or(0);}
  const Bone& bone(std::size_t)const;
  Matrix pristineBone(std::size_t,const Matrix& root,float scale=1)const;
  void capture(std::size_t);
  void release(std::size_t);
  void control(std::size_t,const Matrix&,bool worldSpaceTranslation);
private:
  struct Node {std::string name;std::uint32_t parent;Matrix base,capture;bool captured=false,world=false;};
  std::string m_name;
  std::vector<Node> m_nodes;
  std::vector<Bone> m_pose,m_candidate;
  void evaluate(const Matrix&,const NativeW3DAnimation*,float,const NativeW3DAnimation*,float,float,float,bool runtime=false);
};
