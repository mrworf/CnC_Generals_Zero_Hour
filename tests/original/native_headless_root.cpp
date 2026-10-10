// SPDX-License-Identifier: GPL-3.0-or-later
// Exercises the actual original GameEngine::init path with generated inputs.
#include "PreRTS.h"
#include "Common/NativeHeadlessGameEngine.h"
#include "Common/GameMemory.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/GameClient.h"
#include "GameClient/MapUtil.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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
    const auto path=directory/"assets"/name;
    fs::create_directories(path.parent_path());
    std::ofstream output(path,std::ios::binary);
    output<<text;
    require(bool(output),"generated input write");
  }
  NativeHeadlessConfiguration config()const{
    return {{(directory/"assets").string()}, {(directory/"user-data").string(),(directory/"user-cache").string()}};
  }
};
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
    if(name=="generated" || name=="missing-ini" || name=="storage-refusal")runGenerated(name);
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
