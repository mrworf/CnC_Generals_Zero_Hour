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


#include <sys/stat.h>
#include <strings.h>
#include "Common/GlobalData.h"
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FileSystem.h"
#include "Common/NativeUserStorage.h"
#include <charconv>
#include <string_view>
#include "Common/CommandLine.h"
#include "Common/CRCDebug.h"
#include "Common/version.h"
#include "GameClient/TerrainLOD.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif



Bool TheDebugIgnoreSyncErrors = FALSE;


#ifdef DEBUG_CRC
Int TheCRCFirstFrameToLog = -1;
UnsignedInt TheCRCLastFrameToLog = 0xffffffff;
Bool g_keepCRCSaves = FALSE;
Bool g_crcModuleDataFromLogic = FALSE;
Bool g_crcModuleDataFromClient = FALSE;
Bool g_verifyClientCRC = FALSE; // verify that GameLogic CRC doesn't change from client
Bool g_clientDeepCRC = FALSE;
Bool g_logObjectCRCs = FALSE;
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
extern Bool g_useStringFile;
#endif

namespace {
Int commandInteger(const char* text) {
    if(!text) throw ERROR_BAD_ARG;
    std::string_view value(text);
    while(!value.empty() && (value.front()==' ' || value.front()=='\t')) value.remove_prefix(1);
    while(!value.empty() && (value.back()==' ' || value.back()=='\t')) value.remove_suffix(1);
    if(!value.empty() && value.front()=='+') {
        value.remove_prefix(1);
        if(!value.empty() && value.front()=='-') throw ERROR_BAD_ARG;
    }
    if(value.empty()) throw ERROR_BAD_ARG;
    Int result=0;
    const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) throw ERROR_BAD_ARG;
    return result;
}
struct ModPublicationGuard {
    GlobalData* owner;
    AsciiString big,directory;
    bool admitted=false;
    explicit ModPublicationGuard(GlobalData* value):owner(value),
        big(value?value->m_modBIG:AsciiString{}),directory(value?value->m_modDir:AsciiString{}) {}
    ~ModPublicationGuard() {
        if(owner && !admitted) {owner->m_modBIG.swap(big);owner->m_modDir.swap(directory);}
    }
};
}

// Retval is number of cmd-line args eaten
typedef Int (*FuncPtr)( char *args[], int num );

static const UnsignedByte F_NOCASE = 1; // Case-insensitive

struct CommandLineParam
{
	const char *name;
	FuncPtr func;
};

static void ConvertShortMapPathToLongMapPath(AsciiString &mapName)
{
	AsciiString path = mapName;
	AsciiString token;
	AsciiString actualpath;

	if ((path.find('\\') == NULL) && (path.find('/') == NULL))
	{
		DEBUG_CRASH(("Invalid map name"));
		return;
	}
	path.nextToken(&token, "\\/");
	while (!token.endsWithNoCase(".map") && (token.getLength() > 0))
	{
		actualpath.concat(token);
		actualpath.concat('\\');
		path.nextToken(&token, "\\/");
	}

	if (!token.endsWithNoCase(".map"))
	{
		DEBUG_CRASH(("Invalid map name"));
	}
	// remove the .map from the end.
	token.removeLastChar();
	token.removeLastChar();
	token.removeLastChar();
	token.removeLastChar();

	actualpath.concat(token);
	actualpath.concat('\\');
	actualpath.concat(token);
	actualpath.concat(".map");

	mapName = actualpath;
}

//=============================================================================
//=============================================================================
Int parseNoLogOrCrash(char *args[], int)
{
#ifdef ALLOW_DEBUG_UTILS
	DEBUG_CRASH(("-NoLogOrCrash not supported in this build\n"));
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseWin(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_windowed = true;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoMusic(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_musicOn = false;
	}
	return 1;
}


//=============================================================================
//=============================================================================
Int parseNoVideo(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_videoOn = false;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseFPUPreserve(char *args[], int argc)
{
    // Native simulation owns its floating-point environment; no DX backend
    // is allowed to change it. Preserve is the only meaningful native request.
    if(argc<2 || commandInteger(args[1])!=1) throw ERROR_BAD_ARG;
    return 2;
}

#if defined(_DEBUG) || defined(_INTERNAL)
//=============================================================================
//=============================================================================
Int parseUseCSF(char *args[], int)
{
	g_useStringFile = FALSE;
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoInputDisable(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_disableScriptedInputDisabling = true;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoFade(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_disableCameraFade = true;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoMilCap(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_disableMilitaryCaption = true;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseDebugCRCFromFrame(char *args[], int argc)
{
#ifdef DEBUG_CRC
	if (argc > 1)
	{
		TheCRCFirstFrameToLog = commandInteger(args[1]);
	}
#endif
	return 2;
}

//=============================================================================
//=============================================================================
Int parseDebugCRCUntilFrame(char *args[], int argc)
{
#ifdef DEBUG_CRC
	if (argc > 1)
	{
		TheCRCLastFrameToLog = commandInteger(args[1]);
	}
#endif
	return 2;
}

//=============================================================================
//=============================================================================
Int parseKeepCRCSave(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_keepCRCSaves = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseCRCLogicModuleData(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_crcModuleDataFromLogic = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseCRCClientModuleData(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_crcModuleDataFromClient = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseClientDeepCRC(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_clientDeepCRC = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseVerifyClientCRC(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_verifyClientCRC = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseLogObjectCRCs(char *args[], int argc)
{
#ifdef DEBUG_CRC
	g_logObjectCRCs = TRUE;
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNetCRCInterval(char *args[], int argc)
{
#ifdef DEBUG_CRC
	if (argc > 1)
	{
		NET_CRC_INTERVAL = commandInteger(args[1]);
	}
#endif
	return 2;
}

//=============================================================================
//=============================================================================
Int parseReplayCRCInterval(char *args[], int argc)
{
#ifdef DEBUG_CRC
	if (argc > 1)
	{
		REPLAY_CRC_INTERVAL = commandInteger(args[1]);
	}
#endif
	return 2;
}

//=============================================================================
//=============================================================================
Int parseNoDraw(char *args[], int argc)
{
#ifdef DEBUG_CRC
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_noDraw = TRUE;
	}
#endif
	return 1;
}

//=============================================================================
//=============================================================================
Int parseLogToConsole(char *args[], int)
{
	DebugSetFlags(DebugGetFlags() | DEBUG_FLAG_LOG_TO_CONSOLE);
	return 1;
}

#endif // _DEBUG || _INTERNAL

//=============================================================================
//=============================================================================
Int parseNoAudio(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_audioOn = false;
		TheWritableGlobalData->m_speechOn = false;
		TheWritableGlobalData->m_soundsOn = false;
		TheWritableGlobalData->m_musicOn = false;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoWin(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_windowed = false;
	}
	return 1;
}

Int parseFullVersion(char *args[], int num)
{
	if (TheVersion && num > 1)
	{
		TheVersion->setShowFullVersion(commandInteger(args[1]) != 0);
	}
	return num>1?2:1;
}

Int parseNoShadows(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_useShadowVolumes = false;
		TheWritableGlobalData->m_useShadowDecals = false;
	}
	return 1;
}

Int parseMapName(char *args[], int num)
{
	if (TheWritableGlobalData && num == 2)
	{
		TheWritableGlobalData->m_mapName.set( args[ 1 ] );
		ConvertShortMapPathToLongMapPath(TheWritableGlobalData->m_mapName);
	}
	return 1;
}

Int parseXRes(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_xResolution = commandInteger(args[1]);
		return 2;
	}
	return 1;
}

Int parseYRes(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_yResolution = commandInteger(args[1]);
		return 2;
	}
	return 1;
}

#if defined(_DEBUG) || defined(_INTERNAL)
//=============================================================================
//=============================================================================
Int parseLatencyAverage(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_latencyAverage = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
//=============================================================================
Int parseLatencyAmplitude(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_latencyAmplitude = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
//=============================================================================
Int parseLatencyPeriod(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_latencyPeriod = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
//=============================================================================
Int parseLatencyNoise(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_latencyNoise = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
//=============================================================================
Int parsePacketLoss(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_packetLoss = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
//=============================================================================
Int parseLowDetail(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_terrainLOD = TERRAIN_LOD_MIN;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoDynamicLOD(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_enableDynamicLOD = FALSE;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseNoStaticLOD(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_enableStaticLOD = FALSE;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseUseWaveEditor(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_usingWaterTrackEditor = TRUE;
	}
	return 1;
}

//=============================================================================
//=============================================================================
Int parseFPSLimit(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_framesPerSecondLimit = commandInteger(args[1]);
	}
	return 2;
}

//=============================================================================
Int parseNoViewLimit(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_useCameraConstraints = FALSE;
	}
	return 1;
}

Int parseWireframe(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_wireframe = TRUE;
	}
	return 1;
}

Int parseShowCollision(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_showCollisionExtents = TRUE;
	}
	return 1;
}

Int parseNoShowClientPhysics(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_showClientPhysics = FALSE;
	}
	return 1;
}

Int parseShowTerrainNormals(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_showTerrainNormals = TRUE;
	}
	return 1;
}

Int parseStateMachineDebug(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_stateMachineDebug = TRUE;
	}
	return 1;
}

Int parseJabber(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_jabberOn = TRUE;
	}
	return 1;
}

Int parseMunkee(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_munkeeOn = TRUE;
	}
	return 1;
}
#endif // defined(_DEBUG) || defined(_INTERNAL)

Int parseScriptDebug(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_scriptDebug = TRUE;
		TheWritableGlobalData->m_winCursors = TRUE;
	}
	return 1;
}

Int parseParticleEdit(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_particleEdit = TRUE;
		TheWritableGlobalData->m_winCursors = TRUE;
		TheWritableGlobalData->m_windowed = TRUE;
	}
	return 1;
}


Int parseBuildMapCache(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_buildMapCache = true;
	}
	return 1;
}


#if defined(_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
Int parsePreload( char *args[], int num )
{
	if( TheWritableGlobalData )
		TheWritableGlobalData->m_preloadAssets = TRUE;
	return 1;
}
#endif


#if defined(_DEBUG) || defined(_INTERNAL) 
Int parseDisplayDebug(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_displayDebug = TRUE;
	}
	return 1;
}

Int parseFile(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_initialFile = args[1];
		ConvertShortMapPathToLongMapPath(TheWritableGlobalData->m_initialFile);
	}
	return 2;
}


Int parsePreloadEverything( char *args[], int num )
{
	if( TheWritableGlobalData )
	{
		TheWritableGlobalData->m_preloadAssets = TRUE;
		TheWritableGlobalData->m_preloadEverything = TRUE;
	}
	return 1;
}

Int parseLogAssets(char *[], int)
{
    // This authoring report has downstream CWD writers, not protected owners.
    // Reject until that whole report path is bound to native user storage.
    throw ERROR_BAD_ARG;
}

/// begin stuff for VTUNE
Int parseVTune ( char *args[], int num )
{
	if( TheWritableGlobalData )
		TheWritableGlobalData->m_vTune = TRUE;
	return 1;
}
/// end stuff for VTUNE

#endif // defined(_DEBUG) || defined(_INTERNAL)

//=============================================================================
//=============================================================================

Int parseNoFX(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_useFX = FALSE;
	}
	return 1;
}

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseNoShroud(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_shroudOn = FALSE;
	}
	return 1;
}
#endif

Int parseForceBenchmark(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_forceBenchmark = TRUE;
	}
	return 1;
}

Int parseNoMoveCamera(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_disableCameraMovement = true;
	}
	return 1;
}

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseNoCinematic(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_disableCameraMovement = true;
		TheWritableGlobalData->m_disableMilitaryCaption = true;
		TheWritableGlobalData->m_disableCameraFade = true;
		TheWritableGlobalData->m_disableScriptedInputDisabling = true;
	}
	return 1;
}
#endif

Int parseSync(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheDebugIgnoreSyncErrors = true;
	}
	return 1;
}

Int parseNoShellMap(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_shellMapOn = FALSE;
	}
	return 1;
}

Int parseNoShaders(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_chipSetType = 1;	//force to a voodoo card which uses least amount of features.
	}
	return 1;
}

#if (defined(_DEBUG) || defined(_INTERNAL))
Int parseNoLogo(char *args[], int)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_playIntro = FALSE;
		TheWritableGlobalData->m_afterIntro = TRUE;
		TheWritableGlobalData->m_playSizzle = FALSE;
	}
	return 1;
}
#endif

Int parseNoSizzle( char *args[], int )
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_playSizzle = FALSE;
	}
	return 1;
}

Int parseShellMap(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_shellMapName = args[1];
	}
	return 2;
}

Int parseNoWindowAnimation(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_animateWindows = FALSE;
	}
	return 1;
}

Int parseWinCursors(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_winCursors = TRUE;
	}
	return 1;
}

Int parseQuickStart( char *args[], int num )
{
#if (defined(_DEBUG) || defined(_INTERNAL))
  parseNoLogo( args, num );
#else
	//Kris: Patch 1.01 -- Allow release builds to skip the sizzle video, but still force the EA logo to show up.
	//This is for legal reasons.
	parseNoSizzle( args, num );
#endif
	parseNoShellMap( args, num );
	parseNoWindowAnimation( args, num );
	return 1;
}

Int parseConstantDebug( char *args[], int num )
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_constantDebugUpdate = TRUE;
	}
	return 1;
}

#if (defined(_DEBUG) || defined(_INTERNAL))
Int parseExtraLogging( char *args[], int num )
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_extraLogging = TRUE;
	}
	return 1;
}
#endif

//-allAdvice feature
/*
Int parseAllAdvice( char *args[], int num )
{
	if( TheWritableGlobalData )
	{
		TheWritableGlobalData->m_allAdvice = TRUE;
	}
	return 1;
}
*/

Int parseShowTeamDot( char *args[], int num )
{
	if( TheWritableGlobalData )
	{
		TheWritableGlobalData->m_showTeamDot = TRUE;
	}
	return 1;
}


#if defined(_DEBUG) || defined(_INTERNAL)
Int parseSelectAll( char *args[], int num )
{
	if( TheWritableGlobalData )
	{
		TheWritableGlobalData->m_allowUnselectableSelection = TRUE;
	}
	return 1;
}

Int parseRunAhead( char *args[], Int num )
{
	if (num > 2)
	{
		MIN_RUNAHEAD = commandInteger(args[1]);
		MAX_FRAMES_AHEAD = commandInteger(args[2]);
		FRAME_DATA_LENGTH = (MAX_FRAMES_AHEAD + 1)*2;
	}
	return 3;
}
#endif


Int parseSeed(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_fixedSeed = commandInteger(args[1]);
	}
	return 2;
}

Int parseIncrAGPBuf(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_incrementalAGPBuf = TRUE;
	}
	return 1;
}

Int parseNetMinPlayers(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_netMinPlayers = commandInteger(args[1]);
	}
	return 2;
}

Int parsePlayStats(char *args[], int num)
{
	if (TheWritableGlobalData  && num > 1)
	{
		TheWritableGlobalData->m_playStats  = commandInteger(args[1]);
	}
	return 2;
}

Int parseDemoLoadScreen(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_loadScreenDemo = TRUE;
	}
	return 1;
}

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseSaveStats(char *args[], int num)
{
	if (TheWritableGlobalData  && num > 1)
	{
		TheWritableGlobalData->m_saveStats = TRUE;
		TheWritableGlobalData->m_baseStatsDir = args[1];
	}
	return 2;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseSaveAllStats(char *args[], int num)
{
	if (TheWritableGlobalData  && num > 1)
	{
		TheWritableGlobalData->m_saveStats = TRUE;
		TheWritableGlobalData->m_baseStatsDir = args[1];
		TheWritableGlobalData->m_saveAllStats = TRUE;
	}
	return 2;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseLocalMOTD(char *args[], int num)
{
	if (TheWritableGlobalData  && num > 1)
	{
		TheWritableGlobalData->m_useLocalMOTD = TRUE;
		TheWritableGlobalData->m_MOTDPath = args[1];
	}
	return 2;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseCameraDebug(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_debugCamera = TRUE;
	}
	return 1;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseBenchmark(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_benchmarkTimer = commandInteger(args[1]);
		TheWritableGlobalData->m_playStats  = commandInteger(args[1]);
	}
	return 2;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
#ifdef DUMP_PERF_STATS
Int parseStats(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		TheWritableGlobalData->m_dumpStatsAtInterval = TRUE;
		TheWritableGlobalData->m_statsInterval  = commandInteger(args[1]);
	}
	return 2;
}
#endif
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseIgnoreAsserts(char *args[], int num)
{
	if (TheWritableGlobalData && num > 0)
	{
		TheWritableGlobalData->m_debugIgnoreAsserts = true;
	}
	return 1;
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
Int parseIgnoreStackTrace(char *args[], int num)
{
	if (TheWritableGlobalData && num > 0)
	{
		TheWritableGlobalData->m_debugIgnoreStackTrace = true;
	}
	return 1;
}
#endif

Int parseNoFPSLimit(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_useFpsLimit = false;
		TheWritableGlobalData->m_framesPerSecondLimit = 30000;
	}
	return 1;
}

Int parseDumpAssetUsage(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_dumpAssetUsage = true;
	}
	return 1;
}

Int parseJumpToFrame(char *args[], int num)
{
	if (TheWritableGlobalData && num > 1)
	{
		parseNoFPSLimit(args, num);
		TheWritableGlobalData->m_noDraw = commandInteger(args[1]);
		return 2;
	}
	return 1;
}

Int parseUpdateImages(char *args[], int num)
{
	if (TheWritableGlobalData)
	{
		TheWritableGlobalData->m_shouldUpdateTGAToDDS = TRUE;
	}
	return 1;
}

Int parseMod(char *args[], Int num)
{
    if(!TheWritableGlobalData || num<2 || !args || !args[1]) throw ERROR_BAD_ARG;
    std::string selected=args[1];
    if(selected.empty() || selected.find(':')!=std::string::npos) throw ERROR_BAD_ARG;
    for(char& byte:selected) if(byte=='\\') byte='/';
    if(selected.front()!='/') {
        selected=std::string(TheGlobalData->getPath_UserData().str())+selected;
        if(!TheNativeUserStorage || !TheNativeUserStorage->relativeDataPath(selected)) throw ERROR_BAD_ARG;
    }
    struct stat info{};
    if(::stat(selected.c_str(),&info)!=0) throw ERROR_BAD_ARG;
    if(S_ISDIR(info.st_mode)) TheWritableGlobalData->m_modDir=selected.c_str();
    else if(S_ISREG(info.st_mode)) TheWritableGlobalData->m_modBIG=selected.c_str();
    else throw ERROR_BAD_ARG;
    return 2;
}

static CommandLineParam params[] =
{
	{ "-noshellmap", parseNoShellMap },
	{ "-win", parseWin },
	{ "-xres", parseXRes },
	{ "-yres", parseYRes },
	{ "-fullscreen", parseNoWin },
	{ "-fullVersion", parseFullVersion },
	{	"-particleEdit", parseParticleEdit },
	{ "-scriptDebug", parseScriptDebug },
	{ "-playStats", parsePlayStats },
	{ "-mod", parseMod },
	{ "-noshaders", parseNoShaders },
	{ "-quickstart", parseQuickStart },

#if (defined(_DEBUG) || defined(_INTERNAL))
	{ "-noaudio", parseNoAudio },
	{ "-map", parseMapName },
	{ "-nomusic", parseNoMusic },
	{ "-novideo", parseNoVideo },
	{ "-noLogOrCrash", parseNoLogOrCrash },
	{ "-FPUPreserve", parseFPUPreserve },
	{ "-benchmark", parseBenchmark },
#ifdef DUMP_PERF_STATS
	{ "-stats", parseStats }, 
#endif
  { "-saveStats", parseSaveStats },
	{ "-localMOTD", parseLocalMOTD },
	{ "-UseCSF", parseUseCSF },
	{ "-NoInputDisable", parseNoInputDisable },
	{ "-DebugCRCFromFrame", parseDebugCRCFromFrame },
	{ "-DebugCRCUntilFrame", parseDebugCRCUntilFrame },
	{ "-KeepCRCSaves", parseKeepCRCSave },
	{ "-CRCLogicModuleData", parseCRCLogicModuleData },
	{ "-CRCClientModuleData", parseCRCClientModuleData },
	{ "-ClientDeepCRC", parseClientDeepCRC },
	{ "-VerifyClientCRC", parseVerifyClientCRC },
	{ "-LogObjectCRCs", parseLogObjectCRCs },
	{ "-saveAllStats", parseSaveAllStats },
	{ "-NetCRCInterval", parseNetCRCInterval },
	{ "-ReplayCRCInterval", parseReplayCRCInterval },
	{ "-noDraw", parseNoDraw },
	{ "-nomilcap", parseNoMilCap },
	{ "-nofade", parseNoFade },
	{ "-nomovecamera", parseNoMoveCamera },
	{ "-nocinematic", parseNoCinematic },
	{ "-packetloss", parsePacketLoss },
	{ "-latAvg", parseLatencyAverage },
	{ "-latAmp", parseLatencyAmplitude },
	{ "-latPeriod", parseLatencyPeriod },
	{ "-latNoise", parseLatencyNoise },
	{ "-noViewLimit", parseNoViewLimit },
	{ "-lowDetail", parseLowDetail },
	{ "-noDynamicLOD", parseNoDynamicLOD },
	{ "-noStaticLOD", parseNoStaticLOD },
	{ "-useWaveEditor", parseUseWaveEditor },
	{ "-fps", parseFPSLimit },
	{ "-wireframe", parseWireframe },
	{ "-showCollision", parseShowCollision },
	{ "-noShowClientPhysics", parseNoShowClientPhysics },
	{ "-showTerrainNormals", parseShowTerrainNormals },
	{ "-stateMachineDebug", parseStateMachineDebug },
	{ "-jabber", parseJabber },
	{ "-munkee", parseMunkee },
	{ "-displayDebug", parseDisplayDebug },
	{ "-file", parseFile },
  
//	{ "-preload", parsePreload },
	
  { "-preloadEverything", parsePreloadEverything },
	{ "-logAssets", parseLogAssets },
	{ "-netMinPlayers", parseNetMinPlayers },
	{ "-DemoLoadScreen", parseDemoLoadScreen },
	{ "-cameraDebug", parseCameraDebug },
	{ "-ignoreAsserts", parseIgnoreAsserts },
	{ "-ignoreStackTrace", parseIgnoreStackTrace },
	{ "-logToCon", parseLogToConsole },
	{ "-vTune", parseVTune },
	{ "-selectTheUnselectable", parseSelectAll },
	{ "-RunAhead", parseRunAhead },
	{ "-noshroud", parseNoShroud },
	{ "-forceBenchmark", parseForceBenchmark },
	{ "-buildmapcache", parseBuildMapCache },
	{ "-noshadowvolumes", parseNoShadows },
	{ "-nofx", parseNoFX },
	{ "-ignoresync", parseSync },
	{ "-nologo", parseNoLogo },
	{ "-shellmap", parseShellMap },
	{ "-noShellAnim", parseNoWindowAnimation },
	{ "-winCursors", parseWinCursors },
	{ "-constantDebug", parseConstantDebug },
	{ "-seed", parseSeed },
	{ "-noagpfix", parseIncrAGPBuf },
	{ "-noFPSLimit", parseNoFPSLimit },
	{ "-dumpAssetUsage", parseDumpAssetUsage },
	{ "-jumpToFrame", parseJumpToFrame },
	{ "-updateImages", parseUpdateImages },
	{ "-showTeamDot", parseShowTeamDot },
	{ "-extraLogging", parseExtraLogging },

#endif

	//-allAdvice feature
	//{ "-allAdvice", parseAllAdvice },

#if defined(_DEBUG) || defined(_INTERNAL) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
  { "-preload", parsePreload },
#endif


};

// parseCommandLine ===========================================================
/** Parse command-line parameters. */
//=============================================================================
void parseCommandLine(int argc, char *argv[])
{
	// To parse command-line parameters, we loop through a table holding arguments
	// and functions to handle them.  Comparisons can be case-(in)sensitive, and
	// can check the entire string (for testing the presence of a flag) or check
	// just the start (for a key=val argument).  The handling function can also
	// look at the next argument(s), to accomodate multi-arg parameters, e.g. "-p 1234".
	int arg=1, param;
	Bool found;

    if(argc<0 || (argc && !argv)) throw ERROR_BAD_ARG;
    for(int index=0;index<argc;++index) if(!argv[index]) throw ERROR_BAD_ARG;
    // Validate active value options before dispatch changes any configuration.
    for(int index=1;index<argc;++index) {
        const bool integer=!strcasecmp(argv[index],"-xres") || !strcasecmp(argv[index],"-yres") ||
            !strcasecmp(argv[index],"-fullVersion") || !strcasecmp(argv[index],"-playStats");
        const bool mod=!strcasecmp(argv[index],"-mod");
        if(integer || mod) {
            if(index+1>=argc) throw ERROR_BAD_ARG;
            if(integer) (void)commandInteger(argv[index+1]);
            ++index;
        }
    }
    ModPublicationGuard modPublication(TheWritableGlobalData);


	while (arg<argc)
	{
		// Look at arg #i
		found = false;
		for (param=0; !found && param<sizeof(params)/sizeof(params[0]); ++param)
		{
			int len = strlen(params[param].name);
			int len2 = strlen(argv[arg]);
			if (len2 != len)
				continue;
			if (!strncasecmp(argv[arg], params[param].name, len))
			{
				arg += params[param].func(argv+arg, argc-arg);
				found = true;
			}
		}	// for
		if (!found)
		{
			arg++;
		}
	}

	if(TheWritableGlobalData && (TheGlobalData->m_modBIG.isNotEmpty() || TheGlobalData->m_modDir.isNotEmpty())) {
        if(!TheFileSystem) throw ERROR_BAD_ARG;
        TheFileSystem->mountReadOnlyMods(TheGlobalData->m_modBIG.str(),TheGlobalData->m_modDir.str());
    }
    modPublication.admitted=true;
}
