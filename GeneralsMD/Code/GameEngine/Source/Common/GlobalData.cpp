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

// FILE: GlobalData.cpp ///////////////////////////////////////////////////////////////////////////
// The GameLogicData object
// Author: trolfs, Michael Booth, Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

//#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_TERRAIN_LOD_NAMES
#define DEFINE_TIME_OF_DAY_NAMES
#define DEFINE_WEATHER_NAMES
#define DEFINE_BODYDAMAGETYPE_NAMES
#define DEFINE_PANNING_NAMES

#include "Common/crc.h"
#include "Common/GlobalData.h"
#include "Common/file.h"
#include "Common/FileOwner.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/NativeUserStorage.h"
#include "Common/NativeInputSettings.h"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <type_traits>
#include <limits>
#include "Common/UserPreferences.h"
#include "Common/version.h"

#include "GameLogic/WeaponBonus.h"
#include "GameLogic/AIDebugOptions.h"
#include "GameLogic/BodyDamageType.h"

#include "GameClient/Color.h"
#include "GameClient/TerrainLOD.h"

#include "GameNetwork/FirewallHelper.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
GlobalData* TheWritableGlobalData = NULL;				///< The global data singleton

//-------------------------------------------------------------------------------------------------
GlobalData* GlobalData::m_theOriginal = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/*static*/ const FieldParse GlobalData::s_GlobalDataFieldParseTable[] = 
{
	{ "Windowed",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_windowed ) },
	{ "XResolution",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_xResolution ) },
	{ "YResolution",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_yResolution ) },
	{ "MapName",									INI::parseAsciiString,NULL,			offsetof( GlobalDataConfiguration, m_mapName ) },
	{ "MoveHintName",							INI::parseAsciiString,NULL,			offsetof( GlobalDataConfiguration, m_moveHintName ) },
	{ "UseTrees",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useTrees ) },
	{ "UseFPSLimit",							INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useFpsLimit ) },
	{ "DumpAssetUsage",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_dumpAssetUsage ) },
	{ "FramesPerSecondLimit",			INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_framesPerSecondLimit ) },
	{ "ChipsetType",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_chipSetType ) },
	{ "MaxShellScreens",					INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxShellScreens ) },
	{ "UseCloudMap",							INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useCloudMap ) },
	{ "UseLightMap",							INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useLightMap ) },
	{ "BilinearTerrainTex",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_bilinearTerrainTex ) },
	{ "TrilinearTerrainTex",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_trilinearTerrainTex ) },
	{ "MultiPassTerrain",					INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_multiPassTerrain ) },
	{ "AdjustCliffTextures",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_adjustCliffTextures ) },
	{ "Use3WayTerrainBlends",			INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_use3WayTerrainBlends ) },
	{ "StretchTerrain",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_stretchTerrain ) },
	{ "UseHalfHeightMap",					INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useHalfHeightMap ) },
	
	
	{ "DrawEntireTerrain",					INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_drawEntireTerrain ) },
	{ "TerrainLOD",									INI::parseIndexList,	TerrainLODNames,	offsetof( GlobalDataConfiguration, m_terrainLOD ) },
	{ "TerrainLODTargetTimeMS",			INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_terrainLODTargetTimeMS ) },
	{ "RightMouseAlwaysScrolls",		INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_rightMouseAlwaysScrolls ) },
	{ "UseWaterPlane",							INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useWaterPlane ) },
	{ "UseCloudPlane",							INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useCloudPlane ) },
	{ "DownwindAngle",							INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_downwindAngle ) },
	{ "UseShadowVolumes",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useShadowVolumes ) },
	{ "UseShadowDecals",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useShadowDecals ) },
	{ "TextureReductionFactor",			INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_textureReductionFactor ) },
	{ "UseBehindBuildingMarker",		INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_enableBehindBuildingMarkers ) },
	{ "WaterPositionX",							INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_waterPositionX ) },
	{ "WaterPositionY",							INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_waterPositionY ) },
	{ "WaterPositionZ",							INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_waterPositionZ ) },
	{ "WaterExtentX",								INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_waterExtentX ) },
	{ "WaterExtentY",								INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_waterExtentY ) },
	{ "WaterType",									INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_waterType ) },
	{ "FeatherWater",						  	INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_featherWater ) },
	{ "ShowSoftWaterEdge",					INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showSoftWaterEdge ) },

	// nasty ick, we need to save this data with a map and not hard code INI values
	{ "VertexWaterAvailableMaps1",		INI::parseAsciiString,	NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAvailableMaps[ 0 ] ) },
	{ "VertexWaterHeightClampLow1",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampLow[ 0 ] ) },
	{ "VertexWaterHeightClampHi1",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampHi[ 0 ] ) },
	{ "VertexWaterAngle1",						INI::parseAngleReal,		NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAngle[ 0 ] ) },
	{ "VertexWaterXPosition1",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXPosition[ 0 ] ) },
	{ "VertexWaterYPosition1",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYPosition[ 0 ] ) },
	{ "VertexWaterZPosition1",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterZPosition[ 0 ] ) },
	{ "VertexWaterXGridCells1",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXGridCells[ 0 ] ) },
	{ "VertexWaterYGridCells1",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYGridCells[ 0 ] ) },
	{ "VertexWaterGridSize1",					INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterGridSize[ 0 ] ) },
	{ "VertexWaterAttenuationA1",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationA[ 0 ] ) },
	{ "VertexWaterAttenuationB1",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationB[ 0 ] ) },
	{ "VertexWaterAttenuationC1",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationC[ 0 ] ) },
	{ "VertexWaterAttenuationRange1",	INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationRange[ 0 ] ) },

	// nasty ick, we need to save this data with a map and not hard code INI values
	{ "VertexWaterAvailableMaps2",		INI::parseAsciiString,	NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAvailableMaps[ 1 ] ) },
	{ "VertexWaterHeightClampLow2",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampLow[ 1 ] ) },
	{ "VertexWaterHeightClampHi2",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampHi[ 1 ] ) },
	{ "VertexWaterAngle2",						INI::parseAngleReal,		NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAngle[ 1 ] ) },
	{ "VertexWaterXPosition2",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXPosition[ 1 ] ) },
	{ "VertexWaterYPosition2",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYPosition[ 1 ] ) },
	{ "VertexWaterZPosition2",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterZPosition[ 1 ] ) },
	{ "VertexWaterXGridCells2",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXGridCells[ 1 ] ) },
	{ "VertexWaterYGridCells2",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYGridCells[ 1 ] ) },
	{ "VertexWaterGridSize2",					INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterGridSize[ 1 ] ) },
	{ "VertexWaterAttenuationA2",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationA[ 1 ] ) },
	{ "VertexWaterAttenuationB2",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationB[ 1 ] ) },
	{ "VertexWaterAttenuationC2",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationC[ 1 ] ) },
	{ "VertexWaterAttenuationRange2",	INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationRange[ 1 ] ) },

	// nasty ick, we need to save this data with a map and not hard code INI values
	{ "VertexWaterAvailableMaps3",		INI::parseAsciiString,	NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAvailableMaps[ 2 ] ) },
	{ "VertexWaterHeightClampLow3",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampLow[ 2 ] ) },
	{ "VertexWaterHeightClampHi3",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampHi[ 2 ] ) },
	{ "VertexWaterAngle3",						INI::parseAngleReal,		NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAngle[ 2 ] ) },
	{ "VertexWaterXPosition3",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXPosition[ 2 ] ) },
	{ "VertexWaterYPosition3",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYPosition[ 2 ] ) },
	{ "VertexWaterZPosition3",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterZPosition[ 2 ] ) },
	{ "VertexWaterXGridCells3",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXGridCells[ 2 ] ) },
	{ "VertexWaterYGridCells3",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYGridCells[ 2 ] ) },
	{ "VertexWaterGridSize3",					INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterGridSize[ 2 ] ) },
	{ "VertexWaterAttenuationA3",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationA[ 2 ] ) },
	{ "VertexWaterAttenuationB3",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationB[ 2 ] ) },
	{ "VertexWaterAttenuationC3",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationC[ 2 ] ) },
	{ "VertexWaterAttenuationRange3",	INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationRange[ 2 ] ) },

	// nasty ick, we need to save this data with a map and not hard code INI values
	{ "VertexWaterAvailableMaps4",		INI::parseAsciiString,	NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAvailableMaps[ 3 ] ) },
	{ "VertexWaterHeightClampLow4",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampLow[ 3 ] ) },
	{ "VertexWaterHeightClampHi4",		INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterHeightClampHi[ 3 ] ) },
	{ "VertexWaterAngle4",						INI::parseAngleReal,		NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAngle[ 3 ] ) },
	{ "VertexWaterXPosition4",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXPosition[ 3 ] ) },
	{ "VertexWaterYPosition4",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYPosition[ 3 ] ) },
	{ "VertexWaterZPosition4",				INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterZPosition[ 3 ] ) },
	{ "VertexWaterXGridCells4",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterXGridCells[ 3 ] ) },
	{ "VertexWaterYGridCells4",				INI::parseInt,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterYGridCells[ 3 ] ) },
	{ "VertexWaterGridSize4",					INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterGridSize[ 3 ] ) },
	{ "VertexWaterAttenuationA4",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationA[ 3 ] ) },
	{ "VertexWaterAttenuationB4",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationB[ 3 ] ) },
	{ "VertexWaterAttenuationC4",			INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationC[ 3 ] ) },
	{ "VertexWaterAttenuationRange4",	INI::parseReal,					NULL,		offsetof( GlobalDataConfiguration, m_vertexWaterAttenuationRange[ 3 ] ) },

	{ "SkyBoxPositionZ",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_skyBoxPositionZ ) },
	{ "SkyBoxScale",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_skyBoxScale ) },
	{ "DrawSkyBox",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_drawSkyBox ) },
	{ "CameraPitch",								INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_cameraPitch ) },
	{ "CameraYaw",									INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_cameraYaw ) },
	{ "CameraHeight",								INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_cameraHeight ) },
	{ "MaxCameraHeight",						INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_maxCameraHeight ) },
	{ "MinCameraHeight",						INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_minCameraHeight ) },
	{ "TerrainHeightAtEdgeOfMap",					INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_terrainHeightAtEdgeOfMap ) },
	{ "UnitDamagedThreshold",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_unitDamagedThresh ) },
	{ "UnitReallyDamagedThreshold",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_unitReallyDamagedThresh ) },
	{ "GroundStiffness",					INI::parseReal,				NULL,				offsetof( GlobalDataConfiguration, m_groundStiffness ) },
	{ "StructureStiffness",					INI::parseReal,				NULL,				offsetof( GlobalDataConfiguration, m_structureStiffness ) },
	{ "Gravity",									INI::parseAccelerationReal,				NULL,				offsetof( GlobalDataConfiguration, m_gravity ) },
	{ "StealthFriendlyOpacity",		INI::parsePercentToReal,				NULL,				offsetof( GlobalDataConfiguration, m_stealthFriendlyOpacity ) },
	{ "DefaultOcclusionDelay",				INI::parseDurationUnsignedInt,				NULL,			offsetof( GlobalDataConfiguration, m_defaultOcclusionDelay ) },
	
	{ "PartitionCellSize",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_partitionCellSize ) },

	{ "AmmoPipScaleFactor",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_ammoPipScaleFactor ) },
	{ "ContainerPipScaleFactor",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_containerPipScaleFactor ) },
	{ "AmmoPipWorldOffset",						INI::parseCoord3D,				NULL,			offsetof( GlobalDataConfiguration, m_ammoPipWorldOffset ) },
	{ "ContainerPipWorldOffset",				INI::parseCoord3D,				NULL,			offsetof( GlobalDataConfiguration, m_containerPipWorldOffset ) },
	{ "AmmoPipScreenOffset",						INI::parseCoord2D,				NULL,			offsetof( GlobalDataConfiguration, m_ammoPipScreenOffset ) },
	{ "ContainerPipScreenOffset",				INI::parseCoord2D,				NULL,			offsetof( GlobalDataConfiguration, m_containerPipScreenOffset ) },

	{ "HistoricDamageLimit",				INI::parseDurationUnsignedInt,				NULL,			offsetof( GlobalDataConfiguration, m_historicDamageLimit ) },

	{ "MaxTerrainTracks",					INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxTerrainTracks ) },
	{ "TimeOfDay",								INI::parseIndexList,	TimeOfDayNames,			offsetof( GlobalDataConfiguration, m_timeOfDay ) },
	{ "Weather",									INI::parseIndexList,	WeatherNames,			offsetof( GlobalDataConfiguration, m_weather ) },
	{ "MakeTrackMarks",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_makeTrackMarks ) },
	{ "HideGarrisonFlags",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_hideGarrisonFlags ) },
	{ "ForceModelsToFollowTimeOfDay",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_forceModelsToFollowTimeOfDay ) },
	{ "ForceModelsToFollowWeather",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_forceModelsToFollowWeather ) },

	{ "LevelGainAnimationName",		INI::parseAsciiString,	NULL,	offsetof( GlobalDataConfiguration, m_levelGainAnimationName ) },
	{ "LevelGainAnimationTime",		INI::parseReal,					NULL, offsetof( GlobalDataConfiguration, m_levelGainAnimationDisplayTimeInSeconds ) },
	{ "LevelGainAnimationZRise",	INI::parseReal,					NULL, offsetof( GlobalDataConfiguration, m_levelGainAnimationZRisePerSecond ) },

	{ "GetHealedAnimationName",		INI::parseAsciiString,	NULL,	offsetof( GlobalDataConfiguration, m_getHealedAnimationName ) },
	{ "GetHealedAnimationTime",		INI::parseReal,					NULL, offsetof( GlobalDataConfiguration, m_getHealedAnimationDisplayTimeInSeconds ) },
	{ "GetHealedAnimationZRise",	INI::parseReal,					NULL, offsetof( GlobalDataConfiguration, m_getHealedAnimationZRisePerSecond ) },

	{ "TerrainLightingMorningAmbient",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][0].ambient ) },
	{ "TerrainLightingMorningDiffuse",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][0].diffuse ) },
	{ "TerrainLightingMorningLightPos",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][0].lightPos ) },
	{ "TerrainLightingAfternoonAmbient",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][0].ambient ) },
	{ "TerrainLightingAfternoonDiffuse",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][0].diffuse ) },
	{ "TerrainLightingAfternoonLightPos",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][0].lightPos ) },
	{ "TerrainLightingEveningAmbient",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][0].ambient ) },
	{ "TerrainLightingEveningDiffuse",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][0].diffuse ) },
	{ "TerrainLightingEveningLightPos",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][0].lightPos ) },
	{ "TerrainLightingNightAmbient",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][0].ambient ) },
	{ "TerrainLightingNightDiffuse",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][0].diffuse ) },
	{ "TerrainLightingNightLightPos",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][0].lightPos ) },

	{ "TerrainObjectsLightingMorningAmbient",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][0].ambient ) },
	{ "TerrainObjectsLightingMorningDiffuse",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][0].diffuse ) },
	{ "TerrainObjectsLightingMorningLightPos",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][0].lightPos ) },
	{ "TerrainObjectsLightingAfternoonAmbient",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][0].ambient ) },
	{ "TerrainObjectsLightingAfternoonDiffuse",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][0].diffuse ) },
	{ "TerrainObjectsLightingAfternoonLightPos",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][0].lightPos ) },
	{ "TerrainObjectsLightingEveningAmbient",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][0].ambient ) },
	{ "TerrainObjectsLightingEveningDiffuse",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][0].diffuse ) },
	{ "TerrainObjectsLightingEveningLightPos",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][0].lightPos ) },
	{ "TerrainObjectsLightingNightAmbient",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][0].ambient ) },
	{ "TerrainObjectsLightingNightDiffuse",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][0].diffuse ) },
	{ "TerrainObjectsLightingNightLightPos",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][0].lightPos ) },

	//Secondary global light	
	{ "TerrainLightingMorningAmbient2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][1].ambient ) },
	{ "TerrainLightingMorningDiffuse2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][1].diffuse ) },
	{ "TerrainLightingMorningLightPos2",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][1].lightPos ) },
	{ "TerrainLightingAfternoonAmbient2",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][1].ambient ) },
	{ "TerrainLightingAfternoonDiffuse2",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][1].diffuse ) },
	{ "TerrainLightingAfternoonLightPos2",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][1].lightPos ) },
	{ "TerrainLightingEveningAmbient2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][1].ambient ) },
	{ "TerrainLightingEveningDiffuse2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][1].diffuse ) },
	{ "TerrainLightingEveningLightPos2",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][1].lightPos ) },
	{ "TerrainLightingNightAmbient2",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][1].ambient ) },
	{ "TerrainLightingNightDiffuse2",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][1].diffuse ) },
	{ "TerrainLightingNightLightPos2",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][1].lightPos ) },

	{ "TerrainObjectsLightingMorningAmbient2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][1].ambient ) },
	{ "TerrainObjectsLightingMorningDiffuse2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][1].diffuse ) },
	{ "TerrainObjectsLightingMorningLightPos2",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][1].lightPos ) },
	{ "TerrainObjectsLightingAfternoonAmbient2",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][1].ambient ) },
	{ "TerrainObjectsLightingAfternoonDiffuse2",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][1].diffuse ) },
	{ "TerrainObjectsLightingAfternoonLightPos2",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][1].lightPos ) },
	{ "TerrainObjectsLightingEveningAmbient2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][1].ambient ) },
	{ "TerrainObjectsLightingEveningDiffuse2",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][1].diffuse ) },
	{ "TerrainObjectsLightingEveningLightPos2",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][1].lightPos ) },
	{ "TerrainObjectsLightingNightAmbient2",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][1].ambient ) },
	{ "TerrainObjectsLightingNightDiffuse2",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][1].diffuse ) },
	{ "TerrainObjectsLightingNightLightPos2",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][1].lightPos ) },

	//Third global light
	{ "TerrainLightingMorningAmbient3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][2].ambient ) },
	{ "TerrainLightingMorningDiffuse3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][2].diffuse ) },
	{ "TerrainLightingMorningLightPos3",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_MORNING ][2].lightPos ) },
	{ "TerrainLightingAfternoonAmbient3",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][2].ambient ) },
	{ "TerrainLightingAfternoonDiffuse3",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][2].diffuse ) },
	{ "TerrainLightingAfternoonLightPos3",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_AFTERNOON ][2].lightPos ) },
	{ "TerrainLightingEveningAmbient3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][2].ambient ) },
	{ "TerrainLightingEveningDiffuse3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][2].diffuse ) },
	{ "TerrainLightingEveningLightPos3",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_EVENING ][2].lightPos ) },
	{ "TerrainLightingNightAmbient3",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][2].ambient ) },
	{ "TerrainLightingNightDiffuse3",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][2].diffuse ) },
	{ "TerrainLightingNightLightPos3",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainLighting[ TIME_OF_DAY_NIGHT ][2].lightPos ) },

	{ "TerrainObjectsLightingMorningAmbient3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][2].ambient ) },
	{ "TerrainObjectsLightingMorningDiffuse3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][2].diffuse ) },
	{ "TerrainObjectsLightingMorningLightPos3",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_MORNING ][2].lightPos ) },
	{ "TerrainObjectsLightingAfternoonAmbient3",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][2].ambient ) },
	{ "TerrainObjectsLightingAfternoonDiffuse3",		INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][2].diffuse ) },
	{ "TerrainObjectsLightingAfternoonLightPos3",	INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_AFTERNOON ][2].lightPos ) },
	{ "TerrainObjectsLightingEveningAmbient3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][2].ambient ) },
	{ "TerrainObjectsLightingEveningDiffuse3",			INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][2].diffuse ) },
	{ "TerrainObjectsLightingEveningLightPos3",		INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_EVENING ][2].lightPos ) },
	{ "TerrainObjectsLightingNightAmbient3",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][2].ambient ) },
	{ "TerrainObjectsLightingNightDiffuse3",				INI::parseRGBColor,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][2].diffuse ) },
	{ "TerrainObjectsLightingNightLightPos3",			INI::parseCoord3D,			NULL,			offsetof( GlobalDataConfiguration, m_terrainObjectsLighting[ TIME_OF_DAY_NIGHT ][2].lightPos ) },

	
	{ "NumberGlobalLights",								INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_numGlobalLights)},
	{ "InfantryLightMorningScale",				INI::parseReal,			NULL,			offsetof( GlobalDataConfiguration, m_infantryLightScale[TIME_OF_DAY_MORNING] ) },
	{ "InfantryLightAfternoonScale",				INI::parseReal,			NULL,			offsetof( GlobalDataConfiguration, m_infantryLightScale[TIME_OF_DAY_AFTERNOON] ) },
	{ "InfantryLightEveningScale",				INI::parseReal,			NULL,			offsetof( GlobalDataConfiguration, m_infantryLightScale[TIME_OF_DAY_EVENING] ) },
	{ "InfantryLightNightScale",				INI::parseReal,			NULL,			offsetof( GlobalDataConfiguration, m_infantryLightScale[TIME_OF_DAY_NIGHT] ) },

	{ "MaxTranslucentObjects",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxVisibleTranslucentObjects) },
	{ "OccludedColorLuminanceScale",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_occludedLuminanceScale) },

/* These are internal use only, they do not need file definitons 
	{ "TerrainAmbientRGB",				INI::parseRGBColor,		NULL,			offsetof( GlobalDataConfiguration, m_terrainAmbient ) },
	{ "TerrainDiffuseRGB",				INI::parseRGBColor,		NULL,			offsetof( GlobalDataConfiguration, m_terrainDiffuse ) },
	{ "TerrainLightPos",					INI::parseCoord3D,		NULL,			offsetof( GlobalDataConfiguration, m_terrainLightPos ) },
*/
	{ "MaxRoadSegments",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxRoadSegments ) },
	{ "MaxRoadVertex",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxRoadVertex ) },
	{ "MaxRoadIndex",								INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxRoadIndex ) },
	{ "MaxRoadTypes",								INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxRoadTypes ) },

	{ "ValuePerSupplyBox",					INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_baseValuePerSupplyBox ) },
	
	{ "AudioOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_audioOn ) },
	{ "MusicOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_musicOn ) },
	{ "SoundsOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_soundsOn ) },
	{ "Sounds3DOn",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_sounds3DOn ) },
	{ "SpeechOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_speechOn ) },
	{ "VideoOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_videoOn ) },
	{ "DisableCameraMovements",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_disableCameraMovement ) },

/* These are internal use only, they do not need file definitons 		
	/// @todo remove this hack
	{ "InGame",											INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_inGame ) },
*/

	{ "DebugAI",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_debugAI ) },
	{ "DebugAIObstacles",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_debugAIObstacles ) },
	{ "ShowClientPhysics",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showClientPhysics ) },
	{ "ShowTerrainNormals",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showTerrainNormals ) },
	{ "ShowObjectHealth",						INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showObjectHealth ) },

	{ "ParticleScale",										INI::parseReal,					NULL,	 offsetof( GlobalDataConfiguration, m_particleScale ) },
	{ "AutoFireParticleSmallPrefix",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleSmallPrefix ) },
	{ "AutoFireParticleSmallSystem",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleSmallSystem ) },
	{ "AutoFireParticleSmallMax",					INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoFireParticleSmallMax ) },
	{ "AutoFireParticleMediumPrefix",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleMediumPrefix ) },
	{ "AutoFireParticleMediumSystem",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleMediumSystem ) },
	{ "AutoFireParticleMediumMax",				INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoFireParticleMediumMax ) },
	{ "AutoFireParticleLargePrefix",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleLargePrefix ) },
	{ "AutoFireParticleLargeSystem",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoFireParticleLargeSystem ) },
	{ "AutoFireParticleLargeMax",					INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoFireParticleLargeMax ) },
	{ "AutoSmokeParticleSmallPrefix",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleSmallPrefix ) },
	{ "AutoSmokeParticleSmallSystem",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleSmallSystem ) },
	{ "AutoSmokeParticleSmallMax",				INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoSmokeParticleSmallMax ) },
	{ "AutoSmokeParticleMediumPrefix",		INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleMediumPrefix ) },
	{ "AutoSmokeParticleMediumSystem",		INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleMediumSystem ) },
	{ "AutoSmokeParticleMediumMax",				INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoSmokeParticleMediumMax ) },
	{ "AutoSmokeParticleLargePrefix",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleLargePrefix ) },
	{ "AutoSmokeParticleLargeSystem",			INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoSmokeParticleLargeSystem ) },
	{ "AutoSmokeParticleLargeMax",				INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoSmokeParticleLargeMax ) },
	{ "AutoAflameParticlePrefix",					INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoAflameParticlePrefix ) },
	{ "AutoAflameParticleSystem",					INI::parseAsciiString,  NULL,  offsetof( GlobalDataConfiguration, m_autoAflameParticleSystem ) },
	{ "AutoAflameParticleMax",						INI::parseInt,					NULL,	 offsetof( GlobalDataConfiguration, m_autoAflameParticleMax ) },

/* These are internal use only, they do not need file definitons 	
	{ "LatencyAverage",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_latencyAverage ) },
	{ "LatencyAmplitude",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_latencyAmplitude ) },
	{ "LatencyPeriod",							INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_latencyPeriod ) },
	{ "LatencyNoise",								INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_latencyNoise ) },
	{ "PacketLoss",									INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_packetLoss ) },
*/

	{ "BuildSpeed",									INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_BuildSpeed ) },
	{ "MinDistFromEdgeOfMapForBuild",	 INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_MinDistFromEdgeOfMapForBuild ) },
	{ "SupplyBuildBorder",	 INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_SupplyBuildBorder ) },
	{ "AllowedHeightVariationForBuilding", INI::parseReal,NULL,			offsetof( GlobalDataConfiguration, m_allowedHeightVariationForBuilding ) },
	{ "MinLowEnergyProductionSpeed",INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_MinLowEnergyProductionSpeed ) },
	{ "MaxLowEnergyProductionSpeed",INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_MaxLowEnergyProductionSpeed ) },
	{ "LowEnergyPenaltyModifier",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_LowEnergyPenaltyModifier ) },
	{ "MultipleFactory",						INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_MultipleFactory ) },
	{ "RefundPercent",							INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_RefundPercent ) },

	{ "CommandCenterHealRange",			INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_commandCenterHealRange ) },
	{ "CommandCenterHealAmount",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_commandCenterHealAmount ) },

	{ "StandardMinefieldDensity",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_standardMinefieldDensity ) },
	{ "StandardMinefieldDistance",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_standardMinefieldDistance ) },

	{ "MaxLineBuildObjects",				INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxLineBuildObjects ) },
	{ "MaxTunnelCapacity",					INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxTunnelCapacity ) },

	{ "MaxParticleCount",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxParticleCount ) },
	{ "MaxFieldParticleCount",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxFieldParticleCount ) },
	{ "HorizontalScrollSpeedFactor",INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_horizontalScrollSpeedFactor ) },
	{ "VerticalScrollSpeedFactor",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_verticalScrollSpeedFactor ) },
	{ "ScrollAmountCutoff",					INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_scrollAmountCutoff ) },
	{ "CameraAdjustSpeed",					INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_cameraAdjustSpeed ) },
	{ "EnforceMaxCameraHeight",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_enforceMaxCameraHeight ) },
	{ "KeyboardScrollSpeedFactor",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_keyboardScrollFactor ) },
	{ "KeyboardDefaultScrollSpeedFactor",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_keyboardDefaultScrollFactor ) },
	{ "MovementPenaltyDamageState",	INI::parseIndexList,	TheBodyDamageTypeNames,	 offsetof( GlobalDataConfiguration, m_movementPenaltyDamageState ) },

// you cannot set this; it always has a value of 100%.
//{ "HealthBonus_Regular",				INI::parsePercentToReal, NULL,	offsetof( GlobalDataConfiguration, m_healthBonus[LEVEL_REGULAR]) },
	{ "HealthBonus_Veteran",				INI::parsePercentToReal, NULL,	offsetof( GlobalDataConfiguration, m_healthBonus[LEVEL_VETERAN]) },
	{ "HealthBonus_Elite",					INI::parsePercentToReal, NULL,	offsetof( GlobalDataConfiguration, m_healthBonus[LEVEL_ELITE]) },
	{ "HealthBonus_Heroic",					INI::parsePercentToReal, NULL,	offsetof( GlobalDataConfiguration, m_healthBonus[LEVEL_HEROIC]) },

	{ "HumanSoloPlayerHealthBonus_Easy",					INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_HUMAN][DIFFICULTY_EASY] ) },
	{ "HumanSoloPlayerHealthBonus_Normal",				INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_HUMAN][DIFFICULTY_NORMAL] ) },
	{ "HumanSoloPlayerHealthBonus_Hard",				INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_HUMAN][DIFFICULTY_HARD] ) },

	{ "AISoloPlayerHealthBonus_Easy",					INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_COMPUTER][DIFFICULTY_EASY] ) },
	{ "AISoloPlayerHealthBonus_Normal",				INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_COMPUTER][DIFFICULTY_NORMAL] ) },
	{ "AISoloPlayerHealthBonus_Hard",				INI::parsePercentToReal,			NULL,			offsetof( GlobalDataConfiguration, m_soloPlayerHealthBonusForDifficulty[PLAYER_COMPUTER][DIFFICULTY_HARD] ) },

	{ "WeaponBonus",								WeaponBonusSet::parseWeaponBonusSetPtr,	NULL,	offsetof( GlobalDataConfiguration, m_weaponBonusSet ) },

	{ "DefaultStructureRubbleHeight",	INI::parseReal,			NULL,			offsetof( GlobalDataConfiguration, m_defaultStructureRubbleHeight ) },

	{ "FixedSeed",									INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_fixedSeed ) },

	{ "ShellMapName",								INI::parseAsciiString,NULL,			offsetof( GlobalDataConfiguration, m_shellMapName ) },
	{ "ShellMapOn",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_shellMapOn ) },
	{	"PlayIntro",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_playIntro ) },

	{ "FirewallBehavior",						INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_firewallBehavior ) },
	{ "FirewallPortOverride",				INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_firewallPortOverride ) },
	{	"FirewallPortAllocationDelta",INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_firewallPortAllocationDelta) },

	{	"GroupSelectMinSelectSize",		INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_groupSelectMinSelectSize ) },
	{	"GroupSelectVolumeBase",			INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_groupSelectVolumeBase ) },
	{	"GroupSelectVolumeIncrement",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_groupSelectVolumeIncrement ) },
	{	"MaxUnitSelectSounds",				INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_maxUnitSelectSounds ) },

	{	"SelectionFlashSaturationFactor",	INI::parseReal,		NULL,			offsetof( GlobalDataConfiguration, m_selectionFlashSaturationFactor ) },
	{	"SelectionFlashHouseColor",	      INI::parseBool,		NULL,			offsetof( GlobalDataConfiguration, m_selectionFlashHouseColor ) },

	{	"CameraAudibleRadius",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_cameraAudibleRadius ) },
	{ "GroupMoveClickToGatherAreaFactor", INI::parseReal,	NULL,			offsetof( GlobalDataConfiguration, m_groupMoveClickToGatherFactor ) },
	{ "ShakeSubtleIntensity",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeSubtleIntensity ) },
	{ "ShakeNormalIntensity",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeNormalIntensity ) },
	{ "ShakeStrongIntensity",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeStrongIntensity ) },
	{ "ShakeSevereIntensity",				INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeSevereIntensity ) },
	{ "ShakeCineExtremeIntensity",	INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeCineExtremeIntensity ) },
	{ "ShakeCineInsaneIntensity",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_shakeCineInsaneIntensity ) },
	{ "MaxShakeIntensity",					INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_maxShakeIntensity ) },
	{ "MaxShakeRange",							INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_maxShakeRange) },
	{ "SellPercentage",							INI::parsePercentToReal,	NULL,			offsetof( GlobalDataConfiguration, m_sellPercentage ) },
	{ "BaseRegenHealthPercentPerSecond", INI::parsePercentToReal, NULL,	offsetof( GlobalDataConfiguration, m_baseRegenHealthPercentPerSecond ) },
	{ "BaseRegenDelay",							INI::parseDurationUnsignedInt, NULL,offsetof( GlobalDataConfiguration, m_baseRegenDelay ) },

#ifdef ALLOW_SURRENDER
	{ "PrisonBountyMultiplier",			INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_prisonBountyMultiplier ) },
	{ "PrisonBountyTextColor",			INI::parseColorInt,		NULL,			offsetof( GlobalDataConfiguration, m_prisonBountyTextColor ) },
#endif

	{ "SpecialPowerViewObject",			INI::parseAsciiString,	NULL,			offsetof( GlobalDataConfiguration, m_specialPowerViewObjectName ) },

	{ "StandardPublicBone", INI::parseAsciiStringVectorAppend, NULL, offsetof(GlobalDataConfiguration, m_standardPublicBones) },
	{ "ShowMetrics",								INI::parseBool,				   NULL,		offsetof( GlobalDataConfiguration, m_showMetrics ) },
  { "DefaultStartingCash",				Money::parseMoneyAmount, NULL,		offsetof( GlobalDataConfiguration, m_defaultStartingCash ) },

// NOTE: m_doubleClickTimeMS is still in use, but we disallow setting it from the GameData.ini file. It is now set in the constructor according to the windows parameter.
//	{ "DoubleClickTimeMS",									INI::parseUnsignedInt,			NULL, offsetof( GlobalDataConfiguration, m_doubleClickTimeMS ) },
	
	{ "ShroudColor",		INI::parseRGBColor,						NULL,	offsetof( GlobalDataConfiguration, m_shroudColor) },
	{ "ClearAlpha",			INI::parseUnsignedByte,				NULL,	offsetof( GlobalDataConfiguration, m_clearAlpha) },
	{ "FogAlpha",				INI::parseUnsignedByte,				NULL,	offsetof( GlobalDataConfiguration, m_fogAlpha) },
	{ "ShroudAlpha",		INI::parseUnsignedByte,				NULL,	offsetof( GlobalDataConfiguration, m_shroudAlpha) },

	{ "HotKeyTextColor",										INI::parseColorInt,					NULL,	offsetof( GlobalDataConfiguration, m_hotKeyTextColor ) },

	{ "PowerBarBase",												INI::parseInt,							NULL,	offsetof( GlobalDataConfiguration, m_powerBarBase) },
	{ "PowerBarIntervals",									INI::parseReal,							NULL,	offsetof( GlobalDataConfiguration, m_powerBarIntervals) },
	{ "PowerBarYellowRange",								INI::parseInt,							NULL,	offsetof( GlobalDataConfiguration, m_powerBarYellowRange) },
	{ "UnlookPersistDuration",							INI::parseDurationUnsignedInt, NULL, offsetof( GlobalDataConfiguration, m_unlookPersistDuration) },

	{ "NetworkFPSHistoryLength", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkFPSHistoryLength) },
	{ "NetworkLatencyHistoryLength", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkLatencyHistoryLength) },
	{ "NetworkRunAheadMetricsTime", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkRunAheadMetricsTime) },
	{ "NetworkCushionHistoryLength", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkCushionHistoryLength) },
	{ "NetworkRunAheadSlack", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkRunAheadSlack) },
	{ "NetworkKeepAliveDelay", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkKeepAliveDelay) },
	{ "NetworkDisconnectTime", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkDisconnectTime) },
	{ "NetworkPlayerTimeoutTime", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkPlayerTimeoutTime) },
	{ "NetworkDisconnectScreenNotifyTime", INI::parseInt, NULL, offsetof(GlobalDataConfiguration, m_networkDisconnectScreenNotifyTime) },
	
	{ "KeyboardCameraRotateSpeed", INI::parseReal, NULL, offsetof( GlobalDataConfiguration, m_keyboardCameraRotateSpeed ) },
	{ "PlayStats",									INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_playStats ) },

#if defined(_DEBUG) || defined(_INTERNAL)
	{ "DisableCameraFade",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_disableCameraFade ) },
	{ "DisableScriptedInputDisabling",			INI::parseBool,		NULL,			offsetof( GlobalDataConfiguration, m_disableScriptedInputDisabling ) },
	{ "DisableMilitaryCaption",			INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_disableMilitaryCaption ) },
	{ "BenchmarkTimer",			INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_benchmarkTimer ) },
	{ "CheckMemoryLeaks", INI::parseBool, NULL, offsetof(GlobalDataConfiguration, m_checkForLeaks) },
	{ "Wireframe",								INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_wireframe ) },
	{ "StateMachineDebug",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_stateMachineDebug ) },
	{ "UseCameraConstraints",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useCameraConstraints ) },
	{ "ShroudOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_shroudOn ) },
	{ "FogOfWarOn",										INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_fogOfWarOn ) },
	{ "ShowCollisionExtents",				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showCollisionExtents ) },
  { "ShowAudioLocations",  				INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_showAudioLocations ) },
	{ "DebugProjectileTileWidth",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_debugProjectileTileWidth) },
	{ "DebugProjectileTileDuration",INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_debugProjectileTileDuration) },
	{ "DebugProjectileTileColor",		INI::parseRGBColor,		NULL,			offsetof( GlobalDataConfiguration, m_debugProjectileTileColor) },
	{ "DebugVisibilityTileCount",		INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_debugVisibilityTileCount) },
	{ "DebugVisibilityTileWidth",		INI::parseReal,				NULL,			offsetof( GlobalDataConfiguration, m_debugVisibilityTileWidth) },
	{ "DebugVisibilityTileDuration",INI::parseInt,				NULL,			offsetof( GlobalDataConfiguration, m_debugVisibilityTileDuration) },
	{ "DebugVisibilityTileTargettableColor",INI::parseRGBColor, NULL,	offsetof( GlobalDataConfiguration, m_debugVisibilityTargettableColor) },
	{ "DebugVisibilityTileDeshroudColor",		INI::parseRGBColor,	NULL,	offsetof( GlobalDataConfiguration, m_debugVisibilityDeshroudColor) },
	{ "DebugVisibilityTileGapColor",				INI::parseRGBColor,	NULL,	offsetof( GlobalDataConfiguration, m_debugVisibilityGapColor) },
	{ "DebugThreatMapTileDuration",					INI::parseInt,							NULL,	offsetof( GlobalDataConfiguration, m_debugThreatMapTileDuration) },
	{ "MaxDebugThreatMapValue",							INI::parseUnsignedInt,			NULL,	offsetof( GlobalDataConfiguration, m_maxDebugThreat) },
	{ "DebugCashValueMapTileDuration",			INI::parseInt,							NULL,	offsetof( GlobalDataConfiguration, m_debugCashValueMapTileDuration) },
	{ "MaxDebugCashValueMapValue",					INI::parseUnsignedInt,			NULL,	offsetof( GlobalDataConfiguration, m_maxDebugValue) },
	{ "VTune", INI::parseBool,	NULL,			offsetof( GlobalDataConfiguration, m_vTune ) },
	{ "SaveStats",									INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_saveStats ) },
	{ "UseLocalMOTD",								INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_useLocalMOTD ) },
	{ "BaseStatsDir",								INI::parseAsciiString,NULL,			offsetof( GlobalDataConfiguration, m_baseStatsDir ) },
	{ "LocalMOTDPath",							INI::parseAsciiString,NULL,			offsetof( GlobalDataConfiguration, m_MOTDPath ) },
	{ "ExtraLogging",								INI::parseBool,				NULL,			offsetof( GlobalDataConfiguration, m_extraLogging ) },
#endif

	{ NULL,					NULL,						NULL,						0 }  // keep this last

};


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GlobalData::GlobalData() : GlobalDataConfiguration{}
{
	MemoryPoolObjectHolder bonusOwner;
	if (!TheFileSystem) throw ERROR_BAD_ARG;
	Int i, j;

	// Publication belongs to init/complete parse, never a fallible constructor.

#if defined(_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	m_specialPowerUsesDelay = TRUE;
#endif
  m_TiVOFastMode = FALSE;

#if defined(_DEBUG) || defined(_INTERNAL)
	m_wireframe = 0;
	m_stateMachineDebug = FALSE;
	m_useCameraConstraints = TRUE;
	m_shroudOn = TRUE;
	m_fogOfWarOn = FALSE;
	m_jabberOn = FALSE;
	m_munkeeOn = FALSE;
	m_showCollisionExtents = FALSE;
  m_showAudioLocations = FALSE;
	m_debugCamera = FALSE;
	m_debugVisibility = FALSE;
	m_debugVisibilityTileCount = 32;	// default to 32.
	m_debugVisibilityTileDuration = LOGICFRAMES_PER_SECOND;
	m_debugProjectilePath = FALSE;
	m_debugProjectileTileWidth = 10;
	m_debugProjectileTileDuration = LOGICFRAMES_PER_SECOND;  // Changed By Sadullah Nader
	m_debugThreatMap = FALSE;
	m_maxDebugThreat = 5000;
	m_debugThreatMapTileDuration = LOGICFRAMES_PER_SECOND;  // Changed By Sadullah Nader
	m_debugCashValueMap = FALSE;
	m_maxDebugValue = 10000;
	m_debugCashValueMapTileDuration = LOGICFRAMES_PER_SECOND; // Changed By Sadullah Nader
	m_debugIgnoreAsserts = FALSE;
	m_debugIgnoreStackTrace = FALSE;
	m_vTune = false;
	m_checkForLeaks = TRUE;
	m_benchmarkTimer = -1;
  

	m_allowUnselectableSelection = FALSE;
	m_disableCameraFade = false;
	m_disableScriptedInputDisabling = false;
	m_disableMilitaryCaption = false;
	m_latencyAverage = 0;
	m_latencyAmplitude = 0;
	m_latencyPeriod = 0;
	m_latencyNoise = 0;
	m_packetLoss = 0;
	m_saveStats = FALSE;
	m_saveAllStats = FALSE;
	m_useLocalMOTD = FALSE;
	m_baseStatsDir = ".\\";
	m_MOTDPath = "MOTD.txt";
	m_extraLogging = FALSE;
#endif

	m_playStats = -1;
	m_incrementalAGPBuf = FALSE;
	m_mapName.clear();
	m_moveHintName.clear();
	m_useTrees = 0;
	m_useTreeSway = TRUE;
	m_useDrawModuleLOD = FALSE;
	m_useHeatEffects = TRUE;
	m_useFpsLimit = FALSE;
	m_dumpAssetUsage = FALSE;
	m_framesPerSecondLimit = 0;
	m_chipSetType = 0;
	m_windowed = 0;
	m_xResolution = 800;
	m_yResolution = 600;
	m_maxShellScreens = 0;
	m_useCloudMap = FALSE;
	m_use3WayTerrainBlends = 1;
	m_useLightMap = FALSE;
	m_bilinearTerrainTex = FALSE;
	m_trilinearTerrainTex = FALSE;
	m_multiPassTerrain = FALSE;
	m_adjustCliffTextures = FALSE;
	m_stretchTerrain = FALSE;
	m_useHalfHeightMap = FALSE;
	m_terrainLOD = TERRAIN_LOD_AUTOMATIC;
	m_terrainLODTargetTimeMS = 0;
	m_enableDynamicLOD = TRUE;
	m_enableStaticLOD = TRUE;
	m_rightMouseAlwaysScrolls = FALSE;
	m_useWaterPlane = FALSE;
	m_useCloudPlane = FALSE;
	m_downwindAngle = ( -0.785f );//Northeast!
	m_useShadowVolumes = FALSE;
	m_useShadowDecals = FALSE;
	m_textureReductionFactor = -1;
	m_enableBehindBuildingMarkers = TRUE;
	m_scriptDebug = FALSE;
	m_particleEdit = FALSE;
	m_displayDebug = FALSE;
	m_winCursors = TRUE;
	m_constantDebugUpdate = FALSE;
	m_showTeamDot = FALSE;
	m_fixedSeed = -1; // disabled
	m_horizontalScrollSpeedFactor = 1.0;
	m_verticalScrollSpeedFactor = 1.0;

	m_waterPositionX = 0.0f;
	m_waterPositionY = 0.0f;
	m_waterPositionZ = 0.0f;
	m_waterExtentX = 0.0f;	
	m_waterExtentY = 0.0f;
	m_waterType = 0;
	m_featherWater = FALSE;
	m_showSoftWaterEdge = TRUE;	//display soft water edge
	m_usingWaterTrackEditor = FALSE;
	m_isWorldBuilder = FALSE;

	m_showMetrics = false;

	for( i = 0; i < MAX_WATER_GRID_SETTINGS; i++ )
	{

		m_vertexWaterHeightClampLow[ i ] = 0.0f;
		m_vertexWaterHeightClampHi[ i ] = 0.0f;
		m_vertexWaterAngle[ i ] = 0.0f;
		m_vertexWaterXPosition[ i ] = 0.0f;
		m_vertexWaterYPosition[ i ] = 0.0f;
		m_vertexWaterZPosition[ i ] = 0.0f;
		m_vertexWaterXGridCells[ i ] = 0;
		m_vertexWaterYGridCells[ i ] = 0;
		m_vertexWaterGridSize[ i ] = 0.0f;
		m_vertexWaterAttenuationA[ i ] = 0.0f;
		m_vertexWaterAttenuationB[ i ] = 0.0f;
		m_vertexWaterAttenuationC[ i ] = 0.0f;
		m_vertexWaterAttenuationRange[ i ] = 0.0f;
		//Added By Sadullah Nader
		//Initializations missing and needed
		m_vertexWaterAvailableMaps[i].clear();
	}  // end for i

	m_skyBoxPositionZ = 0.0f;
	m_drawSkyBox = FALSE;
	m_skyBoxScale = 4.5f;

	m_historicDamageLimit = 0;
	m_maxTerrainTracks = 0;

	m_levelGainAnimationDisplayTimeInSeconds = 0.0f;
	m_levelGainAnimationZRisePerSecond = 0.0f;

	m_getHealedAnimationDisplayTimeInSeconds = 0.0f;
	m_getHealedAnimationZRisePerSecond = 0.0f;

	m_maxTankTrackEdges=100;
	m_maxTankTrackOpaqueEdges=25;
	m_maxTankTrackFadeDelay=300000;

	m_timeOfDay = TIME_OF_DAY_AFTERNOON;
	m_weather = WEATHER_NORMAL;
	m_makeTrackMarks = FALSE;
	m_hideGarrisonFlags = FALSE;
	m_forceModelsToFollowTimeOfDay = true;
	m_forceModelsToFollowWeather = true;

	m_partitionCellSize = 0.0f;
	m_ammoPipScaleFactor = 1.0f;
	m_containerPipScaleFactor = 1.0f;
	m_ammoPipWorldOffset.zero();
	m_containerPipWorldOffset.zero();
	m_ammoPipScreenOffset.x = m_ammoPipScreenOffset.y = 0;
	m_containerPipScreenOffset.x = m_containerPipScreenOffset.y = 0;

	for (i=0; i<MAX_GLOBAL_LIGHTS; i++)
	{
		m_terrainAmbient[i].red = 0.0f;
		m_terrainAmbient[i].green = 0.0f;
		m_terrainAmbient[i].blue = 0.0f;
		m_terrainDiffuse[i].red = 0.0f;
		m_terrainDiffuse[i].green = 0.0f;
		m_terrainDiffuse[i].blue = 0.0f;
		m_terrainLightPos[i].x = 0.0f;
		m_terrainLightPos[i].y = 0.0f;
		m_terrainLightPos[i].z = -1.0f;

		for (j=0; j<TIME_OF_DAY_COUNT; j++)
		{	m_terrainLighting[ j ][i].ambient.red=0;
			m_terrainLighting[ j ][i].ambient.green=0;
			m_terrainLighting[ j ][i].ambient.blue=0;
			m_terrainLighting[ j ][i].diffuse.red=0;
			m_terrainLighting[ j ][i].diffuse.green=0;
			m_terrainLighting[ j ][i].diffuse.blue=0;
			m_terrainLighting[ j ][i].lightPos.x=0;
			m_terrainLighting[ j ][i].lightPos.y=0;
			m_terrainLighting[ j ][i].lightPos.z=-1.0f;

			m_terrainObjectsLighting[ j ][i].ambient.red=0;
			m_terrainObjectsLighting[ j ][i].ambient.green=0;
			m_terrainObjectsLighting[ j ][i].ambient.blue=0;
			m_terrainObjectsLighting[ j ][i].diffuse.red=0;
			m_terrainObjectsLighting[ j ][i].diffuse.green=0;
			m_terrainObjectsLighting[ j ][i].diffuse.blue=0;
			m_terrainObjectsLighting[ j ][i].lightPos.x=0;
			m_terrainObjectsLighting[ j ][i].lightPos.y=0;
			m_terrainObjectsLighting[ j ][i].lightPos.z=-1.0f;
		}
	}

	for (j=TIME_OF_DAY_FIRST; j<TIME_OF_DAY_COUNT; j++)
		m_infantryLightScale[j] = 1.5f;
	
	m_scriptOverrideInfantryLightScale = -1.0f;

	m_numGlobalLights = 3;
	m_maxRoadSegments = 0;
	m_maxRoadVertex = 0;
	m_maxRoadIndex = 0;
	m_maxRoadTypes = 0;

	m_baseValuePerSupplyBox = 100;

	m_audioOn = TRUE;
	m_musicOn = TRUE;
	m_soundsOn = TRUE;
	m_sounds3DOn = TRUE;
	m_speechOn = TRUE;
	m_videoOn = TRUE;
	m_disableCameraMovement = FALSE;
	m_maxVisibleTranslucentObjects = 512;
	m_maxVisibleOccluderObjects = 512;
	m_maxVisibleOccludeeObjects = 512;
	m_maxVisibleNonOccluderOrOccludeeObjects = 512;
	m_occludedLuminanceScale = 0.5f;

	m_useFX = TRUE;

//	m_inGame = FALSE;	

	m_noDraw = 0;
	m_particleScale = 1.0f;

	m_autoFireParticleSmallMax = 0;
	m_autoFireParticleMediumMax = 0;
	m_autoFireParticleLargeMax = 0;
	m_autoSmokeParticleSmallMax = 0;
	m_autoSmokeParticleMediumMax = 0;
	m_autoSmokeParticleLargeMax = 0;
	m_autoAflameParticleMax = 0;
  
	// Added By Sadullah Nader
	// Initializations missing and needed
	m_autoFireParticleSmallPrefix.clear();
	m_autoFireParticleMediumPrefix.clear();
	m_autoFireParticleLargePrefix.clear();
	m_autoSmokeParticleSmallPrefix.clear();
	m_autoSmokeParticleMediumPrefix.clear();
	m_autoSmokeParticleLargePrefix.clear();
	m_autoAflameParticlePrefix.clear();

	m_autoFireParticleSmallSystem.clear();
	m_autoFireParticleMediumSystem.clear();
	m_autoFireParticleLargeSystem.clear();
	m_autoSmokeParticleSmallSystem.clear();
	m_autoSmokeParticleMediumSystem.clear();
	m_autoSmokeParticleLargeSystem.clear();
	m_autoAflameParticleSystem.clear();
	m_levelGainAnimationName.clear();
	m_getHealedAnimationName.clear();
	m_specialPowerViewObjectName.clear();

	m_drawEntireTerrain = FALSE;
	m_maxParticleCount = 0;
	m_maxFieldParticleCount = 30;
	
	// End Add

	m_debugAI = AI_DEBUG_NONE;
	m_debugSupplyCenterPlacement = FALSE;
	m_debugAIObstacles = FALSE;
	m_showClientPhysics = TRUE;
	m_showTerrainNormals = FALSE;
	m_showObjectHealth = FALSE;

	m_particleEdit = FALSE;

	m_cameraPitch = 0.0f;
	m_cameraYaw = 0.0f;
	m_cameraHeight = 0.0f;
	m_minCameraHeight = 100.0f;
	m_maxCameraHeight = 300.0f;
	m_terrainHeightAtEdgeOfMap = 0.0f;

	m_unitDamagedThresh = 0.5f;
	m_unitReallyDamagedThresh = 0.1f;
	m_groundStiffness = 0.5f;
	m_structureStiffness = 0.5f;
	m_gravity = -1.0f;
	m_stealthFriendlyOpacity = 0.5f;
	m_defaultOcclusionDelay = LOGICFRAMES_PER_SECOND * 3;	//default to 3 seconds

	m_preloadAssets = FALSE;
	m_preloadEverything = FALSE;
	m_preloadReport = FALSE;

	m_netMinPlayers = 1; // allowing sandbox mode

	m_defaultIP = 0;

	m_BuildSpeed = 0.0f;
	m_MinDistFromEdgeOfMapForBuild = 0.0f;
	m_SupplyBuildBorder = 0.0f;
	m_allowedHeightVariationForBuilding = 0.0f;
	m_MinLowEnergyProductionSpeed = 0.0f;
	m_MaxLowEnergyProductionSpeed = 0.0f;
	m_LowEnergyPenaltyModifier = 0.0f;
	m_MultipleFactory = 0.0f;
	m_RefundPercent = 0.0f;

	m_commandCenterHealRange = 0.0f;
	m_commandCenterHealAmount = 0.0f;
	m_maxTunnelCapacity = 0;
	m_maxLineBuildObjects = 0;

	m_standardMinefieldDensity = 0.01f;
	m_standardMinefieldDistance = 40.0f;
	
	m_groupSelectMinSelectSize = 5;
	m_groupSelectVolumeBase = 0.5f;
	m_groupSelectVolumeIncrement = 0.02f;
	m_maxUnitSelectSounds = 8;

	m_selectionFlashSaturationFactor = 0.5f; /// how colorful should the selection flash be? 0-4
	m_selectionFlashHouseColor = FALSE;  /// skip the house color and just use white.

	m_cameraAudibleRadius = 500.0;
	m_groupMoveClickToGatherFactor = 1.0f;

	m_shakeSubtleIntensity = 0.5f;
	m_shakeNormalIntensity = 1.0f;
	m_shakeStrongIntensity = 2.5f;
	m_shakeSevereIntensity = 5.0f;
	m_shakeCineExtremeIntensity = 8.0f;
	m_shakeCineInsaneIntensity = 12.0f;
	m_maxShakeIntensity = 10.0f;
	m_maxShakeRange = 150.f;

	m_sellPercentage = 1.0f;
	m_baseRegenHealthPercentPerSecond = 0.0f;
	m_baseRegenDelay = 0;

#ifdef ALLOW_SURRENDER
	m_prisonBountyMultiplier = 1.0f;
	m_prisonBountyTextColor = GameMakeColor( 255, 255, 255, 255 );
#endif

	m_hotKeyTextColor = GameMakeColor(255,255,0,255);

  // THis is put ON ice until later
  //  m_cheaterHasBeenSpiedIfMyLowestBitIsTrue = GameMakeColor(255,128,0,0);// orange, to the hacker's eye

	m_shroudColor.setFromInt( 0x00FFFFFF ) ;
	m_clearAlpha = 255;
	m_fogAlpha = 127;
	m_shroudAlpha = 0;

	m_powerBarBase = 7;
	m_powerBarIntervals = 3;
	m_powerBarYellowRange = 5;
	m_displayGamma = 1.0f;	//ramp that does nothing

	m_standardPublicBones.clear();

	m_antiAliasBoxValue = 0;

//	m_languageFilterPref = false;
	m_languageFilterPref = true;
	m_firewallBehavior = FirewallHelperClass::FIREWALL_TYPE_UNKNOWN;
	m_firewallSendDelay = FALSE;
	m_firewallPortOverride = 0;
	m_firewallPortAllocationDelta = 0;
	m_loadScreenDemo = FALSE;
	m_disableRender = false;
	
	m_saveCameraInReplay = FALSE;
	m_useCameraInReplay = FALSE;


	m_debugShowGraphicalFramerate = FALSE;

	// By default, show all asserts.

	m_unlookPersistDuration		= 30;

	//-----------------------------------------------------------------------------------------------

	// network timing values.  Having these default to 0 would be bad. - BGC
	m_networkFPSHistoryLength = 30;
	m_networkLatencyHistoryLength = 200;
	m_networkRunAheadMetricsTime = 500;
	m_networkCushionHistoryLength = 10;
	m_networkRunAheadSlack = 10;
	m_networkKeepAliveDelay = 20;
	m_networkDisconnectTime = 5000;
	m_networkPlayerTimeoutTime = 60000;
	m_networkDisconnectScreenNotifyTime = 15000;

	m_isBreakableMovie = FALSE;
	m_breakTheMovie = FALSE;

	setTimeOfDay( m_timeOfDay );

	m_buildMapCache = FALSE;
	m_initialFile.clear();
	m_pendingFile.clear();

	for (i = LEVEL_FIRST; i <= LEVEL_LAST; ++i)
		m_healthBonus[i] = 1.0f;

	for (i = 0; i < PLAYERTYPE_COUNT; ++i)
	{
		for (j = 0; j < DIFFICULTY_COUNT; ++j)
		{
			m_soloPlayerHealthBonusForDifficulty[i][j] = 1.0f;
		}
	}

	m_defaultStructureRubbleHeight = 1.0f;
	m_weaponBonusSet = newInstance(WeaponBonusSet);
	bonusOwner.hold(m_weaponBonusSet);

	m_shellMapName.set("Maps\\ShellMap1\\ShellMap1.map");
	m_shellMapOn =TRUE;
	m_playIntro = TRUE;
	m_playSizzle = TRUE;
	m_afterIntro = FALSE;
	m_allowExitOutOfMovies = FALSE;
	m_loadScreenRender = FALSE;
  m_musicVolumeFactor = 0.5f;
 	m_SFXVolumeFactor = 0.5f;
  m_voiceVolumeFactor = 0.5f;
  m_3DSoundPref = false;

	m_keyboardDefaultScrollFactor = m_keyboardScrollFactor = 0.5f;
	m_scrollAmountCutoff = 10.0f;
	m_cameraAdjustSpeed = 0.1f;
	m_enforceMaxCameraHeight = TRUE;
	
	m_animateWindows = TRUE;
	
	m_iniCRC = 0;
	m_exeCRC = 0;
	
	// Preserve the source CRC ledger: image, version, skirmish, multiplayer.
	CRC exeCRC;
	unsigned char crcBlock[65536];
	const int executable = ::open("/proc/self/exe", O_RDONLY | O_CLOEXEC);
	if (executable < 0) throw ERROR_BAD_ARG;
	struct DescriptorOwner { int fd; ~DescriptorOwner() { ::close(fd); } } owner{executable};
	for (;;) {
		const ssize_t count = ::read(executable, crcBlock, sizeof(crcBlock));
		if (count < 0 && errno == EINTR) continue;
		if (count < 0) throw ERROR_BAD_ARG;
		if (count == 0) break;
		exeCRC.computeCRC(crcBlock, static_cast<Int>(count));
	}
	if (TheVersion) {
		UnsignedInt version = TheVersion->getVersionNumber();
		exeCRC.computeCRC(&version, sizeof(version));
	}
	for (const char* path : {"Data\\Scripts\\SkirmishScripts.scb", "Data\\Scripts\\MultiplayerScripts.scb"}) {
		FileCloseOwner file(TheFileSystem->openFile(path, File::READ | File::BINARY));
		if (!file.get()) continue; // Original optional-script admission.
		for (;;) {
			const Int count = file.get()->read(crcBlock, sizeof(crcBlock));
			if (count < 0) throw ERROR_BAD_ARG;
			if (count == 0) break;
			exeCRC.computeCRC(crcBlock, count);
		}
	}

	m_exeCRC = exeCRC.get();
	DEBUG_LOG(("EXE CRC: 0x%8.8X\n", m_exeCRC));
	
	m_movementPenaltyDamageState = BODY_REALLYDAMAGED;
	
	m_shouldUpdateTGAToDDS = FALSE;
	
	// Default DoubleClickTime to System double click time.
	m_doubleClickTimeMS = nativeDoubleClickMilliseconds(); // Actual MS, not frames.
	
#ifdef DUMP_PERF_STATS
	m_dumpPerformanceStatistics = FALSE;
  m_dumpStatsAtInterval = FALSE;
  m_statsInterval = 30;      
#endif

	m_forceBenchmark = FALSE;	///<forces running of CPU detection benchmark, even on known cpu's.

	m_keyboardCameraRotateSpeed = 0.1f;

	if (TheNativeUserStorage) TheNativeUserStorage->validateRootOwnership();
	const NativeUserPaths paths = TheNativeUserStorage
		? TheNativeUserStorage->paths() : NativeUserPaths::fromEnvironment();
	m_userDataDir = paths.data.c_str();
	if (m_userDataDir.getCharAt(m_userDataDir.getLength()-1) != '/') m_userDataDir.concat('/');

	//-allAdvice feature
	//m_allAdvice = FALSE;

	m_clientRetaliationModeEnabled = TRUE; //On by default.
	bonusOwner.release();

}  // end GlobalData


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GlobalData::GlobalData(const GlobalData& prior)
    : SubsystemInterface(), GlobalDataConfiguration(prior)
{
    m_weaponBonusSet = nullptr;
    if (!prior.m_weaponBonusSet) throw ERROR_BAD_ARG;
    WeaponBonusSet* clone = newInstance(WeaponBonusSet);
    MemoryPoolObjectHolder owner(clone);
    static_assert(std::is_nothrow_copy_assignable_v<WeaponBonusSet>);
    *clone = *prior.m_weaponBonusSet;
    m_weaponBonusSet = clone;
    owner.release();
}

void GlobalData::retireOverrides() noexcept
{
    while (m_latest && m_latest != this) {
        GlobalData* retired = m_latest;
        m_latest = retired->m_next;
        retired->m_next = nullptr;
        if (TheWritableGlobalData == retired) TheWritableGlobalData = m_latest;
        delete retired;
    }
}

GlobalData::~GlobalData()
{
    if (m_theOriginal == this) {
        retireOverrides();
        m_theOriginal = nullptr;
    }
    if (TheWritableGlobalData == this) TheWritableGlobalData = nullptr;
    if (m_weaponBonusSet) m_weaponBonusSet->deleteInstance();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool GlobalData::setTimeOfDay( TimeOfDay tod )
{
	if( tod >= TIME_OF_DAY_COUNT || tod < TIME_OF_DAY_FIRST )
	{
		return FALSE;
	}

	m_timeOfDay = tod;
	for (Int i=0; i<MAX_GLOBAL_LIGHTS; i++)
	{	m_terrainAmbient[i] = m_terrainLighting[ tod ][i].ambient;
		m_terrainDiffuse[i] = m_terrainLighting[ tod ][i].diffuse;
		m_terrainLightPos[i] = m_terrainLighting[ tod ][i].lightPos;
	}

	return TRUE;

}

void GlobalData::init()
{
    if (TheWritableGlobalData != this || (m_theOriginal && m_theOriginal != this))
        throw ERROR_BAD_ARG;
    m_theOriginal = this;
    m_latest = this;
}

void GlobalData::reset()
{
    if (!m_theOriginal || (this != m_theOriginal && this != TheWritableGlobalData))
        throw ERROR_BAD_ARG;
    m_theOriginal->retireOverrides();
    TheWritableGlobalData = m_theOriginal;
}

//-------------------------------------------------------------------------------------------------
/** Parse GameData entry */
//-------------------------------------------------------------------------------------------------
void GlobalData::parseGameDataDefinition(INI* ini)
{
    if (!ini || (TheWritableGlobalData &&
        (!m_theOriginal || m_theOriginal->m_latest != TheWritableGlobalData)))
        throw ERROR_BAD_ARG;
    GlobalData* accepted = TheWritableGlobalData;
    std::unique_ptr<GlobalData> candidate(accepted ? new GlobalData(*accepted) : new GlobalData);
    ini->initFromINI(static_cast<GlobalDataConfiguration*>(candidate.get()), s_GlobalDataFieldParseTable);

	// override INI values with user preferences
	OptionPreferences optionPref(*candidate);
	candidate->m_useAlternateMouse = optionPref.getAlternateMouseModeEnabled();
	candidate->m_clientRetaliationModeEnabled = optionPref.getRetaliationModeEnabled();
	candidate->m_doubleClickAttackMove = optionPref.getDoubleClickAttackMoveEnabled();
	candidate->m_keyboardScrollFactor = optionPref.getScrollFactor();
	candidate->m_defaultIP = optionPref.getLANIPAddress();
	candidate->m_firewallSendDelay = optionPref.getSendDelay();
	candidate->m_firewallBehavior = optionPref.getFirewallBehavior();
	candidate->m_firewallPortAllocationDelta = optionPref.getFirewallPortAllocationDelta();
	candidate->m_firewallPortOverride = optionPref.getFirewallPortOverride();
	
	candidate->m_saveCameraInReplay = optionPref.saveCameraInReplays();
	candidate->m_useCameraInReplay = optionPref.useCameraInReplays();
	
	const double rawGamma = optionPref.getGammaValue();
	const Int val = rawGamma >= std::numeric_limits<Int>::min() && rawGamma <= std::numeric_limits<Int>::max()
		? static_cast<Int>(rawGamma) : 50;
	//generate a value between 0.6 and 2.0.
	if (val < 50)
	{	//darker gamma
		if (val <= 0)
			candidate->m_displayGamma = 0.6f;
		else
			candidate->m_displayGamma=1.0f-(0.4f) * (Real)(50-val)/50.0f;
	}
	else
	if (val > 50)
		candidate->m_displayGamma=1.0f+(1.0f) * (static_cast<Real>(val)-50.0f)/50.0f;

	Int xres,yres;
	optionPref.getResolution(&xres, &yres);

	candidate->m_xResolution = xres;
	candidate->m_yResolution = yres;
    // All fallible helpers completed; preserve registered owner identity.
    if (!accepted) {
        m_theOriginal = candidate.get();
        candidate->m_latest = candidate.get();
        TheWritableGlobalData = candidate.release();
    } else if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) {
        candidate->m_next = accepted;
        m_theOriginal->m_latest = candidate.get();
        TheWritableGlobalData = candidate.release();
    } else {
        WeaponBonusSet* priorBonus = accepted->m_weaponBonusSet;
        accepted->adoptConfiguration(std::move(*candidate));
        candidate->m_weaponBonusSet = nullptr;
        priorBonus->deleteInstance();
    }

}
