// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeMapPreviewLayout.h"
#include "GameNetwork/GameInfo.h"
#include <cmath>
#include <cstdint>
#include <limits>

namespace {
Bool integer(Real value,Int& output) noexcept {
  if(!std::isfinite(value) || double(value)<std::numeric_limits<Int>::min() ||
      double(value)>std::numeric_limits<Int>::max()) return FALSE;
  output=static_cast<Int>(value);return TRUE;
}
Bool add(Int a,Int b,Int& output) noexcept {
  const auto value=std::int64_t(a)+b;
  if(value<std::numeric_limits<Int>::min() || value>std::numeric_limits<Int>::max())return FALSE;
  output=static_cast<Int>(value);return TRUE;
}
Bool validExtent(Region3D extent) noexcept {
  return std::isfinite(extent.lo.x) && std::isfinite(extent.lo.y) &&
    std::isfinite(extent.hi.x) && std::isfinite(extent.hi.y) &&
    std::isfinite(extent.width()) && std::isfinite(extent.height()) &&
    extent.width()>0 && extent.height()>0;
}
ICoord2D project(const Coord3D& point,Region3D extent,ICoord2D ul,ICoord2D lr,ICoord2D size) {
  ICoord2D result;
  const Real x=(point.x-extent.lo.x)/extent.width();
  const Real y=(point.y-extent.lo.y)/extent.height();
  if(size.x<0 || size.y<0 || !integer(x*Real(lr.x-ul.x)-size.x/2+ul.x,result.x) ||
      !integer((1-y)*Real(lr.y-ul.y)-size.y/2+ul.y,result.y))throw ERROR_BAD_ARG;
  Int end;
  if(!add(result.x,size.x,end) || !add(result.y,size.y,end))throw ERROR_BAD_ARG;
  return result;
}
}

Bool nativeMapDrawPositions(Int x,Int y,Int width,Int height,Region3D extent,
    ICoord2D& ul,ICoord2D& lr) noexcept {
  if(&ul==&lr || width<=0 || height<=0 || !validExtent(extent))return FALSE;
  const Real ratioWidth=extent.width()/Real(width),ratioHeight=extent.height()/Real(height);
  const Real ratio=ratioWidth>=ratioHeight?ratioWidth:ratioHeight;
  if(!std::isfinite(ratio) || ratio<=0)return FALSE;
  const Real radarX=extent.width()/ratio,radarY=extent.height()/ratio;
  ICoord2D first{},last{};
  if(ratioWidth>=ratioHeight){
    if(!integer((Real(height)-radarY)/2,first.y) || !integer(radarX,last.x) ||
        !add(height,-first.y,last.y))return FALSE;
  }else{
    if(!integer((Real(width)-radarX)/2,first.x) || !integer(radarY,last.y) ||
        !add(width,-first.x,last.x))return FALSE;
  }
  if(!add(first.x,x,first.x) || !add(first.y,y,first.y) ||
      !add(last.x,x,last.x) || !add(last.y,y,last.y))return FALSE;
  ul=first;lr=last;return TRUE;
}

void findDrawPositions(Int x,Int y,Int width,Int height,Region3D extent,ICoord2D* ul,ICoord2D* lr){
  if(!ul || !lr || ul==lr || !nativeMapDrawPositions(x,y,width,height,extent,*ul,*lr))throw ERROR_BAD_ARG;
}

void NativeMapPreviewLayout::rebuild(const MapMetaData& metadata,Int width,Int height,
    const std::array<NativeMapPreviewControl,MAX_SLOTS>& controls){
  ICoord2D ul,lr;
  if(metadata.m_numPlayers<0 || metadata.m_numPlayers>MAX_SLOTS ||
      !nativeMapDrawPositions(0,0,width,height,metadata.m_extent,ul,lr))throw ERROR_BAD_ARG;
  NativeMapPreviewLayout candidate;
  for(Int i=0;i<metadata.m_numPlayers && metadata.m_isMultiplayer;++i){
    AsciiString name;name.format("Player_%d_Start",i+1);
    const auto waypoint=metadata.m_waypoints.find(name);
    if(waypoint==metadata.m_waypoints.end())throw ERROR_BAD_ARG;
    if(!controls[i].present)continue;
    auto position=project(waypoint->second,metadata.m_extent,ul,lr,controls[i].size);
    for(Int prior=0;prior<i;++prior){
      if(!candidate.m_starts[prior].visible)continue;
      const auto previous=candidate.m_starts[prior].position;
      Int right,bottom;
      if(!add(previous.x,controls[prior].size.x,right) ||
          !add(previous.y,controls[prior].size.y,bottom))throw ERROR_BAD_ARG;
      if(position.x>previous.x && position.x<right && position.y>previous.y && position.y<bottom){
        const auto dx=std::int64_t(right)-position.x,dy=std::int64_t(bottom)-position.y;
        if(dx<=dy && !add(right,1,position.x))throw ERROR_BAD_ARG;
        if(dy<=dx && !add(bottom,1,position.y))throw ERROR_BAD_ARG;
      }
    }
    Int end;
    if(!add(position.x,controls[i].size.x,end) || !add(position.y,controls[i].size.y,end))throw ERROR_BAD_ARG;
    candidate.m_starts[i]={TRUE,position};
  }
  for(const auto& point:metadata.m_supplyPositions)
    candidate.m_markers.m_supplyPosList.push_front(project(point,metadata.m_extent,ul,lr,{SUPPLY_TECH_SIZE,SUPPLY_TECH_SIZE}));
  for(const auto& point:metadata.m_techPositions)
    candidate.m_markers.m_techPosList.push_front(project(point,metadata.m_extent,ul,lr,{SUPPLY_TECH_SIZE,SUPPLY_TECH_SIZE}));
  m_starts=candidate.m_starts;
  candidate.publishMarkers(m_markers);
}

std::array<Int,MAX_SLOTS> nativeMapPreviewLabels(const GameInfo& game,Int players,
    Bool loadScreen,const std::array<NativeMapPreviewControl,MAX_SLOTS>& controls){
  if(players<0 || players>MAX_SLOTS)throw ERROR_BAD_ARG;
  std::array<Int,MAX_SLOTS> output;output.fill(-1);
  for(Int i=0;i<MAX_SLOTS;++i){
    const auto* slot=game.getConstSlot(i);if(!slot)continue;
    const Int destination=loadScreen?slot->getApparentStartPos():slot->getStartPos();
    if(destination>=0 && destination<players && controls[destination].present &&
        slot->getPlayerTemplate()>PLAYERTEMPLATE_MIN)output[destination]=i;
  }
  return output;
}
