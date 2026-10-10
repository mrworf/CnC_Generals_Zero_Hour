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

// Snow.cpp ////////////////////////////////////////////////////////////////////////////////
// Snow Rendering implementation
// Author: Mark Wilczynski, July 2003
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the Game
#include "GameClient/Snow.h"
#include "GameClient/View.h"
#include "Common/NativeOverrideAdmission.h"
#include <cmath>
#include <limits>
#include <memory>

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

SnowManager *TheSnowManager=NULL;

namespace {
void validateSnowSettings(const WeatherSetting& value) {
  for(Real number:{value.m_snowFrequencyScaleX,value.m_snowFrequencyScaleY,
    value.m_snowAmplitude,value.m_snowPointSize,value.m_snowMaxPointSize,
    value.m_snowMinPointSize,value.m_snowQuadSize,value.m_snowBoxDimensions,
    value.m_snowBoxDensity,value.m_snowVelocity})
    if(!std::isfinite(number))throw INI_INVALID_DATA;
  if(value.m_snowBoxDimensions<1 ||
     double(value.m_snowBoxDimensions)>double(std::numeric_limits<Int>::max()) ||
     value.m_snowBoxDensity<=0 || value.m_snowVelocity<=0 ||
     !std::isfinite(1.f/value.m_snowBoxDensity) ||
     !std::isfinite(value.m_snowBoxDimensions/value.m_snowVelocity))throw INI_INVALID_DATA;
}
}

SnowManager::SnowManager():m_startingHeights(nullptr)
{
	m_time = 0;
	m_velocity = 1;
	m_isVisible = TRUE;	//default to showing if it's enabled via INI.
}

void SnowManager::init( void )
{
	//starting heights of each particle
	//TODO: replace this lookup table with some kind of procedural method that takes x,y as input.
	if(m_startingHeights || !TheWeatherSetting)throw ERROR_BAD_ARG;
	validateSnowSettings(**TheWeatherSetting);
	std::unique_ptr<Real[]> candidate(NEW Real[SNOW_NOISE_X*SNOW_NOISE_Y]);
	m_startingHeights=candidate.get();
	try{updateIniSettings();}catch(...){m_startingHeights=nullptr;throw;}
	candidate.release();
	m_time=0;
}

void SnowManager::updateIniSettings(void)
{
	if(!m_startingHeights || !TheWeatherSetting)throw ERROR_BAD_ARG;
	validateSnowSettings(**TheWeatherSetting);
	Real *dst=m_startingHeights;
	//initialize a table of random starting positions for each particle.
	Int boxDimensions = (Int)TheWeatherSetting->m_snowBoxDimensions;
	for (Int y=0; y<SNOW_NOISE_Y; y++)
	{
		for (Int x=0; x<SNOW_NOISE_X; x++)
		{
			*dst=(Real)(rand()%(boxDimensions));
			dst++;
		}
	}

	m_velocity = TheWeatherSetting->m_snowVelocity;
	m_frequencyScaleX = TheWeatherSetting->m_snowFrequencyScaleX;
	m_frequencyScaleY = TheWeatherSetting->m_snowFrequencyScaleY;
	m_amplitude	= TheWeatherSetting->m_snowAmplitude;	
	m_pointSize = TheWeatherSetting->m_snowPointSize;	
	m_quadSize	= TheWeatherSetting->m_snowQuadSize;		
	m_boxDimensions	= TheWeatherSetting->m_snowBoxDimensions;
	m_emitterSpacing = 1.0f/TheWeatherSetting->m_snowBoxDensity;
	m_maxPointSize = TheWeatherSetting->m_snowMaxPointSize;
	m_minPointSize = TheWeatherSetting->m_snowMinPointSize;

	//Time for snow flake to make it from top to bottom of rendered cube around camera.
	m_fullTimePeriod = m_boxDimensions/m_velocity;
}

void SnowManager::setVisible(Bool showWeather)
{
	m_isVisible = showWeather;
}

void SnowManager::reset(void)
{
	m_isVisible = TRUE;	//default to showing if it's enabled via INI.
}

SnowManager::~SnowManager()
{
	delete [] m_startingHeights;
	m_startingHeights=NULL;
}

OVERRIDE<WeatherSetting> TheWeatherSetting = NULL;

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse WeatherSetting::m_weatherSettingFieldParseTable[] = 
{
	{ "SnowTexture",							INI::parseAsciiString,NULL,			offsetof( WeatherSetting, m_snowTexture ) },
	{ "SnowFrequencyScaleX",					INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowFrequencyScaleX ) },
	{ "SnowFrequencyScaleY",					INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowFrequencyScaleY ) },
	{ "SnowAmplitude",							INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowAmplitude ) },
	{ "SnowPointSize",							INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowPointSize ) },
	{ "SnowMaxPointSize",						INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowMaxPointSize ) },
	{ "SnowMinPointSize",						INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowMinPointSize ) },
	{ "SnowQuadSize",							INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowQuadSize ) },
	{ "SnowBoxDimensions",						INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowBoxDimensions ) },
	{ "SnowBoxDensity",							INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowBoxDensity ) },
	{ "SnowVelocity",							INI::parseReal,NULL,			offsetof( WeatherSetting, m_snowVelocity ) },
	{ "SnowPointSprites",						INI::parseBool,NULL,			offsetof( WeatherSetting, m_usePointSprites ) },
	{ "SnowEnabled",							INI::parseBool,NULL,			offsetof( WeatherSetting, m_snowEnabled ) },
	{ 0, 0, 0, 0 },
};

//-------------------------------------------------------------------------------------------------
void INI::parseWeatherDefinition( INI *ini )
{
	nativeAdmitOverride(TheWeatherSetting,ini->getLoadType()==INI_LOAD_CREATE_OVERRIDES,
		[]{return newInstance(WeatherSetting);},
		[&](WeatherSetting& candidate){ini->initFromINI(&candidate,candidate.getFieldParse());validateSnowSettings(candidate);},
		[](const WeatherSetting*,WeatherSetting&){if(TheSnowManager)TheSnowManager->updateIniSettings();});

	if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) {
		// Check to see if we overrode any textures.
		// If we did, then we need to replace them in the model.
	}
}
