// SPDX-License-Identifier: GPL-3.0-or-later
// Original TerrainLogic Waypoint constructor and graph behavior, shared by the
// original runtime and native staged admission. No replacement simulation.
#include "PreRTS.h"
#include "GameLogic/NativeWaypointGraph.h"
#include "GameLogic/NativeTerrainHeightMap.h"
#include "GameLogic/TerrainLogic.h"
#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include <cmath>
#include <utility>

Waypoint::Waypoint(WaypointID id,AsciiString name,const Coord3D* location,
                   AsciiString label1,AsciiString label2,AsciiString label3,Bool bidirectional):
  m_id(id),m_name(name),m_location(*location),m_pNext(nullptr),m_links{},m_numLinks(0),
  m_pathLabel1(label1),m_pathLabel2(label2),m_pathLabel3(label3),m_biDirectional(bidirectional) {}
Waypoint::~Waypoint() {}

NativeWaypointGraph::~NativeWaypointGraph()noexcept { clear(m_head); }
void NativeWaypointGraph::swap(NativeWaypointGraph& other)noexcept { std::swap(m_head,other.m_head); }
void NativeWaypointGraph::exchange(Waypoint*& external)noexcept { std::swap(m_head,external); }
void NativeWaypointGraph::clear(Waypoint*& head)noexcept {
  while(head){auto* node=head;head=node->getNext();node->setNext(nullptr);node->deleteInstance();}
}
void NativeWaypointGraph::prepend(Waypoint*& head,MapObject* object,Real height) {
  if(!object || !object->isWaypoint() || !std::isfinite(height))throw ERROR_BAD_ARG;
  Coord3D location=*object->getLocation();location.z=height;
  Bool exists;
  const auto label1=object->getProperties()->getAsciiString(TheKey_waypointPathLabel1,&exists);
  const auto label2=object->getProperties()->getAsciiString(TheKey_waypointPathLabel2,&exists);
  const auto label3=object->getProperties()->getAsciiString(TheKey_waypointPathLabel3,&exists);
  const Bool bidirectional=object->getProperties()->getBool(TheKey_waypointPathBiDirectional,&exists);
  auto* node=newInstance(Waypoint)(object->getWaypointID(),object->getWaypointName(),&location,label1,label2,label3,bidirectional);
  node->setNext(head);head=node;
}
void NativeWaypointGraph::link(Waypoint* head,Int source,Int target)noexcept {
  Waypoint *from=nullptr,*to=nullptr;
  for(auto* node=head;node;node=node->getNext()){
    if(node->getID()==static_cast<UnsignedInt>(source))from=node;
    if(node->getID()==static_cast<UnsignedInt>(target))to=node;
  }
  if(!from || !to || from==to)return;
  for(Int n=0;n<from->getNumLinks();++n)if(from->getLink(n)==to)return;
  from->addLink(to);
  if(from->getBiDirectional()){
    for(Int n=0;n<to->getNumLinks();++n)if(to->getLink(n)==from)return;
    to->addLink(from);
  }
}
Bool NativeWaypointGraph::readLinks(Waypoint* head,DataChunkInput& input) {
  const Int count=input.readInt();
  if(count<0 || static_cast<UnsignedInt>(count)>input.getChunkDataSizeLeft()/8)
    throw ERROR_CORRUPT_FILE_FORMAT;
  for(Int n=0;n<count;++n){const Int from=input.readInt(),to=input.readInt();link(head,from,to);}
  if(!input.atEndOfChunk())throw ERROR_CORRUPT_FILE_FORMAT;
  return TRUE;
}
namespace {
struct Read { Waypoint* head;bool seen=false; };
Bool links(DataChunkInput& input,DataChunkInfo*,void* raw) {
  auto& read=*static_cast<Read*>(raw);if(read.seen)throw ERROR_CORRUPT_FILE_FORMAT;
  read.seen=true;return NativeWaypointGraph::readLinks(read.head,input);
}
}
void NativeWaypointGraph::load(ChunkInputStream& stream,MapObject* objects,
                               const NativeTerrainHeightMap* initialHeight) {
  NativeWaypointGraph candidate;
  for(auto* object=objects;object;object=object->getNext())if(object->isWaypoint()){
    const auto* position=object->getLocation();
    prepend(candidate.m_head,object,initialHeight ? initialHeight->groundHeight(position->x,position->y) : 0);
  }
  DataChunkInput input(&stream);Read read{candidate.m_head};
  input.registerParser("WaypointsList",AsciiString::TheEmptyString,links);
  if(!input.parse(&read) || !stream.eof())throw ERROR_CORRUPT_FILE_FORMAT;
  swap(candidate);
}
void NativeWaypointGraph::resnap(const NativeTerrainHeightMap& height) {
  // Validate all arithmetic before changing any accepted waypoint height.
  for(auto* node=m_head;node;node=node->getNext()){
    const auto* p=node->getLocation();(void)height.groundHeight(p->x,p->y);
  }
  for(auto* node=m_head;node;node=node->getNext()){
    const auto* p=node->getLocation();node->setLocationZ(height.groundHeight(p->x,p->y));
  }
}
