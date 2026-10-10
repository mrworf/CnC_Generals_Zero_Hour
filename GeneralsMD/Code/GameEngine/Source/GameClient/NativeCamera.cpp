// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameClient/NativeCamera.h"
#include "Common/Terrain.h"
#include <algorithm>
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
Vector lookDirection(Vector value){
  // Original Matrix3D::Look_At defines coincident eye/target as its default
  // yaw/pitch (+X forward), rather than an invalid/zero orientation.
  if(value[0]==0 && value[1]==0 && value[2]==0)return {1,0,0};
  return unit(value);
}
Vector vector(const Coord3D& p){return {p.x,p.y,p.z};}
Coord3D coordinate(const Vector& v){return {real(v[0]),real(v[1]),real(v[2])};}
void validatePose(const NativeCamera::Pose& p){
  point(p.eye);point(p.right);point(p.up);point(p.backward);
  const double determinant=dot(vector(p.right),cross(vector(p.up),vector(p.backward)));
  if(!std::isfinite(determinant) || determinant==0)throw ERROR_BAD_ARG;
}
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
  candidate.m_forward=lookDirection({target[0]-source[0],target[1]-source[1],target[2]-source[2]});
  // Original Matrix3D::Look_At's vertical yaw fallback chooses -Y as right.
  candidate.m_right=(candidate.m_forward[0]==0 && candidate.m_forward[1]==0)
    ? Vector{0,-1,0} : unit(cross(candidate.m_forward,{0,0,1}));
  candidate.m_up=unit(cross(candidate.m_right,candidate.m_forward));
  candidate.m_projectRight=candidate.m_right;candidate.m_projectUp=candidate.m_up;candidate.m_projectForward=candidate.m_forward;
  candidate.m_halfWidth=std::tan(double(state.fieldOfView)/2);
  candidate.m_halfHeight=candidate.m_halfWidth*state.height/state.width;
  candidate.m_far=(state.drawEntireTerrain || state.fxPitch<.95f || state.zoom>1.05f) ? 1200*MAP_XY_FACTOR : 1200;
  candidate.m_ready=TRUE;*this=candidate;
}
const Coord3D& NativeCamera::eye() const {if(!m_ready)throw ERROR_BAD_ARG;return m_eye;}
void NativeCamera::configurePose(const State& state,const Pose& pose){
  validatePose(pose);
  for(double value:{state.zoom,state.fxPitch,state.fieldOfView})requireFinite(value);
  if(state.width<1 || state.height<1 || state.zoom<=0 || state.fieldOfView<=0 || state.fieldOfView>=PI)throw ERROR_BAD_ARG;
  NativeCamera candidate;candidate.m_state=state;candidate.m_eye=pose.eye;
  candidate.m_right=vector(pose.right);candidate.m_up=vector(pose.up);
  candidate.m_forward={-double(pose.backward.x),-double(pose.backward.y),-double(pose.backward.z)};
  const auto back=vector(pose.backward);
  const double determinant=dot(candidate.m_right,cross(candidate.m_up,back));
  candidate.m_projectRight=cross(candidate.m_up,back);
  candidate.m_projectUp=cross(back,candidate.m_right);
  candidate.m_projectForward=cross(candidate.m_right,candidate.m_up);
  for(unsigned n=0;n<3;++n){
    candidate.m_projectRight[n]/=determinant;candidate.m_projectUp[n]/=determinant;candidate.m_projectForward[n]/=-determinant;
    requireFinite(candidate.m_projectRight[n]);requireFinite(candidate.m_projectUp[n]);requireFinite(candidate.m_projectForward[n]);
  }
  candidate.m_halfWidth=std::tan(double(state.fieldOfView)/2);
  candidate.m_halfHeight=candidate.m_halfWidth*state.height/state.width;
  candidate.m_far=(state.drawEntireTerrain || state.fxPitch<.95f || state.zoom>1.05f) ? 1200*MAP_XY_FACTOR : 1200;
  candidate.m_ready=TRUE;*this=candidate;
}
NativeCamera::Pose NativeCamera::pose()const{
  if(!m_ready)throw ERROR_BAD_ARG;
  return {m_eye,coordinate(m_right),coordinate(m_up),coordinate({-m_forward[0],-m_forward[1],-m_forward[2]})};
}
NativeCamera::Pose NativeCamera::lookAtPose(const Coord3D& eye,const Coord3D& target){
  point(eye);point(target);
  const auto forward=lookDirection({double(target.x)-eye.x,double(target.y)-eye.y,double(target.z)-eye.z});
  const auto right=(forward[0]==0 && forward[1]==0)?Vector{0,-1,0}:unit(cross(forward,{0,0,1}));
  return {eye,coordinate(right),coordinate(unit(cross(right,forward))),coordinate({-forward[0],-forward[1],-forward[2]})};
}
NativeCamera::Pose NativeCamera::rotateLocal(const Pose& input,const Coord3D& radians){
  validatePose(input);point(radians);Pose result=input;
  Vector right=vector(input.right),up=vector(input.up),back=vector(input.backward);
  auto rotate=[](Vector& first,Vector& second,double angle){
    const auto a=first,b=second;const double c=std::cos(angle),s=std::sin(angle);
    for(unsigned n=0;n<3;++n){first[n]=a[n]*c+b[n]*s;second[n]=b[n]*c-a[n]*s;}
  };
  // Original Matrix3D postmultiplies X, then Y, then Z. Y has the reversed
  // column pair/sign; these are local rotations, not world Euler rotations.
  rotate(up,back,radians.x);rotate(back,right,radians.y);rotate(right,up,radians.z);
  result.right=coordinate(right);result.up=coordinate(up);result.backward=coordinate(back);
  validatePose(result);return result;
}
Region2D NativeCamera::constraintRegion(const Region3D& extent,Real ground,Bool debugAI)const{
  if(!m_ready)throw ERROR_BAD_ARG;point(extent.lo);point(extent.hi);requireFinite(ground);
  if(extent.lo.x>extent.hi.x || extent.lo.y>extent.hi.y)throw ERROR_BAD_ARG;
  // Source assigns the complete absolute Real expression to Int, including
  // origin, before querying either ray. Odd sizes retain their truncation.
  const ICoord2D centerPixel{pixel(.5f*m_state.width+m_state.originX,0),pixel(.5f*m_state.height+m_state.originY,0)};
  const ICoord2D bottomPixel{centerPixel.x,pixel(m_state.originY+.95f*m_state.height,0)};
  auto center=atZ(centerPixel,ground);const auto bottom=atZ(bottomPixel,ground);
  center.x-=bottom.x;center.y-=bottom.y;
  // Deliberately retain center.z=ground: subtracting bottom.z or taking only
  // XY length changes the original camera border on elevated maps.
  Real offset=std::sqrt(center.x*center.x+center.y*center.y+center.z*center.z);
  requireFinite(offset);if(debugAI)offset=-1000;
  const Region2D result{{real(double(extent.lo.x)+offset),real(double(extent.lo.y)+offset)},
    {real(double(extent.hi.x)-offset),real(double(extent.hi.y)-offset)}};
  // Inverted result on small maps is valid. Source clips max(lo), then min(hi).
  return result;
}
Coord3D NativeCamera::clipPosition(const Coord3D& input,const Region2D& bounds){
  point(input);for(double value:{bounds.lo.x,bounds.lo.y,bounds.hi.x,bounds.hi.y})requireFinite(value);
  Coord3D result=input;
  result.x=std::min(bounds.hi.x,std::max(bounds.lo.x,result.x));
  result.y=std::min(bounds.hi.y,std::max(bounds.lo.y,result.y));
  return result;
}
View::WorldToScreenReturn NativeCamera::project(const Coord3D* world,ICoord2D* screen)const{
  if(!world || !screen)return View::WTS_INVALID;
  if(!m_ready || !std::isfinite(world->x) || !std::isfinite(world->y) || !std::isfinite(world->z)){
    *screen={0,0};return View::WTS_INVALID;}
  const Vector relative{double(world->x)-m_eye.x,double(world->y)-m_eye.y,double(world->z)-m_eye.z};
  const double depth=dot(relative,m_projectForward);
  if(depth<MAP_XY_FACTOR || depth>m_far){*screen={0,0};return View::WTS_INVALID;}
  const double x=dot(relative,m_projectRight)/(depth*m_halfWidth),y=dot(relative,m_projectUp)/(depth*m_halfHeight);
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
  requireFinite(z);const auto d=direction(screen);
  if(d[2]==0){
    // A coplanar ray starts on the requested plane. Return that closest point
    // instead of the original Find_*_At_Z zero-times-infinity NaN.
    if(z==m_eye.z)return m_eye;
    throw ERROR_BAD_ARG;
  }
  const double distance=(double(z)-m_eye.z)/d[2];
  return {real(m_eye.x+d[0]*distance),real(m_eye.y+d[1]*distance),z};
}
std::array<Coord3D,2> NativeCamera::centerRay()const{
  if(!m_ready)throw ERROR_BAD_ARG;
  const auto forward=unit(m_forward);
  return {m_eye,Coord3D{real(m_eye.x+forward[0]*m_far),real(m_eye.y+forward[1]*m_far),real(m_eye.z+forward[2]*m_far)}};
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
