// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameLogic/NativeTerrainLogic.h"
#include "GameLogic/NativeTerrainLightingState.h"
#include "GameLogic/NativeWaypointGraph.h"
#include "GameClient/NativeTerrainVisual.h"
#include "Common/MapObject.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/Xfer.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include <algorithm>
#include <cmath>
#include <utility>

struct NativeTerrainLogic::Load {
  NativeTerrainLogic& owner;
  ChunkInputStream& stream;
  AsciiString filename;
  Bool query;
  Admission admission;
  void* context;
  NativeTerrainVisual::TerrainState terrain;
  NativeWaypointGraph graph;
  VecICoord2D boundaries;
  Int width=0,height=0,active=0;
  Real minZ=0,maxZ=1;
  GlobalData& configuration;
  NativeTerrainLightingState lighting;
  bool published=false,visualPublished=false,keep=false;
  Load(NativeTerrainLogic& target,ChunkInputStream& source,AsciiString name,
       Bool onlyQuery,Admission callback,void* raw,GlobalData& data)
    :owner(target),stream(source),filename(std::move(name)),query(onlyQuery),
     admission(callback),context(raw),terrain(target.m_visual.heightMap().shadowColor()),
     configuration(data),lighting(data){}
  void exchange()noexcept {
    std::swap(width,owner.m_mapDX);std::swap(height,owner.m_mapDY);
    std::swap(active,owner.m_activeBoundary);boundaries.swap(owner.m_boundaries);
    std::swap(minZ,owner.m_minZ);std::swap(maxZ,owner.m_maxZ);
    filename.swap(owner.m_filenameString);graph.exchange(owner.m_waypointListHead);
  }
  ~Load(){if(!keep){if(published)exchange();if(visualPublished)owner.m_visual.exchangeTerrainState(terrain);lighting.publish(configuration);}}
};
NativeTerrainLogic::~NativeTerrainLogic(){deleteWaypoints();deleteBridges();}
void NativeTerrainLogic::init(){TerrainLogic::init();m_mapDX=m_mapDY=0;m_minZ=0;m_maxZ=1;}
void NativeTerrainLogic::reset(){TerrainLogic::reset();MapObject::WorldState retired;MapObject::exchangeWorldState(retired);m_mapDX=m_mapDY=0;m_minZ=0;m_maxZ=1;}
Bool NativeTerrainLogic::loadMap(AsciiString filename,Bool query){
  if(filename.isEmpty())return FALSE;
  CachedFileInputStream stream;if(!stream.open(filename))return FALSE;
  loadStream(stream,std::move(filename),query);return TRUE;
}
void NativeTerrainLogic::loadStream(ChunkInputStream& stream,AsciiString filename,Bool query,
                                   Admission admission,void* context){
  if(filename.isEmpty() || !TheWritableGlobalData)throw ERROR_BAD_ARG;
  Load load(*this,stream,std::move(filename),query,admission,context,*TheWritableGlobalData);
  m_world.load(stream,admitWorld,&load);
  load.keep=true;
  if(load.visualPublished)m_visual.invalidateTerrainHeights();
}
void NativeTerrainLogic::admitWorld(const NativeTerrainHeightMap& metadata,void* raw){
  auto& load=*static_cast<Load*>(raw);auto& owner=load.owner;
  load.width=metadata.width();load.height=metadata.height();load.boundaries=metadata.boundaries();
  UnsignedByte low=255,high=0;
  for(Int y=0;y<load.height;++y)for(Int x=0;x<load.width;++x){const auto value=metadata.rawHeight(x,y);low=std::min(low,value);high=std::max(high,value);}
  load.minZ=low*MAP_HEIGHT_SCALE;load.maxZ=high*MAP_HEIGHT_SCALE;
  if(!load.stream.absoluteSeek(0))throw ERROR_CORRUPT_FILE_FORMAT;
  load.graph.load(load.stream,MapObject::getFirstMapObject(),&owner.m_visual.heightMap());
  if(!load.query){
    if(!load.stream.absoluteSeek(0))throw ERROR_CORRUPT_FILE_FORMAT;
    load.terrain.filename=load.filename;
    load.terrain.height.loadVisualTerrain(load.stream,load.configuration.m_use3WayTerrainBlends!=0,load.configuration);
    owner.m_visual.exchangeTerrainState(load.terrain);load.visualPublished=true;
  }
  load.exchange();load.published=true;
  if(load.configuration.setTimeOfDay(load.configuration.m_timeOfDay))owner.admitDeviceMap();
  if(load.admission)load.admission(owner,load.context);
}
void NativeTerrainLogic::newMap(Bool saveGame){loadRoadsAndBridges(saveGame);TerrainLogic::newMap(saveGame);}
Real NativeTerrainLogic::getGroundHeight(Real x,Real y,Coord3D* normal)const{return m_visual.heightMap().groundHeight(x,y,normal);}
Bool NativeTerrainLogic::isCliffCell(Real x,Real y)const{return m_visual.heightMap().width() && m_visual.heightMap().isCliffCell(x,y);}
Bool NativeTerrainLogic::isClearLineOfSight(const Coord3D& a,const Coord3D& b)const{
  const auto& map=m_visual.heightMap();
  return map.clearLineOfSight(a,b,map.maximumHeight());
}
Real NativeTerrainLogic::getLayerHeight(Real x,Real y,PathfindLayerEnum layer,Coord3D* normal,Bool clip)const{
  const Real height=getGroundHeight(x,y,normal);
  if(!m_visual.heightMap().width() || layer==LAYER_GROUND)return height;
  Coord3D position{x,y,height};
  if(layer==LAYER_WALL){if(!TheAI || !TheAI->pathfinder())throw ERROR_BAD_ARG;
    return !clip || TheAI->pathfinder()->isPointOnWall(&position) ? TheAI->pathfinder()->getWallHeight() : height;}
  if(auto* bridge=findBridgeLayerAt(&position,layer,clip)){const Real value=bridge->getBridgeHeight(&position,normal);if(value>height)return value;}
  return height;
}
void NativeTerrainLogic::getExtent(Region3D* output)const{
  if(!output)throw ERROR_BAD_ARG;output->lo={0,0,m_minZ};output->hi={0,0,m_maxZ};
  if(!m_boundaries.empty()){if(m_activeBoundary<0 || std::size_t(m_activeBoundary)>=m_boundaries.size())throw ERROR_BAD_ARG;
    output->hi.x=m_boundaries[m_activeBoundary].x*MAP_XY_FACTOR;output->hi.y=m_boundaries[m_activeBoundary].y*MAP_XY_FACTOR;}
}
void NativeTerrainLogic::getMaximumPathfindExtent(Region3D* output)const{
  if(!output)throw ERROR_BAD_ARG;output->lo={0,0,m_minZ};output->hi={0,0,m_maxZ};
  for(const auto& boundary:m_boundaries){output->hi.x=std::max(output->hi.x,boundary.x*MAP_XY_FACTOR);output->hi.y=std::max(output->hi.y,boundary.y*MAP_XY_FACTOR);}
}
void NativeTerrainLogic::getExtentIncludingBorder(Region3D* output)const{
  if(!output)throw ERROR_BAD_ARG;const Real border=m_visual.heightMap().border()*MAP_XY_FACTOR;
  output->lo.x=output->lo.y=-border;output->hi.x=m_mapDX*MAP_XY_FACTOR-border;output->hi.y=m_mapDY*MAP_XY_FACTOR-border;
}
void NativeTerrainLogic::xfer(Xfer* input){XferVersion version=1;input->xferVersion(&version,1);TerrainLogic::xfer(input);}
