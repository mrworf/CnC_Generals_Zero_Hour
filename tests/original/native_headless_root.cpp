// SPDX-License-Identifier: GPL-3.0-or-later
// Exercises the actual original GameEngine::init path with generated inputs.
#include "PreRTS.h"
#include "Common/NativeHeadlessGameEngine.h"
#include "Common/GameMemory.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/DataChunk.h"
#include "Common/NameKeyGenerator.h"
#include "Common/MapObject.h"
#include "Common/RandomValue.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/Terrain.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/DeletionUpdate.h"
#include "GameClient/GameClient.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/MapUtil.h"
#include "AllocationFault.h"
#include "Common/ThingTemplate.h"
#include "Common/Team.h"
#include "Common/Module.h"
#include <filesystem>
#include <bit>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

namespace {
namespace fs=std::filesystem;
fs::path executable;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Root {
  fs::path directory;
  Root() {
    char pattern[]="/tmp/zh-native-root-XXXXXX";
    auto* value=::mkdtemp(pattern);
    require(value,"generated root creation");
    directory=value;
  }
  ~Root(){std::error_code error;fs::remove_all(directory,error);}
  void put(const char* name,const char* text){
    putBytes(name,text);
  }
  void putBytes(const char* name,std::string_view bytes){
    const auto path=directory/"assets"/name;
    fs::create_directories(path.parent_path());
    std::ofstream output(path,std::ios::binary);
    output.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));
    require(bool(output),"generated input write");
  }
  NativeHeadlessConfiguration config()const{
    return {{(directory/"assets").string()}, {(directory/"user-data").string(),(directory/"user-cache").string()}};
  }
};
struct Bytes : OutputStream {
  std::string value;
  Int write(const void* data,Int size) override {
    value.append(static_cast<const char*>(data),static_cast<std::size_t>(size));
    return size;
  }
};
struct MapAuthoring {
  FileSystem* previousFiles=TheFileSystem;
  NameKeyGenerator* previousNames=TheNameKeyGenerator;
  GlobalData* previousData=TheWritableGlobalData;
  ScriptEngine* previousScripts=TheScriptEngine;
  std::unique_ptr<FileSystem> files;
  std::unique_ptr<NameKeyGenerator> names;
  std::unique_ptr<GlobalData> data;
  std::unique_ptr<ScriptEngine> scripts;
  explicit MapAuthoring(const Root& root){
    try {
      files=std::make_unique<FileSystem>();files->mountReadOnly({(root.directory/"assets").string()});TheFileSystem=files.get();
      names=std::make_unique<NameKeyGenerator>();TheNameKeyGenerator=names.get();names->init();
      data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();
      scripts=std::make_unique<ScriptEngine>();TheScriptEngine=scripts.get();scripts->init();
    } catch (...) {retire();throw;}
  }
  void retire() noexcept {
    scripts.reset();data.reset();names.reset();files.reset();
    TheFileSystem=previousFiles;
    TheScriptEngine=previousScripts;TheWritableGlobalData=previousData;TheNameKeyGenerator=previousNames;
  }
  ~MapAuthoring(){retire();}
};
std::string makeH1Map(bool malformedActor=false,bool unsupportedRoad=false){
  Bytes bytes;
  DataChunkOutput out(&bytes);
  constexpr Int width=65,height=65;
  out.openDataChunk("HeightMapData",4);
  out.writeInt(width);out.writeInt(height);out.writeInt(2);
  out.writeInt(1);out.writeInt(61);out.writeInt(61);
  out.writeInt(width*height);
  for(Int y=0;y<height;++y)for(Int x=0;x<width;++x)out.writeByte(x<width/2?20:24);
  out.closeDataChunk();
  out.openDataChunk("WorldInfo",1);
  Dict world;world.setInt(TheKey_weather,WEATHER_NORMAL);out.writeDict(world);out.closeDataChunk();
  out.openDataChunk("SidesList",3);out.writeInt(1);
  Dict side;side.setAsciiString(TheKey_playerName,"H1Player");
  side.setBool(TheKey_playerIsHuman,TRUE);side.setBool(TheKey_multiplayerIsLocal,TRUE);
  out.writeDict(side);out.writeInt(0);
  out.writeInt(1);Dict team;team.setAsciiString(TheKey_teamName,"teamH1Player");
  team.setAsciiString(TheKey_teamOwner,"H1Player");team.setBool(TheKey_teamIsSingleton,TRUE);
  out.writeDict(team);
  out.openDataChunk("PlayerScriptsList",5);out.openDataChunk("ScriptList",1);
  out.openDataChunk("ScriptGroup",2);out.writeAsciiString("H1Group");out.writeByte(TRUE);out.writeByte(FALSE);
  out.openDataChunk("Script",2);
  for(const char* label:{"H1StartScript","H1 generated","H1 condition","H1 action"})out.writeAsciiString(label);
  for(Bool value:{TRUE,FALSE,TRUE,TRUE,TRUE,FALSE})out.writeByte(value);
  out.writeInt(0);
  out.openDataChunk("OrCondition",1);out.openDataChunk("Condition",4);
  out.writeInt(Condition::CONDITION_TRUE);
  out.writeNameKey(TheScriptEngine->getConditionTemplate(Condition::CONDITION_TRUE)->m_internalNameKey);
  out.writeInt(0);out.closeDataChunk();out.closeDataChunk();
  out.openDataChunk("ScriptAction",2);out.writeInt(ScriptAction::SET_FLAG);
  out.writeNameKey(TheScriptEngine->getActionTemplate(ScriptAction::SET_FLAG)->m_internalNameKey);
  out.writeInt(2);
  out.writeInt(Parameter::FLAG);out.writeInt(0);out.writeReal(0);out.writeAsciiString("H1Started");
  out.writeInt(Parameter::BOOLEAN);out.writeInt(1);out.writeReal(0);out.writeAsciiString("");
  out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();
  out.openDataChunk("ObjectsList",3);
  for(const auto& actor:{std::pair{"H1Persistent",Real{200}},std::pair{"H1Expiring",Real{450}}}){
    out.openDataChunk("Object",3);
    if(malformedActor && actor.second==Real{200})
      out.writeInt(std::bit_cast<Int>(std::numeric_limits<Real>::quiet_NaN()));
    else out.writeReal(actor.second);
    out.writeReal(200);out.writeReal(0);
    out.writeReal(0);out.writeInt(unsupportedRoad && actor.second==Real{200} ? FLAG_ROAD_FLAGS : 0);
    out.writeAsciiString(actor.first);
    Dict properties;properties.setAsciiString(TheKey_originalOwner,"teamH1Player");out.writeDict(properties);
    out.closeDataChunk();
  }
  out.closeDataChunk();
  out.openDataChunk("BlendTileData",8);out.writeInt(width*height);
  for(unsigned table=0;table<4;++table)for(Int n=0;n<width*height;++n){out.writeByte(0);out.writeByte(0);}
  for(Int y=0;y<height;++y)for(Int byte=0;byte<(width+7)/8;++byte)out.writeByte(0);
  out.writeInt(16);out.writeInt(1);out.writeInt(1);out.writeInt(1);
  out.writeInt(0);out.writeInt(16);out.writeInt(4);out.writeInt(0);out.writeAsciiString("H1Terrain");
  out.writeInt(0);out.writeInt(0);out.closeDataChunk();
  out.finish();return bytes.value;
}
void admitAndRetire(NativeHeadlessConfiguration config){
  auto engine=std::make_unique<NativeHeadlessGameEngine>(std::move(config));
  require(!TheGameEngine,"no borrowed engine");
  TheGameEngine=engine.get();
  try {
    engine->init(0,nullptr);
    require(TheGameLogic && TheGameClient && TheFileSystem && TheNativeUserStorage &&
      TheWritableGlobalData && TheMapCache,"original root initialized");
    require(!engine->getQuitting(),"startup did not silently quit");
    engine.reset();
    TheGameEngine=nullptr;
    require(!TheGameLogic && !TheGameClient && !TheFileSystem && !TheNativeUserStorage &&
      !TheWritableGlobalData && !TheMapCache,"original root retired");
  } catch (...) {
    engine.reset();
    TheGameEngine=nullptr;
    throw;
  }
}
void putWord(std::string& bytes,std::uint32_t value){
  for(unsigned n=0;n<4;++n)bytes.push_back(static_cast<char>(value>>(n*8)));
}
void putReal(std::string& bytes,Real value){putWord(bytes,std::bit_cast<std::uint32_t>(value));}
void putLabel(std::string& bytes,std::string_view value){
  require(value.size()<16,"canonical label width");
  bytes.append(value);
  bytes.append(16-value.size(),'\0');
}
void putH1Actor(std::string& bytes,ObjectID id,const char* label,bool timed){
  auto* actor=TheGameLogic->findObjectByID(id);
  putLabel(bytes,label);
  putWord(bytes,static_cast<std::uint32_t>(id));
  putWord(bytes,actor?1:0);
  putLabel(bytes,"teamH1Player");
  if(!actor){for(unsigned n=0;n<9;++n)putWord(bytes,0);return;}
  require(actor->getTemplate()->getName()==label && actor->getTeam() &&
    actor->getTeam()->getName()=="teamH1Player","checkpoint actor identity");
  const auto* position=actor->getPosition();
  const auto& geometry=actor->getGeometryInfo();
  putReal(bytes,position->x);putReal(bytes,position->y);putReal(bytes,position->z);
  putWord(bytes,static_cast<std::uint32_t>(geometry.getGeomType()));
  putReal(bytes,geometry.getMajorRadius());putReal(bytes,geometry.getMinorRadius());
  putReal(bytes,geometry.getMaxHeightAbovePosition());
  if(timed){
    auto* module=static_cast<DeletionUpdate*>(actor->findUpdateModule(
      TheNameKeyGenerator->nameToKey("DeletionUpdate")));
    require(module,"checkpoint timed module retained");
    putWord(bytes,module->friend_getNextCallFrame());putWord(bytes,module->getDieFrame());
  }else{putWord(bytes,0);putWord(bytes,0);}
}
std::string h1Record(unsigned tick,ObjectID persistentID,ObjectID expiringID){
  std::string bytes="H1R1";
  putWord(bytes,1);putWord(bytes,tick);putWord(bytes,TheGameLogic->getFrame());
  putWord(bytes,GetGameLogicRandomSeedCRC());
  Bool found=FALSE;
  const Bool flag=TheScriptEngine->readFlagByName("H1Started",&found);
  require(found && flag,"checkpoint script flag");
  putWord(bytes,flag?1:0);
  Int sideIndex=-1;
  require(TheSidesList->findSideInfo("H1Player",&sideIndex) && sideIndex>=0,
    "checkpoint active side");
  auto* player=ThePlayerList->getLocalPlayer();
  require(player && player->getPlayerNameKey()==TheNameKeyGenerator->nameToKey("H1Player"),
    "checkpoint source player identity");
  putWord(bytes,static_cast<std::uint32_t>(sideIndex));
  putWord(bytes,static_cast<std::uint32_t>(player->getPlayerIndex()));
  putLabel(bytes,"H1Player");putLabel(bytes,"H1Player");
  const Real left=TheTerrainLogic->getGroundHeight(200,200);
  const Real right=TheTerrainLogic->getGroundHeight(450,200);
  require(left==20*MAP_HEIGHT_SCALE && right==24*MAP_HEIGHT_SCALE && left!=right,
    "two source-scaled authored terrain plateaus");
  putReal(bytes,left);putReal(bytes,right);
  putH1Actor(bytes,persistentID,"H1Persistent",false);
  putH1Actor(bytes,expiringID,"H1Expiring",true);
  require(bytes.size()==224,"version-one checkpoint record width");
  return bytes;
}
void requireH1PoolsRetired(bool completeCohort=false){
  for(const char* poolName:{"ObjectPool","MapObject","ScriptList","DeletionUpdate"}){
    auto* pool=TheMemoryPoolFactory->findMemoryPool(poolName);
    require((pool || !completeCohort) && (!pool || pool->getUsedBlockCount()==0),
      "H1 actor/map/script/module pool units retired");
  }
}
struct FaultProbe {
  std::size_t ordinal=SIZE_MAX;
  std::size_t attempts=0;
  bool triggered=false;
};
void loadH1(NativeHeadlessConfiguration config,bool runTicks=false,
            std::vector<std::string>* checkpoints=nullptr,FaultProbe* fault=nullptr,
            bool interruptAfterLoad=false){
  require(!checkpoints || runTicks,"checkpoints require original steady updates");
  const auto scriptCursorBefore=checkpoints?ScriptList::currentID():0;
  auto engine=std::make_unique<NativeHeadlessGameEngine>(std::move(config));
  require(!TheGameEngine,"no borrowed engine before H1");
  TheGameEngine=engine.get();
  try {
    engine->init(0,nullptr);
    if(runTicks)InitRandom(0x5A170001u);
    TheWritableGlobalData->m_mapName="Maps/H1.map";
    TheGameLogic->setGameMode(GAME_SINGLE_PLAYER);
    TheGameLogic->startNewGame(FALSE);
    require(TheGameLogic->isLoadingMap(),"original pending H1 load");
    if(fault)AllocationFault::arm(fault->ordinal);
    engine->update();
    if(fault){fault->attempts=AllocationFault::attempts();fault->triggered=AllocationFault::triggered();AllocationFault::disarm();}
    require(!TheGameLogic->isLoadingMap(),"original H1 load completed");
    if(checkpoints)require(ScriptList::currentID()>scriptCursorBefore,
      "production H1 parsing advances source script cursor");
    require(TheSidesList && TheSidesList->findSideInfo("H1Player"),"authored H1 side loaded");
    require(TheSidesList->findTeamInfo("teamH1Player"),"authored H1 team loaded");
    require(TheScriptEngine && TheScriptEngine->findScriptByName("H1StartScript"),"authored H1 script registered");
    unsigned persistent=0,expiring=0,unexpected=0;
    ObjectID persistentID=static_cast<ObjectID>(0),expiringID=static_cast<ObjectID>(0);
    for(Object* actor=TheGameLogic->getFirstObject();actor;actor=actor->getNextObject()){
      const auto& name=actor->getTemplate()->getName();
      require(actor->getTeam() && actor->getTeam()->getName()=="teamH1Player",
        "H1 actor has authored team ownership");
      unsigned deletionModules=0;
      for(auto** module=actor->getBehaviorModules();*module;++module)
        if((*module)->getModuleNameKey()==TheNameKeyGenerator->nameToKey("DeletionUpdate"))
          ++deletionModules;
      if(name=="H1Persistent"){
        require(actor->getPosition()->x==200 && actor->getPosition()->y==200,
          "H1 persistent actor position");
        persistentID=actor->getID();
        require(deletionModules==0,"H1 persistent actor has no timed deletion");
        ++persistent;
      }
      else if(name=="H1Expiring"){
        require(actor->getPosition()->x==450 && actor->getPosition()->y==200,
          "H1 expiring actor position");
        require(deletionModules==1 &&
          actor->findUpdateModule(TheNameKeyGenerator->nameToKey("DeletionUpdate")),
          "exactly one H1 DeletionUpdate parsed and constructed");
        expiringID=actor->getID();
        ++expiring;
      }
      else ++unexpected;
    }
    require(persistent==1 && expiring==1 && unexpected==0,"two exact H1 actors loaded");
    require(persistentID!=0 && expiringID!=0 &&
      persistentID!=expiringID,"source assigned distinct H1 object IDs");
    require(TheTerrainVisual,"H1 terrain visual exists");
    ICoord2D left{20,20},right{45,20};
    require(TheTerrainVisual->getRawMapHeight(&left)==20 && TheTerrainVisual->getRawMapHeight(&right)==24,
      "two authored terrain plateaus loaded");
    if(interruptAfterLoad)throw std::runtime_error("controlled H1 stop");
    if(runTicks){
      auto* expiring=TheGameLogic->findObjectByID(expiringID);
      auto* deletion=static_cast<DeletionUpdate*>(expiring->findUpdateModule(
        TheNameKeyGenerator->nameToKey("DeletionUpdate")));
      const auto loadedFrame=TheGameLogic->getFrame();
      const auto dieFrame=deletion->getDieFrame();
      require(loadedFrame==1 && dieFrame==30,
        "parsed 1000ms H1 deletion schedules at source frame 30");
      Bool found=FALSE;
      require(TheScriptEngine->readFlagByName("H1Started",&found) && found,
        "original script runs in pending-load update before steady ticks");
      if(checkpoints)checkpoints->push_back(h1Record(0,persistentID,expiringID));
      for(unsigned tick=0;tick<120;++tick){
        const auto entered=TheGameLogic->getFrame();
        const bool before=TheGameLogic->findObjectByID(expiringID)!=nullptr;
        engine->update();
        const auto exited=TheGameLogic->getFrame();
        require(exited==entered+1,"exactly one original source frame per steady update");
        require(TheGameLogic->findObjectByID(persistentID),"persistent H1 actor survives");
        require(TheScriptEngine->readFlagByName("H1Started",&found) && found,
          "original script flag remains set through steady updates");
        const bool after=TheGameLogic->findObjectByID(expiringID)!=nullptr;
        require(before==(entered<=dieFrame) && after==(entered<dieFrame),
          "DeletionUpdate removes actor at source deadline");
        if(checkpoints)checkpoints->push_back(h1Record(tick+1,persistentID,expiringID));
      }
      if(checkpoints)require(checkpoints->size()==121,"121 canonical H1 records");
    }
    engine.reset();TheGameEngine=nullptr;
    require(!TheGameLogic && !TheGameClient && !TheFileSystem,"H1 root retired");
    requireH1PoolsRetired(true);
  } catch (...) {
    if(fault){fault->attempts=AllocationFault::attempts();fault->triggered=AllocationFault::triggered();AllocationFault::disarm();}
    engine.reset();TheGameEngine=nullptr;throw;
  }
}
void requireH1(NativeHeadlessConfiguration config,const char* stage){
  try{loadH1(std::move(config));}catch(...){throw std::runtime_error(stage);}
}
void runGenerated(const std::string& mode){
  Root input;
  constexpr auto gameData="GameData\n XResolution = 640\n YResolution = 480\n CameraHeight = 300\n CameraPitch = 45\n CameraYaw = 0\n MaxCameraHeight = 600\n End\n";
  input.put("Data/INI/Default/GameData.ini",gameData);
  input.put("Data/INI/GameData.ini","");
  input.put("Data/INI/Default/Water.ini","WaterTransparency\n TransparentWaterDepth = 7\n End\nWaterSet MORNING\n WaterRepeatCount = 17\n End\n");
  input.put("Data/INI/Default/Weather.ini","Weather\n SnowVelocity = 8\n End\n");
  input.put("Data/INI/GameLOD.ini",
    "StaticGameLOD Low\nTextureReductionFactor = 2\nEnd\n"
    "StaticGameLOD Medium\nTextureReductionFactor = 1\nEnd\n"
    "StaticGameLOD High\nTextureReductionFactor = 0\nEnd\n"
    "DynamicGameLOD Low\nMinimumFPS = 0\nEnd\n"
    "DynamicGameLOD Medium\nMinimumFPS = 10\nEnd\n"
    "DynamicGameLOD High\nMinimumFPS = 20\nEnd\n"
    "DynamicGameLOD VeryHigh\nMinimumFPS = 30\nEnd\n");
  input.put("Data/INI/GameLODPresets.ini","LODPreset High P4 2000 PS20 512\nBenchProfile P4 2000 2 3 4\nReallyLowMHz 400\n");
  input.put("Data/English/Language.ini","");
  input.put("Data/English/CommandMap.ini","");
  input.put("Maps/MapCache.ini","");
  // Every explicit source table must exist, even when this generated scenario
  // intentionally contains no definitions of that type.
  for(const char* path:{"Data/INI/Water.ini","Data/INI/Weather.ini",
    "Data/INI/Default/Science.ini","Data/INI/Science.ini",
    "Data/INI/Default/Multiplayer.ini","Data/INI/Multiplayer.ini",
    "Data/INI/Default/Terrain.ini","Data/INI/Terrain.ini",
    "Data/INI/Default/Roads.ini","Data/INI/Roads.ini",
    "Data/INI/Rank.ini","Data/INI/Default/PlayerTemplate.ini","Data/INI/PlayerTemplate.ini",
    "Data/INI/Default/FXList.ini","Data/INI/FXList.ini","Data/INI/Weapon.ini",
    "Data/INI/Default/ObjectCreationList.ini","Data/INI/ObjectCreationList.ini",
    "Data/INI/Locomotor.ini","Data/INI/Default/SpecialPower.ini","Data/INI/SpecialPower.ini",
    "Data/INI/DamageFX.ini","Data/INI/Armor.ini",
    "Data/INI/ParticleSystem.ini",
    "Data/INI/Campaign.ini",
    "Data/INI/Default/CommandButton.ini","Data/INI/CommandButton.ini","Data/INI/CommandSet.ini",
    "Data/INI/InGameUI.ini",
    "Data/INI/Default/Music.ini","Data/INI/Default/SoundEffects.ini","Data/INI/SoundEffects.ini",
    "Data/INI/Default/Speech.ini","Data/INI/Speech.ini","Data/INI/Default/Voice.ini",
    "Data/INI/Voice.ini","Data/INI/MiscAudio.ini",
    "Data/INI/Default/Object.ini","Data/INI/Default/Upgrade.ini","Data/INI/Upgrade.ini",
    "Data/INI/Default/AIData.ini","Data/INI/AIData.ini",
    "Data/INI/Default/Crate.ini","Data/INI/Crate.ini",
    "Data/INI/CommandMap.ini","Data/INI/DrawGroupInfo.ini"})input.put(path,"");
  input.put("Data/INI/AudioSettings.ini","AudioSettings\n AudioRoot = Data/Audio\n MusicFolder = Tracks\n SoundsFolder = Sounds\n StreamingFolder = Speech\n SoundsExtension = wav\n Preferred3DSW = GeneratedNoDevice\n Default2DSpeakerType = Stereo\n Default3DSpeakerType = Stereo\n End\n");
  input.put("Data/INI/Music.ini","MusicTrack GeneratedMusic\n Filename = generated-track\n Volume = 100%\n Priority = NORMAL\n Type = UI EVERYONE\n End\n");
  input.put("Data/Audio/Tracks/generated-track","generated-not-decoded");
  if(mode=="h1-map" || mode=="h1-map-negative" || mode=="h1-ticks" ||
     mode=="h1-ticks-negative" || mode=="h1-records" ||
     mode=="h1-fault-census" || mode=="h1-fault-late" ||
     mode=="h1-startup-negative" || mode=="h1-interrupt") {
    input.put("Data/INI/Default/Multiplayer.ini",
      "MultiplayerColor H1Color\n RGBColor = R:255 G:0 B:0\n RGBNightColor = R:128 G:0 B:0\n End\n");
    constexpr auto h1Objects=
      "Object H1Persistent\n Geometry = BOX\n GeometryMajorRadius = 5\n GeometryMinorRadius = 3\n Body = InactiveBody H1Body\n End\n End\n"
      "Object H1Expiring\n Geometry = BOX\n GeometryMajorRadius = 5\n GeometryMinorRadius = 3\n Body = InactiveBody H1Body\n End\n Behavior = DeletionUpdate H1Deletion\n MinLifetime = 1000\n MaxLifetime = 1000\n End\n End\n";
    input.put("Data/INI/Default/Object.ini",h1Objects);
    std::string map,malformedActorMap,unsupportedRoadMap;
    {
      MapAuthoring author(input);
      map=makeH1Map();
      if(mode=="h1-map-negative")malformedActorMap=makeH1Map(true);
      if(mode=="h1-ticks-negative")unsupportedRoadMap=makeH1Map(false,true);
    }
    if(mode=="h1-map-negative"){
      input.putBytes("Maps/H1.map",std::string_view(map).substr(0,16));
      bool rejected=false;
      try{loadH1(input.config());}catch(...){rejected=true;}
      require(rejected,"truncated original map rejected");
      require(!TheGameEngine && !TheGameLogic && !TheGameClient && !TheFileSystem,
        "rejected H1 root retired");
      requireH1PoolsRetired();
      input.putBytes("Maps/H1.map",map);
      requireH1(input.config(),"corrected map retry");
      input.putBytes("Maps/H1.map",malformedActorMap);
      rejected=false;
      try{loadH1(input.config());}catch(...){rejected=true;}
      require(rejected,"nonfinite authored actor rejected");
      require(!TheGameEngine && !TheGameLogic && !TheGameClient && !TheFileSystem,
        "rejected H1 actor root retired");
      requireH1PoolsRetired();
      input.putBytes("Maps/H1.map",map);
      requireH1(input.config(),"corrected actor retry");
      input.put("Data/INI/Default/Object.ini",
        "Object H1Persistent\n Behavior = MissingH1Module Bad\n End\n End\n");
      rejected=false;
      try{loadH1(input.config());}catch(...){rejected=true;}
      require(rejected,"unknown H1 module rejected");
      require(!TheGameEngine && !TheGameLogic && !TheGameClient && !TheFileSystem,
        "rejected H1 module root retired");
      requireH1PoolsRetired();
      input.put("Data/INI/Default/Object.ini",h1Objects);
    }
    if(mode=="h1-ticks-negative"){
      input.putBytes("Maps/H1.map",unsupportedRoadMap);
      bool rejected=false;
      try{loadH1(input.config(),true);}catch(...){rejected=true;}
      require(rejected,"H1 road requires absent physical provider");
      require(!TheGameEngine && !TheGameLogic && !TheGameClient && !TheFileSystem,
        "unsupported H1 road root retired");
      requireH1PoolsRetired();
    }
    input.putBytes("Maps/H1.map",map);
    if(mode=="h1-interrupt"){
      const auto config=input.config();
      bool stopped=false;
      try{loadH1(config,false,nullptr,nullptr,true);}
      catch(const std::runtime_error& error){stopped=std::string_view(error.what())=="controlled H1 stop";}
      require(stopped && !TheGameEngine && !TheGameLogic && !TheGameClient && !TheFileSystem,
        "controlled H1 interruption retires loaded root");
      requireH1PoolsRetired();
      loadH1(config,true);
      return;
    }
    if(mode=="h1-startup-negative"){
      const auto config=input.config();
      fs::remove(input.directory/"assets/Data/INI/Default/GameData.ini");
      bool rejected=false;
      try{loadH1(config);}catch(...){rejected=true;}
      require(rejected && !TheGameEngine && !TheGameLogic && !TheFileSystem,
        "missing required H1 GameData rejects and retires");
      requireH1PoolsRetired();
      input.put("Data/INI/Default/GameData.ini",gameData);
      loadH1(config,true);
      input.put("Data/INI/Default/GameData.ini",
        "GameData\n XResolution = invalid\n End\n");
      rejected=false;
      try{loadH1(config);}catch(...){rejected=true;}
      require(rejected && !TheGameEngine && !TheGameLogic && !TheFileSystem,
        "malformed required H1 GameData rejects and retires");
      requireH1PoolsRetired();
      input.put("Data/INI/Default/GameData.ini",gameData);
      loadH1(config,true);
      auto missing=config;
      missing.dataRoots={(input.directory/"missing-assets").string()};
      rejected=false;
      try{loadH1(missing);}catch(...){rejected=true;}
      require(rejected && !TheGameEngine && !TheGameLogic && !TheFileSystem,
        "missing H1 root rejects and retires");
      requireH1PoolsRetired();
      loadH1(config,true);
      return;
    }
    if(mode=="h1-fault-late"){
      const auto config=input.config();
      const auto cursor=ScriptList::currentID();
      auto measured=[&](FaultProbe* probe){
        ScriptList::IDTransaction lease;
        loadH1(config,false,nullptr,probe);
        require(ScriptList::currentID()>cursor,"production map advances script ID before fixture lease");
      };
      measured(nullptr);
      require(ScriptList::currentID()==cursor,"warm-up script cursor lease retired");
      const auto outside=AllocationFault::live();
      FaultProbe first,second;
      measured(&first);
      require(ScriptList::currentID()==cursor && AllocationFault::live()==outside,
        "first warmed fault census retires exact owner baseline");
      measured(&second);
      require(ScriptList::currentID()==cursor && AllocationFault::live()==outside &&
        first.attempts==second.attempts && first.attempts>0,
        "identical warmed H1 map admission fault terminal");
      FaultProbe late{second.attempts-1};
      bool rejected=false;
      {
        ScriptList::IDTransaction lease;
        try{loadH1(config,false,nullptr,&late);}catch(const std::bad_alloc&){rejected=true;}
        require(ScriptList::currentID()>cursor,"late failed production map advanced script cursor");
      }
      require(rejected && late.triggered && late.attempts==second.attempts &&
        ScriptList::currentID()==cursor && AllocationFault::live()==outside,
        "late map allocation failure retires all owned standard state");
      requireH1PoolsRetired();
      measured(nullptr);
      require(ScriptList::currentID()==cursor && AllocationFault::live()==outside,
        "corrected H1 after late allocation failure retires exactly");
      FaultProbe terminal{second.attempts};
      measured(&terminal);
      require(!terminal.triggered && terminal.attempts==second.attempts &&
        ScriptList::currentID()==cursor && AllocationFault::live()==outside,
        "exact terminal after all failure/retry owners retire");
      std::cout<<"H1_FAULT_MAP_TERMINAL "<<second.attempts<<'\n';
      return;
    }
    if(mode=="h1-fault-census"){
      FaultProbe probe;
      loadH1(input.config(),false,nullptr,&probe);
      require(!probe.triggered,"unlimited original map admission fault census");
      std::cout<<"H1_FAULT_MAP_ATTEMPTS "<<probe.attempts<<'\n';
      return;
    }
    if(mode=="h1-records"){
      const auto config=input.config();
      std::vector<std::string> baseline;
      constexpr char hex[]="0123456789abcdef";
      for(unsigned lifetime=0;lifetime<3;++lifetime){
        std::vector<std::string> records;
        loadH1(config,true,&records);
        if(lifetime==0)baseline=records;
        else require(records==baseline,"three complete H1 lifetimes have identical canonical bytes");
        for(const auto& record:records){
          std::cout<<"H1_RECORD ";
          for(unsigned char byte:record)std::cout<<hex[byte>>4]<<hex[byte&15];
          std::cout<<'\n';
        }
      }
      return;
    }
    if(mode=="h1-ticks" || mode=="h1-ticks-negative")loadH1(input.config(),true);
    else requireH1(input.config(),"final H1 admission");
    return;
  }
  if(mode!="generated") {
    auto config=input.config();
    if(mode=="missing-ini")fs::remove(input.directory/"assets/Data/INI/Default/GameData.ini");
    else if(mode=="storage-refusal")config.userPaths.data=(input.directory/"assets/user-data").string();
    else throw std::runtime_error("unknown generated test family");
    bool rejected=false;
    try {admitAndRetire(std::move(config));} catch (...) {rejected=true;}
    require(rejected,"invalid generated root rejected");
    require(!TheGameEngine && !TheFileSystem && !TheGameLogic && !TheGameClient &&
      !TheNativeUserStorage,"rejected root retired");
    if(mode=="missing-ini")input.put("Data/INI/Default/GameData.ini",gameData);
    admitAndRetire(input.config());
    return;
  }
  for(int iteration=0;iteration<2;++iteration)admitAndRetire(input.config());
  const auto config=input.config();
  const pid_t child=::fork();
  require(child>=0,"native process creation");
  if(child==0) {
    ::execl(executable.c_str(),executable.c_str(),"--startup","--data-root",
      config.dataRoots.front().c_str(),"--user-data",config.userPaths.data.c_str(),
      "--user-cache",config.userPaths.cache.c_str(),static_cast<char*>(nullptr));
    ::_exit(127);
  }
  int status=0;
  require(::waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0,
    "documented native startup process");
}
void rejectMissing(){
  Root input;
  auto config=input.config();
  config.dataRoots={ (input.directory/"missing-assets").string() };
  bool rejected=false;
  try { NativeHeadlessGameEngine engine(std::move(config));TheGameEngine=&engine;engine.init(0,nullptr);}
  catch (...) {rejected=true;}
  TheGameEngine=nullptr;
  require(rejected,"missing root rejected");
  require(!TheFileSystem && !TheGameLogic && !TheGameClient,"failed root retired");
}
}
int main(int argc,char** argv){
  try {
    require(argc==2,"one test family");
    initMemoryManager();
    executable=fs::absolute(argv[0]).parent_path()/"zero_hour_headless";
    const std::string name=argv[1];
    if(name=="generated" || name=="missing-ini" || name=="storage-refusal" ||
       name=="h1-map" || name=="h1-map-negative" || name=="h1-ticks" ||
       name=="h1-ticks-negative" || name=="h1-records" ||
       name=="h1-fault-census" || name=="h1-fault-late" ||
       name=="h1-startup-negative" || name=="h1-interrupt")runGenerated(name);
    else if(name=="missing")rejectMissing();
    else throw std::runtime_error("unknown test family");
    shutdownMemoryManager();
    std::cout<<"NATIVE_ROOT "<<name<<" PASS\n";
    return 0;
  } catch(const std::exception& error){
    shutdownMemoryManager();
    std::cerr<<"NATIVE_ROOT FAIL: "<<error.what()<<'\n';
    return 1;
  } catch(ErrorCode code){
    shutdownMemoryManager();
    std::cerr<<"NATIVE_ROOT FAIL: source error category "<<static_cast<int>(code)<<'\n';
    return 1;
  } catch(...){
    shutdownMemoryManager();
    std::cerr<<"NATIVE_ROOT FAIL: non-standard source exception\n";
    return 1;
  }
}
