// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/View.h"
#include <array>

// Game-owned CPU spatial backing. No renderer handle, global eye storage,
// vendor backend or presentation success is represented by this value.
class NativeCamera {
public:
  struct State {
    Coord3D lookAt{},offset{};
    Real ground=10,zoom=1,angle=0,pitch=0,fxPitch=1;
    Real fieldOfView=50*PI/180;
    Int width=640,height=480,originX=0,originY=0;
    Bool drawEntireTerrain=FALSE;
  };
  // Source world camera transform columns. Keep authored scale/shear: original
  // projection uses the general inverse; unprojection uses raw world columns.
  struct Pose {
    Coord3D eye{},right{1,0,0},up{0,1,0},backward{0,0,1};
  };
  void configure(const State&);
  void configurePose(const State&,const Pose&);
  Pose pose() const;
  static Pose lookAtPose(const Coord3D& eye,const Coord3D& target);
  static Pose rotateLocal(const Pose&,const Coord3D& radians);
  // Source cached-constraint query against an already accepted transform.
  // Does not advance shake, invoke models or clip/publish the View position.
  Region2D constraintRegion(const Region3D& authoredExtent,Real ground,Bool debugAI) const;
  static Coord3D clipPosition(const Coord3D&,const Region2D&);
  const Coord3D& eye() const;
  View::WorldToScreenReturn project(const Coord3D*,ICoord2D*) const;
  std::array<Coord3D,2> ray(const ICoord2D&) const;
  std::array<Coord3D,2> centerRay() const;
  Coord3D atZ(const ICoord2D&,Real z) const;
  Coord3D viewPlaneDisplacement(const Coord2D& pixelDelta) const;
  Bool ready() const noexcept {return m_ready;}
private:
  using Vector=std::array<double,3>;
  State m_state{};
  Coord3D m_eye{};
  Vector m_right{},m_up{},m_forward{};
  Vector m_projectRight{},m_projectUp{},m_projectForward{};
  double m_halfWidth=0,m_halfHeight=0,m_far=0;
  Bool m_ready=FALSE;
  Vector direction(const ICoord2D&) const;
};
