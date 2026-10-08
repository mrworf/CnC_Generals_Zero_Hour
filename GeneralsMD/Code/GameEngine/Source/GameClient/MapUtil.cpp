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

// FILE: MapUtil.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, December 2001
// Description: Map utility/convenience routines
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/crc.h"
#include "Common/FileSystem.h"
#include "Common/NativeSourceStrings.h"
#include "Common/NativeUserStorage.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"
#include "Common/FileOwner.h"
#include <cmath>
#include <cstdint>
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "Common/DataChunk.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/MessageStream.h"
#include "Common/WellKnownKeys.h"
#include "Common/INI.h"
#include "Common/QuotedPrintable.h"
#include "Common/SkirmishBattleHonors.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/MapObject.h" // Authoritative MAP_XY_FACTOR; no MapObject acquisition.
#include "GameClient/GameText.h" 
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Image.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/FPUControl.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/NetworkDefs.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static const char* mapExtension=".map";
struct MapQueryContext {
  Int width=0,height=0,border=0,mapDX=0,mapDY=0;
  bool sawHeight=false;
  std::vector<ICoord2D> boundaries;
  Dict world{0};
  WaypointMap waypoints;
  Coord3DList supply,tech;
};
// Metadata parsing is synchronous on the owning simulation thread. Nested
// source queries keep their own offside graph and restore the borrowed context.
static thread_local MapQueryContext* activeQuery=nullptr;
struct MapQueryScope {
  MapQueryContext* prior;
  explicit MapQueryScope(MapQueryContext& candidate):prior(activeQuery) {activeQuery=&candidate;}
  ~MapQueryScope() {activeQuery=prior;}
};
static MapQueryContext& queryState() {
  if(!activeQuery) throw ERROR_BAD_ARG;
  return *activeQuery;
}
static WaypointMap* queryWaypoints() noexcept {
  return activeQuery?&activeQuery->waypoints:nullptr;
}

static UnsignedInt calcCRC( AsciiString dirName, AsciiString fname )
{
	CRC theCRC;
	theCRC.clear();

	// Try the official map dir
	AsciiString asciiFile;

	FileCloseOwner fp(TheFileSystem->openFile(fname.str(),File::READ));
	if( !fp )
	{
    throw ERROR_CORRUPT_FILE_FORMAT;
	}

	UnsignedByte buf[4096];
	Int num;
	while ( (num=fp->read(buf, 4096)) > 0 )
	{
		theCRC.computeCRC(buf, num);
	}

  if(num<0) throw ERROR_CORRUPT_FILE_FORMAT;
	return theCRC.get();
}

static Bool ParseObjectDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Bool readDict = info->version >= K_OBJECTS_VERSION_2;

	Coord3D loc;
	loc.x = file.readReal();
	loc.y = file.readReal();
	loc.z = file.readReal();
	if (info->version <= K_OBJECTS_VERSION_2) 
	{
		loc.z = 0;
	}

	Real angle = file.readReal();
	Int flags = file.readInt(); 
	AsciiString name = file.readAsciiString();
	Dict d;
	if (readDict)
	{
		d = file.readDict();
	}
  if(!TheThingFactory) throw ERROR_BAD_ARG;
  // Preserve source lookup for every record and final override classification.
  // Metadata never observes MapObject's angle/render/shadow/bridge state.
  const ThingTemplate* definition=TheThingFactory->findTemplate(name,FALSE);
  if(definition) definition=static_cast<const ThingTemplate*>(definition->getFinalOverride());
  if(!std::isfinite(loc.x) || !std::isfinite(loc.y) || !std::isfinite(loc.z) || !std::isfinite(angle))
    throw ERROR_CORRUPT_FILE_FORMAT;
  if(d.getType(TheKey_waypointID)==Dict::DICT_INT)
    queryState().waypoints[d.getAsciiString(TheKey_waypointName)]=loc;
  else if(definition && definition->isKindOf(KINDOF_TECH_BUILDING))
    queryState().tech.push_back(loc);
  else if(definition && definition->isKindOf(KINDOF_SUPPLY_SOURCE_ON_PREVIEW))
    queryState().supply.push_back(loc);
  (void)flags; // Only the unobserved MapObject presentation record stored these.

	return TRUE;
}

static Bool ParseObjectsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	file.m_currentObject = NULL;
	file.registerParser( AsciiString("Object"), info->label, ParseObjectDataChunk );
	return (file.parse(userData));
}

static Bool ParseWorldDictDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	queryState().world = file.readDict();
	return true;
}

static Bool ParseSizeOnly(DataChunkInput& file,DataChunkInfo* info,void*)
{
  auto& state=queryState();
  const Int width=file.readInt(),height=file.readInt();
  const Int border=info->version>=K_HEIGHT_MAP_VERSION_3?file.readInt():0;
  if(width<=0 || height<=0 || border<0 || std::int64_t(border)*2>width ||
      std::int64_t(border)*2>height) throw ERROR_CORRUPT_FILE_FORMAT;
  std::vector<ICoord2D> boundaries;
  if(info->version>=K_HEIGHT_MAP_VERSION_4) {
    const Int count=file.readInt();
    if(count<0 || std::uint64_t(count)>file.getChunkDataSizeLeft()/8) throw ERROR_CORRUPT_FILE_FORMAT;
    boundaries.resize(static_cast<std::size_t>(count));
    for(auto& point:boundaries) {point.x=file.readInt();point.y=file.readInt();}
  }
  state.width=width;state.height=height;state.border=border;
  state.boundaries.swap(boundaries);state.sawHeight=true;
  // The source metadata parser returned here; height payload/resampling below
  // its old return was unreachable and is not a metadata conversion contract.
  return TRUE;
}

static Bool ParseSizeOnlyInChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	return ParseSizeOnly(file, info, userData);
}

static Bool loadMap( AsciiString filename )
{
	AsciiString asciiFile;

	CachedFileInputStream fileStrm;

	asciiFile = filename;
	if( !fileStrm.open(asciiFile) )
	{
		return FALSE;
	}

	ChunkInputStream *pStrm = &fileStrm;

	DataChunkInput file( pStrm );


	file.registerParser( AsciiString("HeightMapData"), AsciiString::TheEmptyString, ParseSizeOnlyInChunk );
	file.registerParser( AsciiString("WorldInfo"), AsciiString::TheEmptyString, ParseWorldDictDataChunk );
	file.registerParser( AsciiString("ObjectsList"), AsciiString::TheEmptyString, ParseObjectsDataChunk );
	if (!file.parse(NULL)) {
		throw(ERROR_CORRUPT_FILE_FORMAT);
	}

  if(!queryState().sawHeight) throw ERROR_CORRUPT_FILE_FORMAT;
	queryState().mapDX = queryState().width  - 2*queryState().border;
	queryState().mapDY = queryState().height - 2*queryState().border;

	return TRUE;
}

static void getExtent( Region3D *extent )
{
	extent->lo.x = 0.0f;

	extent->lo.y = 0.0f;

	// Note - queryState().mapDX & Y are the number of height map grids wide, so we have to
	// multiply by the grid width.
	extent->hi.x = queryState().mapDX*MAP_XY_FACTOR;
	extent->hi.y = queryState().mapDY*MAP_XY_FACTOR;

	extent->lo.z = 0;
	extent->hi.z = 0;
}

//-------------------------------------------------------------------------------

void WaypointMap::update( void )
{
	if (!queryWaypoints())
	{
		m_numStartSpots = 1;
		return;
	}

	this->clear();

	AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);
	WaypointMap::const_iterator it;

	it = queryWaypoints()->find(startingCamName);
	if (it != queryWaypoints()->end())
	{
		(*this)[startingCamName] = it->second;
	}

	m_numStartSpots = 0;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		startingCamName.format("Player_%d_Start", i+1); // start pos waypoints are 1-based
		it = queryWaypoints()->find(startingCamName);
		if (it != queryWaypoints()->end())
		{
			(*this)[startingCamName] = it->second;
			++m_numStartSpots;
		}
		else
		{
			break;
		}
	}

	m_numStartSpots = std::max(1, m_numStartSpots);
}

const char * MapCache::m_mapCacheName = "MapCache.ini";

AsciiString MapCache::getMapDir() const 
{ 
	return AsciiString("Maps"); 
}

AsciiString MapCache::getUserMapDir() const
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat(getMapDir());
	return tmp;
}

AsciiString MapCache::getMapExtension() const
{
	return AsciiString("map");
}

void MapCache::updateCache( void )
{
  if(!TheFileSystem || !TheGlobalData || !TheGameText || !TheNativeUserStorage || !TheNameKeyGenerator ||
      TheMapCache!=this) throw ERROR_BAD_ARG;
  setFPMode();
  NameKeyTransaction keys(*TheNameKeyGenerator);
  // All callbacks target the offside owner until every required parser returns.
  MapCache candidate(*this);
  struct Publication {
    MapCache* prior;
    explicit Publication(MapCache& cache):prior(TheMapCache) {TheMapCache=&cache;}
    ~Publication() {TheMapCache=prior;}
  } publication(candidate);
  TheFileSystem->createDirectory(candidate.getUserMapDir());
  const bool changed=candidate.loadUserMaps();
  std::string serialized;
  if(changed) serialized=candidate.serializeCacheINI(candidate.getUserMapDir());
  // Preserve official-over-user selection; do not write before required parsing.
  candidate.loadStandardMaps();
  if(changed) (void)persistCacheINI(*TheNativeUserStorage,serialized);
  std::map<AsciiString,MapMetaData>::swap(candidate);
  m_seen.swap(candidate.m_seen);
  m_allowedMaps.swap(candidate.m_allowedMaps);
  keys.commit();
}

Bool MapCache::clearUnseenMaps( AsciiString dirName )
{
	dirName.toLower();
	Bool erasedSomething = FALSE;

	std::map<AsciiString, Bool>::iterator it = m_seen.begin();

	while (it != m_seen.end())
	{
		AsciiString mapName = it->first;
		if (it->second == FALSE && mapName.startsWithNoCase(dirName.str()))
		{
			// not seen in the dir - clear it out.
			erase(mapName);
			erasedSomething = TRUE;
		}
		++it;
	}
	return erasedSomething;
}

void MapCache::loadStandardMaps(void)
{
	AsciiString fname;
	fname.format("%s\\%s", getMapDir().str(), m_mapCacheName);
  // Required metadata still fails startup on missing/corrupt input, but a
  // foreign block must never escape into unrelated definition owners.
  if (!loadCacheINI(fname)) throw ERROR_BAD_INI;
}

Bool MapCache::loadUserMaps()
{
  AsciiString mapDir=getUserMapDir();
  AsciiString cacheName;
  cacheName.format("%s\\%s",mapDir.str(),m_mapCacheName);
  (void)loadCacheINI(cacheName); // Missing/corrupt optional cache triggers rescan.

	// mark all as unseen
	m_seen.clear();
	MapCache::iterator it = begin();
	while (it != end())
	{
		m_seen[it->first] = FALSE;
		++it;
	}

	FilenameList filenameList;
	FilenameListIter iter;
	AsciiString toplevelPattern;
	toplevelPattern.format("%s\\", mapDir.str());
	Bool parsedAMap = FALSE;
	AsciiString filenamepattern;
	filenamepattern.format("*.%s", getMapExtension().str());

	TheFileSystem->getFileListInDirectory(toplevelPattern, filenamepattern, filenameList, TRUE);

	iter = filenameList.begin();

	while (iter != filenameList.end()) {
		FileInfo fileInfo;
		AsciiString tempfilename;
		tempfilename = (*iter);
		tempfilename.toLower();

		const char *s = tempfilename.reverseFind('\\');
		const char *nativeSlash = tempfilename.reverseFind('/');
		if (!s || (nativeSlash && nativeSlash > s)) s = nativeSlash;
		if (!s)
		{
			DEBUG_CRASH(("Couldn't find \\ in map name!"));
		}
		else
		{
			AsciiString endingStr;
			AsciiString fname = s+1;
			for (Int i=0; i<strlen(mapExtension); ++i)
				fname.removeLastChar();

			endingStr.format("%s\\%s%s", fname.str(), fname.str(), mapExtension);

        std::string comparedPath=tempfilename.str(),comparedSuffix=endingStr.str();
        for(char& byte:comparedPath) if(byte=='\\') byte='/';
        for(char& byte:comparedSuffix) if(byte=='\\') byte='/';
				if (!comparedPath.ends_with(comparedSuffix))
				{
					DEBUG_CRASH(("Found map '%s' in wrong spot (%s)", fname.str(), tempfilename.str()));
				}
				else
				{
					// Fold metadata identity, never the actual physical-root query.
					if (TheFileSystem->getFileInfo(*iter, &fileInfo)) {

						m_seen[tempfilename] = TRUE;
						parsedAMap |= addMap(mapDir, *iter, &fileInfo, FALSE);
					} else {
						DEBUG_CRASH(("Could not get file info for map %s", (*iter).str()));
					}
				}
		}
		iter++;
	}

	// clean out unseen maps
	if (clearUnseenMaps(mapDir))
		return TRUE;

	return parsedAMap;
}

//Bool MapCache::addMap( AsciiString dirName, AsciiString fname, WinTimeStamp timestamp, UnsignedInt filesize, Bool isOfficial )
Bool MapCache::addMap( AsciiString dirName, AsciiString fname, FileInfo *fileInfo, Bool isOfficial)
{
	if (fileInfo == NULL) {
		return FALSE;
	}

	AsciiString lowerFname;
	lowerFname = fname;
	lowerFname.toLower();
	MapCache::iterator it = find(lowerFname);

	MapMetaData md;
	UnsignedInt filesize = fileInfo->sizeLow;

	if (it != end())
	{
		// Found the map in our cache.  Check to see if it has changed.
		md = it->second;

		if ((md.m_filesize == filesize) &&
				(md.m_CRC != 0))
		{
			// Force a lookup so that we don't display the English localization in all builds.
			if (md.m_nameLookupTag.isEmpty())
			{
				// unofficial maps or maps without names
				AsciiString tempdisplayname;
				tempdisplayname = nativePathLeaf(fname.str()).data();
				md.m_displayName.translate(tempdisplayname);
				if (md.m_numPlayers >= 2)
				{
					UnicodeString extension;
					extension.format(L" (%d)", md.m_numPlayers);
					md.m_displayName.concat(extension);
				}
			}
			else
			{
				// official maps with name tags
				md.m_displayName = TheGameText->fetchMapMetadataLabel(AsciiString::TheEmptyString,md.m_nameLookupTag);
				if (md.m_numPlayers >= 2)
				{
					UnicodeString extension;
					extension.format(L" (%d)", md.m_numPlayers);
					md.m_displayName.concat(extension);
				}
			}
//			DEBUG_LOG(("MapCache::addMap - found match for map %s\n", lowerFname.str()));
			publishMetadata(lowerFname,std::move(md));
			return FALSE;	// OK, it checks out.
		}
		DEBUG_LOG(("%s didn't match file in MapCache\n", fname.str()));
		DEBUG_LOG(("size: %d / %d\n", filesize, md.m_filesize));
		DEBUG_LOG(("time1: %d / %d\n", fileInfo->timestampHigh, md.m_timestamp.m_highTimeStamp));
		DEBUG_LOG(("time2: %d / %d\n", fileInfo->timestampLow, md.m_timestamp.m_lowTimeStamp));
//		DEBUG_LOG(("size: %d / %d\n", filesize, md.m_filesize));
//		DEBUG_LOG(("time1: %d / %d\n", timestamp.m_highTimeStamp, md.m_timestamp.m_highTimeStamp));
//		DEBUG_LOG(("time2: %d / %d\n", timestamp.m_lowTimeStamp, md.m_timestamp.m_lowTimeStamp));
	}

	DEBUG_LOG(("MapCache::addMap(): caching '%s' because '%s' was not found\n", fname.str(), lowerFname.str()));

  MapQueryContext query;
  MapQueryScope queryOwner(query);
  if(!loadMap(fname)) throw ERROR_CORRUPT_FILE_FORMAT;

	// The map is now loaded.  Pick out what we need.
	md.m_fileName = lowerFname;
	md.m_filesize = filesize;
	md.m_isOfficial = isOfficial;
	md.m_waypoints.update();
	md.m_numPlayers = md.m_waypoints.m_numStartSpots;
	md.m_isMultiplayer = (md.m_numPlayers >= 2);
	md.m_timestamp.m_highTimeStamp = fileInfo->timestampHigh;
	md.m_timestamp.m_lowTimeStamp = fileInfo->timestampLow;
	md.m_supplyPositions = queryState().supply;
	md.m_techPositions = queryState().tech;
	md.m_CRC = calcCRC(dirName, fname);

	Bool exists = false;
	AsciiString munkee = queryState().world.getAsciiString(TheKey_mapName, &exists);
	md.m_nameLookupTag = munkee;
	if (!exists || munkee.isEmpty())
	{
		DEBUG_LOG(("Missing TheKey_mapName!\n"));
		AsciiString tempdisplayname;
		tempdisplayname = nativePathLeaf(fname.str()).data();
		md.m_displayName.translate(tempdisplayname);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
	}
	else
	{
		const auto companion=nativeMapCompanionPath(fname.str(),"map.str",TheNativeUserStorage);
		AsciiString stringFileName(companion.c_str());
		md.m_displayName = TheGameText->fetchMapMetadataLabel(stringFileName,munkee);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
		DEBUG_LOG(("Map name is now '%ls'\n", md.m_displayName.str()));
	}

	getExtent(&(md.m_extent));

	publishMetadata(lowerFname,md);

	DEBUG_LOG(("  filesize = %d bytes\n", md.m_filesize));
	DEBUG_LOG(("  displayName = %ls\n", md.m_displayName.str()));
	DEBUG_LOG(("  CRC = %X\n", md.m_CRC));
	DEBUG_LOG(("  timestamp = %d\n", md.m_timestamp));
	DEBUG_LOG(("  isOfficial = %s\n", (md.m_isOfficial)?"yes":"no"));

	DEBUG_LOG(("  isMultiplayer = %s\n", (md.m_isMultiplayer)?"yes":"no"));
	DEBUG_LOG(("  numPlayers = %d\n", md.m_numPlayers));

	DEBUG_LOG(("  extent = (%2.2f,%2.2f) -> (%2.2f,%2.2f)\n",
		md.m_extent.lo.x, md.m_extent.lo.y,
		md.m_extent.hi.x, md.m_extent.hi.y));

	Coord3D pos;
	WaypointMap::iterator itw = md.m_waypoints.begin();
	while (itw != md.m_waypoints.end())
	{
		pos = itw->second;
		DEBUG_LOG(("    waypoint %s: (%2.2f,%2.2f)\n", itw->first.str(), pos.x, pos.y));
		++itw;
	}


	return TRUE;
}



// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////////////////////

Bool WouldMapTransfer( const AsciiString& mapName )
{
	return mapName.startsWithNoCase(TheMapCache->getUserMapDir());
}

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListboxNoReset( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;
	
	// reset the listbox content
	//GadgetListBoxReset( listbox );
	
	Int numColumns = GadgetListBoxGetNumColumns( listbox );
	const Image *easyImage = NULL;
	const Image *mediumImage = NULL;
	const Image *brutalImage = NULL;
	const Image *maxBrutalImage = NULL;
	SkirmishBattleHonors *battleHonors = NULL;
	Int w = 10, h = 10;
	if (numColumns > 1)
	{
		easyImage = TheMappedImageCollection->findImageByName("Star-Bronze");
		mediumImage = TheMappedImageCollection->findImageByName("Star-Silver");
		brutalImage = TheMappedImageCollection->findImageByName("Star-Gold");
		maxBrutalImage = TheMappedImageCollection->findImageByName("RedYell_Star");
		battleHonors = new SkirmishBattleHonors;

		w = (brutalImage)?brutalImage->getImageWidth():10;
		w = std::min(GadgetListBoxGetColumnWidth(listbox, 0), w);
		h = w;
	}

	Color color = GameMakeColor( 255, 255, 255, 255 );
	UnicodeString mapDisplayName;

	Int selectionIndex = 0; // always select *something*

	MapCache::iterator it = TheMapCache->begin();
	AsciiString mapDir;
	if (useSystemMaps)
	{
		mapDir = TheMapCache->getMapDir();
	}
	else
	{
		mapDir = TheGlobalData->getPath_UserData();
		mapDir.concat(TheMapCache->getMapDir());
	}
	mapDir.toLower();

typedef std::set<UnicodeString, rts::less_than_nocase<UnicodeString> > MapNameList;
typedef MapNameList::iterator MapNameListIter;

typedef std::map<UnicodeString, AsciiString> MapDisplayToFileNameList;
typedef MapDisplayToFileNameList::iterator MapDisplayToFileNameListIter;

	MapNameList tempCache;
	MapDisplayToFileNameList filenameMap;
	UnsignedInt numMapsListed = 0;
	UnsignedInt curNumPlayersInMap = 0;

	while (numMapsListed < TheMapCache->size()) {

		DEBUG_LOG(("Adding maps with %d players\n", curNumPlayersInMap));
		it = TheMapCache->begin();
		while (it != TheMapCache->end()) {
			const MapMetaData *md = &(it->second);
			if (md != NULL) {
				if (md->m_numPlayers == curNumPlayersInMap) {
					tempCache.insert(it->second.m_displayName);
					filenameMap[it->second.m_displayName] = it->first;
					DEBUG_LOG(("Adding map %s to temp cache.\n", it->first.str()));
					++numMapsListed;
				}
			}
			++it;
		}

		MapNameListIter tempit = tempCache.begin();

		while (tempit != tempCache.end())
		{

			AsciiString asciiMapName;
			asciiMapName = filenameMap[*tempit];
			it = TheMapCache->find(asciiMapName);
			/*
			if (it != TheMapCache->end())
			{
				DEBUG_LOG(("populateMapListbox(): looking at %s (displayName = %ls), mp = %d (== %d?) mapDir=%s (ok=%d)\n",
					it->first.str(), it->second.m_displayName.str(), it->second.m_isMultiplayer, isMultiplayer,
					mapDir.str(), it->first.startsWith(mapDir.str())));
			}
			*/
			
			//Patch 1.03 -- Purposely filter out these broken maps that exist in Generals.
			if( !asciiMapName.compare( "maps\\armored fury\\armored fury.map" ) || 
				!asciiMapName.compare( "maps\\scorched earth\\scorched earth.map" ) )
			{
				++tempit;
				continue;
			}

			DEBUG_ASSERTCRASH(it != TheMapCache->end(), ("Map %s not found in map cache.", *tempit));
			if (it->first.startsWithNoCase(mapDir.str()) && isMultiplayer == it->second.m_isMultiplayer && !it->second.m_displayName.isEmpty())
			{
				/// @todo: mapDisplayName = TheGameText->fetch(it->second.m_displayName.str());
				mapDisplayName = it->second.m_displayName;
				Int index = -1;
				Int imageItemData = -1;
				if (numColumns > 1 && it->second.m_isMultiplayer)
				{
					Int numEasy = battleHonors->getEnduranceMedal(it->first.str(), SLOT_EASY_AI);
					Int numMedium = battleHonors->getEnduranceMedal(it->first.str(), SLOT_MED_AI);
					Int numBrutal = battleHonors->getEnduranceMedal(it->first.str(), SLOT_BRUTAL_AI);
					if (numBrutal)
					{
						int maxBrutalSlots = it->second.m_numPlayers - 1;
						if (numBrutal == maxBrutalSlots)
						{
							index = GadgetListBoxAddEntryImage( listbox, maxBrutalImage, index, 0, w, h, TRUE);
							imageItemData = 4;
						}
						else	
						{
							index = GadgetListBoxAddEntryImage( listbox, brutalImage, index, 0, w, h, TRUE);
							imageItemData = 3;
						}
					}
					else if (numMedium)
					{
						imageItemData = 2;
						index = GadgetListBoxAddEntryImage( listbox, mediumImage, index, 0, w, h, TRUE);
					}
					else if (numEasy)
					{
						imageItemData = 1;
						index = GadgetListBoxAddEntryImage( listbox, easyImage, index, 0, w, h, TRUE);
					}
					else
					{
						imageItemData = 0;
						index = GadgetListBoxAddEntryImage( listbox, NULL, index, 0, w, h, TRUE);
					}
				}
				index = GadgetListBoxAddEntryText( listbox, mapDisplayName, color, index, numColumns-1 );

				if (it->first == mapToSelect)
				{
					selectionIndex = index;
				}

				// now set the char* as the item data.  this works because the map cache isn't being
				// modified while a map listbox is up.
				GadgetListBoxSetItemData( listbox, (void *)(it->first.str()), index );

				if (numColumns > 1)
				{
					GadgetListBoxSetItemData( listbox, (void *)imageItemData, index, 1 );
				}
			}
			++tempit;
		}

		tempCache.clear();
		filenameMap.clear();
		++curNumPlayersInMap;
	}

	if (battleHonors)
	{
		delete battleHonors;
		battleHonors = NULL;
	}

	GadgetListBoxSetSelected(listbox, &selectionIndex, 1);
	if (selectionIndex >= 0)
	{
		Int topIndex = GadgetListBoxGetTopVisibleEntry(listbox);
		Int bottomIndex = GadgetListBoxGetBottomVisibleEntry(listbox);
		Int rowsOnScreen = bottomIndex - topIndex;

		if (selectionIndex >= bottomIndex)
		{
			Int newTop = std::max( 0, selectionIndex - std::max( 1, rowsOnScreen / 2 ) );
		//The trouble is that rowsonscreen/2 can be zero if bottom is 1 and top is zero
			GadgetListBoxSetTopVisibleEntry( listbox, newTop );
		}
	}
	return selectionIndex;

}  // end loadMapListbox

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListbox( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;
	
	// reset the listbox content
	GadgetListBoxReset( listbox );

	return populateMapListboxNoReset( listbox, useSystemMaps, isMultiplayer, mapToSelect );
}
	


//-------------------------------------------------------------------------------------------------
/** Validate a map */
//-------------------------------------------------------------------------------------------------
Bool isValidMap( AsciiString mapName, Bool isMultiplayer )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();

	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return TRUE;
		}
	}

	return FALSE;
}  // end isValidMap

//-------------------------------------------------------------------------------------------------
/** Find a valid map */
//-------------------------------------------------------------------------------------------------
AsciiString getDefaultMap( Bool isMultiplayer )
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return it->first;
		}
		++it;
	}

	return AsciiString::TheEmptyString;
}


AsciiString getDefaultOfficialMap()
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (it->second.m_isMultiplayer && it->second.m_isOfficial)
		{
			return it->first;
		}
		++it;
	}
	return AsciiString::TheEmptyString;
}


Bool isOfficialMap( AsciiString mapName )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();
	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
		return it->second.m_isOfficial;
	return FALSE;
}


const MapMetaData *MapCache::findMap(AsciiString mapName)
{
	mapName.toLower();
	MapCache::iterator it = find(mapName);
	if (it == end())
		return NULL;
	return &(it->second);
}

// ------------------------------------------------------------------------------------------------
/** Embed the pristine map into the xfer stream */
// ------------------------------------------------------------------------------------------------
static void copyFromBigToDir( const AsciiString& infile, const AsciiString& outfile )
{
	// open the map file

	File *file = TheFileSystem->openFile( infile.str(), File::READ | File::BINARY );
	if( file == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error opening source file '%s'\n", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	// how big is the map file
	Int fileSize = file->seek( 0, File::END );


	// rewind to beginning of file
	file->seek( 0, File::START );

	// allocate buffer big enough to hold the entire map file
	char *buffer = NEW char[ fileSize ];
	if( buffer == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Unable to allocate buffer for file '%s'\n", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	// copy the file to the buffer
	if( file->read( buffer, fileSize ) < fileSize )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error reading from file '%s'\n", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if
	// close the BIG file
	file->close();
	
	File *filenew = TheFileSystem->openFile( outfile.str(), File::WRITE | File::CREATE | File::BINARY );
	
	if( !filenew || filenew->write(buffer, fileSize) < fileSize)
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error writing to file '%s'\n", outfile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	filenew->close();

	// delete the buffer
	delete [] buffer;
} // end embedPristineMap

Image *getMapPreviewImage( AsciiString mapName )
{
	if(!TheGlobalData)
		return NULL;
	DEBUG_LOG(("%s Map Name \n", mapName.str()));
	AsciiString tgaName = mapName;
	AsciiString name;
	AsciiString tempName;
	AsciiString filename;
	tgaName.removeLastChar(); // p
	tgaName.removeLastChar(); // a
	tgaName.removeLastChar(); // m
	tgaName.removeLastChar(); // .
	name = tgaName;//.reverseFind('\\') + 1;
	filename = nativePathLeaf(tgaName.str()).data();
	//tgaName = name;
	filename.concat(".tga");
	tgaName.concat(".tga");

	AsciiString portableName = TheGameState->realMapPathToPortableMapPath(name);
	tempName.set(AsciiString::TheEmptyString);
	for(Int i = 0; i < portableName.getLength(); ++i)
	{
		char c = portableName.getCharAt(i);
		if (c == '\\' || c == ':')
			tempName.concat('_');
		else
			tempName.concat(c);
	}
	
	name = tempName;
	name.concat(".tga");

	
	// copy file over	
	// copy source tgaName, to name

	Image *image = (Image *)TheMappedImageCollection->findImageByName(tempName);
	if(!image)
	{

		if(!TheFileSystem->doesFileExist(tgaName.str()))
			return NULL;	
		AsciiString mapPreviewDir;
		mapPreviewDir.format(MAP_PREVIEW_DIR_PATH, TheGlobalData->getPath_UserData().str());
		TheFileSystem->createDirectory(mapPreviewDir);

		mapPreviewDir.concat(name);

		Bool success = false;
		try
		{
			copyFromBigToDir(tgaName, mapPreviewDir);	
			success = true;
		} 
		catch (...)
		{
			success = false;	// no rethrow
		}
		
		if (success)
		{
    	image = newInstance(Image);
            MemoryPoolObjectHolder candidateOwner(image);
			image->setName(tempName);
			//image->setFullPath("mission.tga");
			image->setFilename(name);
			image->setStatus(IMAGE_STATUS_NONE);
			Region2D uv;
			uv.hi.x = 1.0f;
			uv.hi.y = 1.0f;
			uv.lo.x	= 0.0f;
			uv.lo.y = 0.0f;
			image->setUV(&uv);
			image->setTextureHeight(128);
			image->setTextureWidth(128);
			TheMappedImageCollection->addImage(image);
            candidateOwner.release();
		}
		else
		{
			image = NULL;
		}
	}

	return image;

	
	
/*
	// sanity
	if( mapName.isEmpty() )
		return NULL;
	Region2D uv;
	mapPreviewImage = TheMappedImageCollection->findImageByName("MapPreview");
	if(mapPreviewImage)
		mapPreviewImage->deleteInstance();
	
	mapPreviewImage = TheMappedImageCollection->newImage();
	mapPreviewImage->setName("MapPreview");
	mapPreviewImage->setStatus(IMAGE_STATUS_RAW_TEXTURE);
// allocate our terrain texture
	TextureClass * texture = new TextureClass( size.x, size.y, 
																			 WW3D_FORMAT_X8R8G8B8, TextureClass::MIP_LEVELS_1 );
	uv.lo.x = 0.0f;
	uv.lo.y = 1.0f;
	uv.hi.x = 1.0f;
	uv.hi.y = 0.0f;
	mapPreviewImage->setStatus( IMAGE_STATUS_RAW_TEXTURE );
	mapPreviewImage->setRawTextureData( texture );
	mapPreviewImage->setUV( &uv );
	mapPreviewImage->setTextureWidth( size.x );
	mapPreviewImage->setTextureHeight( size.y );
	mapPreviewImage->setImageSize( &size );


	CachedFileInputStream theInputStream;
	if (theInputStream.open(AsciiString(mapName.str()))) 
	{
		ChunkInputStream *pStrm = &theInputStream;
		pStrm->absoluteSeek(0);
		DataChunkInput file( pStrm );
		if (file.isValidFileType()) {	// Backwards compatible files aren't valid data chunk files.
			// Read the waypoints.
			file.registerParser( AsciiString("MapPreview"), AsciiString::TheEmptyString, parseMapPreviewChunk );
			if (!file.parse(NULL)) {
				DEBUG_ASSERTCRASH(false,("Unable to read MapPreview info."));
				mapPreviewImage->deleteInstance();
				return NULL;
			}
		}
		theInputStream.close();
	}
	else
	{
		mapPreviewImage->deleteInstance();
		return NULL;
	}
	

	return mapPreviewImage;
	
*/
	return NULL;
}

Bool parseMapPreviewChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
/*
	ICoord2D size;
	
	SurfaceClass *surface;
	size.x = file.readInt();
	size.y = file.readInt();


	surface = (TextureClass *)mapPreviewImage->getRawTextureData()->Get_Surface_Level();
	//texture->Get_Surface_Level();
	
	DEBUG_LOG(("BeginMapPreviewInfo\n"));
	UnsignedInt *buffer = new UnsignedInt[size.x * size.y];
	Int x,y;
	for (y=0; y<size.y; y++) {
		for(x = 0; x< size.x; x++)
		{
			surface->DrawPixel( x, y, file.readInt() );
			buffer[y + x] = file.readInt();
			DEBUG_LOG(("x:%d, y:%d, %X\n", x, y, buffer[y + x]));
		}
	}
	mapPreviewImage->setRawTextureData(buffer);
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	DEBUG_LOG(("EndMapPreviewInfo\n"));
	REF_PTR_RELEASE(surface);
	return true;
*/
	return FALSE;
}

void findDrawPositions( Int startX, Int startY, Int width, Int height, Region3D extent,
															 ICoord2D *ul, ICoord2D *lr )
{

	Real ratioWidth;
	Real ratioHeight;
	Coord2D radar;
	ratioWidth = extent.width()/(width * 1.0f);
	ratioHeight = extent.height()/(height* 1.0f);
	
	if( ratioWidth >= ratioHeight)
	{
		radar.x = extent.width() / ratioWidth;
		radar.y = extent.height()/ ratioWidth;
		ul->x = 0;
		ul->y = (height - radar.y) / 2.0f;
		lr->x = radar.x;
		lr->y = height - ul->y;
	}
	else
	{
		radar.x = extent.width() / ratioHeight;
		radar.y = extent.height()/ ratioHeight;
		ul->x = (width - radar.x ) / 2.0f;
		ul->y = 0;
		lr->x = width - ul->x;
		lr->y = radar.y;
	}

	// make them pixel positions
	ul->x += startX;
	ul->y += startY;
	lr->x += startX;
	lr->y += startY;

}  // end findDrawPositions
