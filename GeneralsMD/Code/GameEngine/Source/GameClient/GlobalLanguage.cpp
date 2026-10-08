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

// FILE: GlobalLanguage.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename: 	GlobalLanguage.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Contains the member functions for the language munkee
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/GlobalData.h"
#include "PreRTS.h"

#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include <utility>
#include "Common/Registry.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileSystem.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GlobalLanguage *TheGlobalLanguageData = NULL;				///< The global language singalton

static const FieldParse TheGlobalLanguageDataFieldParseTable[] = 
{
	{ "UnicodeFontName",									INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontName ) },
	//{	"UnicodeFontFileName",							INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontFileName ) },
	{ "LocalFontFile",										GlobalLanguage::parseFontFileName,					NULL,			0},
	{ "MilitaryCaptionSpeed",						INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionSpeed ) },
	{ "UseHardWordWrap",						INI::parseBool,					NULL,		offsetof( GlobalLanguage, m_useHardWrap) },
	{ "ResolutionFontAdjustment",						INI::parseReal,					NULL,		offsetof( GlobalLanguage, m_resolutionFontSizeAdjustment) },
	
	{ "CopyrightFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_copyrightFont ) },
	{ "MessageFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_messageFont) },					
	{ "MilitaryCaptionTitleFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionTitleFont) },
	{ "MilitaryCaptionDelayMS",					INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionDelayMS ) },
	{ "MilitaryCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionFont) },
	{ "SuperweaponCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownNormalFont) },
	{ "SuperweaponCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownReadyFont) },
	{ "NamedTimerCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownNormalFont) },
	{ "NamedTimerCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownReadyFont) },
	{ "DrawableCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawableCaptionFont) },
	{ "DefaultWindowFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultWindowFont) },
	{ "DefaultDisplayStringFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultDisplayStringFont) },
	{ "TooltipFontName",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_tooltipFontName) },
	{ "NativeDebugDisplay",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_nativeDebugDisplay) },
	{ "DrawGroupInfoFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawGroupInfoFont) },
	{ "CreditsTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsTitleFont) },
	{ "CreditsMinorTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsPositionFont) },
	{ "CreditsNormalFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsNormalFont) },

	{ NULL,					NULL,						NULL,						0 }  // keep this last
};

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void INI::parseLanguageDefinition( INI *ini )
{
	if( !TheGlobalLanguageData )
	{
		DEBUG_ASSERTCRASH(TheGlobalLanguageData, ("INI::parseLanguageDefinition - TheGlobalLanguage Data is not around, please create it before trying to parse the ini file."));
		return;
	}  // end if
	// parse the ini weapon definition
	ini->initFromINI( TheGlobalLanguageData, TheGlobalLanguageDataFieldParseTable );
}

GlobalLanguage::GlobalLanguage()
{
	m_unicodeFontName.clear();
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_unicodeFontFileName.clear();
	m_unicodeFontName.clear();
	m_militaryCaptionSpeed = 0;
	m_useHardWrap = FALSE;
	m_resolutionFontSizeAdjustment = 0.7f;
	m_militaryCaptionDelayMS = 750;
	//End Add
}

GlobalLanguage::~GlobalLanguage() = default;

void GlobalLanguage::swapConfiguration(GlobalLanguage& other) noexcept
{
	m_unicodeFontName.swap(other.m_unicodeFontName);
	m_unicodeFontFileName.swap(other.m_unicodeFontFileName);
	m_localFonts.swap(other.m_localFonts);
	m_copyrightFont.swap(other.m_copyrightFont);
	m_messageFont.swap(other.m_messageFont);
	m_militaryCaptionTitleFont.swap(other.m_militaryCaptionTitleFont);
	m_militaryCaptionFont.swap(other.m_militaryCaptionFont);
	m_superweaponCountdownNormalFont.swap(other.m_superweaponCountdownNormalFont);
	m_superweaponCountdownReadyFont.swap(other.m_superweaponCountdownReadyFont);
	m_namedTimerCountdownNormalFont.swap(other.m_namedTimerCountdownNormalFont);
	m_namedTimerCountdownReadyFont.swap(other.m_namedTimerCountdownReadyFont);
	m_drawableCaptionFont.swap(other.m_drawableCaptionFont);
	m_defaultWindowFont.swap(other.m_defaultWindowFont);
	m_defaultDisplayStringFont.swap(other.m_defaultDisplayStringFont);
	m_tooltipFontName.swap(other.m_tooltipFontName);
	m_nativeDebugDisplay.swap(other.m_nativeDebugDisplay);
	m_drawGroupInfoFont.swap(other.m_drawGroupInfoFont);
	m_creditsTitleFont.swap(other.m_creditsTitleFont);
	m_creditsPositionFont.swap(other.m_creditsPositionFont);
	m_creditsNormalFont.swap(other.m_creditsNormalFont);
	std::swap(m_useHardWrap, other.m_useHardWrap);
	std::swap(m_militaryCaptionSpeed, other.m_militaryCaptionSpeed);
	std::swap(m_militaryCaptionDelayMS, other.m_militaryCaptionDelayMS);
	std::swap(m_resolutionFontSizeAdjustment, other.m_resolutionFontSizeAdjustment);
}

void GlobalLanguage::init( void )
{
	if (!TheFileSystem || !TheNameKeyGenerator || TheGlobalLanguageData != this)
		throw ERROR_BAD_ARG;
	NameKeyTransaction keys(*TheNameKeyGenerator);
	GlobalLanguage candidate(*this);
	struct Publication {
		GlobalLanguage* previous;
		explicit Publication(GlobalLanguage& value) : previous(TheGlobalLanguageData) {
			TheGlobalLanguageData = &value;
		}
		~Publication() noexcept { TheGlobalLanguageData = previous; }
	} publication(candidate);
	AsciiString filename;
	filename.format("Data\\%s\\Language.ini", GetRegistryLanguage().str());
	const INIBlockDefinition blocks[]{{"Language", INI::parseLanguageDefinition}};
	INI ini;
	ini.loadBlocks(filename, INI_LOAD_OVERWRITE, blocks);
	// Keep supplied font identities for native text output. No host font install
	// or CWD lookup: only configured read-only assets may satisfy local fonts.
	for (const auto& font : candidate.m_localFonts)
		if (!TheFileSystem->doesFileExist(font.str())) throw INI_CANT_OPEN_FILE;
	swapConfiguration(candidate);
	keys.commit();
}
void GlobalLanguage::reset( void ) {}


void GlobalLanguage::parseFontDesc(INI *ini, void *instance, void *store, const void* userData)
{
	FontDesc *fontDesc = (FontDesc *)store;
	fontDesc->name = ini->getNextQuotedAsciiString();
	fontDesc->size = ini->scanInt(ini->getNextToken());
	fontDesc->bold = ini->scanBool(ini->getNextToken());
}

void GlobalLanguage::parseFontFileName( INI *ini, void * instance, void *store, const void* userData )
{
	GlobalLanguage *monkey = (GlobalLanguage *)instance;
	AsciiString asciiString = ini->getNextAsciiString();
	monkey->m_localFonts.push_front(asciiString);
}	 

Int GlobalLanguage::adjustFontSize(Int theFontSize) 
{
	Real adjustFactor = TheGlobalData->m_xResolution/800.0f;
	adjustFactor = 1.0f + (adjustFactor-1.0f) * m_resolutionFontSizeAdjustment;
	if (adjustFactor<1.0f) adjustFactor = 1.0f;
	if (adjustFactor>2.0f) adjustFactor = 2.0f;
	Int pointSize = REAL_TO_INT_FLOOR(theFontSize*adjustFactor);
	return pointSize;
}

FontDesc::FontDesc(void)
{
	name = "Arial Unicode MS";	///<name of font
	size = 12;			///<point size
	bold = FALSE;			///<is bold?
}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
