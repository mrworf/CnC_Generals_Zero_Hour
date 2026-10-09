/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: MapUtil.h /////////////////////////////////////////////////////////
// Author: Matt Campbell, December 2001
// Description: Map utility/convenience routines
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __MAPUTIL_H__
#define __MAPUTIL_H__

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"

#include "Common/STLTypedefs.h"

class GameWindow;
class GameInfo;
class AsciiString;
struct Coord3D;
struct FileInfo;
class Image;
class DataChunkInput;
class NativeUserStorage;
struct DataChunkInfo;
// This matches the windows timestamp.
enum { SUPPLY_TECH_SIZE = 15};
typedef std::list <ICoord2D> ICoord2DList;

class TechAndSupplyImages
{
public:
	ICoord2DList m_techPosList;
	ICoord2DList m_supplyPosList;
};

struct WinTimeStamp
{
	UnsignedInt m_lowTimeStamp;
	UnsignedInt m_highTimeStamp;
};


class WaypointMap : public std::map<AsciiString, Coord3D>
{
public:
	void update( void );	///< returns the number of multiplayer start spots found
	Int m_numStartSpots = 1;
};

typedef std::list <Coord3D> Coord3DList;

class MapMetaData
{
public:
  void swap(MapMetaData& other) noexcept {
    m_displayName.swap(other.m_displayName);m_nameLookupTag.swap(other.m_nameLookupTag);
    std::swap(m_extent,other.m_extent);std::swap(m_numPlayers,other.m_numPlayers);
    std::swap(m_isMultiplayer,other.m_isMultiplayer);std::swap(m_isOfficial,other.m_isOfficial);
    std::swap(m_filesize,other.m_filesize);std::swap(m_CRC,other.m_CRC);
    std::swap(m_timestamp,other.m_timestamp);m_waypoints.swap(other.m_waypoints);
    std::swap(m_waypoints.m_numStartSpots,other.m_waypoints.m_numStartSpots);
    m_supplyPositions.swap(other.m_supplyPositions);m_techPositions.swap(other.m_techPositions);
    m_fileName.swap(other.m_fileName);
  }
	UnicodeString m_displayName;
	AsciiString m_nameLookupTag;
	Region3D m_extent{};
	Int m_numPlayers = 0;
	Bool m_isMultiplayer = FALSE;

	Bool m_isOfficial = FALSE;
	UnsignedInt m_filesize = 0;
	UnsignedInt m_CRC = 0;

	WinTimeStamp m_timestamp{};

	WaypointMap m_waypoints;
	Coord3DList m_supplyPositions;
	Coord3DList m_techPositions;
	AsciiString m_fileName;
};

class MapCache : public std::map<AsciiString, MapMetaData>
{
public:
	MapCache() {}
  // Complete source-format output is prepared before any persistent write.
  std::string serializeCacheINI(const AsciiString& mapDir) const;
  static Bool persistCacheINI(const NativeUserStorage& storage,const std::string& serialized);
  // Optional derived cache only: actual MapCache blocks, whole-file publication.
  Bool loadCacheINI(AsciiString filename);
  void publishMetadata(AsciiString name,MapMetaData candidate) {
    auto entry=find(name);
    if(entry!=end()) entry->second.swap(candidate);
    else emplace(std::move(name),std::move(candidate));
  }
	void updateCache( void );

	AsciiString getMapDir() const;
	AsciiString getUserMapDir() const;
	AsciiString getMapExtension() const;

	const MapMetaData *findMap(AsciiString mapName);

	// allow us to create a set of shippable maps to be in mapcache.ini.  For use with -buildMapCache.
	void addShippingMap(AsciiString mapName) { mapName.toLower(); m_allowedMaps.insert(mapName); }

private:
	Bool clearUnseenMaps( AsciiString dirName );
	void loadStandardMaps(void);
	Bool loadUserMaps(void);				// returns true if we needed to (re)parse a map
//	Bool addMap( AsciiString dirName, AsciiString fname, WinTimeStamp timestamp,
//		UnsignedInt filesize, Bool isOfficial );	///< returns true if it had to (re)parse the map
	Bool addMap( AsciiString dirName, AsciiString fname, FileInfo *fileInfo, Bool isOfficial); ///< returns true if it had to (re)parse the map

	static const char * m_mapCacheName;
	std::map<AsciiString, Bool> m_seen;

	std::set<AsciiString> m_allowedMaps;
};

extern MapCache *TheMapCache;
extern TechAndSupplyImages TheSupplyAndTechImageLocations;
void positionAdditionalImages(MapMetaData*,GameWindow*,Bool force);
void positionStartSpots(AsciiString,GameWindow*[],GameWindow*);
void positionStartSpots(GameInfo*,GameWindow*[],GameWindow*);
void updateMapStartSpots(GameInfo*,GameWindow*[],Bool loadScreen);
Int populateMapListbox( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect = AsciiString::TheEmptyString );		/// Read a list of maps from the run directory and fill in the listbox.  Return the selected index
Int populateMapListboxNoReset( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect = AsciiString::TheEmptyString );		/// Read a list of maps from the run directory and fill in the listbox.  Return the selected index
Bool isValidMap( AsciiString mapName, Bool isMultiplayer );						/// Validate a map
Image *getMapPreviewImage( AsciiString mapName );
AsciiString getDefaultMap( Bool isMultiplayer );											/// Find a valid map
AsciiString getDefaultOfficialMap();
Bool isOfficialMap( AsciiString mapName );
Bool parseMapPreviewChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
void findDrawPositions( Int startX, Int startY, Int width, Int height, Region3D extent,
															 ICoord2D *ul, ICoord2D *lr );
Bool WouldMapTransfer( const AsciiString& mapName );
#endif // __MAPUTIL_H__
