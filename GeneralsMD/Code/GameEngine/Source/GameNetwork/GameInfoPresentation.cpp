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
UnicodeString GameSlot::getApparentPlayerTemplateDisplayName( void ) const
{
	if (TheMultiplayerSettings && TheMultiplayerSettings->showRandomPlayerTemplate() &&
		m_origPlayerTemplate == PLAYERTEMPLATE_RANDOM && !isSlotLocalAlly(this))
	{
		return TheGameText->fetch("GUI:Random");
	}
	else if (m_origPlayerTemplate == PLAYERTEMPLATE_OBSERVER)
	{
		return TheGameText->fetch("GUI:Observer");
	}
	DEBUG_LOG(("Fetching player template display name for player template %d (orig is %d)\n",
		m_playerTemplate, m_origPlayerTemplate));
	if (m_playerTemplate < 0)
	{
		return TheGameText->fetch("GUI:Random");
	}
	return ThePlayerTemplateStore->getNthPlayerTemplate(m_playerTemplate)->getDisplayName();
}

Bool GameInfo::isSkirmish(void)
{
	Bool sawAI = FALSE;

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (i == getLocalSlotNum())
			continue;

		if (getConstSlot(i)->isHuman())
			return FALSE;

		if (getConstSlot(i)->isAI())
		{
			if (isSlotLocalAlly(getConstSlot(i)))
				return FALSE;
			sawAI = TRUE;
		}
	}
	return sawAI;
}
