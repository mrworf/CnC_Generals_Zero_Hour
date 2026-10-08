// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.
// Actual source data constructors/copies, independent of playback providers.
#include "Common/AudioEventRTS.h"
#include "Common/AudioEventInfo.h"

AudioEventRTS::AudioEventRTS()
									: m_eventName(AsciiString::TheEmptyString), 
										m_priority(AP_NORMAL), 
										m_volume(-1.0),
										m_timeOfDay(TIME_OF_DAY_AFTERNOON),
										m_ownerType(OT_INVALID),
										m_shouldFade(false),
										m_isLogicalAudio(false),
										m_filenameToLoad(AsciiString::TheEmptyString),
										m_eventInfo(NULL),
										m_playingHandle(0),
										m_killThisHandle(0),
										m_pitchShift(1.0),
										m_volumeShift(0.0),
										m_loopCount(1),
										m_playingAudioIndex(-1),
										m_allCount(0),
										m_playerIndex(-1),
										m_delay(0.0f),
										m_uninterruptable(FALSE)
{
	m_attackName.clear();
	m_decayName.clear();
	m_positionOfAudio.zero();
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS::AudioEventRTS( const AsciiString& eventName )
									: m_eventName(eventName), 
										m_priority(AP_NORMAL), 
										m_volume(-1.0),
										m_timeOfDay(TIME_OF_DAY_AFTERNOON),
										m_ownerType(OT_INVALID),
										m_shouldFade(false),
										m_isLogicalAudio(false),
										m_filenameToLoad(AsciiString::TheEmptyString),
										m_eventInfo(NULL),
										m_playingHandle(0),
										m_killThisHandle(0),
										m_pitchShift(1.0),
										m_volumeShift(0.0),
										m_loopCount(1),
										m_playingAudioIndex(-1),
										m_allCount(0),
										m_playerIndex(-1),
										m_delay(0.0f),
										m_uninterruptable(FALSE)
{
	m_attackName.clear();
	m_decayName.clear();
	m_positionOfAudio.zero();
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS::AudioEventRTS( const AsciiString& eventName, ObjectID ownerID )
									: m_eventName(eventName), 
										m_priority(AP_NORMAL), 
										m_volume(-1.0),
										m_timeOfDay(TIME_OF_DAY_AFTERNOON),
										m_objectID(ownerID),
										m_ownerType(OT_INVALID),
										m_shouldFade(false),
										m_isLogicalAudio(false),
										m_filenameToLoad(AsciiString::TheEmptyString),
										m_eventInfo(NULL),
										m_playingHandle(0),
										m_killThisHandle(0),
										m_pitchShift(1.0),
										m_volumeShift(0.0),
										m_loopCount(1),
										m_playingAudioIndex(-1),
										m_allCount(0),
										m_playerIndex(-1),
										m_delay(0.0f),
										m_uninterruptable(FALSE)
{						
	m_attackName.clear();
	m_decayName.clear();

	if(	m_objectID ) 
	{
		m_ownerType = OT_Object;
	} 
	else 
	{
		m_objectID = INVALID_ID;
	}
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS::AudioEventRTS( const AsciiString& eventName, DrawableID drawableID )
									: m_eventName(eventName), 
										m_priority(AP_NORMAL), 
										m_volume(-1.0),
										m_timeOfDay(TIME_OF_DAY_AFTERNOON),
										m_drawableID(drawableID),
										m_ownerType(OT_INVALID),
										m_shouldFade(false),
										m_isLogicalAudio(false),
										m_filenameToLoad(AsciiString::TheEmptyString),
										m_eventInfo(NULL),
										m_playingHandle(0),
										m_killThisHandle(0),
										m_pitchShift(1.0),
										m_volumeShift(0.0),
										m_loopCount(1),
										m_playingAudioIndex(-1),
										m_allCount(0),
										m_playerIndex(-1),
										m_delay(0.0f),
										m_uninterruptable(FALSE)
{
	m_attackName.clear();
	m_decayName.clear();

	if( m_drawableID )
	{
		m_ownerType = OT_Drawable;
	} 
	else 
	{
		m_drawableID = INVALID_DRAWABLE_ID;
	}
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS::AudioEventRTS( const AsciiString& eventName, const Coord3D *positionOfAudio )
									: m_eventName(eventName), 
										m_priority(AP_NORMAL), 
										m_volume(-1.0),
										m_timeOfDay(TIME_OF_DAY_AFTERNOON),
										m_ownerType(OT_Positional),
										m_shouldFade(false),
										m_isLogicalAudio(false),
										m_filenameToLoad(AsciiString::TheEmptyString),
										m_eventInfo(NULL),
										m_playingHandle(0),
										m_killThisHandle(0),
										m_pitchShift(1.0),
										m_volumeShift(0.0),
										m_loopCount(1),
										m_playingAudioIndex(-1),
										m_allCount(0),
										m_playerIndex(-1),
										m_delay(0.0f),
										m_uninterruptable(FALSE)
{
	m_positionOfAudio.set( positionOfAudio );
	m_attackName.clear();
	m_decayName.clear();
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS::AudioEventRTS( const AudioEventRTS& right )
{
	m_filenameToLoad			= right.m_filenameToLoad;
	m_eventInfo						= right.m_eventInfo;
	m_playingHandle				= right.m_playingHandle;
	m_killThisHandle			= right.m_killThisHandle;
	m_eventName						= right.m_eventName;
	m_priority						= right.m_priority;
	m_volume							= right.m_volume;
	m_timeOfDay						= right.m_timeOfDay;
	m_ownerType						= right.m_ownerType;
	m_shouldFade					= right.m_shouldFade;
	m_isLogicalAudio			= right.m_isLogicalAudio;
	m_pitchShift					= right.m_pitchShift;
	m_volumeShift					= right.m_volumeShift;
	m_loopCount						= right.m_loopCount;
	m_playingAudioIndex		= right.m_playingAudioIndex;
	m_allCount						= right.m_allCount;
	m_playerIndex					= right.m_playerIndex;
	m_delay								= right.m_delay;
	m_attackName					= right.m_attackName;
	m_decayName						= right.m_decayName;
	m_portionToPlayNext		= right.m_portionToPlayNext;
	m_uninterruptable			= right.m_uninterruptable;

	if( m_ownerType == OT_Positional || m_ownerType == OT_Dead ) 
	{
		m_positionOfAudio.set( &right.m_positionOfAudio );
	} 
	else if( m_ownerType == OT_Drawable ) 
	{
		m_drawableID = right.m_drawableID;
	} 
	else if( m_ownerType == OT_Object ) 
	{
		m_objectID = right.m_objectID;
	}

}

//-------------------------------------------------------------------------------------------------
AudioEventRTS& AudioEventRTS::operator=( const AudioEventRTS& right )
{
	m_filenameToLoad			= right.m_filenameToLoad;
	m_eventInfo						= right.m_eventInfo;
	m_playingHandle				= right.m_playingHandle;
	m_killThisHandle			= right.m_killThisHandle;
	m_eventName						= right.m_eventName;
	m_priority						= right.m_priority;
	m_volume							= right.m_volume;
	m_timeOfDay						= right.m_timeOfDay;
	m_ownerType						= right.m_ownerType;
	m_shouldFade					= right.m_shouldFade;
	m_isLogicalAudio			= right.m_isLogicalAudio;
	m_pitchShift					= right.m_pitchShift;
	m_volumeShift					= right.m_volumeShift;
	m_loopCount						= right.m_loopCount;
	m_playingAudioIndex		= right.m_playingAudioIndex;
	m_allCount						= right.m_allCount;
	m_playerIndex					= right.m_playerIndex;
	m_delay								= right.m_delay;
	m_attackName					= right.m_attackName;
	m_decayName						= right.m_decayName;
	m_portionToPlayNext		= right.m_portionToPlayNext;
	m_uninterruptable			= right.m_uninterruptable;

	if( m_ownerType == OT_Positional || m_ownerType == OT_Dead ) 
	{
		m_positionOfAudio.set( &right.m_positionOfAudio );
	} 
	else if( m_ownerType == OT_Drawable ) 
	{
		m_drawableID = right.m_drawableID;
	} 
	else if( m_ownerType == OT_Object ) 
	{
		m_objectID = right.m_objectID;
	} 
	return *this;

}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void AudioEventRTS::setEventName( AsciiString name )
{
	if ((name != m_eventName) && m_eventInfo != NULL) {
		// Clear out the audio event info, cause its not valid for the new event.
		m_eventInfo = NULL;
	}

	m_eventName = name;
}

//-------------------------------------------------------------------------------------------------
