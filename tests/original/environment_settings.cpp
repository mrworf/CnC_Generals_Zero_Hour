// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "NativeTerrainDeviceFixture.h"
#include "Common/NativeEnvironmentSettings.h"
#include "Common/NativeOverrideAdmission.h"
#include "Common/FileSystem.h"
#include "Common/INIException.h"
#include "Common/GameEngine.h"
#include "Common/MessageStream.h"
#include "Common/GlobalData.h"
#include "Common/NativeUserStorage.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Registry.h"
#include "Common/GameLOD.h"
#include <optional>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void reject(F operation){bool failed=false;try{operation();}catch(ErrorCode){failed=true;}
  catch(const INIException&){failed=true;}require(failed,"invalid environment rejected");}
struct Context {
  std::filesystem::path root;
  FileSystem files;
  FileSystem* prior=TheFileSystem;
  Context(){char pattern[]="/tmp/zh-environment-XXXXXX";auto* name=::mkdtemp(pattern);require(name,"generated root");root=name;
    put("base.ini","WaterTransparency\nTransparentWaterDepth = 7\nEnd\nWeather\nSnowVelocity = 8\nEnd\n");
    put("override.ini","WaterTransparency\nTransparentWaterDepth = 11\nEnd\nWeather\nSnowAmplitude = -3\nEnd\n");
    put("second.ini","WaterTransparency\nTransparentWaterMinOpacity = 0.5\nEnd\nWeather\nSnowBoxDimensions = 99.5\nEnd\n");
    put("bad-water.ini","WaterTransparency\nTransparentWaterDepth = 19\nUnknown = late\nEnd\n");
    put("bad-weather.ini","Weather\nSnowTexture = generated-candidate.tga\nSnowBoxDimensions = 0.5\nEnd\n");
    put("time.ini","WaterSet MORNING\nWaterTexture = generated-water.tga\nWaterRepeatCount = 17\nEnd\n");
    put("bad-time.ini","WaterSet MORNING\nWaterTexture = rejected-water.tga\nUnknown = late\nEnd\n");
    put("weather-only.ini","Weather\nSnowTexture = generated-replacement-texture.tga\nSnowVelocity = 6\nEnd\n");
    files.mountReadOnly({root.string()});TheFileSystem=&files;
  }
  ~Context(){TheFileSystem=prior;std::error_code ignored;std::filesystem::remove_all(root,ignored);}
  void put(const char* name,const char* data){auto path=root/name;std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);out<<data;require(bool(out),"generated INI");}
  void load(const char* name,INILoadType type=INI_LOAD_OVERWRITE){
    const INIBlockDefinition blocks[]{{"WaterTransparency",INI::parseWaterTransparencyDefinition},
      {"Weather",INI::parseWeatherDefinition},{"WaterSet",INI::parseWaterSettingDefinition}};
    INI ini;ini.loadBlocks(name,type,blocks);
  }
};
template<class T>unsigned length(const T* head){unsigned n=0;for(;head;head=static_cast<const T*>(head->getNextOverride())){
  require(++n<=8,"bounded acyclic source chain");}return n;}
struct DeviceLease {DeviceFixture terrain;TerrainVisual* prior=TheTerrainVisual;
  DeviceLease(){TheTerrainVisual=&terrain;}~DeviceLease(){TheTerrainVisual=prior;}};
void parsing(Context& context){NativeEnvironmentSettings owner;owner.load([&]{context.load("base.ini");});
  auto* water=TheWaterTransparency.getNonOverloadedPointer();auto* weather=TheWeatherSetting.getNonOverloadedPointer();
  {DeviceLease device;owner.load([&]{context.load("override.ini",INI_LOAD_CREATE_OVERRIDES);});
    owner.load([&]{context.load("second.ini",INI_LOAD_CREATE_OVERRIDES);});
    require(length(water)==3 && length(weather)==3,"multiple actual source overrides remain acyclic");
    require(TheWaterTransparency->m_transparentWaterDepth==7 && TheWaterTransparency->m_minWaterOpacity==0.5f &&
      TheWeatherSetting->m_snowAmplitude==5 && TheWeatherSetting->m_snowVelocity==8 &&
      TheWeatherSetting->m_snowBoxDimensions==99.5f,"source copies root payload rather than previous final override");
  }
  owner.load([&]{context.load("time.ini");});require(WaterSettings[TIME_OF_DAY_MORNING].m_waterRepeatCount==17,"source time payload");
}
void negative(Context& context){NativeEnvironmentSettings owner;
  reject([&]{owner.load([&]{context.load("bad-water.ini");});});require(!TheWaterTransparency,"malformed first base unpublished");
  reject([&]{owner.load([&]{context.load("bad-weather.ini");});});require(!TheWeatherSetting,"unsafe snow divisor unpublished");
  owner.load([&]{context.load("base.ini");});auto* water=TheWaterTransparency.getNonOverloadedPointer();
  auto* weather=TheWeatherSetting.getNonOverloadedPointer();
  reject([&]{owner.load([&]{context.load("base.ini");});});
  reject([&]{owner.load([&]{context.load("override.ini",INI_LOAD_CREATE_OVERRIDES);});});
  require(length(water)==1 && length(weather)==1,"missing terrain notification rolls back owning admission");
  reject([&]{owner.load([&]{context.load("bad-weather.ini",INI_LOAD_CREATE_OVERRIDES);});});
  const auto* texture=WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str();
  reject([&]{owner.load([&]{context.load("bad-time.ini");});});
  require(WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str()==texture && length(weather)==1,"failed payload preserves identity");
  DeviceLease device;owner.load([&]{context.load("override.ini",INI_LOAD_CREATE_OVERRIDES);});
}
void ownership(Context& context){
  auto water=std::unique_ptr<WaterTransparencySetting,NativePooledRetire>(newInstance(WaterTransparencySetting));
  auto weather=std::unique_ptr<WeatherSetting,NativePooledRetire>(newInstance(WeatherSetting));
  auto* priorWater=TheWaterTransparency.getNonOverloadedPointer();auto* priorWeather=TheWeatherSetting.getNonOverloadedPointer();
  struct Restore{const WaterTransparencySetting* water;const WeatherSetting* weather;
    ~Restore(){TheWaterTransparency=water;TheWeatherSetting=weather;}}restore{priorWater,priorWeather};
  TheWaterTransparency=water.get();TheWeatherSetting=weather.get();
  const auto* texture=WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str();
  const auto live=AllocationFault::live();
  {NativeEnvironmentSettings owner;require(!TheWeatherSetting && !TheWaterTransparency,"borrowed heads isolated");
    owner.load([&]{context.load("base.ini");context.load("time.ini");});
    // Foreign publication does not transfer ownership to the acquired cohort.
    TheWeatherSetting=weather.get();TheWaterTransparency=water.get();
  }
  require(TheWeatherSetting.getNonOverloadedPointer()==weather.get() && TheWaterTransparency.getNonOverloadedPointer()==water.get() &&
    WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str()==texture && AllocationFault::live()==live,"whole owner restores borrowed identities and retires acquired backing");
}
struct Snow:SnowManager {bool fail=false;Real* backing()const{return m_startingHeights;}
  void update()override{}
  Real period()const{return m_fullTimePeriod;}Real spacing()const{return m_emitterSpacing;}
  void updateIniSettings()override{if(fail)throw ERROR_BAD_ARG;SnowManager::updateIniSettings();}};
void snow(Context& context){Snow empty;require(!empty.backing(),"constructor has safe empty backing");reject([&]{empty.init();});
  NativeEnvironmentSettings owner;owner.load([&]{context.load("base.ini");});
  empty.fail=true;const auto live=AllocationFault::live();reject([&]{empty.init();});
  require(!empty.backing() && AllocationFault::live()==live,"fallible virtual initialization retires candidate array");
  empty.fail=false;empty.init();auto* identity=empty.backing();require(empty.period()==25 && empty.spacing()==1,"source valid snow arithmetic");
  reject([&]{empty.init();});require(identity==empty.backing(),"duplicate init retains backing");
  for(unsigned i=0;i<Snow::SNOW_NOISE_X*Snow::SNOW_NOISE_Y;++i)require(identity[i]>=0 && identity[i]<200,"source visual noise bounds");
  auto* weather=const_cast<WeatherSetting*>(TheWeatherSetting.getNonOverloadedPointer());const auto box=weather->m_snowBoxDimensions;
  for(Real value:{0.f,0.5f,-1.f,std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::max()}){
    weather->m_snowBoxDimensions=value;reject([&]{empty.updateIniSettings();});require(empty.backing()==identity,"malformed update retains backing");}
  weather->m_snowBoxDimensions=box;empty.updateIniSettings();
}
void snowFaults(Context& context){NativeEnvironmentSettings owner;owner.load([&]{context.load("base.ini");});
  for(std::size_t ordinal=0;ordinal<=1;++ordinal){Snow snow;const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{snow.init();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal==0) && hit==failed && attempts==1,"snow backing exact terminal one");
    if(failed){require(!snow.backing() && AllocationFault::live()==live,"failed backing retires");snow.init();}
    require(snow.backing(),"every failure and terminal has admitted backing");
  }
}
void parserFaults(Context& context,bool time){
  auto attempt=[&](std::size_t ordinal,bool measured){const auto live=AllocationFault::live();std::size_t demand=0;
    {NativeEnvironmentSettings owner;owner.load([&]{context.load("base.ini");});
      const auto* weather=TheWeatherSetting.getNonOverloadedPointer();
      const auto* texture=WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str();
      auto operation=[&]{owner.load([&]{context.load(time?"time.ini":"weather-only.ini",
          time?INI_LOAD_OVERWRITE:INI_LOAD_CREATE_OVERRIDES);});};
      bool failed=false;AllocationFault::arm(ordinal);
      try{operation();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();demand=AllocationFault::attempts();AllocationFault::disarm();
      require(hit==failed,"parser allocation failure propagates truthfully");
      if(measured)require(!failed,"complete source parser census");
      else require(failed==(demand>ordinal),"every source parser prefix and terminal");
      if(failed)require(time?WaterSettings[TIME_OF_DAY_MORNING].m_waterTextureFile.str()==texture:length(weather)==1,
        "failed parser preserves owning chain or time payload");
      operation(); // Every failure/retry pair uses the identical configuration owner.
      require(time?WaterSettings[TIME_OF_DAY_MORNING].m_waterRepeatCount==17:TheWeatherSetting->m_snowVelocity==6,
        "corrected same-owner parser retry");
    }
    require(AllocationFault::live()==live,"parser whole-owner failure/retry retires all standard backing");return demand;
  };
  const auto demand=attempt(std::numeric_limits<std::size_t>::max(),true);
  require(demand<128,"bounded complete parser census");
  for(std::size_t ordinal=0;ordinal<=demand;++ordinal){auto attempts=attempt(ordinal,false);
    require(attempts==(ordinal<demand?ordinal+1:demand),"exact source parser terminal");}
  std::cout<<"parser "<<(time?"time":"weather")<<" prefixes [0,"<<demand<<"); terminal "<<demand<<"; every retry\n";
}
void poolFailures(Context& context){for(const char* name:{"WaterTransparencySetting","WeatherSetting"}){
  auto* pool=TheMemoryPoolFactory->findMemoryPool(name);void* held[8]{};unsigned count=0;
  struct Free{MemoryPool* pool;void** held;unsigned& count;~Free(){while(count)pool->freeBlock(held[--count]);}}free{pool,held,count};
  require(pool->getFreeBlockCount()<=8,"bounded fixed environment pool");
  while(pool->getFreeBlockCount())held[count++]=pool->allocateBlock("generated environment exhaustion");
  const auto used=pool->getUsedBlockCount();NativeEnvironmentSettings owner;
  reject([&]{owner.load([&]{context.load(name[0]=='W' && name[1]=='a'?"base.ini":"weather-only.ini");});});
  require(pool->getUsedBlockCount()==used,"failed pooled candidate does not acquire an owning unit");
  pool->freeBlock(held[--count]);
  owner.load([&]{context.load(name[0]=='W' && name[1]=='a'?"base.ini":"weather-only.ini");});
  require(TheWeatherSetting,"corrected pooled admission retry");
}}
struct PrefixRoot:GameEngine {
  const std::string mount;bool reached=false;
  explicit PrefixRoot(std::string value):mount(std::move(value)){}
  FileSystem* createFileSystem()override{auto files=std::make_unique<FileSystem>();files->mountReadOnly({mount});return files.release();}
  GameLogic* createGameLogic()override{throw ERROR_BAD_ARG;}
  GameClient* createGameClient()override{throw ERROR_BAD_ARG;}
  ModuleFactory* createModuleFactory()override{throw ERROR_BAD_ARG;}
  ThingFactory* createThingFactory()override{throw ERROR_BAD_ARG;}
  FunctionLexicon* createFunctionLexicon()override{throw ERROR_BAD_ARG;}
  Radar* createRadar()override{throw ERROR_BAD_ARG;}
  WebBrowser* createWebBrowser()override{throw ERROR_BAD_ARG;}
  ParticleSystemManager* createParticleSystemManager()override{throw ERROR_BAD_ARG;}
  AudioManager* createAudioManager()override{
    require(TheWaterTransparency && TheWeatherSetting && TheWaterTransparency->m_transparentWaterDepth==7 &&
      TheWeatherSetting->m_snowVelocity==8 && WaterSettings[TIME_OF_DAY_MORNING].m_waterRepeatCount==17 &&
      TheNativeUserStorage && TheWritableGlobalData && TheNameKeyGenerator && TheGameLODManager,
      "real original startup parsed acquired environment and initialized its actual parents");
    reached=true;throw ERROR_BAD_ARG; // Explicit device boundary, never a fabricated successful audio provider.
  }
};
struct UserEnvironment {
  std::filesystem::path root;
  std::optional<std::string> data,cache;
  UserEnvironment(){if(auto* value=std::getenv("XDG_DATA_HOME"))data=value;if(auto* value=std::getenv("XDG_CACHE_HOME"))cache=value;
    char pattern[]="/tmp/zh-environment-user-XXXXXX";auto* value=::mkdtemp(pattern);require(value,"separate generated user root");root=value;
    require(::setenv("XDG_DATA_HOME",(root/"data").c_str(),1)==0 && ::setenv("XDG_CACHE_HOME",(root/"cache").c_str(),1)==0,"isolated user environment");
  }
  ~UserEnvironment(){if(data)::setenv("XDG_DATA_HOME",data->c_str(),1);else ::unsetenv("XDG_DATA_HOME");
    if(cache)::setenv("XDG_CACHE_HOME",cache->c_str(),1);else ::unsetenv("XDG_CACHE_HOME");
    std::error_code ignored;std::filesystem::remove_all(root,ignored);}
};
void rootPrefix(Context& context){
  context.put("Data/INI/Default/GameData.ini","GameData\nXResolution = 640\nYResolution = 480\nEnd\n");
  context.put("Data/INI/Default/Water.ini","WaterTransparency\nTransparentWaterDepth = 7\nEnd\nWaterSet MORNING\nWaterRepeatCount = 17\nEnd\n");
  context.put("Data/INI/Default/Weather.ini","Weather\nSnowVelocity = 8\nEnd\n");
  // INI::prepFile requires every explicitly selected table, including custom
  // files. Empty generated tables exercise admission, not complete-game data.
  for(const char* path:{"Data/INI/GameData.ini","Data/INI/GameLOD.ini","Data/INI/GameLODPresets.ini",
    "Data/INI/Water.ini","Data/INI/Weather.ini","Data/INI/Default/Science.ini","Data/INI/Science.ini",
    "Data/INI/Default/Multiplayer.ini","Data/INI/Multiplayer.ini","Data/INI/Default/Terrain.ini",
    "Data/INI/Terrain.ini","Data/INI/Default/Roads.ini","Data/INI/Roads.ini"})context.put(path,"");
  AsciiString language;language.format("Data/%s/Language.ini",GetRegistryLanguage().str());context.put(language.str(),"");
  UserEnvironment user;auto* priorFiles=TheFileSystem;auto* priorEngine=TheGameEngine;
  struct Restore{FileSystem* files;GameEngine* engine;~Restore(){TheFileSystem=files;TheGameEngine=engine;}}restore{priorFiles,priorEngine};
  TheFileSystem=nullptr;
  {PrefixRoot root(context.root.string());TheGameEngine=&root;
    bool stopped=false;try{root.init(0,nullptr);}catch(ErrorCode code){stopped=true;
      if(!root.reached)std::cout<<"prefix stop code "<<unsigned(code)<<"; registered "<<(TheSubsystemList?TheSubsystemList->ownedCount():0)
        <<"; configuration mask "<<((TheWritableGlobalData?1:0)|(TheGameLODManager?2:0)|(TheWeatherSetting?4:0)|(TheWaterTransparency?8:0))<<'\n';}
    require(stopped && root.reached,"actual GameEngine init reaches declared device boundary");
  }
  require(!TheSubsystemList && !TheFileSystem && !TheNativeUserStorage && !TheNameKeyGenerator && !TheCommandList &&
    !TheWritableGlobalData && !TheGameLODManager && !TheWaterTransparency && !TheWeatherSetting,
    "partial original root retires admitted services and environment");
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"environment family");initMemoryManager();initialized=true;
  TheMemoryPoolFactory->createMemoryPool("WaterTransparencySetting",sizeof(WaterTransparencySetting),8,0);
  TheMemoryPoolFactory->createMemoryPool("WeatherSetting",sizeof(WeatherSetting),8,0);
  {Context context;parsing(context);negative(context);ownership(context);snow(context);snowFaults(context);
    parserFaults(context,false);parserFaults(context,true);poolFailures(context);rootPrefix(context);
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const std::string_view family=argv[1];
      if(family=="parsing")parsing(context);else if(family=="negative")negative(context);
      else if(family=="ownership")ownership(context);else if(family=="snow")snow(context);
      else if(family=="snow-faults")snowFaults(context);else if(family=="weather-faults")parserFaults(context,false);
      else if(family=="time-faults")parserFaults(context,true);else if(family=="pools")poolFailures(context);
      else if(family=="root-prefix")rootPrefix(context);
      else throw std::runtime_error("unknown family");
      require(!TheWaterTransparency && !TheWeatherSetting && !TheSnowManager && !TheTerrainVisual && AllocationFault::live()==live,"repeated environment owner retirement");
      for(const char* name:{"WaterTransparencySetting","WeatherSetting"})require(TheMemoryPoolFactory->findMemoryPool(name)->getUsedBlockCount()==0,"all acquired pooled units retired");
    }
  }shutdownMemoryManager();initialized=false;std::cout<<"PASS original environment configuration (full root pending)\n";return 0;
 }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL source error\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
