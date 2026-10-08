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

// MessageStream.cpp
// Implementation of the message stream
// Author: Michael S. Booth, February 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/MessageStream.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Recorder.h"
#include <limits>
#include <memory>

#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"

/// The singleton message stream for messages going to TheGameLogic
MessageStream *TheMessageStream = NULL;
CommandList *TheCommandList = NULL;



#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif



//------------------------------------------------------------------------------------------------
// GameMessage
//

/**
 * Constructor
 */
GameMessage::GameMessage(GameMessage::Type type) : GameMessage(type,-1)
{
    if (!ThePlayerList || !ThePlayerList->getLocalPlayer()) throw ERROR_BAD_ARG;
    m_playerIndex=ThePlayerList->getLocalPlayer()->getPlayerIndex();
}


//------------------------------------------------------------------------------------------------
// MessageStream
//


/**
 * Constructor
 */
MessageStream::MessageStream( void )
{
	m_firstTranslator = 0;
	m_lastTranslator = 0;
	m_nextTranslatorID = 1;
}

/**
 * Destructor
 */
MessageStream::~MessageStream()
{
	// destroy all translators
	TranslatorData *trans, *nextTrans;
	for( trans=m_firstTranslator; trans; trans=nextTrans )
	{
		nextTrans = trans->m_next;
		delete trans;
	}
}

/**
	* Init
	*/
void MessageStream::init( void )
{
	// extend
	GameMessageList::init();
} 

/**
	* Reset
	*/
void MessageStream::reset( void )
{

	/// @todo Reset the MessageStream

	// extend
	GameMessageList::reset();

}

/**
	* Update
	*/
void MessageStream::update( void )
{
	// extend
	GameMessageList::update();

}

/**
 * Create a new message of the given message type and append it
 * to this message stream.  Return the message such that any data
 * associated with this message can be attached to it.
 */
GameMessage *MessageStream::appendMessage( GameMessage::Type type )
{
	GameMessage *msg = newInstance(GameMessage)( type );

	// add message to list
	GameMessageList::appendMessage( msg );

	return msg;
}

/**
 * Create a new message of the given message type and insert it
 * in the stream after messageToInsertAfter, which must not be NULL.
 */
GameMessage *MessageStream::insertMessage( GameMessage::Type type, GameMessage *messageToInsertAfter )
{
	GameMessage *msg = newInstance(GameMessage)(type);

	GameMessageList::insertMessage(msg, messageToInsertAfter);

	return msg;
}

/**
 * Attach the given Translator to the message stream, and return a
 * unique TranslatorID identifying it.
 * Translators are placed on a list, sorted by priority order.  If two
 * Translators share a priority, they are kept in the same order they
 * were attached.
 */
TranslatorID MessageStream::attachTranslator( GameMessageTranslator *translator, 
																							UnsignedInt priority)
{
	// Ownership enters before node allocation. Do not consume an ID or publish
	// any link on allocation/rejection; retry retains exact source ordering.
	std::unique_ptr<GameMessageTranslator> candidate(translator);
	if (!translator || m_nextTranslatorID == TRANSLATOR_ID_INVALID ||
		m_nextTranslatorID == std::numeric_limits<TranslatorID>::max())
		throw ERROR_BAD_ARG;
	MessageStream::TranslatorData *newSS = NEW MessageStream::TranslatorData;
	MessageStream::TranslatorData *ss;

	newSS->m_translator = candidate.release();
	newSS->m_priority = priority;
	newSS->m_id = m_nextTranslatorID++;

	if (m_firstTranslator == NULL)
	{
		// first Translator to be attached
		newSS->m_prev = NULL;
		newSS->m_next = NULL;
		m_firstTranslator = newSS;
		m_lastTranslator = newSS;
		return newSS->m_id;
	}

	// seach the Translator list for our priority location
	for( ss=m_firstTranslator; ss; ss=ss->m_next )
		if (ss->m_priority > newSS->m_priority)
			break;

	if (ss)
	{
		// insert new Translator just BEFORE this one,
		// therefore, m_lastTranslator cannot be affected
		if (ss->m_prev)
		{
			ss->m_prev->m_next = newSS;
			newSS->m_prev = ss->m_prev;
			newSS->m_next = ss;
			ss->m_prev = newSS;
		}
		else
		{
			// insert at head of list
			newSS->m_prev = NULL;
			newSS->m_next = m_firstTranslator;
			m_firstTranslator->m_prev = newSS;
			m_firstTranslator = newSS;
		}
	}
	else
	{
		// append Translator to end of list
		m_lastTranslator->m_next = newSS;
		newSS->m_prev = m_lastTranslator;
		newSS->m_next = NULL;
		m_lastTranslator = newSS;
	}

	return newSS->m_id;
}

/**
	* Find a translator attached to this message stream given the ID 
	*/
GameMessageTranslator* MessageStream::findTranslator( TranslatorID id )
{
	MessageStream::TranslatorData *translatorData;

	for( translatorData = m_firstTranslator; translatorData; translatorData = translatorData->m_next )
	{

		if( translatorData->m_id == id )
			return translatorData->m_translator;

	}

	return NULL;

}

/**
 * Remove a previously attached translator.
 */
void MessageStream::removeTranslator( TranslatorID id )
{
	MessageStream::TranslatorData *ss;

	for( ss=m_firstTranslator; ss; ss=ss->m_next )
		if (ss->m_id == id)
		{
			// found the translator - remove it
			if (ss->m_prev)
				ss->m_prev->m_next = ss->m_next;
			else
				m_firstTranslator = ss->m_next;

			if (ss->m_next)
				ss->m_next->m_prev = ss->m_prev;
			else
				m_lastTranslator = ss->m_prev;

			// delete the translator data
			delete ss;

			break;
		}
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#if defined(_DEBUG) || defined(_INTERNAL)

Bool isInvalidDebugCommand( GameMessage::Type t )
{
	// see if this is something that should be prevented in multiplayer games
	// Don't reject this stuff in skirmish games.
	if (TheGameLogic && !TheGameLogic->isInSkirmishGame() && 
			(TheRecorder && TheRecorder->isMultiplayer() && TheRecorder->getMode() == RECORDERMODETYPE_RECORD))
	{
		switch (t)
		{
		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS:
		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA:
		case GameMessage::MSG_META_DEMO_KILL_ALL_ENEMIES:
		case GameMessage::MSG_META_DEMO_KILL_SELECTION:
		case GameMessage::MSG_META_DEMO_TOGGLE_HURT_ME_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS:
		case GameMessage::MSG_META_DEMO_TIME_OF_DAY:
		case GameMessage::MSG_META_DEMO_LOCK_CAMERA_TO_PLANES:
		case GameMessage::MSG_META_DEMO_REMOVE_PREREQ:
		case GameMessage::MSG_META_DEMO_INSTANT_BUILD:
		case GameMessage::MSG_META_DEMO_FREE_BUILD:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT1:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT2:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT3:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT4:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT5:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT6:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT7:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT8:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT9:
		case GameMessage::MSG_META_DEMO_ENSHROUD:
		case GameMessage::MSG_META_DEMO_DESHROUD:
		case GameMessage::MSG_META_DEBUG_GIVE_VETERANCY:
		case GameMessage::MSG_META_DEBUG_TAKE_VETERANCY:
//#pragma MESSAGE ("WARNING - DEBUG key in multiplayer!")
		case GameMessage::MSG_META_DEMO_ADD_CASH:
		case GameMessage::MSG_META_DEBUG_INCR_ANIM_SKATE_SPEED:
		case GameMessage::MSG_META_DEBUG_DECR_ANIM_SKATE_SPEED:
		case GameMessage::MSG_META_DEBUG_CYCLE_EXTENT_TYPE:
		case GameMessage::MSG_META_DEMO_TOGGLE_RENDER:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MAJOR:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MAJOR_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MAJOR:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MAJOR_BIG:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MINOR:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MINOR_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MINOR:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MINOR_BIG:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT_BIG:
		case GameMessage::MSG_META_DEMO_KILL_AREA_SELECTION:
		case GameMessage::MSG_DEBUG_KILL_SELECTION:
		case GameMessage::MSG_DEBUG_HURT_OBJECT:
		case GameMessage::MSG_DEBUG_KILL_OBJECT:
		case GameMessage::MSG_META_DEMO_GIVE_SCIENCEPURCHASEPOINTS:
		case GameMessage::MSG_META_DEMO_GIVE_ALL_SCIENCES:
		case GameMessage::MSG_META_DEMO_GIVE_RANKLEVEL:
		case GameMessage::MSG_META_DEMO_TAKE_RANKLEVEL:
		case GameMessage::MSG_META_DEBUG_WIN:

			return true;
		}
	}
	return false;
}
#endif

/**
 * Propagate messages thru attached Translators, invoking each Translator's
 * callback for each message in the stream.
 * Once all Translators have evaluated the message stream, all messages
 * in the stream are destroyed.
 */
void MessageStream::propagateMessages( void )
{
	MessageStream::TranslatorData *ss;
	GameMessage *msg, *next;

	// process each Translator
	for( ss=m_firstTranslator; ss; ss=ss->m_next )
	{
		for( msg=m_firstMessage; msg; msg=next )
		{			
			if (ss->m_translator 
#if defined(_DEBUG) || defined(_INTERNAL)
				&& !isInvalidDebugCommand(msg->getType())
#endif
				)
			{
				GameMessageDisposition disp = ss->m_translator->translateGameMessage(msg);
				next = msg->next();
				if (disp == DESTROY_MESSAGE)
				{
					msg->deleteInstance();
				}
			} 
			else 
			{
				next = msg->next();
			}
		}
	}


	// transfer all messages that reached the end of the stream to TheCommandList
	TheCommandList->appendMessageList( m_firstMessage );

	// clear the stream
	m_firstMessage = NULL;
	m_lastMessage = NULL;

}


//------------------------------------------------------------------------------------------------
// CommandList
//

/**
 * Constructor
 */
CommandList::CommandList( void )
{
}

/**
 * Destructor
 */
CommandList::~CommandList()
{
	destroyAllMessages();
}

/**
	* Init
	*/
void CommandList::init( void )
{

	// extend
	GameMessageList::init();

} 

/**
	* Destroy all messages on the list, and reset list to empty
	*/
void CommandList::reset( void )
{

	// extend
	GameMessageList::reset();

	// destroy all messages
	destroyAllMessages();

}

/**
	* Update
	*/
void CommandList::update( void )
{

	// extend
	GameMessageList::update();

}

/**
	* Destroy all messages on the command list, this will get called from the
	* destructor and reset methods, DO NOT throw exceptions
	*/
void CommandList::destroyAllMessages( void )
{
	GameMessage *msg, *next;

	for( msg=m_firstMessage; msg; msg=next )
	{
		next = msg->next();
		msg->deleteInstance();
	}
	
	m_firstMessage = NULL;
	m_lastMessage = NULL;

}

/** 
 * Adds messages to the end of TheCommandList.
 * Primarily used by TheMessageStream to put the final messages that reach the end of the 
 * stream on TheCommandList. Since TheGameClient will update faster than TheNetwork 
 * and TheGameLogic, messages will accumulate on this list.
 */
void CommandList::appendMessageList( GameMessage *list ) 
{ 
	GameMessage *msg, *next;

	for( msg = list; msg; msg = next )
	{
		next = msg->next();
		transferMessage( msg );
	}
}

//-----------------------------------------------------------------------------
/**
 * Given an "anchor" point and the current mouse position (dest),
 * construct a valid 2D bounding region.
 */
void buildRegion( const ICoord2D *anchor, const ICoord2D *dest, IRegion2D *region )
{
	// build rectangular region defined by the drag selection
	if (anchor->x < dest->x)
	{
		region->lo.x = anchor->x;
		region->hi.x = dest->x;
	}
	else
	{
		region->lo.x = dest->x;
		region->hi.x = anchor->x;
	}

	if (anchor->y < dest->y)
	{
		region->lo.y = anchor->y;
		region->hi.y = dest->y;
	}
	else
	{
		region->lo.y = dest->y;
		region->hi.y = anchor->y;
	}
}
