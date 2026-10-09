// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeWaterGrid.h"
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/Xfer.h"
#include "Common/XferLoad.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
void finite(Real value){if(!std::isfinite(value))throw ERROR_BAD_ARG;}
std::size_t size(Int x,Int y){
  if(x<=0 || y<=0 || x>std::numeric_limits<Int>::max()-3 || y>std::numeric_limits<Int>::max()-3)
    throw ERROR_BAD_ARG;
  const auto width=std::size_t(x)+3,height=std::size_t(y)+3;
  if(width>std::vector<NativeWaterGrid::Cell>().max_size()/height)throw ERROR_BAD_ARG;
  return width*height;
}
Int dimension(Real value){finite(value);
  if(value<0 || double(value)>std::numeric_limits<Int>::max()-3)throw ERROR_BAD_ARG;
  return static_cast<Int>(value); // Original truncation, not rounded grid sizes.
}
struct Box{Int x0,x1,y0,y1;};
Box box(Real gx,Real gy,Real range,Int x,Int y){
  return {static_cast<Int>(std::max(0.0,std::floor(double(gx-range)))),
          static_cast<Int>(std::min(double(x),std::ceil(double(gx+range)))),
          static_cast<Int>(std::max(0.0,std::floor(double(gy-range)))),
          static_cast<Int>(std::min(double(y),std::ceil(double(gy+range))))};
}
NativeWaterGrid::Cell step(NativeWaterGrid::Cell value,Real gravity){
  if(value.status&1){value.velocity*=0.93f;
    if(value.height<value.preferredHeight)value.velocity-=gravity*3.0f;
    else value.velocity+=gravity*3.0f;
    value.height+=value.velocity;
    finite(value.velocity);finite(value.height);
    if(std::fabs(value.height-value.preferredHeight)<1.0f && std::fabs(value.velocity)<1.0f){
      value.status &= ~UnsignedByte(1);value.height=value.preferredHeight;value.velocity=0;}
  }
  return value;
}
}
std::size_t NativeWaterGrid::index(Int x,Int y)const noexcept{
  return (std::size_t(y)+1)*(std::size_t(m_x)+3)+std::size_t(x)+1;
}
const NativeWaterGrid::Cell& NativeWaterGrid::vertex(Int x,Int y)const{
  if(x<0 || y<0 || x>m_x || y>m_y || m_cells.empty())throw ERROR_BAD_ARG;
  return m_cells[index(x,y)];
}
void NativeWaterGrid::init(const GlobalData& data){
  NativeWaterGrid candidate;
  candidate.setClamps(data.m_vertexWaterHeightClampLow[0],data.m_vertexWaterHeightClampHi[0]);
  candidate.setTransform(data.m_vertexWaterAngle[0],data.m_vertexWaterXPosition[0],data.m_vertexWaterYPosition[0],data.m_vertexWaterZPosition[0]);
  candidate.setResolution(data.m_vertexWaterXGridCells[0],data.m_vertexWaterYGridCells[0],data.m_vertexWaterGridSize[0]);
  candidate.setAttenuation(data.m_vertexWaterAttenuationA[0],data.m_vertexWaterAttenuationB[0],data.m_vertexWaterAttenuationC[0],data.m_vertexWaterAttenuationRange[0]);
  swap(candidate);
}
void NativeWaterGrid::swap(NativeWaterGrid& other)noexcept{
  using std::swap;
  swap(m_x,other.m_x);swap(m_y,other.m_y);swap(m_spacing,other.m_spacing);
  swap(m_low,other.m_low);swap(m_high,other.m_high);swap(m_a,other.m_a);swap(m_b,other.m_b);swap(m_c,other.m_c);swap(m_range,other.m_range);
  swap(m_originX,other.m_originX);swap(m_originY,other.m_originY);swap(m_axisXX,other.m_axisXX);swap(m_axisXY,other.m_axisXY);swap(m_axisYX,other.m_axisYX);swap(m_axisYY,other.m_axisYY);
  swap(m_transform,other.m_transform);swap(m_enabled,other.m_enabled);swap(m_motion,other.m_motion);swap(m_disableRiver,other.m_disableRiver);swap(m_lastFrame,other.m_lastFrame);
  m_cells.swap(other.m_cells);
}
void NativeWaterGrid::enable(Bool enabled){
  if(enabled){if(m_spacing<=0)throw ERROR_BAD_ARG;
    if(m_cells.empty()){std::vector<Cell> candidate(size(m_x,m_y));m_cells.swap(candidate);m_motion=false;}}
  m_enabled=enabled;m_disableRiver=false;
}
void NativeWaterGrid::reset()noexcept{std::fill(m_cells.begin(),m_cells.end(),Cell{});m_motion=false;}
void NativeWaterGrid::setResolution(Real x,Real y,Real spacing){
  const auto nx=dimension(x),ny=dimension(y);finite(spacing);if(spacing<0)throw ERROR_BAD_ARG;
  if(m_enabled && (nx<=0 || ny<=0 || spacing<=0))throw ERROR_BAD_ARG;
  if((nx!=m_x || ny!=m_y) && !m_cells.empty()){
    if(spacing<=0)throw ERROR_BAD_ARG;
    std::vector<Cell> candidate(size(nx,ny));m_cells.swap(candidate);m_motion=false;m_disableRiver=false;
  }
  m_x=nx;m_y=ny;m_spacing=spacing;
}
void NativeWaterGrid::setClamps(Real low,Real high){finite(low);finite(high);if(low>high)throw ERROR_BAD_ARG;m_low=low;m_high=high;}
void NativeWaterGrid::setAttenuation(Real a,Real b,Real c,Real range){
  finite(a);finite(b);finite(c);finite(range);if(range<0)throw ERROR_BAD_ARG;
  const Real converted=m_spacing>0 ? range/m_spacing : 0;finite(converted);
  if(m_spacing==0 && range!=0)throw ERROR_BAD_ARG;
  m_a=a;m_b=b;m_c=c;m_range=converted;
}
void NativeWaterGrid::setTransform(Real angle,Real x,Real y,Real z){
  finite(angle);finite(x);finite(y);finite(z);
  Matrix3D candidate(1);candidate.Rotate_Z(angle);candidate.Set_Translation(Vector3(x,y,z));
  m_originX=x;m_originY=y;m_axisXX=candidate.Get_X_Vector().X;m_axisXY=candidate.Get_X_Vector().Y;
  m_axisYX=candidate.Get_Y_Vector().X;m_axisYY=candidate.Get_Y_Vector().Y;m_transform=candidate;
}
void NativeWaterGrid::setTransform(const Matrix3D& transform){
  for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column)finite(transform[row][column]);
  m_transform=transform;
}
Bool NativeWaterGrid::worldToGrid(Real x,Real y,Real& gx,Real& gy)const{
  finite(x);finite(y);if(m_spacing<=0 || m_x<=0 || m_y<=0)return false;
  Real dx=x-m_originX,dy=y-m_originY,reciprocal=1.0f/m_spacing;
  Real xx=reciprocal*(dx*m_axisXX+dy*m_axisXY),yy=reciprocal*(dx*m_axisYX+dy*m_axisYY);
  if(!std::isfinite(xx) || !std::isfinite(yy) || xx<0 || yy<0 || xx>m_x-1 || yy>m_y-1)return false;
  gx=xx;gy=yy;return true;
}
Bool NativeWaterGrid::height(Real x,Real y,Real* output)const{
  if(!output)throw ERROR_BAD_ARG;Real gx,gy;
  if(!m_enabled || m_cells.empty() || !worldToGrid(x,y,gx,gy))return false;
  *output=m_cells[index(static_cast<Int>(gx),static_cast<Int>(gy))].height+m_transform.Get_Translation().Z;
  return true;
}
void NativeWaterGrid::changeHeight(Real x,Real y,Real delta){
  finite(delta);Real gx,gy;if(m_cells.empty() || !worldToGrid(x,y,gx,gy))return;
  const auto bounds=box(gx,gy,m_range,m_x,m_y);
  auto changed=[&](Int x,Int y){Real distance=(gx-Real(x))*(gx-Real(x))+(gy-Real(y))*(gy-Real(y));
    distance=static_cast<Real>(std::sqrt(double(distance)));
    Real denominator=m_a+m_b*distance+distance*distance*m_c;
    if(!std::isfinite(denominator) || denominator==0)throw ERROR_BAD_ARG;
    Real value=m_cells[index(x,y)].height+1.0f/denominator*delta;
    if(value<m_low)value=m_low;if(value>m_high)value=m_high;finite(value);return value;};
  for(Int y=bounds.y0;y<=bounds.y1;++y)for(Int x=bounds.x0;x<=bounds.x1;++x)(void)changed(x,y);
  for(Int y=bounds.y0;y<=bounds.y1;++y)for(Int x=bounds.x0;x<=bounds.x1;++x)m_cells[index(x,y)].height=changed(x,y);
}
void NativeWaterGrid::addVelocity(Real x,Real y,Real velocity,Real preferred){
  finite(velocity);finite(preferred);
  const auto truncated=std::trunc(double(preferred));if(truncated<0 || truncated>255)throw ERROR_BAD_ARG;
  if(!m_enabled)return;Real gx,gy;
  if(!worldToGrid(x,y,gx,gy)){m_disableRiver=true;return;}
  const auto bounds=box(gx,gy,m_range,m_x,m_y);
  for(Int y=bounds.y0;y<=bounds.y1;++y)for(Int x=bounds.x0;x<=bounds.x1;++x)finite(m_cells[index(x,y)].velocity+velocity);
  for(Int y=bounds.y0;y<=bounds.y1;++y)for(Int x=bounds.x0;x<=bounds.x1;++x){auto& cell=m_cells[index(x,y)];
    cell.preferredHeight=static_cast<UnsignedByte>(preferred);cell.velocity+=velocity;cell.status|=1;}
  m_disableRiver=true;m_motion=true;
}
void NativeWaterGrid::update(UnsignedInt logicFrame,Real gravity){
  finite(gravity);if(logicFrame==m_lastFrame)return;
  if(m_enabled && m_motion){
    for(const auto& cell:m_cells)(void)step(cell,gravity);
    Bool motion=false;for(auto& cell:m_cells){cell=step(cell,gravity);motion|=(cell.status&1)!=0;}m_motion=motion;
  }
  m_lastFrame=logicFrame;
}
void NativeWaterGrid::xfer(Xfer* transfer){
  if(!transfer || !m_enabled || m_cells.empty())throw SC_INVALID_DATA;
  XferVersion version=1;transfer->xferVersion(&version,1);
  Int x=m_x,y=m_y;transfer->xferInt(&x);transfer->xferInt(&y);
  if(x!=m_x || y!=m_y)throw SC_INVALID_DATA;
  const Bool loading=transfer->getXferMode()==XFER_LOAD;
  std::vector<Cell> candidate;if(loading)candidate=m_cells;
  auto& cells=loading ? candidate : m_cells;Bool motion=false;
  for(auto& cell:cells){transfer->xferReal(&cell.height);transfer->xferReal(&cell.velocity);
    transfer->xferUnsignedByte(&cell.status);transfer->xferUnsignedByte(&cell.preferredHeight);
    if(!std::isfinite(cell.height) || !std::isfinite(cell.velocity) || cell.status>1)throw SC_INVALID_DATA;
    motion|=(cell.status&1)!=0;}
  if(loading){m_cells.swap(candidate);m_motion=motion;}
}
