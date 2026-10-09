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

// FILE: GameLOD.cpp ///////////////////////////////////////////////////////////
//
// Used to set detail levels of various game systems.
//
// Author: Mark Wilczynski, Sept 2002
//
//
///////////////////////////////////////////////////////////////////////////////

#include <strings.h>
#include "Common/GlobalData.h"
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameLOD.h"
#include "Common/FileSystem.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/GameClient.h"
#include "Common/UserPreferences.h"
#include "Common/NativeUserStorage.h"
#include <cmath>
#include <limits>
#include <type_traits>
#include <cstdio>

#define DEFINE_PARTICLE_SYSTEM_NAMES
#include "GameClient/ParticleSys.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#define PROFILE_ERROR_LIMIT	0.94f	//fraction of profiled result needed to get a match.  Allows some room for error/fluctuation.

GameLODManager *TheGameLODManager=NULL;

static const FieldParse TheStaticGameLODFieldParseTable[] = 
{
	{ "MinimumFPS",						INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_minFPS)},
	{ "MinimumProcessorFps",			INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_minProcessorFPS)},
	{ "SampleCount2D",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_sampleCount2D ) },
	{ "SampleCount3D",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_sampleCount3D ) },
	{ "StreamCount",					INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_streamCount ) },
	{ "MaxParticleCount",				INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxParticleCount ) },
	{ "UseShadowVolumes",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useShadowVolumes ) },
	{ "UseShadowDecals",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useShadowDecals ) },
	{ "UseCloudMap",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useCloudMap ) },
	{ "UseLightMap",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useLightMap ) },
	{ "ShowSoftWaterEdge",				INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_showSoftWaterEdge ) },
	{ "MaxTankTrackEdges",				INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackEdges) },
	{ "MaxTankTrackOpaqueEdges",		INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackOpaqueEdges) },
	{ "MaxTankTrackFadeDelay",			INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_maxTankTrackFadeDelay) },
	{ "UseBuildupScaffolds",			INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useBuildupScaffolds ) },
	{ "UseTreeSway",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useTreeSway ) },
	{ "UseEmissiveNightMaterials",		INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useEmissiveNightMaterials ) },
	{ "UseHeatEffects",					INI::parseBool,					NULL,	offsetof( StaticGameLODInfo, m_useHeatEffects ) },
	{ "TextureReductionFactor",		INI::parseInt,					NULL,	offsetof( StaticGameLODInfo, m_textureReduction ) },
};

static const char *StaticGameLODNames[]=
{
	"Low",
	"Medium",
	"High",
	"Custom"
};

StaticGameLODInfo::StaticGameLODInfo(void)
{
	m_minFPS=0;
	m_minProcessorFPS=0;
	m_sampleCount2D=6;
	m_sampleCount3D=24;
	m_streamCount=2;
	m_maxParticleCount=2500;

	m_useShadowVolumes=TRUE;
	m_useShadowDecals=TRUE;
	m_useCloudMap=TRUE;
	m_useLightMap=TRUE;
	m_showSoftWaterEdge=TRUE;
	m_maxTankTrackEdges=100;
	m_maxTankTrackOpaqueEdges=25;
	m_maxTankTrackFadeDelay=300000;
	m_useBuildupScaffolds=TRUE;
	m_useTreeSway=TRUE;
	m_useEmissiveNightMaterials=TRUE;
	m_useHeatEffects=TRUE;
	m_textureReduction = 0;	//none
	m_useFpsLimit = TRUE;
	m_enableDynamicLOD = TRUE;
	m_useTrees = TRUE;
}

static const FieldParse TheDynamicGameLODFieldParseTable[] = 
{
	{ "MinimumFPS",						INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_minFPS)},
	{ "ParticleSkipMask",				INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_dynamicParticleSkipMask)},
	{ "DebrisSkipMask",					INI::parseInt,					NULL,	offsetof( DynamicGameLODInfo, m_dynamicDebrisSkipMask)},
	{ "SlowDeathScale",					INI::parseReal,					NULL,	offsetof( DynamicGameLODInfo, m_slowDeathScale)},
	{ "MinParticlePriority",			INI::parseIndexList, ParticlePriorityNames,	offsetof( DynamicGameLODInfo, m_minDynamicParticlePriority)},
	{ "MinParticleSkipPriority",		INI::parseIndexList, ParticlePriorityNames,	offsetof( DynamicGameLODInfo, m_minDynamicParticleSkipPriority)},
};

static const char *DynamicGameLODNames[]=
{
	"Low",
	"Medium",
	"High",
	"VeryHigh"
};

DynamicGameLODInfo::DynamicGameLODInfo(void)
{
	m_minFPS=0;
	m_dynamicParticleSkipMask=0;
	m_dynamicDebrisSkipMask=0;
	m_slowDeathScale=1.0f;
	m_minDynamicParticlePriority = PARTICLE_PRIORITY_LOWEST;
	m_minDynamicParticleSkipPriority = PARTICLE_PRIORITY_LOWEST;
};

//Keep this in sync with enum in GameLOD.h
static char *CPUNames[] = 
{
	"XX","P3", "P4","K7", NULL
};

//Keep this in sync with enum in GameLOD.h
static char *VideoNames[] = 
{
	"XX","V2","V3","V4","V5","TNT","TNT2","GF2","R100","PS11","GF3","GF4","PS14","R200","PS20","R300", NULL
};

void parseReallyLowMHz(INI* ini)
{
	Int mhz;
	INI::parseInt(ini,NULL,&mhz,NULL);
	if (TheGameLODManager)
	{
		TheGameLODManager->setReallyLowMHz(mhz);
	}
}

void INI::parseBenchProfile( INI* ini)
{
	if( TheGameLODManager )
	{
			BenchProfile *preset = TheGameLODManager->newBenchProfile();

			if (preset)
			{
				INI::parseIndexList(ini,NULL,&preset->m_cpuType,CPUNames);
				INI::parseInt(ini,NULL,&preset->m_mhz,NULL);
				INI::parseReal(ini,NULL,&preset->m_intBenchIndex,NULL);
				INI::parseReal(ini,NULL,&preset->m_floatBenchIndex,NULL);
				INI::parseReal(ini,NULL,&preset->m_memBenchIndex,NULL);
			}
	}
}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseLODPreset( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );	//name of detail level - low, medium, high

	if( TheGameLODManager )
	{
		StaticGameLODLevel index = (StaticGameLODLevel)TheGameLODManager->getStaticGameLODIndex(name);
		if (index == STATIC_GAME_LOD_UNKNOWN) throw INI_INVALID_DATA;
		if (index != STATIC_GAME_LOD_UNKNOWN)
		{
			LODPresetInfo *preset = TheGameLODManager->newLODPreset(index);

			if (preset)
			{
				INI::parseIndexList(ini,NULL,&preset->m_cpuType,CPUNames);
				INI::parseInt(ini,NULL,&preset->m_mhz,NULL);
				INI::parseIndexList(ini,NULL,&preset->m_videoType,VideoNames);
				INI::parseInt(ini,NULL,&preset->m_memory,NULL);
			}
		}
	}
}

GameLODManager::GameLODManager():GameLODManager(nativeLODProbe(),TheNativeUserStorage){}
GameLODManager::GameLODManager(NativeLODProbe& probe,NativeUserStorage* storage)
{
  m_probe=&probe;m_storage=storage;
	m_currentStaticLOD = STATIC_GAME_LOD_UNKNOWN;
	m_currentDynamicLOD = DYNAMIC_GAME_LOD_HIGH;
	m_numParticleGenerations=0;
	m_dynamicParticleSkipMask=0;
	m_numDebrisGenerations=0;
	m_dynamicDebrisSkipMask=0;
	m_videoPassed=false;
	m_cpuPassed=false;
	m_memPassed=false;
	m_slowDeathScale=1.0f;
  m_minDynamicParticlePriority=PARTICLE_PRIORITY_LOWEST;
  m_minDynamicParticleSkipPriority=PARTICLE_PRIORITY_LOWEST;
	m_idealDetailLevel = STATIC_GAME_LOD_UNKNOWN;
	m_videoChipType = DC_MAX;
	m_cpuType = XX;
	m_numRAM=0;
	m_cpuFreq=0;
	m_intBenchIndex=0;
	m_floatBenchIndex=0;
	m_memBenchIndex=0;
	m_compositeBenchIndex=0;
	m_numBenchProfiles=0;
	m_currentTextureReduction=0;
	m_reallyLowMHz = 400;
	
	for (Int i=0; i<STATIC_GAME_LOD_CUSTOM; i++)
		m_numLevelPresets[i]=0;
};

GameLODManager::~GameLODManager()
{

}

BenchProfile *GameLODManager::newBenchProfile(void)
{
	if (m_numBenchProfiles < MAX_BENCH_PROFILES)
	{	
		m_numBenchProfiles++;
		return &m_benchProfiles[m_numBenchProfiles-1];
	}

  throw INI_INVALID_DATA;
}

LODPresetInfo *GameLODManager::newLODPreset(StaticGameLODLevel index)
{
  if(index<STATIC_GAME_LOD_LOW || index>=STATIC_GAME_LOD_CUSTOM)throw INI_INVALID_DATA;
	if (m_numLevelPresets[index] < MAX_LOD_PRESETS_PER_LEVEL)
	{	
		m_numLevelPresets[index]++;
		return &m_lodPresets[index][m_numLevelPresets[index]-1];
	}

  throw INI_INVALID_DATA;
}

namespace {
struct LODPublication {
  GameLODManager* previous=TheGameLODManager;
  explicit LODPublication(GameLODManager& candidate){TheGameLODManager=&candidate;}
  ~LODPublication(){TheGameLODManager=previous;}
};
NativeLODReportStatus writeLODReport(NativeUserStorage* storage,const NativeLODHardware& hardware,
    const NativeLODLegacyScores& scores) {
  if(!storage)return NativeLODReportStatus::Unavailable;
  char bytes[512];const int size=std::snprintf(bytes,sizeof(bytes),"BenchProfile = %s %d %f %f %f",
      CPUNames[hardware.cpu],hardware.frequencyMHz,scores.integer,scores.floating,scores.memory);
  if(size<0 || size>=sizeof(bytes))throw ERROR_BAD_ARG;
  try {
    auto output=storage->beginWrite(NativeUserArea::Data,"Benchmark.txt");output->write(bytes,size);output->commit();
    return NativeLODReportStatus::Published;
  }catch(const NativeStorageError&){return NativeLODReportStatus::Unavailable;}
}
}
void GameLODManager::loadNativePresetData(){
  const INIBlockDefinition blocks[]{
    {"StaticGameLOD",INI::parseStaticGameLODDefinition},
    {"DynamicGameLOD",INI::parseDynamicGameLODDefinition},
    {"LODPreset",INI::parseLODPreset},{"BenchProfile",INI::parseBenchProfile},
    {"ReallyLowMHz",parseReallyLowMHz}};
  INI ini;ini.loadBlocks("Data\\INI\\GameLOD.ini",INI_LOAD_OVERWRITE,blocks);
  ini.loadBlocks("Data\\INI\\GameLODPresets.ini",INI_LOAD_OVERWRITE,blocks);
  refreshCustomStaticLODLevel();validateNativePresetData();
}
void GameLODManager::validateNativePresetData() const {
  if(m_reallyLowMHz<0)throw INI_INVALID_DATA;
  for(Int level=0;level<STATIC_GAME_LOD_CUSTOM;++level){
    for(Int index=0;index<m_numLevelPresets[level];++index){const auto& preset=m_lodPresets[level][index];
      if(preset.m_mhz<=0 || preset.m_memory<=0 || preset.m_videoType==DC_MAX)throw INI_INVALID_DATA;
    }
  }
  for(Int index=0;index<m_numBenchProfiles;++index){const auto& profile=m_benchProfiles[index];
    if(profile.m_mhz<=0 || !std::isfinite(profile.m_intBenchIndex) || profile.m_intBenchIndex<=0 ||
       !std::isfinite(profile.m_floatBenchIndex) || profile.m_floatBenchIndex<=0 ||
       !std::isfinite(profile.m_memBenchIndex) || profile.m_memBenchIndex<=0)throw INI_INVALID_DATA;
  }
  for(const auto& profile:m_dynamicGameLODInfo){
    if(!std::isfinite(profile.m_slowDeathScale) || profile.m_slowDeathScale<0 ||
       profile.m_minDynamicParticlePriority<PARTICLE_PRIORITY_LOWEST || profile.m_minDynamicParticlePriority>PARTICLE_PRIORITY_HIGHEST ||
       profile.m_minDynamicParticleSkipPriority<PARTICLE_PRIORITY_LOWEST || profile.m_minDynamicParticleSkipPriority>PARTICLE_PRIORITY_HIGHEST)throw INI_INVALID_DATA;
  }
}
void GameLODManager::calibrateNativeCPU(const NativeLODLegacyScores& scores){
  if(!std::isfinite(scores.integer) || scores.integer<=0 || !std::isfinite(scores.floating) || scores.floating<=0 ||
     !std::isfinite(scores.memory) || scores.memory<=0 || !std::isfinite(scores.integer+scores.floating))throw ERROR_BAD_ARG;
  m_intBenchIndex=scores.integer;m_floatBenchIndex=scores.floating;m_memBenchIndex=scores.memory;
  m_compositeBenchIndex=m_intBenchIndex+m_floatBenchIndex;m_calibrated=TRUE;
			StaticGameLODLevel currentLevel=STATIC_GAME_LOD_LOW;
			BenchProfile *prof=m_benchProfiles;
			m_cpuType = P3;	//assume lowest spec.
			m_cpuFreq = 1000;	//assume lowest spec.
			for (Int k=0; k<m_numBenchProfiles; k++)
			{
				//Check if we're within 5% of the performance of this cpu profile.
				if (m_intBenchIndex/prof->m_intBenchIndex >= PROFILE_ERROR_LIMIT && m_floatBenchIndex/prof->m_floatBenchIndex >= PROFILE_ERROR_LIMIT && m_memBenchIndex/prof->m_memBenchIndex >= PROFILE_ERROR_LIMIT)
				{	
					for (Int i=STATIC_GAME_LOD_HIGH; i >= STATIC_GAME_LOD_LOW; i--)
					{
						LODPresetInfo *preset=&m_lodPresets[i][0];	//pointer to first preset at this LOD level.
						for (Int j=0; j<m_numLevelPresets[i]; j++)
						{
							if(	prof->m_cpuType == preset->m_cpuType &&	((Real)prof->m_mhz/(Real)preset->m_mhz >= PROFILE_ERROR_LIMIT))
							{	currentLevel = (StaticGameLODLevel)i;
								m_cpuType = prof->m_cpuType;
								m_cpuFreq = prof->m_mhz;
								break;
							}
							preset++;	//skip to next preset
						}
						if (currentLevel >= i)
							break;	//we already found a higher level than the remaining presets so no need to keep searching.
					}
				}
				prof++;
			}
  // The source profile equivalence supplies its own measured MHz.
  m_frequencyKnown=TRUE;
}
void GameLODManager::init(void){
  if(!TheGlobalData || !TheFileSystem || TheGameLODManager!=this || !m_probe)throw ERROR_BAD_ARG;
  GameLODManager candidate(*m_probe,m_storage);
  StaticGameLODLevel userSetDetail;
  std::optional<NativeLODLegacyScores> scores;
  NativeLODHardware hardware{};
  struct CustomValues {Int texture,particles;Bool volumes,decals,markers,dynamic,limit,light,cloud,water,heat,drawLOD,trees;} custom{};
  {
    LODPublication publication(candidate);
    candidate.loadNativePresetData();
    OptionPreferences preferences(*TheGlobalData,m_storage);
    userSetDetail=static_cast<StaticGameLODLevel>(preferences.getStaticGameDetail());
    candidate.m_idealDetailLevel=static_cast<StaticGameLODLevel>(preferences.getIdealStaticGameDetail());
    hardware=m_probe->hardware();
    if(!hardware.ramBytes || hardware.frequencyMHz<0 || (hardware.frequencyKnown && hardware.frequencyMHz==0) ||
       (!hardware.frequencyKnown && hardware.frequencyMHz!=0))throw ERROR_BAD_ARG;
    candidate.m_cpuType=hardware.cpu;candidate.m_cpuFreq=hardware.frequencyMHz;
    candidate.m_frequencyKnown=hardware.frequencyKnown;candidate.m_numRAM=hardware.ramBytes;
    candidate.m_memPassed=(Real(candidate.m_numRAM)/Real(256*1024*1024)>=PROFILE_ERROR_LIMIT);
    if(candidate.m_idealDetailLevel==STATIC_GAME_LOD_UNKNOWN || TheGlobalData->m_forceBenchmark){
      if(candidate.m_cpuType==XX || TheGlobalData->m_forceBenchmark){
        scores=m_probe->legacyScores();
        if(scores)candidate.calibrateNativeCPU(*scores);
        else if(TheGlobalData->m_forceBenchmark)throw NativeLODCalibrationUnavailable();
      }
    }
    if(userSetDetail==STATIC_GAME_LOD_CUSTOM){
      custom={preferences.getTextureReduction(),preferences.getParticleCap(),preferences.get3DShadowsEnabled(),
        preferences.get2DShadowsEnabled(),preferences.getBuildingOcclusionEnabled(),preferences.getDynamicLODEnabled(),
        preferences.getFPSLimitEnabled(),preferences.getLightmapEnabled(),preferences.getCloudShadowsEnabled(),
        preferences.getSmoothWaterEnabled(),preferences.getUseHeatEffects(),preferences.getExtraAnimationsDisabled(),
        preferences.getTreesEnabled()};
    }
  }
  // Optional reporting is the last fallible acquisition before metadata commit.
  if(TheGlobalData->m_forceBenchmark && scores)candidate.m_reportStatus=writeLODReport(m_storage,hardware,*scores);
  static_assert(std::is_nothrow_copy_assignable_v<GameLODManager>);
  *this=candidate;
  if(userSetDetail==STATIC_GAME_LOD_CUSTOM){
    auto& data=*TheWritableGlobalData;data.m_textureReductionFactor=custom.texture;data.m_maxParticleCount=custom.particles;
    data.m_useShadowVolumes=custom.volumes;data.m_useShadowDecals=custom.decals;data.m_enableBehindBuildingMarkers=custom.markers;
    data.m_enableDynamicLOD=custom.dynamic;data.m_useFpsLimit=custom.limit;data.m_useLightMap=custom.light;
    data.m_useCloudMap=custom.cloud;data.m_showSoftWaterEdge=custom.water;data.m_useHeatEffects=custom.heat;
    data.m_useDrawModuleLOD=custom.drawLOD;data.m_useTreeSway=!custom.drawLOD;data.m_useTrees=custom.trees;
  }
  // Actual device callbacks retain their source behavior. Nonnull presentation
  // rollback is not established by candidate metadata or headless fixtures.
  setStaticLODLevel(userSetDetail);
}

void GameLODManager::refreshCustomStaticLODLevel(void)
{
	StaticGameLODInfo *lodInfo=&m_staticGameLODInfo[STATIC_GAME_LOD_CUSTOM];

	lodInfo->m_maxParticleCount=TheGlobalData->m_maxParticleCount;
	lodInfo->m_useShadowVolumes=TheGlobalData->m_useShadowVolumes;
	lodInfo->m_useShadowDecals=TheGlobalData->m_useShadowDecals;
	lodInfo->m_useCloudMap=TheGlobalData->m_useCloudMap;
	lodInfo->m_useLightMap=TheGlobalData->m_useLightMap;
	lodInfo->m_showSoftWaterEdge=TheGlobalData->m_showSoftWaterEdge;
	lodInfo->m_maxTankTrackEdges=TheGlobalData->m_maxTankTrackEdges;
	lodInfo->m_maxTankTrackOpaqueEdges=TheGlobalData->m_maxTankTrackOpaqueEdges;
	lodInfo->m_maxTankTrackFadeDelay=TheGlobalData->m_maxTankTrackFadeDelay;
	lodInfo->m_useBuildupScaffolds=!TheGlobalData->m_useDrawModuleLOD;
	lodInfo->m_useHeatEffects = TheGlobalData->m_useHeatEffects;
	lodInfo->m_useTreeSway=lodInfo->m_useBuildupScaffolds;// Borrow same setting. //TheGlobalData->m_useTreeSway;
	lodInfo->m_textureReduction=TheGlobalData->m_textureReductionFactor;
	lodInfo->m_useFpsLimit = TheGlobalData->m_useFpsLimit;
	lodInfo->m_enableDynamicLOD=TheGlobalData->m_enableDynamicLOD;
	lodInfo->m_useTrees = TheGlobalData->m_useTrees;

}

/**Convert LOD name to an index*/
Int GameLODManager::getStaticGameLODIndex(AsciiString name)
{
	for (Int i=0; i<STATIC_GAME_LOD_COUNT; ++i)
	{
		if (name.compareNoCase(StaticGameLODNames[i]) == 0)
			return i;
	}

	return STATIC_GAME_LOD_UNKNOWN;
}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseStaticGameLODDefinition( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );	

	if( TheGameLODManager )
	{
		Int index = TheGameLODManager->getStaticGameLODIndex(name);
    if(index==STATIC_GAME_LOD_UNKNOWN)throw INI_INVALID_DATA;
		if (index != STATIC_GAME_LOD_UNKNOWN)
		{
			StaticGameLODInfo *lodInfo = &(TheGameLODManager->m_staticGameLODInfo[index]);

			// parse the ini definition
			ini->initFromINI( lodInfo, TheStaticGameLODFieldParseTable );
		}
	}
}

/**Parse an LOD level*/
void INI::parseStaticGameLODLevel( INI* ini, void * , void *store, const void*)
{
	const char *tok=ini->getNextToken();
	for (Int i=0; i<STATIC_GAME_LOD_COUNT; i++)
		if( strcasecmp(tok, StaticGameLODNames[i]) == 0 )
		{	*(StaticGameLODLevel*)store = (StaticGameLODLevel)i;
			return;
		}

	DEBUG_CRASH(("invalid GameLODLevel token %s -- expected LOW/MEDIUM/HIGH\n",tok));
	throw INI_INVALID_DATA;
}

const char *GameLODManager::getStaticGameLODLevelName(StaticGameLODLevel level)
{
  if(level==STATIC_GAME_LOD_UNKNOWN)return "Unknown";
  if(level<STATIC_GAME_LOD_LOW || level>=STATIC_GAME_LOD_COUNT)throw ERROR_BAD_ARG;
	return StaticGameLODNames[level];
}

/**Function which calculates the recommended LOD level for current hardware
configuration.*/
NativeLODReportStatus GameLODManager::persistNativeRecommendation(StaticGameLODLevel level){
  if(!m_storage)return NativeLODReportStatus::Unavailable;
  OptionPreferences preferences(*TheGlobalData,m_storage);
  preferences["IdealStaticGameLOD"]=getStaticGameLODLevelName(level);
  if(getStaticLODLevel()==STATIC_GAME_LOD_UNKNOWN)preferences["StaticGameLOD"]=getStaticGameLODLevelName(level);
  try {preferences.write();return NativeLODReportStatus::Published;}
  catch(const NativeStorageError&){return NativeLODReportStatus::Unavailable;}
}
StaticGameLODLevel GameLODManager::findStaticLODLevel(void){
  if(!TheGlobalData || !m_probe)throw ERROR_BAD_ARG;
  if(m_idealDetailLevel!=STATIC_GAME_LOD_UNKNOWN){
    if(m_recommendationStatus==NativeLODReportStatus::Unavailable)
      m_recommendationStatus=persistNativeRecommendation(m_idealDetailLevel);
    return m_idealDetailLevel;
  }
  // Unknown modern hardware is not a justified legacy equivalence or low-quality
  // recommendation. Existing game/user settings remain unchanged and uncached.
  if(m_cpuType==XX || !m_frequencyKnown)return STATIC_GAME_LOD_UNKNOWN;
  const auto chip=m_probe->chipset();
  if(!chip || *chip==DC_UNKNOWN)return STATIC_GAME_LOD_UNKNOWN;
  if(*chip==DC_MAX)throw ERROR_BAD_ARG;
  const auto ramMB=m_numRAM/(1024*1024);
  StaticGameLODLevel selected=STATIC_GAME_LOD_LOW;
  for(Int level=STATIC_GAME_LOD_HIGH;level>=STATIC_GAME_LOD_LOW;--level){
    for(Int index=0;index<m_numLevelPresets[level];++index){const auto& preset=m_lodPresets[level][index];
      if(m_cpuType==preset.m_cpuType && Real(m_cpuFreq)/Real(preset.m_mhz)>=PROFILE_ERROR_LIMIT &&
         *chip>=preset.m_videoType && Real(ramMB)/Real(preset.m_memory)>=PROFILE_ERROR_LIMIT){selected=static_cast<StaticGameLODLevel>(level);break;}
    }
    if(selected>=level)break;
  }
  const auto persistence=persistNativeRecommendation(selected);
  m_videoChipType=*chip;m_idealDetailLevel=selected;m_recommendationStatus=persistence;
  return selected;
}

/**Set all game systems to match the desired LOD level.*/
Bool GameLODManager::setStaticLODLevel(StaticGameLODLevel level)
{
  if(level<STATIC_GAME_LOD_UNKNOWN || level>=STATIC_GAME_LOD_COUNT || !TheGlobalData)throw ERROR_BAD_ARG;
	if (!TheGlobalData->m_enableStaticLOD)
	{	m_currentStaticLOD = STATIC_GAME_LOD_CUSTOM; 
		return FALSE;
	}

	if (level == STATIC_GAME_LOD_UNKNOWN || (level != STATIC_GAME_LOD_CUSTOM && m_currentStaticLOD == level))
		return FALSE;	//level is already applied.  Custom levels are always applied since random options could change.

	applyStaticLODLevel(level);
	m_currentStaticLOD = level;

	return TRUE;
}

void GameLODManager::applyStaticLODLevel(StaticGameLODLevel level)
{
///@todo: Still need to implement these settings:
//	m_sampleCount2D=6;
//	m_sampleCount3D=24;
//	m_streamCount=2;
//	m_useEmissiveNightMaterials=TRUE;

	//save previous info for this level since it may be overwritten by refreshCustomStaticLODLevel().
	StaticGameLODInfo prevLodBackup;
	if (m_currentStaticLOD != STATIC_GAME_LOD_UNKNOWN)
		prevLodBackup=m_staticGameLODInfo[m_currentStaticLOD];

	if (level == STATIC_GAME_LOD_CUSTOM)
		refreshCustomStaticLODLevel();	//store current settings into custom preset

	StaticGameLODInfo *lodInfo=&m_staticGameLODInfo[level];
	StaticGameLODInfo *prevLodInfo=&prevLodBackup;

	Int requestedTextureReduction = 0;
	Bool requestedTrees = m_memPassed;	//only use trees if memory requirement passed.
	if (level == STATIC_GAME_LOD_CUSTOM)
	{	requestedTextureReduction = lodInfo->m_textureReduction;
		requestedTrees = lodInfo->m_useTrees;
	}
	else
	if (level >= STATIC_GAME_LOD_LOW)
	{	//normal non-custom level gets texture reduction based on recommendation
		requestedTextureReduction = getRecommendedTextureReduction();
	}

	if (TheGlobalData)
	{
		TheWritableGlobalData->m_maxParticleCount=lodInfo->m_maxParticleCount;
		TheWritableGlobalData->m_useShadowVolumes=lodInfo->m_useShadowVolumes;
		TheWritableGlobalData->m_useShadowDecals=lodInfo->m_useShadowDecals;

		//Check if texture resolution changed.  No need to apply when current is unknown because display will do it
		if (requestedTextureReduction != m_currentTextureReduction)
		{
				TheWritableGlobalData->m_textureReductionFactor = requestedTextureReduction;
				if (TheGameClient)
					TheGameClient->adjustLOD(0);	//apply the new setting stored in globaldata
		}

		//Check if shadow state changed
		if (m_currentStaticLOD == STATIC_GAME_LOD_UNKNOWN	||
			lodInfo->m_useShadowVolumes != prevLodInfo->m_useShadowVolumes ||
			lodInfo->m_useShadowDecals != prevLodInfo->m_useShadowDecals)
		{
			if (TheGameClient)
			{
				TheGameClient->releaseShadows();	//free all shadows
				TheGameClient->allocateShadows();	//allocate those shadows that are enabled.
			}
		}

		TheWritableGlobalData->m_useCloudMap=lodInfo->m_useCloudMap;
		TheWritableGlobalData->m_useLightMap=lodInfo->m_useLightMap;
		TheWritableGlobalData->m_showSoftWaterEdge=lodInfo->m_showSoftWaterEdge;
		//Check if shoreline blending mode has changed
		if (m_currentStaticLOD == STATIC_GAME_LOD_UNKNOWN || lodInfo->m_showSoftWaterEdge != prevLodInfo->m_showSoftWaterEdge)
		{
			if (TheTerrainVisual)
				TheTerrainVisual->setShoreLineDetail();
		}

		TheWritableGlobalData->m_maxTankTrackEdges=lodInfo->m_maxTankTrackEdges;
		TheWritableGlobalData->m_maxTankTrackOpaqueEdges=lodInfo->m_maxTankTrackOpaqueEdges;
		TheWritableGlobalData->m_maxTankTrackFadeDelay=lodInfo->m_maxTankTrackFadeDelay;
		TheWritableGlobalData->m_useTreeSway=lodInfo->m_useTreeSway;
		TheWritableGlobalData->m_useDrawModuleLOD=!lodInfo->m_useBuildupScaffolds;
		TheWritableGlobalData->m_useHeatEffects=lodInfo->m_useHeatEffects;
		TheWritableGlobalData->m_enableDynamicLOD = lodInfo->m_enableDynamicLOD;
		TheWritableGlobalData->m_useFpsLimit = lodInfo->m_useFpsLimit;
		TheWritableGlobalData->m_useTrees = requestedTrees;
	}
	if (!m_memPassed || isReallyLowMHz()) {
		TheWritableGlobalData->m_shellMapOn = false;
	}
	if (TheTerrainVisual)
		TheTerrainVisual->setTerrainTracksDetail();

}

/**Parse a description of all the LOD settings for a given detail level*/
void INI::parseDynamicGameLODDefinition( INI* ini )
{
	const char *c;
	AsciiString name;

	// read the name
	c = ini->getNextToken();
	name.set( c );	

	if( TheGameLODManager )
	{
		Int index = TheGameLODManager->getDynamicGameLODIndex(name);
    if(index==DYNAMIC_GAME_LOD_UNKNOWN)throw INI_INVALID_DATA;
		if (index != DYNAMIC_GAME_LOD_UNKNOWN)
		{
			DynamicGameLODInfo *lodInfo = &(TheGameLODManager->m_dynamicGameLODInfo[index]);

			// parse the ini weapon definition
			ini->initFromINI( lodInfo, TheDynamicGameLODFieldParseTable );
		}
	}
}

/**Parse an LOD level*/
void INI::parseDynamicGameLODLevel( INI* ini, void * , void *store, const void*)
{
	const char *tok=ini->getNextToken();
	for (Int i=0; i<DYNAMIC_GAME_LOD_COUNT; i++)
		if( strcasecmp(tok, DynamicGameLODNames[i]) == 0 )
		{	*(DynamicGameLODLevel*)store = (DynamicGameLODLevel)i;
			return;
		}

	DEBUG_CRASH(("invalid GameLODLevel token %s -- expected LOW/MEDIUM/HIGH\n",tok));
	throw INI_INVALID_DATA;
}

/**Convert LOD name to an index*/
Int GameLODManager::getDynamicGameLODIndex(AsciiString name)
{
	for (Int i=0; i<DYNAMIC_GAME_LOD_COUNT; ++i)
	{
		if (name.compareNoCase(DynamicGameLODNames[i]) == 0)
			return i;
	}

	return DYNAMIC_GAME_LOD_UNKNOWN;
}

const char *GameLODManager::getDynamicGameLODLevelName(DynamicGameLODLevel level)
{
  if(level==DYNAMIC_GAME_LOD_UNKNOWN)return "Unknown";
  if(level<DYNAMIC_GAME_LOD_LOW || level>=DYNAMIC_GAME_LOD_COUNT)throw ERROR_BAD_ARG;
	return DynamicGameLODNames[level];
}

/**Given an average fps, return the optimal dynamic LOD level that matches this fps.*/
DynamicGameLODLevel GameLODManager::findDynamicLODLevel(Real averageFPS)
{
  if(!std::isfinite(averageFPS) || averageFPS<0 || double(averageFPS)>double(std::numeric_limits<Int>::max()))throw ERROR_BAD_ARG;
	Int ifps=(Int)(averageFPS);	//convert to integer.

	for (Int i=DYNAMIC_GAME_LOD_VERY_HIGH; i>=DYNAMIC_GAME_LOD_LOW; i--)
	{	//check which of the LOD levels matches our fps
		if (m_dynamicGameLODInfo[i].m_minFPS < ifps)
			return (DynamicGameLODLevel)i;
	}
	return DYNAMIC_GAME_LOD_LOW;	//none of the low levels were slow enough so pick the lowest.
}

/**Set all game systems to match the desired LOD level.*/
Bool GameLODManager::setDynamicLODLevel(DynamicGameLODLevel level)
{
  if(level<DYNAMIC_GAME_LOD_UNKNOWN || level>=DYNAMIC_GAME_LOD_COUNT)throw ERROR_BAD_ARG;
	if (level == DYNAMIC_GAME_LOD_UNKNOWN || m_currentDynamicLOD == level)
		return FALSE;

	m_currentDynamicLOD = level;

	applyDynamicLODLevel(level);

	return TRUE;
}

void GameLODManager::applyDynamicLODLevel(DynamicGameLODLevel level)
{
	m_numParticleGenerations=0;
	m_dynamicParticleSkipMask=m_dynamicGameLODInfo[level].m_dynamicParticleSkipMask;

	m_numDebrisGenerations=0;
	m_dynamicDebrisSkipMask=m_dynamicGameLODInfo[level].m_dynamicDebrisSkipMask;

	m_slowDeathScale=m_dynamicGameLODInfo[level].m_slowDeathScale;
	m_minDynamicParticlePriority=m_dynamicGameLODInfo[level].m_minDynamicParticlePriority;
	m_minDynamicParticleSkipPriority=m_dynamicGameLODInfo[level].m_minDynamicParticleSkipPriority;
}

Int GameLODManager::getRecommendedTextureReduction(void)
{
	if (m_idealDetailLevel == STATIC_GAME_LOD_UNKNOWN)
		findStaticLODLevel();	//it was never tested, so test now.

	if (!m_memPassed)	//if they have < 256 MB, force them to low res textures.
		return m_staticGameLODInfo[STATIC_GAME_LOD_LOW].m_textureReduction;
  if(m_idealDetailLevel==STATIC_GAME_LOD_UNKNOWN){
    if(!TheGlobalData)throw ERROR_BAD_ARG;
    return TheGlobalData->m_textureReductionFactor;
  }
	return m_staticGameLODInfo[m_idealDetailLevel].m_textureReduction;
}

Int GameLODManager::getLevelTextureReduction(StaticGameLODLevel level)
{
  if(level<STATIC_GAME_LOD_LOW || level>=STATIC_GAME_LOD_COUNT)throw ERROR_BAD_ARG;
	return m_staticGameLODInfo[level].m_textureReduction;
}

Bool GameLODManager::didMemPass( void )
{ 
	return m_memPassed;	
}
