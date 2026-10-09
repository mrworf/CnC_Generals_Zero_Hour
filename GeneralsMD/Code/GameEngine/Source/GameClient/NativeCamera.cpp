// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameClient/NativeCamera.h"
#include "Common/Terrain.h"
#include <cmath>
#include <limits>

namespace {
using Vector=std::array<double,3>;
void requireFinite(double value){if(!std::isfinite(value))throw ERROR_BAD_ARG;}
void point(const Coord3D& p){requireFinite(p.x);requireFinite(p.y);requireFinite(p.z);}
Real real(double value){requireFinite(value);if(std::abs(value)>std::numeric_limits<Real>::max())throw ERROR_BAD_ARG;return static_cast<Real>(value);}
Vector cross(const Vector& a,const Vector& b){return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};}
double dot(const Vector& a,const Vector& b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
Vector unit(Vector value){const double length=std::hypot(value[0],value[1],value[2]);
  if(!std::isfinite(length) || length==0)throw ERROR_BAD_ARG;
  for(auto& component:value)component/=length;return value;}
Int pixel(double value,Int origin){
  const double rounded=std::trunc(value);requireFinite(rounded);
  if(rounded<std::numeric_limits<Int>::min() || rounded>std::numeric_limits<Int>::max())throw ERROR_BAD_ARG;
  const auto absolute=static_cast<std::int64_t>(rounded)+origin;
  if(absolute<std::numeric_limits<Int>::min() || absolute>std::numeric_limits<Int>::max())throw ERROR_BAD_ARG;
  return static_cast<Int>(absolute);
}
}
void NativeCamera::configure(const State& state){
  point(state.lookAt);point(state.offset);
  for(double value:{state.ground,state.zoom,state.angle,state.pitch,state.fxPitch,state.fieldOfView})requireFinite(value);
  if(state.width<1 || state.height<1 || state.offset.z==0 || state.zoom<=0 ||
    state.fieldOfView<=0 || state.fieldOfView>=PI)throw ERROR_BAD_ARG;
  NativeCamera candidate;candidate.m_state=state;
  Vector source{double(state.offset.x)*state.zoom,double(state.offset.y)*state.zoom,double(state.offset.z)*state.zoom};
  const double factor=1-state.ground/source[2];
  const double sp=std::sin(state.pitch),cp=std::cos(state.pitch);
  const double sa=std::sin(state.angle),ca=std::cos(state.angle);
  const double y=cp*source[1]-sp*source[2],z=sp*source[1]+cp*source[2];
  source={ca*source[0]-sa*y,sa*source[0]+ca*y,z};
  for(auto& component:source)component*=factor;
  source[0]+=state.lookAt.x;source[1]+=state.lookAt.y;source[2]+=state.ground;
  Vector target{state.lookAt.x,state.lookAt.y,state.ground};
  if(state.fxPitch<=1)target[2]=source[2]-(source[2]-target[2])*state.fxPitch;
  else {source[0]=target[0]+(source[0]-target[0])/state.fxPitch;source[1]=target[1]+(source[1]-target[1])/state.fxPitch;}
  candidate.m_eye={real(source[0]),real(source[1]),real(source[2])};
  source={candidate.m_eye.x,candidate.m_eye.y,candidate.m_eye.z};
  for(auto& component:target)component=real(component);
  candidate.m_forward=unit({target[0]-source[0],target[1]-source[1],target[2]-source[2]});
  // Original Matrix3D::Look_At's vertical yaw fallback chooses -Y as right.
  candidate.m_right=(candidate.m_forward[0]==0 && candidate.m_forward[1]==0)
    ? Vector{0,-1,0} : unit(cross(candidate.m_forward,{0,0,1}));
  candidate.m_up=unit(cross(candidate.m_right,candidate.m_forward));
  candidate.m_halfWidth=std::tan(double(state.fieldOfView)/2);
  candidate.m_halfHeight=candidate.m_halfWidth*state.height/state.width;
  candidate.m_far=(state.drawEntireTerrain || state.fxPitch<.95f || state.zoom>1.05f) ? 1200*MAP_XY_FACTOR : 1200;
  candidate.m_ready=TRUE;*this=candidate;
}
const Coord3D& NativeCamera::eye() const {if(!m_ready)throw ERROR_BAD_ARG;return m_eye;}
View::WorldToScreenReturn NativeCamera::project(const Coord3D* world,ICoord2D* screen)const{
  if(!world || !screen)return View::WTS_INVALID;
  if(!m_ready || !std::isfinite(world->x) || !std::isfinite(world->y) || !std::isfinite(world->z)){
    *screen={0,0};return View::WTS_INVALID;}
  const Vector relative{double(world->x)-m_eye.x,double(world->y)-m_eye.y,double(world->z)-m_eye.z};
  const double depth=dot(relative,m_forward);
  if(depth<MAP_XY_FACTOR || depth>m_far){*screen={0,0};return View::WTS_INVALID;}
  const double x=dot(relative,m_right)/(depth*m_halfWidth),y=dot(relative,m_up)/(depth*m_halfHeight);
  ICoord2D result;
  try{const Real nx=real(x),ny=real(y);
    result={pixel((Real(m_state.width)*(nx+1.0f))/2.0f,m_state.originX),
      pixel((Real(m_state.height)*(1.0f-ny))/2.0f,m_state.originY)};}
  catch(ErrorCode){*screen={0,0};return View::WTS_INVALID;}
  *screen=result;
  return (x<-1 || x>1 || y<-1 || y>1) ? View::WTS_OUTSIDE_FRUSTUM : View::WTS_INSIDE_FRUSTUM;
}
NativeCamera::Vector NativeCamera::direction(const ICoord2D& screen)const{
  if(!m_ready)throw ERROR_BAD_ARG;
  const double x=(2*(double(screen.x)-m_state.originX)/m_state.width-1)*m_halfWidth;
  const double y=(1-2*(double(screen.y)-m_state.originY)/m_state.height)*m_halfHeight;
  Vector result;for(unsigned n=0;n<3;++n)result[n]=m_forward[n]+x*m_right[n]+y*m_up[n];
  return unit(result);
}
std::array<Coord3D,2> NativeCamera::ray(const ICoord2D& screen)const{
  const auto d=direction(screen);
  const Coord3D end{real(m_eye.x+d[0]*m_far),real(m_eye.y+d[1]*m_far),real(m_eye.z+d[2]*m_far)};
  return {m_eye,end};
}
Coord3D NativeCamera::atZ(const ICoord2D& screen,Real z)const{
  requireFinite(z);const auto d=direction(screen);if(d[2]==0)throw ERROR_BAD_ARG;
  const double distance=(double(z)-m_eye.z)/d[2];
  return {real(m_eye.x+d[0]*distance),real(m_eye.y+d[1]*distance),z};
}
std::array<Coord3D,2> NativeCamera::centerRay()const{
  if(!m_ready)throw ERROR_BAD_ARG;
  return {m_eye,Coord3D{real(m_eye.x+m_forward[0]*m_far),real(m_eye.y+m_forward[1]*m_far),real(m_eye.z+m_forward[2]*m_far)}};
}
Coord3D NativeCamera::viewPlaneDisplacement(const Coord2D& delta)const{
  if(!m_ready)throw ERROR_BAD_ARG;requireFinite(delta.x);requireFinite(delta.y);
  // Difference of source Device_To_World_Space at local Z=-1. Do not
  // normalize: terrain/far-depth rays have a different displacement scale.
  const double x=2*double(delta.x)*m_halfWidth/m_state.width;
  const double y=-2*double(delta.y)*m_halfHeight/m_state.height;
  return {real(x*m_right[0]+y*m_up[0]),real(x*m_right[1]+y*m_up[1]),
    real(x*m_right[2]+y*m_up[2])};
}
