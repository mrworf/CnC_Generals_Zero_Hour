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


// Actual original provider split; no alternate implementation.
// Convenience Functions ----------------------------------------

static const char slotListID		= 'S';

AsciiString GameInfoToAsciiString( const GameInfo *game )
{
	if (!game)
		return AsciiString::TheEmptyString;

	AsciiString mapName = game->getMap();
	mapName = TheGameState->realMapPathToPortableMapPath(mapName);
	AsciiString newMapName;
	if (mapName.getLength() > 0)
	{
		AsciiString token;
		mapName.nextToken(&token, "\\/");
		// add all the tokens except the last one.
		// that way we don't add the filename, just the
		// directory name, we can do this since the filename
		// is just the directory name with the file extention
		// added onto it.
		while (mapName.find('\\') != NULL)
		{
			if (newMapName.getLength() > 0)
			{
				newMapName.concat('/');
			}
			newMapName.concat(token);
			mapName.nextToken(&token, "\\/");
		}
		DEBUG_LOG(("Map name is %s\n", mapName.str()));
	}

	AsciiString optionsString;
	optionsString.format("US=%d;M=%2.2x%s;MC=%X;MS=%d;SD=%d;C=%d;SR=%u;SC=%u;O=%c;", game->getUseStats(), game->getMapContentsMask(), newMapName.str(),
		game->getMapCRC(), game->getMapSize(), game->getSeed(), game->getCRCInterval(), game->getSuperweaponRestriction(),
		game->getStartingCash().countMoney(), game->oldFactionsOnly() ? 'Y' : 'N' );

	//add player info for each slot
	optionsString.concat(slotListID);
	optionsString.concat('=');
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);

		AsciiString str;
		if (slot && slot->isHuman())
		{
			AsciiString tmp;  //all this data goes after name
			tmp.format( ",%X,%d,%c%c,%d,%d,%d,%d,%d:",
				slot->getIP(), slot->getPort(),
				(slot->isAccepted()?'T':'F'),
				(slot->hasMap()?'T':'F'),
				slot->getColor(), slot->getPlayerTemplate(),
				slot->getStartPos(), slot->getTeamNumber(),
				slot->getNATBehavior() );
			//make sure name doesn't cause overflow of m_lanMaxOptionsLength
			int lenCur = tmp.getLength() + optionsString.getLength() + 2;  //+2 for H and trailing ;
			int lenRem = m_lanMaxOptionsLength - lenCur;  //length remaining before overflowing
			int lenMax = lenRem / (MAX_SLOTS-i);  //share lenRem with all remaining slots
            if (lenMax<0) throw ERROR_BAD_ARG;
            const auto encoded=nativeEncodePlayerName(slot->getName().str());
            const auto prefix=nativePlayerNamePrefix(encoded,static_cast<size_t>(lenMax));
            AsciiString name(std::string(prefix).c_str());

			str.format( "H%s%s", name.str(), tmp.str() );
		}
		else if (slot && slot->isAI())
		{
			Char c;
			if (slot->getState() == SLOT_EASY_AI)
				c = 'E';
			else if (slot->getState() == SLOT_MED_AI)
				c = 'M';
			else
				c = 'H';
			str.format("C%c,%d,%d,%d,%d:", c,
				slot->getColor(), slot->getPlayerTemplate(),
				slot->getStartPos(), slot->getTeamNumber());
		}
		else if (slot && slot->getState() == SLOT_OPEN)
		{
			str = "O:";
		}
		else if (slot && slot->getState() == SLOT_CLOSED)
		{
			str = "X:";
		}
		else
		{
			DEBUG_ASSERTCRASH(false, ("Bad slot type"));
			str = "X:";
		}
		optionsString.concat(str);
	}
	optionsString.concat(';');

	DEBUG_ASSERTCRASH(!TheLAN || (optionsString.getLength() < m_lanMaxOptionsLength),
		("WARNING: options string is longer than expected!  Length is %d, but max is %d!\n",
		optionsString.getLength(), m_lanMaxOptionsLength));

	return optionsString;
}

static Int grabHexInt(const char *s)
{
    return nativeSourceInteger<UnsignedByte>(std::string_view(s,2),16);
}
Bool ParseAsciiStringToGameInfo(GameInfo *game, AsciiString options)
{
    try {
    if (!TheGlobalData || !TheMapCache || !TheGameState || !ThePlayerTemplateStore || !TheMultiplayerSettings)
        throw ERROR_BAD_ARG;
	// Parse game options
	std::string buffer(options.str(),static_cast<size_t>(options.getLength()));
    char *buf=buffer.data();
	char *bufPtr = buf;
	char *strPos, *keyValPair;
	GameSlot newSlot[MAX_SLOTS];
	Bool optionsOk = true;
	AsciiString mapName;
	Int mapContentsMask;
	UnsignedInt mapCRC, mapSize;
	Int seed = 0;
	Int crc = 100;
	Bool sawCRC = FALSE;
  Bool oldFactionsOnly = FALSE;
	Int useStats = TRUE;
  Money startingCash = TheGlobalData->m_defaultStartingCash;
  UnsignedShort restriction = 0; // Always the default

	Bool sawMap, sawMapCRC, sawMapSize, sawSeed, sawSlotlist, sawUseStats, sawSuperweaponRestriction, sawStartingCash, sawOldFactions;
	sawMap = sawMapCRC = sawMapSize = sawSeed = sawSlotlist = sawUseStats = sawSuperweaponRestriction = sawStartingCash = sawOldFactions = FALSE;

	//DEBUG_LOG(("Saw options of %s\n", options.str()));
	DEBUG_LOG(("ParseAsciiStringToGameInfo - parsing [%s]\n", options.str()));


	while ( (keyValPair = strtok_r(bufPtr, ";", &strPos)) != NULL )
	{
		bufPtr = NULL; // strtok within the same string

		AsciiString key, val;
		char *pos = NULL;
		char *keyPtr, *valPtr;
		keyPtr = (strtok_r(keyValPair, "=", &pos));
		valPtr = (strtok_r(NULL, "\n", &pos));
		if (keyPtr)
			key = keyPtr;
		if (valPtr)
			val = valPtr;

		if (val.isEmpty())
		{
			optionsOk = false;
			DEBUG_LOG(("ParseAsciiStringToGameInfo - saw empty value, quitting\n"));
			break;
		}

		if (key.compare("US") == 0)
		{
			useStats = nativeSourceInteger<Int>(val.str());
			sawUseStats = true;
		}
		else
		if (key.compare("M") == 0)
		{
			if (val.getLength() < 3)
			{
				optionsOk = FALSE;
				DEBUG_LOG(("ParseAsciiStringToGameInfo - saw bogus map; quitting\n"));
				break;
			}
			mapContentsMask = grabHexInt(val.str());
			AsciiString tempstr;
			AsciiString token;
			tempstr = val.str()+2;
			tempstr.nextToken(&token, "\\/");
			while (tempstr.getLength() > 0)
			{
				mapName.concat(token);
				mapName.concat('\\');
				tempstr.nextToken(&token, "\\/");
			}
			mapName.concat(token);
			mapName.concat('\\');
			mapName.concat(token);
			mapName.concat('.');
			mapName.concat(TheMapCache->getMapExtension());
			mapName = TheGameState->portableMapPathToRealMapPath(mapName);
			sawMap = true;
			DEBUG_LOG(("ParseAsciiStringToGameInfo - map name is %s\n", mapName.str()));
		}
		else if (key.compare("MC") == 0)
		{
			mapCRC = 0;
			mapCRC=nativeSourceInteger<UnsignedInt>(val.str(),16);
			sawMapCRC = true;
		}
		else if (key.compare("MS") == 0)
		{
			mapSize = nativeSourceInteger<UnsignedInt>(val.str());
			sawMapSize = true;
		}
		else if (key.compare("SD") == 0)
		{
			seed = nativeSourceInteger<Int>(val.str());
			sawSeed = true;
//			DEBUG_LOG(("ParseAsciiStringToGameInfo - random seed is %d\n", seed));
		}
		else if (key.compare("C") == 0)
		{
			crc = nativeSourceInteger<Int>(val.str());
			sawCRC = TRUE;
		}
    else if (key.compare("SR") == 0 )
    {
      restriction = nativeSourceInteger<UnsignedShort>(val.str());
      sawSuperweaponRestriction = TRUE;
    }
    else if (key.compare("SC") == 0 )
    {
      UnsignedInt startingCashAmount = nativeSourceInteger<UnsignedInt>(val.str());
      startingCash.init();
      startingCash.deposit( startingCashAmount, FALSE );
      sawStartingCash = TRUE;
    }
    else if (key.compare("O") == 0 )
    {
      oldFactionsOnly = ( val.compareNoCase( "Y" ) == 0 );
      sawOldFactions = TRUE;
    }
		else if (key.getLength() == 1 && *key.str() == slotListID)
		{
			sawSlotlist = true;
			/// @TODO: Need to read in all the slot info... big mess right now.
			std::string slotBuffer(val.str(),static_cast<size_t>(val.getLength()));
            char *rawSlotBuf=slotBuffer.data();
			AsciiString rawSlot;
//			Bool slotsOk = true;	//flag that lets us know whether or not the slot list is good.

//			DEBUG_LOG(("ParseAsciiStringToGameInfo - Parsing slot list\n"));
			for (int i=0; i<MAX_SLOTS; ++i)
				{
					rawSlot = strtok_r(rawSlotBuf,":",&pos);
					rawSlotBuf = NULL;
                    std::string rawSlotBacking(rawSlot.str(),static_cast<size_t>(rawSlot.getLength()));
					switch (*rawSlot.str())
					{
						case 'H':
						{
//							DEBUG_LOG(("ParseAsciiStringToGameInfo - Human player\n"));
							char *slotPos = NULL;
							//Parse out the Name
							AsciiString slotValue(strtok_r(rawSlotBacking.data(),",",&slotPos));
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue name is empty, quitting\n"));
								break;
							}
							UnicodeString name;
                            name.set(nativeDecodePlayerName(slotValue.str()+1).c_str());

							//DEBUG_LOG(("ParseAsciiStringToGameInfo - name is %s\n", slotValue.str()+1));

							//Parse out the IP
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue IP address is empty, quitting\n"));
								break;
							}
							UnsignedInt playerIP = 0;
							playerIP=nativeSourceInteger<UnsignedInt>(slotValue.str(),16);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - IP address is %x\n", playerIP));

							//set the state of the slot
							newSlot[i].setState(SLOT_PLAYER, name, playerIP);

							// parse out the port
							slotValue = strtok_r(NULL, ",", &slotPos);
							if (slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue port is empty, quitting\n"));
								break;
							}
							UnsignedInt playerPort = 0;
							playerPort=nativeSourceInteger<UnsignedShort>(slotValue.str());
							newSlot[i].setPort(playerPort);
							DEBUG_LOG(("ParseAsciiStringToGameInfo - port is %d\n", playerPort));

							//Read if it's accepted or not
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.getLength() != 2)
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue accepted is mis-sized, quitting\n"));
								break;
							}
							const char *svs = slotValue.str();
							if(*svs == 'T') {
								newSlot[i].setAccept();
								//DEBUG_LOG(("ParseAsciiStringToGameInfo - player has accepted\n"));
							} else if (*svs == 'F') {
								newSlot[i].unAccept();
								//DEBUG_LOG(("ParseAsciiStringToGameInfo - player has not accepted\n"));
							}
							++svs;
							if(*svs == 'T') {
								newSlot[i].setMapAvailability(TRUE);
								//DEBUG_LOG(("ParseAsciiStringToGameInfo - player has map\n"));
							} else {
								newSlot[i].setMapAvailability(FALSE);
								//DEBUG_LOG(("ParseAsciiStringToGameInfo - player does not have map\n"));
							}

							//Read color index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue color is empty, quitting\n"));
								break;
							}
							Int color = nativeSourceInteger<Int>(slotValue.str());
							if (color < -1 || color >= TheMultiplayerSettings->getNumColors())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - player color was invalid, quitting\n"));
								break;
							}
							newSlot[i].setColor(color);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - player color set to %d\n", color));

							//Read playerTemplate index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue player template is empty, quitting\n"));
								break;
							}
							Int playerTemplate = nativeSourceInteger<Int>(slotValue.str());
							if (playerTemplate < PLAYERTEMPLATE_MIN || playerTemplate >= ThePlayerTemplateStore->getPlayerTemplateCount())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - player template value is invalid, quitting\n"));
								break;
							}
							newSlot[i].setPlayerTemplate(playerTemplate);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - player template is %d\n", playerTemplate));

							//Read start position index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue start position is empty, quitting\n"));
								break;
							}
							Int startPos = nativeSourceInteger<Int>(slotValue.str());
							if (startPos < -1 || startPos >= MAX_SLOTS)
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - player start position is invalid, quitting\n"));
								break;
							}
							newSlot[i].setStartPos(startPos);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - player start position is %d\n", startPos));

							//Read team index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue team number is empty, quitting\n"));
								break;
							}
							Int team = nativeSourceInteger<Int>(slotValue.str());
							if (team < -1 || team >= MAX_SLOTS/2)
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - team number is invalid, quitting\n"));
								break;
							}
							newSlot[i].setTeamNumber(team);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - team number is %d\n", team));

							// Read the NAT behavior
							slotValue = strtok_r(NULL, ",",&slotPos);
							if (slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - NAT behavior is empty, quitting\n"));
								break;
							}
							Int NATType = nativeSourceInteger<Int>(slotValue.str());
							if ((NATType < FirewallHelperClass::FIREWALL_MIN) ||
									(NATType > FirewallHelperClass::FIREWALL_MAX)) {
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - NAT behavior is invalid, quitting\n"));
								break;
							}
							newSlot[i].setNATBehavior(static_cast<FirewallHelperClass::FirewallBehaviorType>(NATType));
							DEBUG_LOG(("ParseAsciiStringToGameInfo - NAT behavior is %X\n", NATType));
						}// case 'H':
						break;
						case 'C':
						{
							DEBUG_LOG(("ParseAsciiStringToGameInfo - AI player\n"));
							char *slotPos = NULL;
							//Parse out the Name
							AsciiString slotValue(strtok_r(rawSlotBacking.data(),",",&slotPos));
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue AI Type is empty, quitting\n"));
								break;
							}

							switch(*(slotValue.str() + 1))
							{
								case 'E':
								{
									newSlot[i].setState(SLOT_EASY_AI);
									//DEBUG_LOG(("ParseAsciiStringToGameInfo - Easy AI\n"));
								}
								break;
								case 'M':
								{
									newSlot[i].setState(SLOT_MED_AI);
									//DEBUG_LOG(("ParseAsciiStringToGameInfo - Medium AI\n"));
								}
								break;
								case 'H':
								{
									newSlot[i].setState(SLOT_BRUTAL_AI);
									//DEBUG_LOG(("ParseAsciiStringToGameInfo - Brutal AI\n"));
								}
								break;
								default:
								{
									optionsOk = false;
									DEBUG_LOG(("ParseAsciiStringToGameInfo - Unknown AI, quitting\n"));
								}
								break;
							}//switch(*rawSlot.str()+1)

							//Read color index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue color is empty, quitting\n"));
								break;
							}
							Int color = nativeSourceInteger<Int>(slotValue.str());
							if (color < -1 || color >= TheMultiplayerSettings->getNumColors())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - player color was invalid, quitting\n"));
								break;
							}
							newSlot[i].setColor(color);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - player color set to %d\n", color));

							//Read playerTemplate index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue player template is empty, quitting\n"));
								break;
							}
							Int playerTemplate = nativeSourceInteger<Int>(slotValue.str());
							if (playerTemplate < PLAYERTEMPLATE_MIN || playerTemplate >= ThePlayerTemplateStore->getPlayerTemplateCount())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - player template value is invalid, quitting\n"));
								break;
							}
							newSlot[i].setPlayerTemplate(playerTemplate);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - player template is %d\n", playerTemplate));

							//Read start pos
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue start pos is empty, quitting\n"));
								break;
							}
							Int startPos = nativeSourceInteger<Int>(slotValue.str());
							Bool isStartPosBad = FALSE;
							if (startPos < -1 || startPos >= MAX_SLOTS)
							{
								isStartPosBad = TRUE;
							}
							for (Int j=0; j<i; ++j)
							{
								if (startPos >= 0 && startPos == newSlot[j].getStartPos())
								{
									isStartPosBad = TRUE; // can't have multiple people using the same start pos
								}
							}
							if (isStartPosBad)
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - start pos is invalid, quitting\n"));
								break;
							}
							newSlot[i].setStartPos(startPos);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - start spot is %d\n", startPos));

							//Read team index
							slotValue = strtok_r(NULL,",",&slotPos);
							if(slotValue.isEmpty())
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - slotValue team number is empty, quitting\n"));
								break;
							}
							Int team = nativeSourceInteger<Int>(slotValue.str());
							if (team < -1 || team >= MAX_SLOTS/2)
							{
								optionsOk = false;
								DEBUG_LOG(("ParseAsciiStringToGameInfo - team number is invalid, quitting\n"));
								break;
							}
							newSlot[i].setTeamNumber(team);
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - team number is %d\n", team));

						}//case 'C':
						break;
						case 'O':
						{
							newSlot[i].setState( SLOT_OPEN );
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - Slot is open\n"));
						}// case 'O':
						break;
						case 'X':
						{
							newSlot[i].setState( SLOT_CLOSED );
							//DEBUG_LOG(("ParseAsciiStringToGameInfo - Slot is closed\n"));
						}// case 'X':
						break;
						default:
						{
							optionsOk = false;
							DEBUG_LOG(("ParseAsciiStringToGameInfo - unrecognized slot entry, quitting\n"));
						}
						break;
					}
				}
		}
		else
		{
			optionsOk = false;
			break;
		}
	}

	//DEBUG_LOG(("Options were ok == %d\n", optionsOk));
	if (optionsOk && sawMap && sawMapCRC && sawMapSize && sawSeed && sawSlotlist && sawCRC && sawUseStats && sawSuperweaponRestriction && sawStartingCash && sawOldFactions )
	{
		// We were setting the Global Data directly here, but Instead, I'm now
		// first setting the data in game.  We'll set the global data when
		// we start a game.
		if (!game)
			return true;

		//DEBUG_LOG(("ParseAsciiStringToGameInfo - game options all good, setting info\n"));

		NativeGameInfoTransaction transaction(*game);
		for(Int i = 0; i<MAX_SLOTS; i++)
			game->setSlot(i,newSlot[i]);

		game->setMap(mapName);
		game->setMapCRC(mapCRC);
		game->setMapSize(mapSize);
		game->setMapContentsMask(mapContentsMask);
		game->setSeed(seed);
		game->setCRCInterval(crc);
		game->setUseStats(useStats);
    game->setSuperweaponRestriction(restriction);
    game->setStartingCash( startingCash );
    game->setOldFactionsOnly( oldFactionsOnly );

		transaction.commit();
		return true;
	}

	DEBUG_LOG(("ParseAsciiStringToGameInfo - game options messed up\n"));
	return false;
    } catch (ErrorCode error) { if (error==ERROR_BAD_ARG) return false; throw; }

}


//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

//------------------------- SkirmishGameInfo ---------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SkirmishGameInfo::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void SkirmishGameInfo::xfer( Xfer *xfer )
{
	const XferVersion currentVersion = 4;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );


	xfer->xferInt(&m_preorderMask);
	xfer->xferInt(&m_crcInterval);
	xfer->xferBool(&m_inGame);
	xfer->xferBool(&m_inProgress);
	xfer->xferBool(&m_surrendered);
	xfer->xferInt(&m_gameID);

	Int slot = MAX_SLOTS;
	xfer->xferInt(&slot);
	DEBUG_ASSERTCRASH(slot==MAX_SLOTS, ("MAX_SLOTS changed, need to change version. jba."));

	for (slot = 0; slot < MAX_SLOTS; slot++)
	{
		Int state = m_slot[slot]->getState();
		xfer->xferInt(&state);

		UnicodeString name=m_slot[slot]->getName();
		if (version >= 2)
		{
			xfer->xferUnicodeString(&name);
		}

		Bool isAccepted=m_slot[slot]->isAccepted();
		xfer->xferBool(&isAccepted);

		Bool isMuted=m_slot[slot]->isMuted();
		xfer->xferBool(&isMuted);
		m_slot[slot]->mute(isMuted);

		Int color=m_slot[slot]->getColor();
		xfer->xferInt(&color);

		Int startPos=m_slot[slot]->getStartPos();
		xfer->xferInt(&startPos);

		Int playerTemplate=m_slot[slot]->getPlayerTemplate();
		xfer->xferInt(&playerTemplate);

		Int teamNumber=m_slot[slot]->getTeamNumber();
		xfer->xferInt(&teamNumber);

		Int origColor=m_slot[slot]->getOriginalColor();
		xfer->xferInt(&origColor);

		Int origStartPos=m_slot[slot]->getOriginalStartPos();
		xfer->xferInt(&origStartPos);

		Int origPlayerTemplate=m_slot[slot]->getOriginalPlayerTemplate();
		xfer->xferInt(&origPlayerTemplate);

		if( xfer->getXferMode() == XFER_LOAD ) {
			m_slot[slot]->setState((SlotState)state, name);
			if (isAccepted) m_slot[slot]->setAccept();

			m_slot[slot]->setPlayerTemplate(origPlayerTemplate);
			m_slot[slot]->setStartPos(origStartPos);
			m_slot[slot]->setColor(origColor);
			m_slot[slot]->saveOffOriginalInfo();

			m_slot[slot]->setTeamNumber(teamNumber);
			m_slot[slot]->setColor(color);
			m_slot[slot]->setStartPos(startPos);
			m_slot[slot]->setPlayerTemplate(playerTemplate);
		}
	}

	xfer->xferUnsignedInt(&m_localIP);

	xfer->xferMapName(&m_mapName);
	xfer->xferUnsignedInt(&m_mapCRC);
	xfer->xferUnsignedInt(&m_mapSize);
	xfer->xferInt(&m_mapMask);
	xfer->xferInt(&m_seed);

  if ( version >= 3 )
  {
    xfer->xferUnsignedShort( &m_superweaponRestriction );

    if ( version == 3 )
    {
      // Version 3 had a bool which is now gone
      Bool obsoleteBool;
      xfer->xferBool( &obsoleteBool );
    }

    xfer->xferSnapshot( &m_startingCash );
  }
  else if ( xfer->getXferMode() == XFER_LOAD )
  {
    m_superweaponRestriction = 0;
    m_startingCash = TheGlobalData->m_defaultStartingCash;
  }

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SkirmishGameInfo::loadPostProcess( void )
{
}  // end loadPostProcess
