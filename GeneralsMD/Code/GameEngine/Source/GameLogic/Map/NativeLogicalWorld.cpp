// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameLogic/NativeLogicalWorld.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/ThingFactory.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/Scripts.h"
#include <cmath>

namespace {
struct Publication {
  MapObject::WorldState objects;
  PolygonTrigger::WorldState triggers;
  ScriptList::ReadState reads;
  SidesList sides;
  SidesList* parent = TheSidesList;
  GlobalData* data = TheWritableGlobalData;
  Weather weather = data->m_weather;
  bool keep = false;
  Publication() {
    // Source omission of WorldInfo preserves the previous Dict/weather.
    Dict inherited = MapObject::TheWorldDict;
    MapObject::exchangeWorldState(objects);
    MapObject::TheWorldDict.swap(inherited);
    PolygonTrigger::exchangeWorldState(triggers);
    ScriptList::exchangeReadLists(reads);
    TheSidesList = &sides;
  }
  ~Publication() noexcept {
    if(!keep) {
      MapObject::exchangeWorldState(objects);
      PolygonTrigger::exchangeWorldState(triggers);
      data->m_weather = weather;
    }
    ScriptList::exchangeReadLists(reads);
    objects.clear();
    reads.clear();
    sides.clear();
    TheSidesList = parent;
  }
};
struct Candidate {
  NativeTerrainHeightMap height;
  unsigned seen = 0;
  void admit(unsigned bit) {
    if(seen & bit) throw ERROR_CORRUPT_FILE_FORMAT;
    seen |= bit;
  }
};
Bool height(DataChunkInput& file, DataChunkInfo* info, void* context) {
  auto& candidate = *static_cast<Candidate*>(context);
  candidate.admit(1);
  candidate.height.readHeightChunk(file,info->version,NativeTerrainHeightMap::Purpose::LogicalMetadata);
  return true;
}
Bool world(DataChunkInput& file, DataChunkInfo*, void* context) {
  static_cast<Candidate*>(context)->admit(2);
  Dict dict = file.readDict();
  if(!file.atEndOfChunk()) throw ERROR_CORRUPT_FILE_FORMAT;
  Bool exists;
  const Int weather = dict.getInt(TheKey_weather,&exists);
  if(exists) {
    if(weather<WEATHER_NORMAL || weather>=WEATHER_COUNT) throw ERROR_CORRUPT_FILE_FORMAT;
    TheWritableGlobalData->m_weather = static_cast<Weather>(weather);
  }
  MapObject::TheWorldDict.swap(dict);
  return true;
}
Bool object(DataChunkInput& file, DataChunkInfo* info, void*) {
  if(info->version<1 || info->version>3) throw ERROR_CORRUPT_FILE_FORMAT;
  Coord3D location{file.readReal(),file.readReal(),file.readReal()};
  const Real angle = file.readReal();
  const Int flags = file.readInt();
  const AsciiString name = file.readAsciiString();
  Dict dict;
  if(info->version>=2) dict = file.readDict();
  if(!file.atEndOfChunk() || !std::isfinite(location.x) || !std::isfinite(location.y) ||
     !std::isfinite(location.z) || !std::isfinite(angle)) throw ERROR_CORRUPT_FILE_FORMAT;
  if(info->version<=2) location.z=0;
  if(location.z < -100*MAP_XY_FACTOR || location.z > (255*10)*MAP_HEIGHT_SCALE) return true;
  auto* value = newInstance(MapObject)(location,name,angle,flags,&dict,TheThingFactory->findTemplate(name,FALSE));
  MemoryPoolObjectHolder owner(value);
  // WellKnownKey lookup can still admit a namespace entry before linkage.
  if(dict.getType(TheKey_waypointID)==Dict::DICT_INT) value->setIsWaypoint();
  if(dict.getType(TheKey_lightHeightAboveTerrain)==Dict::DICT_REAL) value->setIsLight();
  if(dict.getType(TheKey_scorchType)==Dict::DICT_INT) value->setIsScorch();
  auto* previous = static_cast<MapObject*>(file.m_currentObject);
  if(previous) previous->setNextMap(value);
  else MapObject::TheMapObjectListPtr=value;
  file.m_currentObject=value;
  owner.release();
  return true;
}
Bool objects(DataChunkInput& file, DataChunkInfo* info, void* context) {
  static_cast<Candidate*>(context)->admit(4);
  file.m_currentObject=nullptr;
  file.registerParser("Object",info->label,object);
  return file.parse(context);
}
Bool sides(DataChunkInput& file, DataChunkInfo* info, void* context) {
  static_cast<Candidate*>(context)->admit(8);
  return SidesList::ParseSidesDataChunk(file,info,context);
}
Bool triggers(DataChunkInput& file, DataChunkInfo* info, void* context) {
  static_cast<Candidate*>(context)->admit(16);
  return PolygonTrigger::ParsePolygonTriggersDataChunk(file,info,context);
}
}

void NativeLogicalWorld::load(ChunkInputStream& stream) {
  if(!TheNameKeyGenerator || !TheSidesList || !TheWritableGlobalData || !TheThingFactory)
    throw ERROR_BAD_ARG;
  NameKeyTransaction names(*TheNameKeyGenerator);
  ScriptList::IDTransaction ids;
  Publication publication;
  Candidate candidate;
  DataChunkInput file(&stream);
  file.registerParser("HeightMapData",AsciiString::TheEmptyString,::height);
  file.registerParser("WorldInfo",AsciiString::TheEmptyString,world);
  file.registerParser("ObjectsList",AsciiString::TheEmptyString,objects);
  file.registerParser("SidesList",AsciiString::TheEmptyString,sides);
  file.registerParser("PolygonTriggers",AsciiString::TheEmptyString,triggers);
  if(!file.isValidFileType() || !file.parse(&candidate) || !stream.eof() || !(candidate.seen & 1)) throw ERROR_CORRUPT_FILE_FORMAT;
  // Source logical construction empties sides only. A present SidesList chunk
  // clears teams itself, but an omitted chunk preserves both prior team sets.
  if(!(candidate.seen & 8)) publication.sides.copyTeamsFrom(*publication.parent);
  publication.sides.validateSides();
  publication.parent->swapContents(publication.sides);
  m_height.swap(candidate.height);
  publication.keep=true;
  ids.commit();
  names.commit();
}
