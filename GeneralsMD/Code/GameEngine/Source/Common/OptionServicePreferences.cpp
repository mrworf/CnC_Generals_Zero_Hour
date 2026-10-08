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


// Actual presentation/campaign preference defaults; their services retain ownership.
#include "PreRTS.h"
#include "Common/UserPreferences.h"
#include "Common/GlobalData.h"
#include "Common/GameAudio.h"
#include "Common/AudioSettings.h"
#include "Common/GameLOD.h"
#include "GameLogic/ScriptEngine.h"
Int OptionPreferences::getCampaignDifficulty(void)
{
	OptionPreferences::const_iterator it = find("CampaignDifficulty");
	if (it == end())
		return TheScriptEngine->getGlobalDifficulty();

	Int factor = getInt(it->first, 0);
	if (factor < DIFFICULTY_EASY)
		factor = DIFFICULTY_EASY;
	if (factor > DIFFICULTY_HARD)
		factor = DIFFICULTY_HARD;
	
	return factor;
}

void OptionPreferences::setCampaignDifficulty( Int diff )
{
	AsciiString prefString;
	prefString.format("%d", diff );
	(*this)["CampaignDifficulty"] = prefString;
}

Int OptionPreferences::getIdealStaticGameDetail(void)
{
	OptionPreferences::const_iterator it = find("IdealStaticGameLOD");
	if (it == end())
		return STATIC_GAME_LOD_UNKNOWN;

	return TheGameLODManager->getStaticGameLODIndex(it->second);
}

Int OptionPreferences::getStaticGameDetail(void)
{
	OptionPreferences::const_iterator it = find("StaticGameLOD");
	if (it == end())
		return TheGameLODManager->getStaticLODLevel();

	return TheGameLODManager->getStaticGameLODIndex(it->second);
}

AsciiString OptionPreferences::getPreferred3DProvider(void)
{
	OptionPreferences::const_iterator it = find("3DAudioProvider");
	if (it == end())
		return TheAudio->getAudioSettings()->m_preferred3DProvider[MAX_HW_PROVIDERS];
	return it->second;
}

AsciiString OptionPreferences::getSpeakerType(void)
{
	OptionPreferences::const_iterator it = find("SpeakerType");
	if (it == end())
		return TheAudio->translateUnsignedIntToSpeakerType(TheAudio->getAudioSettings()->m_defaultSpeakerType2D);
	return it->second;
}

Real OptionPreferences::getSoundVolume(void)
{
	OptionPreferences::const_iterator it = find("SFXVolume");
	if (it == end())
	{
		Real relative = TheAudio->getAudioSettings()->m_relative2DVolume;
		if( relative < 0 )
		{
			Real scale = 1.0f + relative;
			return TheAudio->getAudioSettings()->m_defaultSoundVolume * 100.0f * scale;
		}
		return TheAudio->getAudioSettings()->m_defaultSoundVolume * 100.0f;
	}

	Real volume = getReal(it->first, 0.0f);
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	return volume;
}

Real OptionPreferences::get3DSoundVolume(void)
{
	OptionPreferences::const_iterator it = find("SFX3DVolume");
	if (it == end())
	{
		Real relative = TheAudio->getAudioSettings()->m_relative2DVolume;
		if( relative > 0 )
		{
			Real scale = 1.0f - relative;
			return TheAudio->getAudioSettings()->m_default3DSoundVolume * 100.0f * scale;
		}
		return TheAudio->getAudioSettings()->m_default3DSoundVolume * 100.0f;
	}

	Real volume = getReal(it->first, 0.0f);
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	return volume;
}

Real OptionPreferences::getSpeechVolume(void)
{
	OptionPreferences::const_iterator it = find("VoiceVolume");
	if (it == end())
		return TheAudio->getAudioSettings()->m_defaultSpeechVolume * 100.0f;

	Real volume = getReal(it->first, 0.0f);
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	return volume;
}

Real OptionPreferences::getMusicVolume(void)
{
	OptionPreferences::const_iterator it = find("MusicVolume");
	if (it == end())
		return TheAudio->getAudioSettings()->m_defaultMusicVolume * 100.0f;

	Real volume = getReal(it->first, 0.0f);
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	return volume;
}
