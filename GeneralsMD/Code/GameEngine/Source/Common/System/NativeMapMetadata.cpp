// SPDX-License-Identifier: GPL-3.0-or-later
#include "GameClient/MapUtil.h"
#include "Common/QuotedPrintable.h"
#include "Common/Errors.h"
#include "Common/NativeUserStorage.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/FileSystem.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameText.h"
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

// This is the actual source store used by INI and runtime queries. Its metadata
// storage/serialization do not depend on the presentation provider.
MapCache* TheMapCache=nullptr;

namespace {
template<class... Args>
void append(std::string& output,const char* format,Args... args) {
  const int count=std::snprintf(nullptr,0,format,args...);
  if(count<0 || output.size()>std::size_t(INT32_MAX) ||
      std::size_t(count)>std::size_t(INT32_MAX)-output.size()) throw ERROR_BAD_ARG;
  std::vector<char> bytes(std::size_t(count)+1);
  if(std::snprintf(bytes.data(),bytes.size(),format,args...)!=count) throw ERROR_BAD_ARG;
  output.append(bytes.data(),std::size_t(count));
}
}
std::string MapCache::serializeCacheINI(const AsciiString& mapDir) const {
  std::string output;
  append(output,"; FILE: %s\\MapCache.ini /////////////////////////////////////////////////////////////\n",mapDir.str());
  output+="; This INI file is auto-generated - do not modify\n";
  output+="; /////////////////////////////////////////////////////////////////////////////\n";
  for(const auto& [name,md]:*this) {
    if(!name.startsWithNoCase(mapDir.str())) continue;
    append(output,"\nMapCache %s\n",AsciiStringToQuotedPrintable(name.str()).str());
    append(output,"  fileSize = %u\n",md.m_filesize);
    append(output,"  fileCRC = %u\n",md.m_CRC);
    // Preserve the source signed decimal spelling of the two timestamp words.
    append(output,"  timestampLo = %d\n",static_cast<std::int32_t>(md.m_timestamp.m_lowTimeStamp));
    append(output,"  timestampHi = %d\n",static_cast<std::int32_t>(md.m_timestamp.m_highTimeStamp));
    append(output,"  isOfficial = %s\n",md.m_isOfficial?"yes":"no");
    append(output,"  isMultiplayer = %s\n",md.m_isMultiplayer?"yes":"no");
    append(output,"  numPlayers = %d\n",md.m_numPlayers);
    append(output,"  extentMin = X:%2.2f Y:%2.2f Z:%2.2f\n",md.m_extent.lo.x,md.m_extent.lo.y,md.m_extent.lo.z);
    append(output,"  extentMax = X:%2.2f Y:%2.2f Z:%2.2f\n",md.m_extent.hi.x,md.m_extent.hi.y,md.m_extent.hi.z);
    append(output,"  nameLookupTag = %s\n",md.m_nameLookupTag.str());
    for(const auto& [waypoint,pos]:md.m_waypoints)
      append(output,"  %s = X:%2.2f Y:%2.2f Z:%2.2f\n",waypoint.str(),pos.x,pos.y,pos.z);
    for(const auto& pos:md.m_techPositions)
      append(output,"  techPosition = X:%2.2f Y:%2.2f Z:%2.2f\n",pos.x,pos.y,pos.z);
    for(const auto& pos:md.m_supplyPositions)
      append(output,"  supplyPosition = X:%2.2f Y:%2.2f Z:%2.2f\n",pos.x,pos.y,pos.z);
    output+="END\n\n";
  }
  if(output.size()>std::size_t(INT32_MAX)) throw ERROR_BAD_ARG;
  return output;
}
Bool MapCache::persistCacheINI(const NativeUserStorage& storage,const std::string& serialized) {
  if(serialized.size()>std::size_t(INT32_MAX)) throw ERROR_BAD_ARG;
  try {
    auto output=storage.beginWrite(NativeUserArea::Data,"Maps/MapCache.ini");
    output->write(serialized.data(),static_cast<Int>(serialized.size()));
    output->commit();
    return TRUE;
  } catch(const NativeStorageError&) {
    // This derived cache is optional. bad_alloc and unexpected callbacks remain
    // failures; only unavailable persistence is an in-memory fallback.
    return FALSE;
  }
}
Bool MapCache::loadCacheINI(AsciiString filename) {
  if(TheMapCache!=this || !TheFileSystem || !TheNameKeyGenerator || !TheGameText) throw ERROR_BAD_ARG;
  NameKeyTransaction keys(*TheNameKeyGenerator);
  MapCache candidate(*this);
  struct Publication {
    MapCache* prior;
    explicit Publication(MapCache& value):prior(TheMapCache) {TheMapCache=&value;}
    ~Publication() {TheMapCache=prior;}
  } publication(candidate);
  const INIBlockDefinition blocks[]{{"MapCache",INI::parseMapCacheDefinition}};
  try {
    INI ini;ini.loadBlocks(filename,INI_LOAD_OVERWRITE,blocks);
  } catch(const INIException&) {return FALSE;}
  catch(const NativeStorageError&) {return FALSE;}
  catch(ErrorCode error) {
    if(error==ERROR_BAD_INI || error==ERROR_CORRUPT_FILE_FORMAT) return FALSE;
    throw;
  }
  // No callbacks/allocation after complete admission. Bookkeeping is unchanged:
  // a cache file owns metadata, not the discovery/allowed-map journals.
  std::map<AsciiString,MapMetaData>::swap(candidate);
  keys.commit();
  return TRUE;
}

// Original logical lookup, independent of terrain/presentation ownership.
const MapMetaData *MapCache::findMap(AsciiString mapName)
{
	mapName.toLower();
	MapCache::iterator it = find(mapName);
	if (it == end())
		return NULL;
	return &(it->second);
}
