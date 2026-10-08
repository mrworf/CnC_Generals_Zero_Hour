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

// FILE: GameInfo.cpp //////////////////////////////////////////////////////
// game setup state info
// Author: Matthew D. Campbell, December 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/CRCDebug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/NativeClock.h"
#include "Common/NativeSourceStrings.h"
#include <string>
#include <bit>
#include <utility>
#include <type_traits>
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "Common/Xfer.h"
#include "GameNetwork/FileTransfer.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/LANAPI.h"						// for testing packet size
#include "GameNetwork/LANAPICallbacks.h"	// for testing packet size
#include "strtok_r.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


GameInfo *TheGameInfo = NULL;

// Original network CRC policy belongs to the shared game setup owner.
#if defined(DEBUG_CRC)
Int NET_CRC_INTERVAL = 1;
#else
Int NET_CRC_INTERVAL = 100;
#endif

// GameSlot ----------------------------------------

GameSlot::GameSlot() : m_IP(0)
{
	reset();
}

void GameSlot::swapPayload(GameSlot& other) noexcept
{
    using std::swap;
    swap(m_state,other.m_state); swap(m_isAccepted,other.m_isAccepted);
    swap(m_hasMap,other.m_hasMap); swap(m_isMuted,other.m_isMuted);
    swap(m_color,other.m_color); swap(m_startPos,other.m_startPos);
    swap(m_playerTemplate,other.m_playerTemplate); swap(m_teamNumber,other.m_teamNumber);
    swap(m_origColor,other.m_origColor); swap(m_origStartPos,other.m_origStartPos);
    swap(m_origPlayerTemplate,other.m_origPlayerTemplate); m_name.swap(other.m_name);
    swap(m_IP,other.m_IP); swap(m_port,other.m_port); swap(m_NATBehavior,other.m_NATBehavior);
    swap(m_lastFrameInGame,other.m_lastFrameInGame); swap(m_disconnected,other.m_disconnected);
}

NativeGameInfoTransaction::NativeGameInfoTransaction(GameInfo& owner)
    : m_owner(owner),m_preorderMask(owner.m_preorderMask),m_crcInterval(owner.m_crcInterval),
      m_gameID(owner.m_gameID),m_mapMask(owner.m_mapMask),m_seed(owner.m_seed),m_useStats(owner.m_useStats),
      m_inGame(owner.m_inGame),m_inProgress(owner.m_inProgress),m_surrendered(owner.m_surrendered),
      m_oldFactionsOnly(owner.m_oldFactionsOnly),m_localIP(owner.m_localIP),m_mapCRC(owner.m_mapCRC),
      m_mapSize(owner.m_mapSize),m_mapName(owner.m_mapName),m_startingCash(owner.m_startingCash),
      m_superweaponRestriction(owner.m_superweaponRestriction)
{
    for (Int i=0;i<MAX_SLOTS;++i) {
        m_links[i]=owner.m_slot[i];
        if (m_links[i]) m_slots[i]=*m_links[i];
    }
}

NativeGameInfoTransaction::~NativeGameInfoTransaction() noexcept
{
    if (m_committed) return;
    m_owner.m_preorderMask=m_preorderMask; m_owner.m_crcInterval=m_crcInterval;
    m_owner.m_gameID=m_gameID; m_owner.m_mapMask=m_mapMask; m_owner.m_seed=m_seed;
    m_owner.m_useStats=m_useStats; m_owner.m_inGame=m_inGame; m_owner.m_inProgress=m_inProgress;
    m_owner.m_surrendered=m_surrendered; m_owner.m_oldFactionsOnly=m_oldFactionsOnly;
    m_owner.m_localIP=m_localIP; m_owner.m_mapCRC=m_mapCRC; m_owner.m_mapSize=m_mapSize;
    m_owner.m_mapName.swap(m_mapName);
    static_assert(std::is_nothrow_copy_assignable_v<Money>);
    m_owner.m_startingCash=m_startingCash; m_owner.m_superweaponRestriction=m_superweaponRestriction;
    for (Int i=0;i<MAX_SLOTS;++i) {
        m_owner.m_slot[i]=m_links[i];
        if (m_links[i]) m_links[i]->swapPayload(m_slots[i]);
    }
}

void GameInfo::swapSetupPayload(GameInfo& other) noexcept
{
    if (this==&other) return;
    using std::swap;
    swap(m_preorderMask,other.m_preorderMask); swap(m_crcInterval,other.m_crcInterval);
    swap(m_inGame,other.m_inGame); swap(m_inProgress,other.m_inProgress);
    swap(m_surrendered,other.m_surrendered); swap(m_gameID,other.m_gameID);
    swap(m_localIP,other.m_localIP); m_mapName.swap(other.m_mapName);
    swap(m_mapCRC,other.m_mapCRC); swap(m_mapSize,other.m_mapSize);
    swap(m_mapMask,other.m_mapMask); swap(m_seed,other.m_seed); swap(m_useStats,other.m_useStats);
    static_assert(std::is_nothrow_swappable_v<Money>);
    swap(m_startingCash,other.m_startingCash);
    swap(m_superweaponRestriction,other.m_superweaponRestriction);
    swap(m_oldFactionsOnly,other.m_oldFactionsOnly);
    for (Int i=0;i<MAX_SLOTS;++i) {
        // ReplayGameInfo attaches every slot to its own embedded array.
        m_slot[i]->swapPayload(*other.m_slot[i]);
    }
}

void GameSlot::reset()
{
	m_state = SLOT_CLOSED; // decent default
	m_isAccepted = false;
	m_hasMap = true;
	m_color = -1;
	m_startPos = -1;
	m_playerTemplate = -1;
	m_teamNumber = -1;
	m_NATBehavior = FirewallHelperClass::FIREWALL_TYPE_SIMPLE;
	m_lastFrameInGame = 0;
	m_disconnected = FALSE;
	m_port = 0;
	m_isMuted = FALSE;
	m_origPlayerTemplate = -1;
	m_origStartPos = -1;
	m_origColor = -1;
}

void GameSlot::saveOffOriginalInfo( void )
{
	DEBUG_LOG(("GameSlot::saveOffOriginalInfo() - orig was color=%d, pos=%d, house=%d\n",
		m_origColor, m_origStartPos, m_origPlayerTemplate));
	m_origPlayerTemplate = m_playerTemplate;
	m_origStartPos = m_startPos;
	m_origColor = m_color;
	DEBUG_LOG(("GameSlot::saveOffOriginalInfo() - color=%d, pos=%d, house=%d\n",
		m_color, m_startPos, m_playerTemplate));
}

void GameSlot::unAccept( void )
{
	if (isHuman())
	{
		m_isAccepted = false;
	}
}

void GameSlot::setMapAvailability( Bool hasMap )
{
	if (isHuman())
	{
		m_hasMap = hasMap;
	}
}

void GameSlot::setState( SlotState state, UnicodeString name, UnsignedInt IP )
{
    // Localization may allocate or reject. Admit the complete name before any
    // accepted scalar changes; no virtual parent is replaced by a base clone.
    UnicodeString preparedName=name;
    if (state!=SLOT_PLAYER) {
        if (!TheGameText) throw ERROR_BAD_ARG;
        const char* label="GUI:Closed";
        switch(state) {
        case SLOT_OPEN: label="GUI:Open"; break;
        case SLOT_EASY_AI: label="GUI:EasyAI"; break;
        case SLOT_MED_AI: label="GUI:MediumAI"; break;
        case SLOT_BRUTAL_AI: label="GUI:HardAI"; break;
        default: break;
        }
        preparedName=TheGameText->fetch(label);
    }
	if (!(isAI() &&  (state == SLOT_EASY_AI || state == SLOT_MED_AI || state == SLOT_BRUTAL_AI)))
	{
		m_color = -1;
		m_startPos = -1;
		m_playerTemplate = -1;
		m_teamNumber = -1;

	}
	if (state == SLOT_PLAYER)
	{
		reset();
		m_state = state;
		m_name.swap(preparedName);
	}// state == SLOT_PLAYER
	else
	{
		m_state = state;
		m_isAccepted = true;
		m_hasMap = true;
		m_name.swap(preparedName);
	}

	m_IP = IP;
}

// Various tests
Bool GameSlot::isHuman( void ) const
{
	return m_state == SLOT_PLAYER;
}

Bool GameSlot::isOccupied( void ) const
{
	return m_state == SLOT_PLAYER || m_state == SLOT_EASY_AI || m_state == SLOT_MED_AI || m_state == SLOT_BRUTAL_AI;
}

Bool GameSlot::isAI( void ) const
{
	return m_state == SLOT_EASY_AI || m_state == SLOT_MED_AI || m_state == SLOT_BRUTAL_AI;
}

Bool GameSlot::isPlayer( AsciiString userName ) const
{
	UnicodeString uName;
	uName.translate(userName);
	return (m_state == SLOT_PLAYER && !m_name.compareNoCase(uName));
}

Bool GameSlot::isPlayer( UnicodeString userName ) const
{
	return (m_state == SLOT_PLAYER && !m_name.compareNoCase(userName));
}

Bool GameSlot::isPlayer( UnsignedInt ip ) const
{
	return (m_state == SLOT_PLAYER && m_IP == ip);
}

Bool GameSlot::isOpen( void ) const
{
	return m_state == SLOT_OPEN;
}

// GameInfo ----------------------------------------

GameInfo::GameInfo()
    : m_preorderMask(0),m_crcInterval(0),m_inGame(false),m_inProgress(false),
      m_surrendered(false),m_gameID(0),m_localIP(0),m_mapCRC(0),m_mapSize(0),
      m_mapMask(0),m_seed(0),m_useStats(0),m_superweaponRestriction(0),m_oldFactionsOnly(false)
{
    m_localIP=0;
	for (int i=0; i<MAX_SLOTS; ++i)
	{
		m_slot[i] = NULL;
	}
	reset();
}

void GameInfo::init( void )
{
	reset();
}

void GameInfo::reset( void )
{
    if (!TheGlobalData) throw ERROR_BAD_ARG;
    NativeGameInfoTransaction transaction(*this);
	m_crcInterval = NET_CRC_INTERVAL;
	m_inGame = false;
	m_inProgress = false;
	m_gameID = 0;
	m_mapName = AsciiString("NOMAP");
	m_mapMask = 0;
	m_seed = std::bit_cast<Int>(nativeMilliseconds()); //GameClientRandomValue(0, INT_MAX - 1);
	m_useStats = TRUE;
	m_surrendered = FALSE;
  m_oldFactionsOnly = FALSE;
	// Added By Sadullah Nader
	// Initializations missing and needed
//	m_localIP = 0; // BGC - actually we don't want this to be reset since the m_localIP is 
										// set properly in the constructor of LANGameInfo which uses this as a base class.
	m_mapCRC = 0;
	m_mapSize = 0;
  m_superweaponRestriction = 0; 
  m_startingCash = TheGlobalData->m_defaultStartingCash;
  
	//

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_slot[i])
			m_slot[i]->reset();
	}

	m_preorderMask = 0;
    transaction.commit();
}


Bool GameInfo::isMultiPlayer(void)
{
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (i == getLocalSlotNum())
			continue;

		if (getConstSlot(i)->isHuman())
			return TRUE;
	}

	return FALSE;
}

Bool GameInfo::isSandbox(void)
{
	Int localSlotNum = getLocalSlotNum();
	Int localTeam = getConstSlot(localSlotNum)->getTeamNumber();
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (i == localSlotNum)
			continue;

		const GameSlot *slot = getConstSlot(i);
		if (slot->isOccupied() && (slot->getTeamNumber() < 0 || slot->getTeamNumber() != localTeam))
			return FALSE;
	}
	return TRUE;
}


Bool GameInfo::isPlayerPreorder(Int index)
{
	if (index >= 0 && index < MAX_SLOTS)
		return ((m_preorderMask & (1 << index)) != 0);
	return FALSE;
}

void GameInfo::markPlayerAsPreorder(Int index)
{
	if (index >= 0 && index < MAX_SLOTS)
		m_preorderMask |= 1 << index;
}


void GameInfo::clearSlotList( void )
{
    NativeGameInfoTransaction transaction(*this);
	for (int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_slot[i])
			m_slot[i]->setState(SLOT_CLOSED);
	}
    transaction.commit();
}
Int GameInfo::getNumPlayers( void ) const
{
	Int numPlayers = 0;
	for (int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_slot[i] && m_slot[i]->isOccupied())
			numPlayers++;
	}
	return numPlayers;
}

Int GameInfo::getNumNonObserverPlayers( void ) const
{
	Int numPlayers = 0;
	for (int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_slot[i] && m_slot[i]->isOccupied() && m_slot[i]->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
			numPlayers++;
	}
	return numPlayers;
}

Int GameInfo::getMaxPlayers( void ) const
{
	if (!TheMapCache)
		return -1;

	AsciiString lowerMap = m_mapName;
	lowerMap.toLower();
	MapCache::iterator it = TheMapCache->find(lowerMap);
	if (it == TheMapCache->end())
		return -1;
	MapMetaData data = it->second;
	return data.m_numPlayers;
}

void GameInfo::enterGame( void )
{
	DEBUG_ASSERTCRASH(!m_inGame && !m_inProgress, ("Entering game at a bad time!"));
	reset();
	m_inGame = true;
	m_inProgress = false;
}

void GameInfo::leaveGame( void )
{
	DEBUG_ASSERTCRASH(m_inGame && !m_inProgress, ("Leaving game at a bad time!"));
	reset();
}

void GameInfo::startGame( Int gameID )
{
    NativeGameInfoTransaction transaction(*this);
	DEBUG_ASSERTCRASH(m_inGame && !m_inProgress, ("Starting game at a bad time!"));
	m_gameID = gameID;
	closeOpenSlots();
	m_inProgress = true;
    transaction.commit();
}

void GameInfo::endGame( void )
{
	DEBUG_ASSERTCRASH(m_inGame && m_inProgress, ("Ending game without playing one!"));
	m_inGame = false;
	m_inProgress = false;
}

void GameInfo::setSlot( Int slotNum, GameSlot slotInfo )
{
	DEBUG_ASSERTCRASH( slotNum >= 0 && slotNum < MAX_SLOTS, ("GameInfo::setSlot - Invalid slot number"));
	if (slotNum < 0 || slotNum >= MAX_SLOTS)
		return;

	DEBUG_ASSERTCRASH( m_slot[slotNum], ("NULL slot pointer"));
	if (!m_slot[slotNum])
		return;

//	Bool isHuman = slotInfo.isHuman();
//	Bool wasHuman = m_slot[slotNum]->isHuman();

	if (slotNum == 0)
	{
		slotInfo.setAccept();
		slotInfo.setMapAvailability(true);
	}
	m_slot[slotNum]->swapPayload(slotInfo);

#ifdef DEBUG_LOGGING
	UnsignedInt ip = m_slot[slotNum]->getIP();
#endif

	DEBUG_LOG(("GameInfo::setSlot - setting slot %d to be player %ls with IP %d.%d.%d.%d\n", slotNum, m_slot[slotNum]->getName().str(),
							ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff));
}

GameSlot* GameInfo::getSlot( Int slotNum )
{
	DEBUG_ASSERTCRASH( slotNum >= 0 && slotNum < MAX_SLOTS, ("GameInfo::getSlot - Invalid slot number"));
	if (slotNum < 0 || slotNum >= MAX_SLOTS)
		return NULL;

	return m_slot[slotNum];
}

const GameSlot* GameInfo::getConstSlot( Int slotNum ) const
{
	DEBUG_ASSERTCRASH( slotNum >= 0 && slotNum < MAX_SLOTS, ("GameInfo::getSlot - Invalid slot number"));
	if (slotNum < 0 || slotNum >= MAX_SLOTS)
		return NULL;

	return m_slot[slotNum];
}

Int GameInfo::getLocalSlotNum( void ) const
{
	DEBUG_ASSERTCRASH(m_inGame, ("Looking for local game slot while not in game"));
	if (!m_inGame)
		return -1;

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = getConstSlot(i);
		if (slot == NULL) {
			continue;
		}
		if (slot->isPlayer(m_localIP))
			return i;
	}
	return -1;
}

Int GameInfo::getSlotNum( AsciiString userName ) const
{
	DEBUG_ASSERTCRASH(m_inGame, ("Looking for game slot while not in game"));
	if (!m_inGame)
		return -1;

	UnicodeString uName;
	uName.translate(userName);
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = getConstSlot(i);
		if (slot->isPlayer( uName ))
			return i;
	}
	return -1;
}

Bool GameInfo::amIHost( void ) const
{
	DEBUG_ASSERTCRASH(m_inGame, ("Looking for game slot while not in game"));
	if (!m_inGame)
		return false;

	return getConstSlot(0)->isPlayer(m_localIP);
}

void GameInfo::setMap( AsciiString mapName )
{
    NativeGameInfoTransaction transaction(*this);
	m_mapName = mapName;
	if (m_inGame && amIHost())
	{
        if (!TheMapCache || !TheFileSystem) throw ERROR_BAD_ARG;
		const MapMetaData *mapData = TheMapCache->findMap( mapName );
		if (mapData)
		{
			m_mapMask = 1;
			AsciiString path = mapName;
			path.removeLastChar();
			path.removeLastChar();
			path.removeLastChar();
			path.concat("tga");
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", path.str()));
			File *fp = TheFileSystem->openFile(path.str());
			if (fp)
			{
				m_mapMask |= 2;
				fp->close();
				fp = NULL;
			}

			AsciiString newMapName = GetINIFromMap(m_mapName);
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", newMapName.str()));
			fp = TheFileSystem->openFile(newMapName.str());
			if (fp)
			{
				m_mapMask |= 4;
				fp->close();
				fp = NULL;
			}

			path = GetStrFileFromMap(m_mapName);
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", path.str()));
			fp = TheFileSystem->openFile(path.str());
			if (fp)
			{
				m_mapMask |= 8;
				fp->close();
				fp = NULL;
			}

			path = GetSoloINIFromMap(m_mapName);
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", path.str()));
			fp = TheFileSystem->openFile(path.str());
			if (fp)
			{
				m_mapMask |= 16;
				fp->close();
				fp = NULL;
			}

			path = GetAssetUsageFromMap(m_mapName);
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", path.str()));
			fp = TheFileSystem->openFile(path.str());
			if (fp)
			{
				m_mapMask |= 32;
				fp->close();
				fp = NULL;
			}

			path = GetReadmeFromMap(m_mapName);
			DEBUG_LOG(("GameInfo::setMap() - Looking for '%s'\n", path.str()));
			fp = TheFileSystem->openFile(path.str());
			if (fp)
			{
				m_mapMask |= 64;
				fp->close();
				fp = NULL;
			}
		}
		else
		{
			m_mapMask = 0;
		}
	}
    transaction.commit();
}

void GameInfo::setMapContentsMask( Int mask )
{
	m_mapMask = mask;
}

void GameInfo::setMapCRC( UnsignedInt mapCRC )
{
    NativeGameInfoTransaction transaction(*this);
	m_mapCRC = mapCRC;
	if (!TheMapCache) { transaction.commit(); return; }

	// check the map cache
	if (m_inGame && getLocalSlotNum() >= 0)
	{
		//TheMapCache->updateCache();
		AsciiString lowerMap = m_mapName;
		lowerMap.toLower();
		//DEBUG_LOG(("GameInfo::setMapCRC - looking for map file \"%s\" in the map cache\n", lowerMap.str()));
		std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->find(lowerMap);
		if (it == TheMapCache->end())
		{
			/*
			DEBUG_LOG(("GameInfo::setMapCRC - could not find map file.\n"));
			it = TheMapCache->begin();
			while (it != TheMapCache->end())
			{
				DEBUG_LOG(("\t\"%s\"\n", it->first.str()));
				++it;
			}
			*/
			getSlot(getLocalSlotNum())->setMapAvailability(false);
		}
		else if (m_mapCRC != it->second.m_CRC)
		{
			DEBUG_LOG(("GameInfo::setMapCRC - map CRC's do not match (%X/%X).\n", m_mapCRC, it->second.m_CRC));
			getSlot(getLocalSlotNum())->setMapAvailability(false);
		}
		else
		{
			//DEBUG_LOG(("GameInfo::setMapCRC - map CRC's match.\n"));
			getSlot(getLocalSlotNum())->setMapAvailability(true);
		}
	}
    transaction.commit();
}

void GameInfo::setMapSize( UnsignedInt mapSize )
{
    NativeGameInfoTransaction transaction(*this);
	m_mapSize = mapSize;
	if (!TheMapCache) { transaction.commit(); return; }

	// check the map cache
	if (m_inGame && getLocalSlotNum() >= 0)
	{
		//TheMapCache->updateCache();
		AsciiString lowerMap = m_mapName;
		lowerMap.toLower();
		std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->find(lowerMap);
		if (it == TheMapCache->end())
		{
			DEBUG_LOG(("GameInfo::setMapSize - could not find map file.\n"));
			getSlot(getLocalSlotNum())->setMapAvailability(false);
		}
		else if (m_mapCRC != it->second.m_CRC)
		{
			DEBUG_LOG(("GameInfo::setMapSize - map CRC's do not match.\n"));
			getSlot(getLocalSlotNum())->setMapAvailability(false);
		}
		else
		{
			//DEBUG_LOG(("GameInfo::setMapSize - map CRC's match.\n"));
			getSlot(getLocalSlotNum())->setMapAvailability(true);
		}
	}
    transaction.commit();
}

void GameInfo::setSeed( Int seed )
{
	m_seed = seed;
}

void GameInfo::setSlotPointer( Int index, GameSlot *slot )
{
	if (index < 0 || index >= MAX_SLOTS)
		return;

	m_slot[index] = slot;
}

void GameInfo::setSuperweaponRestriction( UnsignedShort restriction )
{
  m_superweaponRestriction = restriction;
}

void GameInfo::setStartingCash( const Money & startingCash )
{
  m_startingCash = startingCash;
}

Bool GameInfo::isColorTaken(Int colorIdx, Int slotToIgnore ) const
{
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = getConstSlot(i);
		if (slot && slot->getColor() == colorIdx && i != slotToIgnore)
			return true;
	}
	return false;
}

Bool GameInfo::isStartPositionTaken(Int positionIdx, Int slotToIgnore ) const
{
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = getConstSlot(i);
		if (slot && slot->getStartPos() == positionIdx && i != slotToIgnore)
			return true;
	}
	return false;
}

void GameInfo::resetAccepted( void )
{
	GameSlot *slot = getSlot(0);
	if (slot)
		slot->setAccept();
	for(int i = 1; i< MAX_SLOTS; i++)
	{
		slot = getSlot(i);
		if (slot)
			slot->unAccept();
	}
}

void GameInfo::resetStartSpots()
{
	GameSlot *slot = NULL;
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		slot = getSlot(i);
		if (slot != NULL)
		{
			slot->setStartPos(-1);
		}
	}
}

// adjust the slots in the game to open or closed
// depending on the players in there now and the number of
// players the map can hold.
void GameInfo::adjustSlotsForMap()
{
    NativeGameInfoTransaction transaction(*this);
    if (!TheMapCache) throw ERROR_BAD_ARG;
	const MapMetaData *md = TheMapCache->findMap(m_mapName);
	if (md != NULL)
	{
		// get the number of players allowed from the map.
		Int numPlayers = md->m_numPlayers;
		Int numPlayerSlots = 0;

		// first get the number of occupied slots.
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			GameSlot *tempSlot = getSlot(i);
			if (tempSlot->isOccupied())
			{
				++numPlayerSlots;
			}
		}

		// now go through and close the appropriate number of slots.
		// note that no players are kicked in this process, we leave
		// that up to the user.
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			// we have room for more players, if this slot is unoccupied, set it to open.
			GameSlot *slot = getSlot(i);
			if (numPlayers > numPlayerSlots)
			{
				if (!(slot->isOccupied()))
				{
					GameSlot newSlot;
					newSlot.setState(SLOT_OPEN);
					setSlot(i, newSlot);
					++numPlayerSlots;
				}
			}
			else
			{
				if (!(slot->isOccupied()))
				{
					// we don't have any more room, set this slot to closed.
					GameSlot newSlot;
					newSlot.setState(SLOT_CLOSED);
					setSlot(i, newSlot);
				}
			}
		}
	}
    transaction.commit();
}

void GameInfo::closeOpenSlots()
{
    NativeGameInfoTransaction transaction(*this);
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = getSlot(i);
		if (!(slot->isOccupied()))
		{
			GameSlot newSlot;
			newSlot.setState(SLOT_CLOSED);
			setSlot(i, newSlot);
		}
	}
    transaction.commit();
}
