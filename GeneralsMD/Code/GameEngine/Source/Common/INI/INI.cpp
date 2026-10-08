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

// FILE: INI.cpp //////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   INI Reader
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <strings.h>
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_DEATH_NAMES

#include "Common/INI.h"
#include "Common/INIException.h"

#include "Common/DamageFX.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameAudio.h"
#include "Common/Science.h"
#include "Common/SpecialPower.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "Common/Xfer.h"
#include "Common/XferCRC.h"

#include "GameClient/Anim2D.h"
#include "GameClient/Color.h"
#include "GameClient/FXList.h"
#include "GameClient/GameText.h"
#include "GameClient/Image.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/Armor.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/FPUControl.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Weapon.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// INI transfer ownership is per instance, not a process-global borrowed pointer.

//-------------------------------------------------------------------------------------------------
/** This is the table of data types we can have in INI files.  To add a new data type
	* block make a new entry in this table and add an appropriate parsing function */
//-------------------------------------------------------------------------------------------------
extern void parseReallyLowMHz( INI* ini);		// yeah, so sue me (srj)
static const INIBlockDefinition theTypeTable[] =
{
	{ "AIData",							INI::parseAIDataDefinition },
	{ "Animation",					INI::parseAnim2DDefinition },
	{ "Armor",							INI::parseArmorDefinition },
	{ "AudioEvent",					INI::parseAudioEventDefinition },
	{ "AudioSettings",			INI::parseAudioSettingsDefinition },
	{ "Bridge",							INI::parseTerrainBridgeDefinition },
	{ "Campaign",						INI::parseCampaignDefinition },
 	{ "ChallengeGenerals",				INI::parseChallengeModeDefinition },
	{ "CommandButton",			INI::parseCommandButtonDefinition },
	{ "CommandMap",					INI::parseMetaMapDefinition },
	{ "CommandSet",					INI::parseCommandSetDefinition },
	{ "ControlBarScheme",		INI::parseControlBarSchemeDefinition },
	{ "ControlBarResizer",	INI::parseControlBarResizerDefinition },
	{ "CrateData",					INI::parseCrateTemplateDefinition },
	{ "Credits",						INI::parseCredits},
	{ "WindowTransition",		INI::parseWindowTransitions},
	{ "DamageFX",						INI::parseDamageFXDefinition },
	{ "DialogEvent",				INI::parseDialogDefinition },
	{ "DrawGroupInfo",		INI::parseDrawGroupNumberDefinition },
	{ "EvaEvent",						INI::parseEvaEvent },
	{ "FXList",							INI::parseFXListDefinition },
	{ "GameData",						INI::parseGameDataDefinition },
	{ "InGameUI",						INI::parseInGameUIDefinition },
	{ "Locomotor",					INI::parseLocomotorTemplateDefinition },
	{ "Language",						INI::parseLanguageDefinition },
	{ "MapCache",						INI::parseMapCacheDefinition },
	{ "MapData",						INI::parseMapDataDefinition },
	{ "MappedImage",				INI::parseMappedImageDefinition },
	{ "MiscAudio",					INI::parseMiscAudio},
	{ "Mouse",							INI::parseMouseDefinition },
	{ "MouseCursor",				INI::parseMouseCursorDefinition },
	{ "MultiplayerColor",		INI::parseMultiplayerColorDefinition },
  { "MultiplayerStartingMoneyChoice",		INI::parseMultiplayerStartingMoneyChoiceDefinition },
	{ "OnlineChatColors",		INI::parseOnlineChatColorDefinition },
	{ "MultiplayerSettings",INI::parseMultiplayerSettingsDefinition },
	{ "MusicTrack",					INI::parseMusicTrackDefinition },
	{ "Object",							INI::parseObjectDefinition },
	{ "ObjectCreationList",	INI::parseObjectCreationListDefinition },
	{ "ObjectReskin",				INI::parseObjectReskinDefinition },
	{ "ParticleSystem",			INI::parseParticleSystemDefinition },
	{ "PlayerTemplate",			INI::parsePlayerTemplateDefinition },
	{ "Road",								INI::parseTerrainRoadDefinition },
	{ "Science",						INI::parseScienceDefinition },
	{ "Rank",								INI::parseRankDefinition },
	{ "SpecialPower",				INI::parseSpecialPowerDefinition },
	{ "ShellMenuScheme",		INI::parseShellMenuSchemeDefinition },
	{ "Terrain",						INI::parseTerrainDefinition },
	{ "Upgrade",						INI::parseUpgradeDefinition },
	{ "Video",							INI::parseVideoDefinition },
	{ "WaterSet",						INI::parseWaterSettingDefinition },
	{ "WaterTransparency",	INI::parseWaterTransparencyDefinition},
	{ "Weather",	INI::parseWeatherDefinition},
	{ "Weapon",							INI::parseWeaponTemplateDefinition },
	{ "WebpageURL",					INI::parseWebpageURLDefinition },
	{ "HeaderTemplate",			INI::parseHeaderTemplateDefinition },
	{ "StaticGameLOD",			INI::parseStaticGameLODDefinition },
	{ "DynamicGameLOD",			INI::parseDynamicGameLODDefinition },
	{ "LODPreset",					INI::parseLODPreset },
	{	"BenchProfile",				INI::parseBenchProfile },
	{	"ReallyLowMHz",				parseReallyLowMHz },
	{	"ScriptAction",				ScriptEngine::parseScriptAction },
	{	"ScriptCondition",		ScriptEngine::parseScriptCondition },

	{ NULL,									NULL },		// keep this last!
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
Bool INI::isValidINIFilename( const char *filename )
{
	if( filename == NULL )
		return FALSE;

	Int len = strlen( filename );
	if( len < 3 )
		return FALSE;

	if( filename[ len - 1 ] != 'I' && filename[ len - 1 ] != 'i' )
		return FALSE;

	if( filename[ len - 2 ] != 'N' && filename[ len - 2 ] != 'n' )
		return FALSE;

	if( filename[ len - 3 ] != 'I' && filename[ len - 3 ] != 'i' )
		return FALSE;

	return TRUE;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
  // end INI

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
  // end ~INI

//-------------------------------------------------------------------------------------------------
/** Load all INI files in the specified directory (and subdirectories if indicated).
	* If we are to load subdirectories, we will load them *after* we load all the
	* files in the current directory */
//-------------------------------------------------------------------------------------------------
void INI::loadDirectory(AsciiString directory, Bool subdirectories, INILoadType type, Xfer* transfer)
{
    const INILineTransfer lines = transfer ? INILineTransfer{transfer,
        +[](void* owner,const char* bytes,Int count) {
            static_cast<Xfer*>(owner)->xferUser(const_cast<char*>(bytes),count);
        }} : INILineTransfer{};
    loadDirectoryBlocks(directory,subdirectories,type,
        std::span(theTypeTable).first(std::size(theTypeTable)-1),lines);
}  // end loadDirectory

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
void INI::load(AsciiString filename, INILoadType type, Xfer* transfer)
{
    // Keep the actual Xfer RTTI/typed adapter with the complete registry owner,
    // not the independent token/input reader.
    const INILineTransfer lines = transfer ? INILineTransfer{transfer,
        +[](void* owner,const char* bytes,Int count) {
            static_cast<Xfer*>(owner)->xferUser(const_cast<char*>(bytes),count);
        }} : INILineTransfer{};
    loadBlocks(filename,type,std::span(theTypeTable).first(std::size(theTypeTable)-1),lines);
}  // end load

//-------------------------------------------------------------------------------------------------
/** Read a line from the already open file.  Any comments will be remved and
	* therefore ignored from any given line */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse UnsignedByte from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse signed short from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse unsigned short from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse integer from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse unsigned integer from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse real from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse real from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a degree value (0 to 360) and store the radian value of that degree
	* in a Real */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse an angular velocity in degrees-per-sec and store the rads-per-frame value of that degree
	* in a Real */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse Bool from buffer and assign at location 'store'.  The buffer token must
	* be in the form of a string "Yes" or "No" (case is ignored) */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse Bool from buffer; if true, or in MASK, otherwise and out MASK. The buffer token must
	* be in the form of a string "Yes" or "No" (case is ignored) */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse an *ASCII* string from buffer and assign at location 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse an *ASCII* string from buffer and assign at location 'store'. Has better support for quoted strings.
We don't really need this function, but parseString() is broken and we want to leave it broken to
maintain existing code.
 */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* static */void INI::parseScienceVector( INI *ini, void * /*instance*/, void *store, const void *userData )
{
	ScienceVec* asv = (ScienceVec*)store;
	asv->clear();
	for (const char *token = ini->getNextTokenOrNull(); token != NULL; token = ini->getNextTokenOrNull())
	{
		if (strcasecmp(token, "None") == 0)
		{
			asv->clear();
			return;
		}
		asv->push_back(INI::scanScience( token ));
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a string label, get the *translated* actual text from the label and store
	* into a *UNICODE* string. */
//-------------------------------------------------------------------------------------------------
void INI::parseAndTranslateLabel( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	// translate
	UnicodeString translated = TheGameText->fetch( token );
	if( translated.isEmpty() )
		throw INI_INVALID_DATA;

	// save the translated text
	UnicodeString *theString = (UnicodeString *)store;
	theString->set( translated.str() );

}  // end parseAndTranslateLabel

//-------------------------------------------------------------------------------------------------
/** Parse a string label assumed as an image as part of the image collection.  Translate
	* to an image pointer for storage */
//-------------------------------------------------------------------------------------------------
void INI::parseMappedImage( INI *ini, void * /*instance*/, void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	if( TheMappedImageCollection )
	{
		typedef const Image* ConstImagePtr;
		*(ConstImagePtr*)store = TheMappedImageCollection->findImageByName( AsciiString( token ) );
	}

	//KM: If we are in the worldbuilder, we want to parse commandbuttons for informational purposes,
	//but we don't care about the images -- because we never access them. In RTS/GUIEdit, they always
	//exist -- and in those cases, it will never call this code anyways because it'll throw long before.
	//else
	//	throw INI_UNKNOWN_ERROR;

}  // end parseMappedImage

// ------------------------------------------------------------------------------------------------
/** Parse a string label assumed as a Anim2D template name.  Translate that name to an
	* actual template pointer for storage */
// ------------------------------------------------------------------------------------------------
/*static*/ void INI::parseAnim2DTemplate( INI *ini, void *instance, void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	if( TheAnim2DCollection )
	{
		Anim2DTemplate **anim2DTemplate = (Anim2DTemplate **)store;
		*anim2DTemplate = TheAnim2DCollection->findTemplate( AsciiString( token ) );
	}  // end if
	else
	{

		DEBUG_CRASH(( "INI::parseAnim2DTemplate - TheAnim2DCollection is NULL\n" ));
		throw INI_UNKNOWN_ERROR;

	}  // end else

}  // end parseAnim2DTemplate

//-------------------------------------------------------------------------------------------------
/** Parse a percent in int or real form such as "23%" or "95.4%" and assign
	* to location 'store' as a number from 0.0 to 1.0 */
//-------------------------------------------------------------------------------------------------
  // end parsePercentToReal

//-------------------------------------------------------------------------------------------------
/** 'store' points to an 32 bit unsigned integer.  We will zero that integer, parse each token
	* in the buffer, if the token is in the userData table of strings, we will set the
	* according bit flag for it */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** 'store' points to an 32 bit unsigned integer.  We will zero that integer, parse each token
	* in the buffer, if the token is in the userData table of strings, we will set the
	* according bit flag for it */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a color in the form of
	*
	* RGB_COLOR = R:100 G:114 B:245
	* and store in "RGBColor" structure pointed to by 'store' */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a color in the form of
	*
	* RGB_COLOR = R:100 G:114 B:245 [A:233]
	* and store in "RGBAColorInt" structure pointed to by 'store' */
//-------------------------------------------------------------------------------------------------
  // end parseRGBAColorInt

//-------------------------------------------------------------------------------------------------
/** Parse a color in the form of
	*
	* RGB_COLOR = R:100 G:114 B:245 [A:233]
	* and store in "Color" structure pointed to by 'store' */
//-------------------------------------------------------------------------------------------------
  // end parseColorInt

//-------------------------------------------------------------------------------------------------
/** Parse a 3D coordinate of reals in the form of:
	* FIELD_NAME = X:400 Y:-214.3 Z:8.6 */
//-------------------------------------------------------------------------------------------------
  // end parseCoord3D

//-------------------------------------------------------------------------------------------------
/** Parse a 2D coordinate of reals in the form of:
	* FIELD_NAME = X:400 Y:-214.3 */
//-------------------------------------------------------------------------------------------------
  // end parseCoord2D

//-------------------------------------------------------------------------------------------------
/** Parse a 2D coordinate of Ints in the form of:
	* FIELD_NAME = X:400 Y:-214 */
//-------------------------------------------------------------------------------------------------
  // end parseICoord2D

//-------------------------------------------------------------------------------------------------
/** Parse an audio event and assign to the 'AudioEventRTS*' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseDynamicAudioEventRTS( INI *ini, void * /*instance*/, void *store, const void* userData )
{
	const char *token = ini->getNextToken();
	DynamicAudioEventRTS** theSound = (DynamicAudioEventRTS**)store;

	// translate the string into a sound
	if (strcasecmp(token, "NoSound") == 0)
	{
		if (*theSound)
		{
			(*theSound)->deleteInstance();
			*theSound = NULL;
		}
	}
	else
	{
		if (*theSound == NULL)
			*theSound = newInstance(DynamicAudioEventRTS);
		(*theSound)->m_event.setEventName(AsciiString(token));
	}

	if (*theSound)
		TheAudio->getInfoForAudioEvent(&(*theSound)->m_event);
}

//-------------------------------------------------------------------------------------------------
/** Parse an audio event and assign to the 'AudioEventRTS*' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseAudioEventRTS( INI *ini, void * /*instance*/, void *store, const void* userData )
{
	const char *token = ini->getNextToken();

	AudioEventRTS *theSound = (AudioEventRTS*)store;

	// translate the string into a sound
	if (strcasecmp(token, "NoSound") != 0) {
		theSound->setEventName(AsciiString(token));
	}

	TheAudio->getInfoForAudioEvent(theSound);
}

//-------------------------------------------------------------------------------------------------
/** Parse an ThingTemplate and assign to the 'ThingTemplate *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseThingTemplate( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	if (!TheThingFactory)
	{
		DEBUG_CRASH(("TheThingFactory not inited yet"));
		throw ERROR_BUG;
	}

	typedef const ThingTemplate *ConstThingTemplatePtr;
	ConstThingTemplatePtr* theThingTemplate = (ConstThingTemplatePtr*)store;

	if (strcasecmp(token, "None") == 0)
	{
		*theThingTemplate = NULL;
	}
	else
	{
		const ThingTemplate *tt = TheThingFactory->findTemplate(token);	// could be null!
		DEBUG_ASSERTCRASH(tt, ("ThingTemplate %s not found!\n",token));
		// assign it, even if null!
		*theThingTemplate = tt;
	}

}

//-------------------------------------------------------------------------------------------------
/** Parse an ArmorTemplate and assign to the 'ArmorTemplate *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseArmorTemplate( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	typedef const ArmorTemplate *ConstArmorTemplatePtr;
	ConstArmorTemplatePtr* theArmorTemplate = (ConstArmorTemplatePtr*)store;

	if (strcasecmp(token, "None") == 0)
	{
		*theArmorTemplate = NULL;
	}
	else
	{
		const ArmorTemplate *tt = TheArmorStore->findArmorTemplate(token);	// could be null!
		DEBUG_ASSERTCRASH(tt, ("ArmorTemplate %s not found!\n",token));
		// assign it, even if null!
		*theArmorTemplate = tt;
	}

}

//-------------------------------------------------------------------------------------------------
/** Parse an WeaponTemplate and assign to the 'WeaponTemplate *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseWeaponTemplate( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	typedef const WeaponTemplate *ConstWeaponTemplatePtr;
	ConstWeaponTemplatePtr* theWeaponTemplate = (ConstWeaponTemplatePtr*)store;

	const WeaponTemplate *tt = TheWeaponStore->findWeaponTemplate(token);	// could be null!
	DEBUG_ASSERTCRASH(tt || strcasecmp(token, "None") == 0, ("WeaponTemplate %s not found!\n",token));
	// assign it, even if null!
	*theWeaponTemplate = tt;

}

//-------------------------------------------------------------------------------------------------
/** Parse an FXList and assign to the 'FXList *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseFXList( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	typedef const FXList *ConstFXListPtr;
	ConstFXListPtr* theFXList = (ConstFXListPtr*)store;

	const FXList *fxl = TheFXListStore->findFXList(token);	// could be null!
	DEBUG_ASSERTCRASH(fxl != NULL || strcasecmp(token, "None") == 0, ("FXList %s not found!\n",token));
	// assign it, even if null!
	*theFXList = fxl;

}

//-------------------------------------------------------------------------------------------------
/** Parse a particle system and assign to 'ParticleSystemTemplate *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseParticleSystemTemplate( INI *ini, void * /*instance*/, void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	const ParticleSystemTemplate *pSystemT = TheParticleSystemManager->findTemplate( AsciiString( token ) );
	DEBUG_ASSERTCRASH( pSystemT || strcasecmp( token, "None" ) == 0, ("ParticleSystem %s not found!\n",token) );

	typedef const ParticleSystemTemplate* ConstParticleSystemTemplatePtr;
	ConstParticleSystemTemplatePtr* theParticleSystemTemplate = (ConstParticleSystemTemplatePtr*)store;

	*theParticleSystemTemplate = pSystemT;

}  // end parseParticleSystemTemplate

//-------------------------------------------------------------------------------------------------
/** Parse an DamageFX and assign to the 'DamageFX *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseDamageFX( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	typedef const DamageFX *ConstDamageFXPtr;
	ConstDamageFXPtr* theDamageFX = (ConstDamageFXPtr*)store;

	if (strcasecmp(token, "None") == 0)
	{
		*theDamageFX = NULL;
	}
	else
	{
		const DamageFX *fxl = TheDamageFXStore->findDamageFX(token);	// could be null!
		DEBUG_ASSERTCRASH(fxl, ("DamageFX %s not found!\n",token));
		// assign it, even if null!
		*theDamageFX = fxl;
	}

}

//-------------------------------------------------------------------------------------------------
/** Parse an ObjectCreationList and assign to the 'ObjectCreationList *' at store */
//-------------------------------------------------------------------------------------------------
void INI::parseObjectCreationList( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	typedef const ObjectCreationList *ConstObjectCreationListPtr;
	ConstObjectCreationListPtr* theObjectCreationList = (ConstObjectCreationListPtr*)store;

	const ObjectCreationList *ocl = TheObjectCreationListStore->findObjectCreationList(token);	// could be null!
	DEBUG_ASSERTCRASH(ocl || strcasecmp(token, "None") == 0, ("ObjectCreationList %s not found!\n",token));
	// assign it, even if null!
	*theObjectCreationList = ocl;

}

//-------------------------------------------------------------------------------------------------
/** Parse a upgrade template string and store as template pointer */
//-------------------------------------------------------------------------------------------------
void INI::parseUpgradeTemplate( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	if (!TheUpgradeCenter)
	{
		DEBUG_CRASH(("TheUpgradeCenter not inited yet"));
		throw ERROR_BUG;
	}

	const UpgradeTemplate *uu = TheUpgradeCenter->findUpgrade( AsciiString( token ) );
	DEBUG_ASSERTCRASH( uu || strcasecmp( token, "None" ) == 0, ("Upgrade %s not found!\n",token) );

	typedef const UpgradeTemplate* ConstUpgradeTemplatePtr;
	ConstUpgradeTemplatePtr* theUpgradeTemplate = (ConstUpgradeTemplatePtr *)store;
	*theUpgradeTemplate = uu;
}

//-------------------------------------------------------------------------------------------------
/** Parse a special power template string and store as template pointer */
//-------------------------------------------------------------------------------------------------
void INI::parseSpecialPowerTemplate( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	const char *token = ini->getNextToken();

	if (!TheSpecialPowerStore)
	{
		DEBUG_CRASH(("TheSpecialPowerStore not inited yet"));
		throw ERROR_BUG;
	}

	const SpecialPowerTemplate *sPowerT = TheSpecialPowerStore->findSpecialPowerTemplate( AsciiString( token ) );
	if( !sPowerT && strcasecmp( token, "None" ) != 0 )
	{
		DEBUG_CRASH( ("[LINE: %d in '%s'] Specialpower %s not found!\n", ini->getLineNum(), ini->getFilename().str(), token) );
	}

	typedef const SpecialPowerTemplate* ConstSpecialPowerTemplatePtr;
	ConstSpecialPowerTemplatePtr* theSpecialPowerTemplate = (ConstSpecialPowerTemplatePtr *)store;
	*theSpecialPowerTemplate = sPowerT;
}

//-------------------------------------------------------------------------------------------------
/** Parse a science string and store as science type */
//-------------------------------------------------------------------------------------------------
/* static */void INI::parseScience( INI *ini, void * /*instance*/, void *store, const void *userData )
{
	const char *token = ini->getNextToken();

	if (!TheScienceStore)
	{
		DEBUG_CRASH(("TheScienceStore not inited yet"));
		throw ERROR_BUG;
	}

	*((ScienceType *)store) = INI::scanScience(token);

}

//-------------------------------------------------------------------------------------------------
/** Parse a single string token, check for that token in the index list
	* of names provided and store the index into that list.
	*
	* NOTE: Is is assumed that we are going to store the index into
	*				a 4 byte integer.  This works well for INT and ENUM definitions */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a single string token, check for that token in the index list
	* of names provided and store the index into that list.
	*
	* NOTE: Is is assumed that we are going to store the index into
	*				a 4 byte integer.  This works well for INT and ENUM definitions */
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** Parse a single string token, check for that token in the index list
	* of names provided and store the associated value into that list.
	*
	* NOTE: Is is assumed that we are going to store the index into
	*				a 4 byte integer.  This works well for INT and ENUM definitions */
//-------------------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/*static*/ ScienceType INI::scanScience(const char* token)
{
	return TheScienceStore->friend_lookupScience( token );
}

//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/**
 * Parse a "random variable".
 * The format is "FIELD = low high [distribution]".
 */
void INI::parseGameClientRandomVariable( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	GameClientRandomVariable *var = static_cast<GameClientRandomVariable *>(store);

	const char* token;

	token = ini->getNextToken();
	Real low = INI::scanReal(token);

	token = ini->getNextToken();
	Real high = INI::scanReal(token);

	// if omitted, assume uniform
	GameClientRandomVariable::DistributionType type = GameClientRandomVariable::UNIFORM;
	token = ini->getNextTokenOrNull();
	if (token)
		type = (GameClientRandomVariable::DistributionType)INI::scanIndexList(token, GameClientRandomVariable::DistributionTypeNames);

	// set the range of the random variable
	var->setRange( low, high, type );
}

//-------------------------------------------------------------------------------------------------
// parse a duration in msec and convert to duration in frames


//-------------------------------------------------------------------------------------------------
// parse a duration in msec and convert to duration in integral number of frames, (unsignedint) rounding UP


// ------------------------------------------------------------------------------------------------
// parse a duration in msec and convert to duration in integral number of frames, (unsignedshort) rounding UP


//-------------------------------------------------------------------------------------------------
// parse acceleration in (dist/sec) and convert to (dist/frame)


//-------------------------------------------------------------------------------------------------
// parse acceleration in (dist/sec^2) and convert to (dist/frame^2)


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void INI::parseVeterancyLevelFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	VeterancyLevelFlags flags = VETERANCY_LEVEL_FLAGS_ALL;
	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (strcasecmp(token, "ALL") == 0)
		{
			flags = VETERANCY_LEVEL_FLAGS_ALL;
			continue;
		}
		else if (strcasecmp(token, "NONE") == 0)
		{
			flags = VETERANCY_LEVEL_FLAGS_NONE;
			continue;
		}
		else if (token[0] == '+')
		{
			VeterancyLevel dt = (VeterancyLevel)INI::scanIndexList(token+1, TheVeterancyNames);
			flags = setVeterancyLevelFlag(flags, dt);
			continue;
		}
		else if (token[0] == '-')
		{
			VeterancyLevel dt = (VeterancyLevel)INI::scanIndexList(token+1, TheVeterancyNames);
			flags = clearVeterancyLevelFlag(flags, dt);
			continue;
		}
		else
		{
			throw INI_UNKNOWN_TOKEN;
		}
	}
	*(VeterancyLevelFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void INI::parseSoundsList( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	std::vector<AsciiString> *vec = (std::vector<AsciiString>*) store;
	vec->clear();

	const char* SEPS = " \t,=";
	const char *c = ini->getNextTokenOrNull(SEPS);
	while ( c )
	{
		vec->push_back( c );
		c = ini->getNextTokenOrNull(SEPS);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void INI::parseDamageTypeFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	DamageTypeFlags flags = DAMAGE_TYPE_FLAGS_NONE;
	flags.flip();

	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (strcasecmp(token, "ALL") == 0)
		{
			flags = DAMAGE_TYPE_FLAGS_NONE;
			flags.flip();
			continue;
		}
		if (strcasecmp(token, "NONE") == 0)
		{
			flags = DAMAGE_TYPE_FLAGS_NONE;
			continue;
		}
		if (token[0] == '+')
		{
			DamageType dt = (DamageType)DamageTypeFlags::getSingleBitFromName(token+1);
			flags = setDamageTypeFlag(flags, dt);
			continue;
		}
		if (token[0] == '-')
		{
			DamageType dt = (DamageType)DamageTypeFlags::getSingleBitFromName(token+1);
			flags = clearDamageTypeFlag(flags, dt);
			continue;
		}
		throw INI_UNKNOWN_TOKEN;
	}
	*(DamageTypeFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void INI::parseDeathTypeFlags(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	DeathTypeFlags flags = DEATH_TYPE_FLAGS_ALL;
	for (const char* token = ini->getNextToken(); token; token = ini->getNextTokenOrNull())
	{
		if (strcasecmp(token, "ALL") == 0)
		{
			flags = DEATH_TYPE_FLAGS_ALL;
			continue;
		}
		if (strcasecmp(token, "NONE") == 0)
		{
			flags = DEATH_TYPE_FLAGS_NONE;
			continue;
		}
		if (token[0] == '+')
		{
			DeathType dt = (DeathType)INI::scanIndexList(token+1, TheDeathNames);
			flags = setDeathTypeFlag(flags, dt);
			continue;
		}
		if (token[0] == '-')
		{
			DeathType dt = (DeathType)INI::scanIndexList(token+1, TheDeathNames);
			flags = clearDeathTypeFlag(flags, dt);
			continue;
		}
		throw INI_UNKNOWN_TOKEN;
	}
	*(DeathTypeFlags*)store = flags;
}

//-------------------------------------------------------------------------------------------------
// parse the line and return whether the given line is a Block declaration of the form
// [whitespace] blockType [whitespace] blockName [EOL]
// both blockType and blockName are case insensitive
Bool INI::isDeclarationOfType( AsciiString blockType, AsciiString blockName, char *bufferToCheck )
{
	Bool retVal = true;
	if (!bufferToCheck || blockType.isEmpty() || blockName.isEmpty()) {
		return false;
	}
	// DO NOT RETURN EARLY FROM THIS FUNCTION. (beyond this point)
	// we have to restore the bufferToCheck to its previous state before returning, so
	// it is important to get through all the checks.

	char restoreChar;
	char *tempBuff = bufferToCheck;
	int blockTypeLength = blockType.getLength();
	int blockNameLength = blockName.getLength();

	while (isspace(*tempBuff)) {
		++tempBuff;
	}

	if (strlen(tempBuff) > blockTypeLength) {
		restoreChar = tempBuff[blockTypeLength];
		tempBuff[blockTypeLength] = 0;

		if (strcasecmp(blockType.str(), tempBuff) != 0) {
			retVal = false;
		}

		tempBuff[blockTypeLength] = restoreChar;
		tempBuff = tempBuff + blockTypeLength;
	} else {
		retVal = false;
	}

	while (isspace(*tempBuff)) {
		++tempBuff;
	}

	if (strlen(tempBuff) > blockNameLength) {
		restoreChar = tempBuff[blockNameLength];
		tempBuff[blockNameLength] = 0;

		if (strcasecmp(blockName.str(), tempBuff) != 0) {
			retVal = false;
		}

		tempBuff[blockNameLength] = restoreChar;
		tempBuff = tempBuff + blockNameLength;
	} else {
		retVal = false;
	}

	while (strlen(tempBuff)) {
		retVal = retVal && isspace(tempBuff[0]);
		++tempBuff;
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
// parse the line and return whether the given line is a Block declaration of the form
// [whitespace] end [EOL]
Bool INI::isEndOfBlock( char *bufferToCheck )
{
	Bool retVal = true;
	if (!bufferToCheck) {
		return false;
	}

	// DO NOT RETURN EARLY FROM THIS FUNCTION (beyond this point)
	// we have to restore the bufferToCheck to its previous state before returning, so
	// it is important to get through all the checks.

	static const char* endString = "End";
	int endStringLength = strlen(endString);
	char restoreChar;
	char *tempBuff = bufferToCheck;


	while (isspace(*tempBuff)) {
		++tempBuff;
	}

	if (strlen(tempBuff) > endStringLength) {
		restoreChar = tempBuff[endStringLength];
		tempBuff[endStringLength] = 0;

		if (strcasecmp(endString, tempBuff) != 0) {
			retVal = false;
		}

		tempBuff[endStringLength] = restoreChar;
		tempBuff = tempBuff + endStringLength;
	} else {
		retVal = false;
	}

	while (strlen(tempBuff)) {
		retVal = retVal && isspace(tempBuff[0]);
		++tempBuff;
	}

	return retVal;
}
