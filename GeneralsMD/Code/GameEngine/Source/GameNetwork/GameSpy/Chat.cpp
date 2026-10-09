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

// FILE: Chat.cpp //////////////////////////////////////////////////////
// Generals GameSpy chat-related code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/INI.h"
#include "GameClient/GameText.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/LanguageFilter.h"
#include "GameClient/GameWindowManager.h"
#include "GameNetwork/GameSpy/PeerDefsImplementation.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameClient/InGameUI.h"
#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// Shared OnlineChatColors data/parser lives in Common/INI/INIOnlineChatColors.cpp.

Bool GameSpyInfo::sendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox )
{
	static UnicodeString s_prevMsg = UnicodeString::TheEmptyString;  //stop spam before it happens

	RoomType roomType = StagingRoom;
	if (getCurrentGroupRoom())
		roomType = GroupRoom;

	PeerRequest req;
	req.text = message.str();

	message.trim();
	// Echo the user's input to the chat window
	if (!message.isEmpty())
	{
		if (!playerListbox)
		{	// Public message
			if( isAction  ||  message.compare(s_prevMsg) != 0 )  //don't send duplicate messages
			{
				req.message.isAction = isAction;
				req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEROOM;
				TheGameSpyPeerMessageQueue->addRequest(req);
				s_prevMsg = message;
			}
			return false;
		}

		// Get the selections (is this a private message?)
		Int maxSel = GadgetListBoxGetListLength(playerListbox);
		Int *selections;
		GadgetListBoxGetSelected(playerListbox, (Int *)&selections);

		if (selections[0] == -1)
		{	// Public message
			if( isAction  ||  message.compare(s_prevMsg) != 0 )  //don't send duplicate messages
			{
				req.message.isAction = isAction;
				req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEROOM;
				TheGameSpyPeerMessageQueue->addRequest(req);
				s_prevMsg = message;
			}
			return false;
		}
		else
		{
			// Private message

			// Construct a list
			AsciiString names = AsciiString::TheEmptyString;
			AsciiString tmp = AsciiString::TheEmptyString;
			AsciiString aStr; // AsciiString buf for translating Unicode entries
			names.format("%s", TheGameSpyInfo->getLocalName().str());
			for (int i=0; i<maxSel; i++)
			{
				if (selections[i] != -1)
				{
					aStr.translate(GadgetListBoxGetText(playerListbox, selections[i], GadgetListBoxGetNumColumns(playerListbox)-1));
					if (aStr.compareNoCase(TheGameSpyInfo->getLocalName()))
					{
						tmp.format(",%s", aStr.str());
						names.concat(tmp);
					}
				}
				else
				{
					break;
				}
			}

			if (!names.isEmpty())
			{
				req.nick = names.str();
				req.message.isAction = isAction;
				req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEPLAYER;
				TheGameSpyPeerMessageQueue->addRequest(req);
			}
			s_prevMsg = message;
			return true;
		}
	}
	s_prevMsg = message;
	return false;
}

void GameSpyInfo::addChat( AsciiString nick, Int profileID, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win )
{
	PlayerInfoMap::iterator it = getPlayerInfoMap()->find(nick);
	if (it != getPlayerInfoMap()->end())
	{
		addChat( it->second, msg, isPublic, isAction, win );
	}
	else
	{
	}
}

void GameSpyInfo::addChat( PlayerInfo p, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win )
{
	Int style;
	if(isSavedIgnored(p.m_profileID) || isIgnored(p.m_name))
		return;
	
	Bool isOwner = p.m_flags & PEER_FLAG_OP;
	Bool isBuddy = getBuddyMap()->find(p.m_profileID) != getBuddyMap()->end();

	Bool isMe = p.m_name.compare(TheGameSpyInfo->getLocalName()) == 0;

	if(!isMe)
	{
		if(m_disallowAsainText)
		{
			const WideChar *buff = msg.str();
			Int length =  msg.getLength();	
			for(Int i = 0; i < length; ++i)
			{
				if(buff[i] >= 256)
					return;
			}
		}
		else if(m_disallowNonAsianText)
		{
			const WideChar *buff = msg.str();
			Int length =  msg.getLength();	
			Bool hasUnicode = FALSE;
			for(Int i = 0; i < length; ++i)
			{
				if(buff[i] >= 256)
				{
					hasUnicode = TRUE;
					break;
				}
			}
			if(!hasUnicode)
				return;
		}

		if (!isPublic)
		{
			AudioEventRTS privMsgAudio("GUIMessageReceived");

			if( TheAudio )
			{
				TheAudio->addAudioEvent( &privMsgAudio );
			}  // end if
		}
	}


	if (isBuddy)
	{
		style = GSCOLOR_CHAT_BUDDY;
	}
	else if (isPublic && isAction)
	{
		style = (isOwner)?GSCOLOR_CHAT_OWNER_EMOTE:GSCOLOR_CHAT_EMOTE;
	}
	else if (isPublic)
	{
		style = (isOwner)?GSCOLOR_CHAT_OWNER:GSCOLOR_CHAT_NORMAL;
	}
	else if (isAction)
	{
		style = (isOwner)?GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE:GSCOLOR_CHAT_PRIVATE_EMOTE;
	}
	else
	{
		style = (isOwner)?GSCOLOR_CHAT_PRIVATE_OWNER:GSCOLOR_CHAT_PRIVATE;
	}

	UnicodeString name;
	name.translate(p.m_name);

	// filters language
//  if( TheGlobalData->m_languageFilterPref )
//  {
    TheLanguageFilter->filterLine(msg);
//  }

	UnicodeString fullMsg;
	if (isAction)
	{
		fullMsg.format( L"%ls %ls", name.str(), msg.str() );
	}
	else
	{
		fullMsg.format( L"[%ls] %ls", name.str(), msg.str() );
	}

	Int index = addText(fullMsg, GameSpyColor[style], win);
	if (index >= 0)
	{
		GadgetListBoxSetItemData(win, (void *)p.m_profileID, index);
	}
}

Int GameSpyInfo::addText( UnicodeString message, Color c, GameWindow *win )
{
	if (TheGameSpyGame && TheGameSpyGame->isInGame() && TheGameSpyGame->isGameInProgress())
	{
		static AudioEventRTS messageFromChatSound("GUIMessageReceived");
		TheAudio->addAudioEvent(&messageFromChatSound);

		TheInGameUI->message(message);
	}

	if (!win)
	{
		// try to pick up a registered text window
		if (m_textWindows.empty())
			return -1;

		win = *(m_textWindows.begin());
	}
	Int index = GadgetListBoxAddEntryText(win, message, c, -1, -1);
	GadgetListBoxSetItemData(win, (void *)-1, index);

	return index;
}

void GameSpyInfo::registerTextWindow( GameWindow *win )
{
	m_textWindows.insert(win);
}

void GameSpyInfo::unregisterTextWindow( GameWindow *win )
{
	m_textWindows.erase(win);
}

