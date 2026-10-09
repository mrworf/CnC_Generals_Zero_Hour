// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "ScriptCameraFixture.h"
#include "NativeTerrainDeviceFixture.h"
#include "GameLogic/NativeTerrainLogic.h"
#include "GameLogic/NativeTerrainLightingState.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/NativeUserStorage.h"
#include "Common/NativeServiceOwners.h"
#include "Common/MapObject.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/NativeLogicalWorld.h"
#include "GameLogic/NativeWaypointGraph.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/GhostObject.h"
#include "GameLogic/RankInfo.h"
#include "Common/ModuleFactory.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/Upgrade.h"
#include "Common/MessageStream.h"
#include "Common/INI.h"
#include "Common/DrawModule.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Module/InactiveBody.h"
#include "GameLogic/Module/VeterancyGainCreate.h"
#include "Common/Team.h"
#include "Common/Radar.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ControlBar.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Input:ChunkInputStream {
  const std::string& bytes;UnsignedInt position=0;
  explicit Input(const std::string& source):bytes(source){}
  Int read(void* output,Int count)override{if(count<0 || (!output && count))throw ERROR_BAD_ARG;
    auto size=std::min<std::size_t>(count,bytes.size()-position);if(size)std::memcpy(output,bytes.data()+position,size);
    position+=size;return size;}
  UnsignedInt tell()override{return position;}
  Bool absoluteSeek(UnsignedInt value)override{if(value>bytes.size())return FALSE;position=value;return TRUE;}
  Bool eof()override{return position==bytes.size();}
};
struct Output:OutputStream{std::string bytes;Int write(const void* data,Int size)override{bytes.append(static_cast<const char*>(data),size);return size;}};
struct Tree {std::filesystem::path path;
  Tree(){char pattern[]="/tmp/zh-logical-world-XXXXXX";auto* root=::mkdtemp(pattern);require(root,"generated empty root");path=root;}
  ~Tree(){std::error_code error;std::filesystem::remove_all(path,error);}
};
struct Context {
  Tree tree;FileSystem files;NameKeyGenerator names;SidesList sides;
  std::unique_ptr<GlobalData> data;std::unique_ptr<ThingFactory> factory;std::unique_ptr<ScriptEngine> scripts;
  NativeLogicalWorld world;
  FileSystem* previousFiles=TheFileSystem;NameKeyGenerator* previousNames=TheNameKeyGenerator;
  GlobalData* previousData=TheWritableGlobalData;ThingFactory* previousFactory=TheThingFactory;
  ScriptEngine* previousScripts=TheScriptEngine;SidesList* previousSides=TheSidesList;
  ScriptList::ReadState previousRead;MapObject::WorldState previousObjects;PolygonTrigger::WorldState previousTriggers;
  Context(){files.mountReadOnly({tree.path.string()});names.init();TheFileSystem=&files;TheNameKeyGenerator=&names;
    TheSidesList=&sides;ScriptList::exchangeReadState(previousRead);MapObject::exchangeWorldState(previousObjects);
    PolygonTrigger::exchangeWorldState(previousTriggers);
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();
      factory=std::make_unique<ThingFactory>();TheThingFactory=factory.get();factory->init();factory->newTemplate("GeneratedTemplate");
      scripts=std::make_unique<ScriptEngine>();TheScriptEngine=scripts.get();scripts->init();}
    catch(...){retire();restore();throw;}}
  void retire()noexcept{ScriptList::ReadState pending;ScriptList::exchangeReadLists(pending);pending.clear();
    MapObject::WorldState objects;MapObject::exchangeWorldState(objects);objects.clear();
    PolygonTrigger::deleteTriggers();sides.clear();scripts.reset();factory.reset();data.reset();}
  void restore()noexcept{MapObject::exchangeWorldState(previousObjects);PolygonTrigger::exchangeWorldState(previousTriggers);
    ScriptList::exchangeReadState(previousRead);TheSidesList=previousSides;TheScriptEngine=previousScripts;
    TheThingFactory=previousFactory;TheWritableGlobalData=previousData;TheNameKeyGenerator=previousNames;TheFileSystem=previousFiles;}
  ~Context(){retire();restore();}
  void load(const std::string& bytes){Input input(bytes);world.load(input);require(input.eof(),"complete physical logical world");}
};
struct InspectRead:ScriptList{static Int id(){return m_curId;}static Int count(){return s_numInReadList;}
  static ScriptList* first(){return s_readLists[0];}};
constexpr const char* poolNames[]{"MapObject","PolygonTrigger","BuildListInfo","ScriptList","ScriptGroup","Script","OrCondition","Condition","ScriptAction","Parameter"};
struct PoolUsage {
  Int used[std::size(poolNames)]{};
  PoolUsage(){for(unsigned n=0;n<std::size(poolNames);++n){auto* pool=TheMemoryPoolFactory->findMemoryPool(poolNames[n]);
    require(pool,"source world pool exists");used[n]=pool->getUsedBlockCount();}}
  bool matches()const{for(unsigned n=0;n<std::size(poolNames);++n)
    if(TheMemoryPoolFactory->findMemoryPool(poolNames[n])->getUsedBlockCount()!=used[n])return false;return true;}
};

void height(DataChunkOutput& out,unsigned version){out.openDataChunk("HeightMapData",version);
  out.writeInt(7);out.writeInt(7);if(version>=3)out.writeInt(1);
  if(version>=4){out.writeInt(1);out.writeInt(5);out.writeInt(5);}out.writeInt(49);
  for(unsigned n=0;n<49;++n)out.writeByte(20+n%3);out.closeDataChunk();}
void sides(DataChunkOutput& out,const char* name){out.openDataChunk("SidesList",3);out.writeInt(1);
  Dict dict;dict.setAsciiString(TheKey_playerName,name);out.writeDict(dict);out.writeInt(1);
  out.writeAsciiString("GeneratedBuilding");out.writeAsciiString("GeneratedTemplate");
  out.writeReal(10);out.writeReal(20);out.writeReal(99);out.writeReal(0.5f);out.writeByte(true);out.writeInt(-1);
  out.writeAsciiString("GeneratedBuildScript");out.writeInt(85);out.writeByte(false);out.writeByte(true);out.writeByte(false);
  out.writeInt(1);Dict team;team.setAsciiString(TheKey_teamName,"GeneratedTeam");team.setAsciiString(TheKey_teamOwner,name);out.writeDict(team);
  out.openDataChunk("PlayerScriptsList",5);out.openDataChunk("ScriptList",1);
  out.openDataChunk("ScriptGroup",2);out.writeAsciiString("GeneratedGroup");out.writeByte(true);out.writeByte(true);
  out.openDataChunk("Script",2);for(const char* text:{"GeneratedScript","Comment","ConditionComment","ActionComment"})out.writeAsciiString(text);
  for(bool value:{true,false,true,false,true,false})out.writeByte(value);out.writeInt(3);
  auto parameters=[&]{out.writeInt(2);for(auto type:{Parameter::FLAG,Parameter::BOOLEAN}){
    out.writeInt(type);out.writeInt(1);out.writeReal(0.5f);out.writeAsciiString("GeneratedFlag");}};
  out.openDataChunk("OrCondition",1);out.openDataChunk("Condition",4);out.writeInt(Condition::FLAG);
  out.writeNameKey(TheScriptEngine->getConditionTemplate(Condition::FLAG)->m_internalNameKey);parameters();
  out.closeDataChunk();out.closeDataChunk();out.openDataChunk("ScriptAction",2);out.writeInt(ScriptAction::SET_FLAG);
  out.writeNameKey(TheScriptEngine->getActionTemplate(ScriptAction::SET_FLAG)->m_internalNameKey);parameters();out.closeDataChunk();
  out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();out.closeDataChunk();}
void objects(DataChunkOutput& out,unsigned version,Real angle){out.openDataChunk("ObjectsList",3);
  for(unsigned n=0;n<4;++n){out.openDataChunk("Object",version);out.writeReal(10+n);out.writeReal(20);
    out.writeReal(n==1 ? 2000 : 99);
    // The legal writer rejects nonfinite floats. Malformed fixtures explicitly
    // emit their defined IEEE raw word, not an invalid enum or aliased object.
    if(std::isfinite(angle))out.writeReal(angle);else out.writeInt(std::bit_cast<Int>(angle));
    out.writeInt(FLAG_DRAWS_IN_MIRROR);
    out.writeAsciiString(n==3 ? "UnknownGeneratedTemplate" : "GeneratedTemplate");
    if(version>=2){Dict dict;if(n==0)dict.setInt(TheKey_waypointID,17);
      if(n==2)dict.setReal(TheKey_lightHeightAboveTerrain,5);if(n==3)dict.setInt(TheKey_scorchType,2);out.writeDict(dict);}
    out.closeDataChunk();}out.closeDataChunk();}
void triggers(DataChunkOutput& out){out.openDataChunk("PolygonTriggers",4);out.writeInt(1);
  out.writeAsciiString("GeneratedTrigger");out.writeAsciiString("GeneratedLayer");out.writeInt(12);
  out.writeByte(false);out.writeByte(false);out.writeInt(0);out.writeInt(3);
  for(const ICoord3D p: {ICoord3D{0,0,0},ICoord3D{100,0,0},ICoord3D{0,100,0}}){out.writeInt(p.x);out.writeInt(p.y);out.writeInt(p.z);}
  out.closeDataChunk();}
std::string wire(const char* name="AcceptedSide",unsigned objectVersion=3,unsigned heightVersion=4,
                 Real angle=0.5f,Int weather=WEATHER_SNOWY,unsigned omit=0,unsigned duplicate=0,bool heightLast=false,const char* extraWorldKey=nullptr,bool includeVisual=false){
  Output output;DataChunkOutput out(&output);
  if(!(omit&1) && !heightLast)height(out,heightVersion);
  if(!(omit&2)){out.openDataChunk("WorldInfo",1);Dict dict;dict.setInt(TheKey_weather,weather);
    if(extraWorldKey)dict.setInt(TheNameKeyGenerator->nameToKey(extraWorldKey),42);
    out.writeDict(dict);out.closeDataChunk();}
  if(!(omit&8))sides(out,name);
  if(!(omit&4))objects(out,objectVersion,angle);
  if(!(omit&16))triggers(out);
  if(duplicate==1)height(out,heightVersion);if(duplicate==2){out.openDataChunk("WorldInfo",1);out.writeDict(Dict());out.closeDataChunk();}
  if(duplicate==4)objects(out,objectVersion,angle);if(duplicate==8)sides(out,name);if(duplicate==16)triggers(out);
  if(!(omit&1) && heightLast)height(out,heightVersion);
  if(includeVisual){out.openDataChunk("GlobalLighting",3);
    const Bool replacement=std::strcmp(name,"ReplacementSide")==0;
    out.writeInt(replacement ? TIME_OF_DAY_NIGHT : TIME_OF_DAY_AFTERNOON);
    for(unsigned t=TIME_OF_DAY_FIRST;t<TIME_OF_DAY_COUNT;++t)for(unsigned light=0;light<6;++light)
      for(unsigned field=0;field<9;++field)out.writeReal(Real(t*100+light*10+field+(replacement ? 1000 : 0))*0.25f);
    out.writeInt(std::bit_cast<Int>(replacement ? 0x89abcdefu : 0x12345678u));out.closeDataChunk();
    out.openDataChunk("BlendTileData",8);out.writeInt(49);
    for(unsigned table=0;table<4;++table)for(unsigned n=0;n<49;++n){out.writeByte(0);out.writeByte(0);}
    for(unsigned y=0;y<7;++y)out.writeByte(y==1 ? 2 : 0);
    out.writeInt(16);out.writeInt(1);out.writeInt(1);out.writeInt(1);
    out.writeInt(0);out.writeInt(16);out.writeInt(4);out.writeInt(0);out.writeAsciiString("GeneratedTerrain");
    out.writeInt(0);out.writeInt(0);out.closeDataChunk();}
  out.finish();return output.bytes;
}
void check(Context& context,const char* name="AcceptedSide",unsigned version=3,unsigned hv=4){
  require(TheSidesList==&context.sides && context.sides.getNumSides()==2 &&
    context.sides.getSideInfo(0)->getDict()->getAsciiString(TheKey_playerName)==name,"source sides plus neutral and borrowed identity");
  auto* list=context.sides.getSideInfo(0)->getScriptList();require(list && list->getScriptGroup() &&
    list->getScriptGroup()->getScript()->getOrCondition()->getFirstAndCondition()->getConditionType()==Condition::FLAG &&
    list->getScriptGroup()->getScript()->getAction()->getNumParameters()==2,
    "actual nonempty original scripts and registry");
  require(context.sides.getSideInfo(0)->getBuildList() && context.sides.getSideInfo(0)->getBuildList()->getLocation()->z==0 &&
    context.sides.findTeamInfo("GeneratedTeam"),"actual world builder and team owners");
  auto* node=MapObject::getFirstMapObject();unsigned n=0;
  for(;node;node=node->getNext(),++n){unsigned index=(version==3 && n>=1) ? n+1 : n;
    require(node->getLocation()->x==10+index && node->getLocation()->z==(version<=2 ? 0 : 99),"source file order, old z and discard");
    require(node->getFlags()==FLAG_DRAWS_IN_MIRROR && node->getThingTemplate()==(index==3 ? nullptr : context.factory->findTemplate("GeneratedTemplate",FALSE)),"actual factory binding and unknown template");
    require(node->isWaypoint()==(version>=2 && index==0) && node->isLight()==(version>=2 && index==2) &&
      node->isScorch()==(version>=2 && index==3),"typed original property classification");}
  require(n==(version==3 ? 3u : 4u),"source retained draw-independent object cardinality");
  require(context.world.height().width()==(hv==1 ? 4 : 7) && context.world.height().bytes().size()==49,"source logical height purpose");
  auto* trigger=PolygonTrigger::getFirstPolygonTrigger();require(trigger && trigger->getID()==12 && trigger->getNumPoints()==3,"real source trigger ownership");
  require(context.data->m_weather==WEATHER_SNOWY && MapObject::TheWorldDict.getInt(TheKey_weather)==WEATHER_SNOWY,"source world weather");
}
void functional(Context& context){for(unsigned ov=1;ov<=3;++ov)for(unsigned hv=1;hv<=4;++hv){auto bytes=wire("AcceptedSide",ov,hv);
    context.load(bytes);check(context,"AcceptedSide",ov,hv);}
  for(Real angle:{std::numeric_limits<Real>::max(),-std::numeric_limits<Real>::max(),1e30f,-1e30f,Real(-PI),0.5f}){
    context.load(wire("AcceptedSide",3,4,angle));check(context);
    auto normalized=MapObject::getFirstMapObject()->getAngle();require(std::isfinite(normalized) && normalized>-PI && normalized<=PI,"large finite source angle makes progress");}
  context.load(wire());auto dict=MapObject::TheWorldDict;
  Dict skirmish;skirmish.setAsciiString(TheKey_teamName,"GeneratedSkirmishTeam");
  skirmish.setAsciiString(TheKey_teamOwner,"AcceptedSide");context.sides.addSkirmishTeam(&skirmish);
  context.load(wire("Unused",3,4,0.5f,WEATHER_NORMAL,2|4|8|16));
  require(!MapObject::getFirstMapObject() && !PolygonTrigger::getFirstPolygonTrigger() && context.sides.getNumSides()==1 &&
    MapObject::TheWorldDict.getInt(TheKey_weather)==dict.getInt(TheKey_weather) && context.data->m_weather==WEATHER_SNOWY,
    "source optional chunks retain only dictionary/weather, neutral repair");
  require(context.sides.findTeamInfo("GeneratedTeam") && context.sides.getNumSkirmishTeams()==1 &&
    context.sides.findTeamInfo("GeneratedTeam")->getDict()->getAsciiString(TheKey_teamOwner).isEmpty() &&
    context.sides.getSkirmishTeamInfo(0)->getDict()->getAsciiString(TheKey_teamName)=="GeneratedSkirmishTeam",
    "omitted SidesList retains both original team sets without side content");
}
void negative(Context& context){const auto valid=wire();context.load(valid);
  auto reject=[&](const std::string& bytes){auto* objects=MapObject::getFirstMapObject();auto* trigger=PolygonTrigger::getFirstPolygonTrigger();
    auto* scripts=context.sides.getSideInfo(0)->getScriptList();auto id=InspectRead::id();auto live=AllocationFault::live();bool rejected=false;
    try{context.load(bytes);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"source malformed world class");rejected=true;}
    require(rejected && MapObject::getFirstMapObject()==objects && PolygonTrigger::getFirstPolygonTrigger()==trigger &&
      context.sides.getSideInfo(0)->getScriptList()==scripts && InspectRead::id()==id && AllocationFault::live()==live,"whole accepted world identities/cursor survive late rejection");
    check(context);context.load(valid);check(context);};
  // Optional chunks may legitimately be absent at a complete boundary. Place
  // required height last to make every truncated prefix an incomplete world,
  // including failures after otherwise successful object/side/trigger chunks.
  const auto late=wire("ReplacementSide",3,4,0.5f,WEATHER_NORMAL,0,0,true);
  for(std::size_t n=0;n<late.size();++n)reject(late.substr(0,n));
  for(unsigned bit:{1u,2u,4u,8u,16u})reject(wire("ReplacementSide",3,4,0.5f,WEATHER_NORMAL,0,bit));
  reject(wire("ReplacementSide",3,4,0.5f,WEATHER_NORMAL,1));
  reject(wire("ReplacementSide",0));reject(wire("ReplacementSide",4));
  reject(wire("ReplacementSide",3,0));reject(wire("ReplacementSide",3,5));
  for(Int weather:{-1,Int(WEATHER_COUNT),std::numeric_limits<Int>::max()})reject(wire("ReplacementSide",3,4,0.5f,weather));
  for(Real angle:{std::numeric_limits<Real>::infinity(),-std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN()})
    reject(wire("ReplacementSide",3,4,angle,WEATHER_NORMAL));
}
void faults(Context& context){const auto accepted=wire(),replacement=wire("ReplacementSide");context.load(accepted);
  // Warm simultaneous old/new candidates before ownership baselines.
  context.load(replacement);context.load(accepted);
  ScriptList::IDTransaction stable;
  std::size_t census;
  {ScriptList::IDTransaction start;AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try{context.load(replacement);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();require(!AllocationFault::triggered(),"complete world census");AllocationFault::disarm();}
  {ScriptList::IDTransaction restore;context.load(accepted);}
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){ScriptList::IDTransaction start;
    auto* objects=MapObject::getFirstMapObject();auto* trigger=PolygonTrigger::getFirstPolygonTrigger();
    auto* scripts=context.sides.getSideInfo(0)->getScriptList();const auto live=AllocationFault::live();const PoolUsage pools;
    auto id=InspectRead::id();bool failed=false;
    AllocationFault::arm(ordinal);try{context.load(replacement);}catch(const std::bad_alloc&){failed=true;}
      catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"original world allocation class");failed=true;}
    const bool hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<census),"every world prefix plus exact terminal");
    require(AllocationFault::live()==live && pools.matches(),"all failed/replaced standard and pooled world owners retire");
    if(failed){require(MapObject::getFirstMapObject()==objects && PolygonTrigger::getFirstPolygonTrigger()==trigger &&
      context.sides.getSideInfo(0)->getScriptList()==scripts && InspectRead::id()==id,"fault preserves entire old world");check(context);}
    else{check(context,"ReplacementSide");require(InspectRead::id()==id+1,"source group advancement commits");}
    context.load(replacement);check(context,"ReplacementSide");context.load(accepted);check(context);
  }
  std::cout<<"world prefixes [0,"<<census<<"); terminal "<<census<<"; all corrected retries\n";
}
void optionalFaults(Context& context){const auto valid=wire(),omitted=wire("Unused",3,4,0.5f,WEATHER_NORMAL,8);
  auto accepted=[&]{require(TheSidesList==&context.sides && context.sides.getNumSides()==1 &&
    context.sides.findTeamInfo("GeneratedTeam") && context.sides.getNumSkirmishTeams()==1 &&
    context.sides.findTeamInfo("GeneratedTeam")->getDict()->getAsciiString(TheKey_teamOwner).isEmpty() &&
    context.sides.getSkirmishTeamInfo(0)->getDict()->getAsciiString(TheKey_teamName)=="GeneratedSkirmishTeam" &&
    context.data->m_weather==WEATHER_NORMAL && MapObject::getFirstMapObject() && PolygonTrigger::getFirstPolygonTrigger(),
    "omitted side chunk retains both team records while replacing other owners");};
  auto original=[&]{context.load(valid);Dict dict;dict.setAsciiString(TheKey_teamName,"GeneratedSkirmishTeam");
    dict.setAsciiString(TheKey_teamOwner,"AcceptedSide");context.sides.addSkirmishTeam(&dict);check(context);};
  original();context.load(omitted);accepted();original();ScriptList::IDTransaction stable;
  std::size_t census;{ScriptList::IDTransaction start;AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try{context.load(omitted);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();require(!AllocationFault::triggered(),"complete omitted-side census");AllocationFault::disarm();accepted();}
  {ScriptList::IDTransaction restore;original();}
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){ScriptList::IDTransaction start;
    const auto live=AllocationFault::live();const PoolUsage pools;auto* objects=MapObject::getFirstMapObject();
    auto* script=context.sides.getSideInfo(0)->getScriptList();auto* team=context.sides.getTeamInfo(0);
    auto* skirmish=context.sides.getSkirmishTeamInfo(0);const auto id=InspectRead::id();bool failed=false;
    AllocationFault::arm(ordinal);try{context.load(omitted);}catch(const std::bad_alloc&){failed=true;}
    catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"omitted-side allocation class");failed=true;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<census),"every omitted-side prefix and exact terminal");
    if(failed){require(AllocationFault::live()==live && pools.matches() && MapObject::getFirstMapObject()==objects &&
      context.sides.getSideInfo(0)->getScriptList()==script && context.sides.getTeamInfo(0)==team &&
      context.sides.getSkirmishTeamInfo(0)==skirmish && InspectRead::id()==id,"partial team carryover preserves all accepted owner identities");check(context);}
    else accepted();
    context.load(omitted);accepted();original();
    require(AllocationFault::live()==live && pools.matches(),"every optional failure/retry restores complete resource baseline");
  }
  std::cout<<"omitted-side prefixes [0,"<<census<<"); terminal "<<census<<"; all corrected retries\n";
}
void ownership(Context& context){const auto valid=wire();context.load(valid);
  // Separately pending read lists are borrowed across the enclosing world.
  Output pendingOutput;{DataChunkOutput out(&pendingOutput);out.openDataChunk("PlayerScriptsList",5);
    out.openDataChunk("ScriptList",1);context.sides.getSideInfo(0)->getScriptList()->WriteScriptListDataChunk(out);
    out.closeDataChunk();out.closeDataChunk();out.finish();}
  {Input stream(pendingOutput.bytes);DataChunkInput file(&stream);
    file.registerParser("PlayerScriptsList",AsciiString::TheEmptyString,ScriptList::ParseScriptsDataChunk);
    require(file.parse() && stream.eof(),"actual pending script writer/reader");}
  auto* pending=InspectRead::first();require(pending && InspectRead::count()==1,"nonempty pending source publication");
  auto bad=wire("ReplacementSide",3,4,0.5f,WEATHER_NORMAL,0,0,true);bad.pop_back();bool rejected=false;
  try{context.load(bad);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"late world reject");rejected=true;}
  require(rejected && InspectRead::first()==pending && InspectRead::count()==1,"late rejection preserves borrowed pending owner");
  check(context);context.load(valid);check(context);
  require(InspectRead::first()==pending && InspectRead::count()==1,"successful replacement preserves borrowed pending owner");

  // Prepare a wire key then withdraw its fixture-only registration. Admission
  // during WorldInfo must roll back even after that callback has succeeded.
  const auto boundary=context.names.nameToKey("GeneratedWorldNamespaceBoundary");
  const auto next=static_cast<NameKeyType>(static_cast<UnsignedInt>(boundary)+1);std::string fresh;
  {NameKeyTransaction generation(context.names);Output output;DataChunkOutput out(&output);
    out.openDataChunk("WorldInfo",1);Dict dict;dict.setInt(context.names.nameToKey("GeneratedFreshWorldKey"),42);
    dict.setInt(TheKey_weather,WEATHER_NORMAL);out.writeDict(dict);out.closeDataChunk();height(out,4);out.finish();fresh=output.bytes;}
  require(context.names.keyToName(next).isEmpty(),"generated key is not pre-admitted");
  auto truncated=fresh;truncated.pop_back();const auto live=AllocationFault::live();rejected=false;
  try{context.load(truncated);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"late namespace world reject");rejected=true;}
  require(rejected && context.names.keyToName(next).isEmpty() && AllocationFault::live()==live &&
    InspectRead::first()==pending,"whole-world rollback retires newly admitted namespace and keeps pending");check(context);
  context.load(fresh);require(context.names.nameToKey("GeneratedFreshWorldKey")==next &&
    MapObject::TheWorldDict.getInt(next)==42 && context.data->m_weather==WEATHER_NORMAL &&
    InspectRead::first()==pending,"corrected same-owner retry commits key ordinal/dictionary/weather");
}
void fixedPools(){
  TheMemoryPoolFactory->createMemoryPool("MapObject",sizeof(MapObject),64,0);
  TheMemoryPoolFactory->createMemoryPool("PolygonTrigger",sizeof(PolygonTrigger),32,0);
  TheMemoryPoolFactory->createMemoryPool("BuildListInfo",sizeof(BuildListInfo),64,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptList",sizeof(ScriptList),64,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptGroup",sizeof(ScriptGroup),64,0);
  TheMemoryPoolFactory->createMemoryPool("Script",sizeof(Script),64,0);
  TheMemoryPoolFactory->createMemoryPool("OrCondition",sizeof(OrCondition),64,0);
  TheMemoryPoolFactory->createMemoryPool("Condition",sizeof(Condition),64,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptAction",sizeof(ScriptAction),64,0);
  TheMemoryPoolFactory->createMemoryPool("Parameter",sizeof(Parameter),128,0);
}
struct AdmissionState {
  Context& context;const std::string& bytes;NativeWaypointGraph graph;
  ScriptList* acceptedScript=context.sides.getSideInfo(0)->getScriptList();
  bool reject=false,called=false;
};
void admitWaypoints(const NativeTerrainHeightMap& height,void* raw){auto& state=*static_cast<AdmissionState*>(raw);
  require(TheSidesList!=&state.context.sides && TheSidesList->getNumSides()==2 &&
    state.context.sides.getSideInfo(0)->getScriptList()==state.acceptedScript,
    "candidate sides remain legitimately published while accepted parent contents are unmodified");
  require(height.bytes().data()!=state.context.world.height().bytes().data() && height.bytes().size()==49,
    "continuation sees offside logical metadata, not prematurely committed backing");
  Input input(state.bytes);state.graph.load(input,MapObject::getFirstMapObject());state.called=true;
  require(state.graph.first() && !state.graph.first()->getNext() && state.graph.first()->getLocation()->z==0,
    "actual pooled graph admits source waypoint classification with absent initial grid");
  if(state.reject)throw ERROR_CORRUPT_FILE_FORMAT;
}
void continuation(Context& context){const auto valid=wire(),replacement=wire("ReplacementSide");context.load(valid);
  {AdmissionState warm{context,replacement};Input input(replacement);context.world.load(input,admitWaypoints,&warm);}
  context.load(valid);ScriptList::IDTransaction stable;
  auto* pool=TheMemoryPoolFactory->findMemoryPool("Waypoint");require(pool,"actual continuation Waypoint pool exists");
  auto reject=[&]{auto* objects=MapObject::getFirstMapObject();auto* trigger=PolygonTrigger::getFirstPolygonTrigger();
    auto* script=context.sides.getSideInfo(0)->getScriptList();auto* backing=context.world.height().bytes().data();
    const auto live=AllocationFault::live();const auto id=InspectRead::id();const auto used=pool->getUsedBlockCount();bool failed=false;
    {AdmissionState state{context,replacement};state.reject=true;Input input(replacement);
      try{context.world.load(input,admitWaypoints,&state);}catch(ErrorCode error){failed=error==ERROR_CORRUPT_FILE_FORMAT;}
      require(state.called,"late rejection occurs after actual graph construction");}
    require(failed && MapObject::getFirstMapObject()==objects && PolygonTrigger::getFirstPolygonTrigger()==trigger &&
      context.sides.getSideInfo(0)->getScriptList()==script && context.world.height().bytes().data()==backing &&
      InspectRead::id()==id && pool->getUsedBlockCount()==used && AllocationFault::live()==live,
      "failure after source world parse and waypoint phase restores complete accepted owner group");check(context);
  };
  reject();
  const auto boundary=context.names.nameToKey("GeneratedContinuationNamespaceBoundary");
  const auto next=static_cast<NameKeyType>(static_cast<UnsignedInt>(boundary)+1);std::string fresh;
  {NameKeyTransaction generation(context.names);fresh=wire("ReplacementSide",3,4,0.5f,WEATHER_NORMAL,0,0,false,"GeneratedFreshContinuationWorldKey");}
  require(context.names.keyToName(next).isEmpty(),"continuation wire generation withdraws fixture-only namespace entry");
  {AdmissionState state{context,fresh};state.reject=true;Input input(fresh);bool failed=false;
    try{context.world.load(input,admitWaypoints,&state);}catch(ErrorCode error){failed=error==ERROR_CORRUPT_FILE_FORMAT;}
    require(failed && state.called && context.names.keyToName(next).isEmpty(),"failure after original graph phase also rolls back fresh world namespace");}
  check(context);
  {AdmissionState corrected{context,fresh};Input input(fresh);context.world.load(input,admitWaypoints,&corrected);
    require(context.names.nameToKey("GeneratedFreshContinuationWorldKey")==next && MapObject::TheWorldDict.getInt(next)==42,
      "corrected late-phase retry commits identical namespace ordinal and actual world dictionary");}
  context.load(valid);
  std::size_t census;
  {ScriptList::IDTransaction start;AdmissionState state{context,replacement};Input input(replacement);AllocationFault::arm(SIZE_MAX);
    try{context.world.load(input,admitWaypoints,&state);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();require(!AllocationFault::triggered() && state.called,"whole continuation allocation census");AllocationFault::disarm();}
  {ScriptList::IDTransaction restore;context.load(valid);}
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){ScriptList::IDTransaction start;
    auto* objects=MapObject::getFirstMapObject();auto* trigger=PolygonTrigger::getFirstPolygonTrigger();auto* script=context.sides.getSideInfo(0)->getScriptList();
    const auto id=InspectRead::id();const auto live=AllocationFault::live();const PoolUsage pools;const auto used=pool->getUsedBlockCount();bool failed=false;
    {AdmissionState state{context,replacement};Input input(replacement);AllocationFault::arm(ordinal);
      try{context.world.load(input,admitWaypoints,&state);}catch(const std::bad_alloc&){failed=true;}
      catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"continuation ownership failure class");failed=true;}
      const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<census),"complete combined world/waypoint manifest and exact terminal");}
    if(failed){require(MapObject::getFirstMapObject()==objects && PolygonTrigger::getFirstPolygonTrigger()==trigger &&
      context.sides.getSideInfo(0)->getScriptList()==script && InspectRead::id()==id,"every late acquisition restores accepted world identities and cursor");check(context);}
    require(AllocationFault::live()==live && pools.matches() && pool->getUsedBlockCount()==used,"combined world/waypoint partial owners retire exactly");
    {AdmissionState corrected{context,replacement};Input input(replacement);context.world.load(input,admitWaypoints,&corrected);
      require(corrected.called,"each combined failure has immediate successful same-world/graph retry");check(context,"ReplacementSide");}
    context.load(valid);check(context);require(AllocationFault::live()==live && pools.matches() && pool->getUsedBlockCount()==used,"complete continuation retry restores resource baseline");
  }
  std::cout<<"world-waypoint prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}
struct LogicFixture:NativeTerrainLogic {
  unsigned deviceCalls=0;
  bool failDevice=false;
  explicit LogicFixture(DeviceFixture& visual):NativeTerrainLogic(visual){}
  void admitDeviceMap()override{++deviceCalls;if(failDevice)throw ERROR_CORRUPT_FILE_FORMAT;}
  void loadRoadsAndBridges(Bool)override{throw ERROR_BAD_ARG;} // Never claim empty bridges as accepted newMap.
};
std::string providerWorld(const char* name="AcceptedSide"){return wire(name,3,4,0.5f,WEATHER_SNOWY,0,0,false,nullptr,true);}
bool sameLighting(const NativeTerrainLightingState& a,const NativeTerrainLightingState& b){
  auto rgb=[](const RGBColor& x,const RGBColor& y){return x.red==y.red && x.green==y.green && x.blue==y.blue;};
  auto xyz=[](const Coord3D& x,const Coord3D& y){return x.x==y.x && x.y==y.y && x.z==y.z;};
  if(a.time!=b.time)return false;
  for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){if(!rgb(a.ambient[l],b.ambient[l]) || !rgb(a.diffuse[l],b.diffuse[l]) || !xyz(a.direction[l],b.direction[l]))return false;
    for(unsigned t=0;t<TIME_OF_DAY_COUNT;++t)for(const auto pair:{std::pair{&a.terrain[t][l],&b.terrain[t][l]},std::pair{&a.objects[t][l],&b.objects[t][l]}})
      if(!rgb(pair.first->ambient,pair.second->ambient) || !rgb(pair.first->diffuse,pair.second->diffuse) || !xyz(pair.first->lightPos,pair.second->lightPos))return false;}
  return true;
}
void logicLoad(LogicFixture& owner,const std::string& bytes,const AsciiString& name,Bool query=FALSE,
               NativeTerrainLogic::Admission callback=nullptr,void* context=nullptr){Input input(bytes);owner.loadStream(input,name,query,callback,context);require(input.eof(),"complete integrated terrain/world admission");}
struct PartitionPublication {
  PartitionManager* previous=ThePartitionManager;
  std::unique_ptr<PartitionManager> owner=std::make_unique<PartitionManager>();
  PartitionPublication(){ThePartitionManager=owner.get();}
  ~PartitionPublication(){owner.reset();ThePartitionManager=previous;}
};
// Real constructor/callback parents. Only presentation is omitted; this common
// ghost owner is used solely for mobile actors and is never fog acceptance.
struct ActorUI:InGameUI{void draw()override{} View* createView()override{throw ERROR_BAD_ARG;}};
struct ActorRadar:Radar{void draw(Int,Int,Int,Int)override{} void clearShroud()override{} void setShroudLevel(Int,Int,CellShroudStatus)override{}};
// Test-only callback faults, not replacement gameplay or presentation modules.
// Execute original registry, pooled construction and binding notification paths.
unsigned bindingFaultPhase=0,bindingDrawCalls=0,bindingBehaviorCalls=0,bindingUnbindCalls=0;
struct BindingDrawProbe:DrawModule{
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(BindingDrawProbe,"BindingDrawProbe",8,0)
public:
  BindingDrawProbe(Thing* thing,const ModuleData* data):DrawModule(thing,data){}
  NameKeyType getModuleNameKey()const override{static const StaticNameKey name("BindingDrawProbe");return name.key();}
  void onDrawableBoundToObject()override{++bindingDrawCalls;auto* actor=getDrawable()->getObject();
    require(actor && !actor->getDrawable(),"first binding callback sees only Drawable-to-Object publication");
    if(bindingFaultPhase==1)throw ERROR_BAD_ARG;}
  void doDrawModule(const Matrix3D*)override{throw ERROR_BAD_ARG;}
  void setShadowsEnabled(Bool)override{} void releaseShadows()override{} void allocateShadows()override{throw ERROR_BAD_ARG;}
  void setFullyObscuredByShroud(Bool)override{} void reactToTransformChange(const Matrix3D*,const Coord3D*,Real)override{}
  void reactToGeometryChange()override{}
};
BindingDrawProbe::~BindingDrawProbe()=default;
struct BindingBehaviorProbe:BehaviorModule{
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(BindingBehaviorProbe,"BindingBehaviorProbe",8,0)
public:
  BindingBehaviorProbe(Thing* thing,const ModuleData* data):BehaviorModule(thing,data){}
  NameKeyType getModuleNameKey()const override{static const StaticNameKey name("BindingBehaviorProbe");return name.key();}
  void onDrawableBoundToObject()override{
    // The original complete Object destructor also notifies a null binding.
    // Fault only real acquisition, never legitimate destruction callbacks.
    if(!getObject()->getDrawable()){++bindingUnbindCalls;return;}
    ++bindingBehaviorCalls;
    require(getObject()->getDrawable()->getObject()==getObject(),"second binding callback sees reciprocal publication");
    if(bindingFaultPhase==2)throw ERROR_BAD_ARG;}
};
BindingBehaviorProbe::~BindingBehaviorProbe()=default;
struct ActorModuleFactory:ModuleFactory{
  void addBindingProbes(){
    addModuleInternal([](Thing* thing,const ModuleData* data)->Module*{return newInstance(BindingDrawProbe)(thing,data);},
      [](INI* ini)->ModuleData*{if(ini)throw ERROR_BAD_ARG;return new ModuleData;},MODULETYPE_DRAW,"BindingDrawProbe",MODULEINTERFACE_DRAW);
    addModuleInternal([](Thing* thing,const ModuleData* data)->Module*{return newInstance(BindingBehaviorProbe)(thing,data);},
      [](INI* ini)->ModuleData*{if(ini)throw ERROR_BAD_ARG;return new BehaviorModuleData;},MODULETYPE_BEHAVIOR,"BindingBehaviorProbe",0);
  }
};
template<class Base,class Concrete=Base>struct ActorParent{
  Base*& slot;Base* previous;std::unique_ptr<Concrete> owner;
  explicit ActorParent(Base*& value):slot(value),previous(value){}
  void create(){owner=std::make_unique<Concrete>();slot=owner.get();}
  void retire(){if(owner){slot=owner.get();owner.reset();slot=previous;}}
  ~ActorParent(){retire();}
};
struct ActorParents{
  ControlBar* previousControl=TheControlBar;
  ActorParent<RankInfoStore> ranks{TheRankInfoStore};ActorParent<AI> ai{TheAI};
  ActorParent<GameLogic> logic{TheGameLogic};ActorParent<InGameUI,ActorUI> ui{TheInGameUI};
  ActorParent<Radar,ActorRadar> radar{TheRadar};ActorParent<ModuleFactory,ActorModuleFactory> modules{TheModuleFactory};
  ActorParent<PlayerList> players{ThePlayerList};ActorParent<TeamFactory> teams{TheTeamFactory};
  ActorParent<GhostObjectManager> ghosts{TheGhostObjectManager};
  ActorParent<MessageStream> messages{TheMessageStream};ActorParent<GameClient,NativeHeadlessGameClient> client{TheGameClient};
  ActorParent<UpgradeCenter> upgrades{TheUpgradeCenter};
  explicit ActorParents(bool fullCreation=false){TheControlBar=nullptr;try{ranks.create();ranks.owner->init();ai.create();ai.owner->init();
    logic.create();logic.owner->setObjectIDCounter(ObjectID(1));
    if(fullCreation){messages.create();client.create();client.owner->init();upgrades.create();logic.owner->setDrawIconUI(FALSE);}
    ui.create();radar.create();modules.create();modules.owner->init();
    players.create();teams.create();teams.owner->init();Dict definition;teams.owner->initTeam("team","",TRUE,&definition);
    players.owner->getNeutralPlayer()->setDefaultTeam();ghosts.create();}catch(...){retire();throw;}}
  void retire(){if(logic.owner)logic.owner->destroyAllObjectsImmediate();client.retire();ui.retire();radar.retire();teams.retire();players.retire();
    ghosts.retire();ai.retire();upgrades.retire();modules.retire();logic.retire();messages.retire();ranks.retire();TheControlBar=previousControl;}
  ~ActorParents(){retire();}
};
void partitionActors(Context& context){
  context.load(wire());DeviceFixture visual;LogicFixture terrain(visual);logicLoad(terrain,providerWorld(),"GeneratedActorPartition.map");
  const auto oldTerrain=TheTerrainLogic;TheTerrainLogic=&terrain;
  struct Restore{TerrainLogic* previous;~Restore(){TheTerrainLogic=previous;}} restore{oldTerrain};
  const auto oldSize=context.data->m_partitionCellSize;context.data->m_partitionCellSize=10;
  struct RestoreSize{Real& slot;Real value;~RestoreSize(){slot=value;}} size{context.data->m_partitionCellSize,oldSize};
  PartitionPublication partition;partition.owner->init();ActorParents parents;
  auto make=[&]{return parents.logic.owner->friend_createObject(context.factory->findTemplate("GeneratedTemplate",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);};
  auto* survivor=make();auto* candidate=make();const auto survivorID=survivor->getID();
  partition.owner->registerObject(survivor);auto* accepted=survivor->friend_getPartitionData();
  accepted->friend_updateCellsTouched();require(accepted->friend_getCoiInUseCount()>0,"actual actor populates nonempty source cell intersections");
  AllocationFault::arm(SIZE_MAX);try{partition.owner->registerObject(candidate);}catch(...){AllocationFault::disarm();throw;}
  const auto count=AllocationFault::attempts();AllocationFault::disarm();partition.owner->unRegisterObject(candidate);
  require(count==1,"warmed actual actor attachment exact COI allocation terminal");
  auto* pool=TheMemoryPoolFactory->findMemoryPool("PartitionDataPool");require(pool,"actual partition data pool");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();bool failed=false;
    AllocationFault::arm(ordinal);try{partition.owner->registerObject(candidate);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count),"every actual attachment failure prefix and terminal");
    if(failed)require(!candidate->friend_getPartitionData() && pool->getUsedBlockCount()==used && AllocationFault::live()==live,"failed candidate withdraws module/dirty/master-list ownership");
    else partition.owner->unRegisterObject(candidate);
    require(survivor->friend_getPartitionData()==accepted && parents.logic.owner->findObjectByID(survivorID)==survivor && accepted->friend_getCoiInUseCount()>0,"attachment failure preserves accepted actor and cell membership");
    partition.owner->registerObject(candidate);candidate->friend_getPartitionData()->friend_updateCellsTouched();
    require(candidate->friend_getPartitionData()->friend_getCoiInUseCount()>0,"corrected same-actor retry populates actual intersections");
    partition.owner->unRegisterObject(candidate);require(pool->getUsedBlockCount()==used && AllocationFault::live()==live,"corrected attachment retires every acquired unit");}
  const auto geometry=candidate->getGeometryInfo();
  for(Real radius:{-1.0f,std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::max()}){
    auto bad=geometry;bad.setMajorRadius(radius);candidate->setGeometryInfo(bad);
    const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();bool rejected=false;
    try{partition.owner->registerObject(candidate);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
    require(rejected && !candidate->friend_getPartitionData() && pool->getUsedBlockCount()==used && AllocationFault::live()==live &&
      survivor->friend_getPartitionData()==accepted,"malformed geometry rejects and withdraws candidate without touching survivor");
    candidate->setGeometryInfo(geometry);partition.owner->registerObject(candidate);partition.owner->unRegisterObject(candidate);}
  for(GeometryType shape:{GEOMETRY_SPHERE,GEOMETRY_CYLINDER,GEOMETRY_BOX}){
    candidate->setGeometryInfo(GeometryInfo(shape,FALSE,1,5,3));partition.owner->registerObject(candidate);
    candidate->friend_getPartitionData()->friend_updateCellsTouched();
    require(candidate->friend_getPartitionData()->friend_getCoiInUseCount()>0,"all original shape families retain real cell coverage");
    partition.owner->unRegisterObject(candidate);}
  candidate->setGeometryInfo(geometry);
  {std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated partition module exhaustion"));
    const auto used=pool->getUsedBlockCount();const auto live=AllocationFault::live();bool rejected=false;
    try{partition.owner->registerObject(candidate);}catch(ErrorCode code){rejected=code==ERROR_OUT_OF_MEMORY;}
    require(rejected && !candidate->friend_getPartitionData() && pool->getUsedBlockCount()==used && AllocationFault::live()==live &&
      survivor->friend_getPartitionData()==accepted,"actual partition pool exhaustion preserves candidate and accepted ownership");
    for(auto* value:held)pool->freeBlock(value);held.clear();partition.owner->registerObject(candidate);partition.owner->unRegisterObject(candidate);}
  parents.logic.owner->destroyAllObjectsImmediate();require(pool->getUsedBlockCount()==0,"actual actor destruction withdraws partition units while parents live");
  for(Int y=0;y<partition.owner->getCellCountY();++y)for(Int x=0;x<partition.owner->getCellCountX();++x)
    require(!partition.owner->getCellAt(x,y)->getFirstCoiInCell(),"actual cell intersection publication retires completely");
  std::cout<<"mobile actor partition prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; fog pending\n";
}
// Isolate the common PartitionData ghost-attachment contract. No render snapshot
// or fog behavior is implemented by this test object or accepted from this test.
struct GhostAttachment:GhostObject{
  GhostAttachment(){m_parentObject=nullptr;m_partitionData=nullptr;m_parentGeometryType=GEOMETRY_BOX;
    m_parentGeometryIsSmall=FALSE;m_parentGeometryMajorRadius=5;m_parentGeometryminorRadius=3;
    m_parentAngle=0;m_parentPosition={20,20,0};}
  void snapShot(int)override{throw ERROR_BAD_ARG;}
  void freeSnapShot(int)override{throw ERROR_BAD_ARG;}
  void updateParentObject(Object* object,PartitionData* data)override{m_parentObject=object;m_partitionData=data;}
};
void factoryCreation(Context& context,bool faults,bool binding=false,bool cameraTracking=false){
  std::unique_ptr<Tree> generatedUser;
  NativeServiceOwners<1> userOwner;
  if(cameraTracking){
    generatedUser=std::make_unique<Tree>();
    userOwner.create(TheNativeUserStorage,[&]{return new NativeUserStorage(
      NativeUserPaths::resolve(generatedUser->path.string(),(generatedUser->path/"data").string(),
        (generatedUser->path/"cache").string()),context.files);});
  }
  context.load(wire());DeviceFixture visual;LogicFixture terrain(visual);logicLoad(terrain,providerWorld(),"GeneratedCreation.map");
  const auto oldTerrain=TheTerrainLogic;TheTerrainLogic=&terrain;
  struct Restore{TerrainLogic* previous;~Restore(){TheTerrainLogic=previous;}} restore{oldTerrain};
  PartitionPublication partition;partition.owner->init();ActorParents parents(true);
  {std::ofstream definitions(context.tree.path/"creation.ini");
    definitions<<"Object GeneratedCreatedActor\n Geometry = BOX\n GeometryMajorRadius = 5\n GeometryMinorRadius = 3\n IsTrainable = Yes\n"
      " Body = InactiveBody GeneratedBody\n End\n Behavior = VeterancyGainCreate GeneratedCreate\n StartingLevel = VETERAN\n End\n End\n";
    if(cameraTracking)definitions<<"GameData\n CameraHeight = 300\n CameraPitch = 45\n CameraYaw = 0\n End\n";
    definitions.close();require(bool(definitions),"generated actual body/create definitions");}
  // Generated files must exist before the read-only mount indexes this tree.
  // No user-storage lease or open input survives here; refresh through the VFS.
  context.files.mountReadOnly({context.tree.path.string()});
  if(cameraTracking)context.data->init(); // Actual GameData parser requires an initialized registered root.
  INI ini;ini.load("creation.ini",INI_LOAD_OVERWRITE,nullptr);
  auto* definition=const_cast<ThingTemplate*>(context.factory->findTemplate("GeneratedCreatedActor",FALSE));
  if(binding){parents.modules.owner->addBindingProbes();
    auto add=[&](ModuleInfo& info,const char* name,ModuleType type,Int interfaces){const AsciiString tag(name);
      auto* data=parents.modules.owner->newModuleDataFromINI(nullptr,name,type,tag);
      require(data,"test-only binding callback data");info.addModuleInfo(definition,name,tag,data,interfaces,FALSE);};
    add(const_cast<ModuleInfo&>(definition->getDrawModuleInfo()),"BindingDrawProbe",MODULETYPE_DRAW,MODULEINTERFACE_DRAW);
    add(const_cast<ModuleInfo&>(definition->getBehaviorModuleInfo()),"BindingBehaviorProbe",MODULETYPE_BEHAVIOR,0);}
  auto make=[&]{return context.factory->newObject(context.factory->findTemplate("GeneratedCreatedActor",FALSE),nullptr,OBJECT_STATUS_MASK_NONE);};
  auto verify=[&](Object* actor){auto* draw=actor->getDrawable();require(actor->getBodyModule() && actor->getExperienceTracker()->getVeterancyLevel()==LEVEL_VETERAN &&
      actor->friend_getPartitionData() && draw && draw->getObject()==actor && parents.client.owner->findDrawableByID(draw->getID())==draw &&
      parents.logic.owner->findObjectByID(actor->getID())==actor,"full source create callback/body/partition/Drawable binding and lookup");};
  auto retire=[&](Object* actor){parents.logic.owner->destroyObject(actor);parents.logic.owner->processDestroyList();};
  auto* survivor=make();verify(survivor);auto* survivorDraw=survivor->getDrawable();auto* survivorPartition=survivor->friend_getPartitionData();
  auto* warm=make();verify(warm);retire(warm);
  if(cameraTracking){
    ScriptCameraFixture camera;camera.init();camera.setFrameLengthMilliseconds(10);
    const Coord3D origin{},east{40,0,0},north{0,40,0};camera.lookAt(&origin);
    survivor->setPosition(&east);camera.rotateCameraTowardObject(survivor->getID(),20,20,0,0);camera.cameraModFreezeTime();
    camera.updateCameraMovements(10);require(!camera.isCameraMovementFinished() && camera.isTimeFrozen(),"actual tracked actor keeps camera freeze active");
    camera.rotateCameraTowardPosition(&origin,100,0,0,FALSE); // Near-coincident facing is an intended no-op, including live tracking/hold.
    survivor->setPosition(&north);camera.updateCameraMovements(10);require(std::abs(camera.getAngle())<0.0001f,"source GameLogic lookup tracks changed actual actor position");
    const auto id=survivor->getID();retire(survivor);require(!parents.logic.owner->findObjectByID(id),"actual actor destruction withdraws lookup");
    camera.updateCameraMovements(10);require(!camera.isCameraMovementFinished() && std::abs(camera.getAngle())<0.0001f,"source hold preserves last valid position after actual actor retirement");
    camera.updateCameraMovements(10);require(camera.isCameraMovementFinished() && !camera.isTimeFrozen(),"tracking plus hold exact retirement clears freeze");
    bool rejected=false;try{camera.rotateCameraTowardObject(id,20,20,0,0);}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG;}
    require(rejected && camera.isCameraMovementFinished(),"missing initial tracked actor rejects without stale target activation");
    auto* retry=make();verify(retry);retry->setPosition(&east);camera.rotateCameraTowardObject(retry->getID(),10,0,0,0);camera.updateCameraMovements(10);
    require(camera.isCameraMovementFinished() && std::abs(camera.getAngle()+PI/2)<0.0001f,"corrected source actor tracking retry");retire(retry);return;
  }
  auto units=[] {const std::array<const char*,17> names{"ObjectPool","SightingInfo","ObjectSMCHelperPool","StatusDamageHelper",
      "SubdualDamageHelper","ObjectDefectionHelperPool","ExperienceTrackerPool","RadarObject","PartitionDataPool","InactiveBody","VeterancyGainCreate","Drawable","TintEnvelope","DrawableLocoInfo","DrawableIconInfo","BindingDrawProbe","BindingBehaviorProbe"};
    std::array<Int,17> result{};for(unsigned n=0;n<names.size();++n)if(auto* pool=TheMemoryPoolFactory->findMemoryPool(names[n]))result[n]=pool->getUsedBlockCount();return result;};
  const auto start=parents.logic.owner->getObjectIDCounter();const auto drawStart=parents.client.owner->getDrawableIDCounter();
  if(binding){for(unsigned phase:{1u,2u}){const auto live=AllocationFault::live();const auto used=units();
      const auto drawCalls=bindingDrawCalls,behaviorCalls=bindingBehaviorCalls,unbindCalls=bindingUnbindCalls;bindingFaultPhase=phase;
      struct Disarm{~Disarm(){bindingFaultPhase=0;}} disarm;bool failed=false;
      try{make();}catch(ErrorCode code){failed=code==ERROR_BAD_ARG;}
      bindingFaultPhase=0;
      require(failed && bindingDrawCalls==drawCalls+1 && bindingBehaviorCalls==behaviorCalls+(phase==2) && bindingUnbindCalls==unbindCalls+1,
        "exact acquisition callback order; complete actor retirement retains source unbind notification");
      require(units()==used && AllocationFault::live()==live && parents.logic.owner->getObjectIDCounter()==start &&
        !parents.logic.owner->findObjectByID(ObjectID(start)) && parents.logic.owner->getFirstObject()==survivor &&
        parents.client.owner->getDrawableList()==survivorDraw,"half-bound candidate rollback retires every owner/registry");
      require(survivor->getDrawable()==survivorDraw && survivorDraw->getObject()==survivor && survivor->friend_getPartitionData()==survivorPartition,"callback failure preserves accepted reciprocal pair and partition");
      auto* retry=make();verify(retry);require(retry->getID()==ObjectID(start) && parents.logic.owner->getObjectIDCounter()==start+1,"corrected binding retry advances real actor cursor");retire(retry);
      require(units()==used && AllocationFault::live()==live,"binding retry retires pooled and standard ownership");
      parents.logic.owner->setObjectIDCounter(start);parents.client.owner->setDrawableIDCounter(drawStart);}
    return;}
  if(!faults){auto* candidate=make();verify(candidate);const auto live=AllocationFault::live();bool rejected=false;
    try{parents.logic.owner->sendObjectCreated(candidate);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
    require(rejected && candidate->getDrawable()->getObject()==candidate && AllocationFault::live()==live,"duplicate creation notification preserves accepted pair");retire(candidate);return;}
  std::size_t count;AllocationFault::arm(SIZE_MAX);Object* discovered=nullptr;
  try{discovered=make();}catch(...){AllocationFault::disarm();throw;}count=AllocationFault::attempts();AllocationFault::disarm();
  verify(discovered);retire(discovered);parents.logic.owner->setObjectIDCounter(start);parents.client.owner->setDrawableIDCounter(drawStart);
  require(count>2,"complete creation census includes callback/partition/Drawable acquisitions");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();const auto used=units();Object* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=make();}catch(const std::bad_alloc&){failed=true;}
    catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"source creation allocation error class");failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count),"every full source creation prefix and exact terminal");
    if(failed)require(!parents.logic.owner->findObjectByID(ObjectID(start)) && parents.logic.owner->getObjectIDCounter()==start &&
      parents.logic.owner->getFirstObject()==survivor && parents.client.owner->getDrawableList()==survivorDraw && AllocationFault::live()==live && units()==used,"late creation failure retires candidate actor/partition/Drawable registration");
    else{verify(candidate);require(candidate->getID()==ObjectID(start) && parents.logic.owner->getObjectIDCounter()==start+1,"terminal proves production actor cursor advancement");retire(candidate);}
    require(survivor->getDrawable()==survivorDraw && survivorDraw->getObject()==survivor && survivor->friend_getPartitionData()==survivorPartition,"creation failure preserves actual accepted survivor");
    candidate=make();verify(candidate);require(candidate->getID()==ObjectID(start+(failed?0:1)),"corrected same-root creation uses valid source cursor");retire(candidate);
    require(AllocationFault::live()==live && units()==used && parents.logic.owner->getFirstObject()==survivor && parents.client.owner->getDrawableList()==survivorDraw,"every corrected complete creation retires all standard/pooled ownership");
    parents.logic.owner->setObjectIDCounter(start);parents.client.owner->setDrawableIDCounter(drawStart);}
  for(const char* name:{"InactiveBody","VeterancyGainCreate","Drawable"}){
    auto* pool=TheMemoryPoolFactory->findMemoryPool(name);require(pool,"actual body/create/Drawable pool");std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} release{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated full creation exhaustion"));
    const auto used=units();const auto live=AllocationFault::live();bool rejected=false;
    try{make();}catch(ErrorCode code){rejected=code==ERROR_OUT_OF_MEMORY;}
    require(rejected && units()==used && AllocationFault::live()==live && parents.logic.owner->getObjectIDCounter()==start &&
      parents.logic.owner->getFirstObject()==survivor && parents.client.owner->getDrawableList()==survivorDraw,"actual late pooled exhaustion retires candidate and preserves survivor");
    for(auto* value:held)pool->freeBlock(value);held.clear();auto* candidate=make();verify(candidate);retire(candidate);
    parents.logic.owner->setObjectIDCounter(start);parents.client.owner->setDrawableIDCounter(drawStart);}
  std::cout<<"full creation prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; child/root/frame parity pending\n";
}
void partitionGhostAttachment(Context& context){
  context.load(wire());DeviceFixture visual;LogicFixture terrain(visual);logicLoad(terrain,providerWorld(),"GeneratedGhostPartition.map");
  const auto oldTerrain=TheTerrainLogic;TheTerrainLogic=&terrain;
  struct Restore{TerrainLogic* previous;~Restore(){TheTerrainLogic=previous;}} restore{oldTerrain};
  const auto oldSize=context.data->m_partitionCellSize;context.data->m_partitionCellSize=10;
  struct RestoreSize{Real& slot;Real value;~RestoreSize(){slot=value;}} size{context.data->m_partitionCellSize,oldSize};
  PartitionPublication partition;partition.owner->init();GhostAttachment ghost;
  auto retire=[&]{partition.owner->unRegisterGhostObject(&ghost);ghost.updateParentObject(nullptr,nullptr);};
  struct Retire{decltype(retire)& action;~Retire(){action();}} cleanup{retire};
  partition.owner->registerGhostObject(&ghost);ghost.friend_getPartitionData()->friend_updateCellsTouched();retire();
  auto* pool=TheMemoryPoolFactory->findMemoryPool("PartitionDataPool");require(pool,"actual ghost partition data pool");
  std::size_t count;AllocationFault::arm(SIZE_MAX);
  try{partition.owner->registerGhostObject(&ghost);}catch(...){AllocationFault::disarm();throw;}
  count=AllocationFault::attempts();AllocationFault::disarm();retire();require(count==1,"parallel ghost attachment exact COI terminal");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto used=pool->getUsedBlockCount();const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{partition.owner->registerGhostObject(&ghost);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count),"parallel ghost attachment every failure prefix/terminal");
    if(failed)require(!ghost.friend_getPartitionData() && pool->getUsedBlockCount()==used && AllocationFault::live()==live,"failed ghost attachment retires backing/dirty/master-list ownership");
    else retire();
    partition.owner->registerGhostObject(&ghost);auto* data=ghost.friend_getPartitionData();data->friend_updateCellsTouched();
    require(data->friend_getCoiInUseCount()>0,"corrected ghost attachment populates actual intersections");retire();
    require(pool->getUsedBlockCount()==used && AllocationFault::live()==live,"parallel ghost corrected retry retires exactly");}
  std::cout<<"ghost attachment prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; full fog pending\n";
}
void partitionGrid(Context& context,bool faults){
  context.load(wire());DeviceFixture visual;LogicFixture terrain(visual);
  logicLoad(terrain,providerWorld(),"GeneratedPartition.map");
  const auto oldTerrain=TheTerrainLogic;TheTerrainLogic=&terrain;
  struct RestoreTerrain{TerrainLogic* previous;~RestoreTerrain(){TheTerrainLogic=previous;}} restore{oldTerrain};
  const Real oldSize=context.data->m_partitionCellSize;context.data->m_partitionCellSize=10;
  struct RestoreSize{Real& slot;Real previous;~RestoreSize(){slot=previous;}} size{context.data->m_partitionCellSize,oldSize};
  auto check=[](PartitionManager& owner){require(owner.getCellCountX()==5 && owner.getCellCountY()==5 && owner.getCellAt(4,4) &&
    !owner.getCellAt(5,0) && !owner.getCellAt(-1,0),"actual generated terrain admits bounded nonempty original partition cells");};
  if(!faults){PartitionPublication partition;auto& owner=*partition.owner;
    for(unsigned n=0;n<3;++n){owner.init();check(owner);const auto* backing=owner.getCellAt(0,0);bool rejected=false;
      try{owner.init();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
      require(rejected && owner.getCellAt(0,0)==backing,"duplicate grid admission preserves accepted cells");owner.reset();
      require(!owner.getCellAt(0,0),"reset withdraws grid backing");}
    for(Real bad:{std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity()}){
      context.data->m_partitionCellSize=bad;bool rejected=false;try{owner.init();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
      require(rejected && !owner.getCellAt(0,0),"nonfinite grid profile rejects before backing");
      context.data->m_partitionCellSize=10;owner.init();check(owner);owner.reset();}
    context.data->m_partitionCellSize=0;owner.init();require(owner.getCellSize()==1 && owner.getCellCountX()==50,"source minimum cell-size behavior retained");
    owner.reset();return;}
  std::size_t count;
  {PartitionPublication discovery;AllocationFault::arm(SIZE_MAX);
    try{discovery.owner->init();}catch(...){AllocationFault::disarm();throw;}
    count=AllocationFault::attempts();AllocationFault::disarm();check(*discovery.owner);}
  require(count>1,"grid census includes cells and real radius-vector backing");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto outside=AllocationFault::live();
    {PartitionPublication partition;auto& owner=*partition.owner;const auto live=AllocationFault::live();bool failed=false;
      AllocationFault::arm(ordinal);try{owner.init();}catch(const std::bad_alloc&){failed=true;}
      catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal<count),"every cold grid prefix and exact terminal");
      if(failed)require(!owner.getCellAt(0,0) && owner.getCellSize()==0 && AllocationFault::live()==live,"failed grid retires all cells/radius backing");
      else{check(owner);owner.reset();}
      owner.init();check(owner);owner.reset();require(!owner.getCellAt(0,0),"same-owner corrected grid retry and reset");}
    require(AllocationFault::live()==outside,"whole grid owner retires legitimate retained radius capacity");}
  std::cout<<"partition grid prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void terrainLogic(Context& context){context.load(wire());DeviceFixture visual;LogicFixture logic(visual);TerrainLogic& api=logic;
  const auto valid=providerWorld();Coord3D normal;Region3D region;
  require(api.getGroundHeight(0,0,&normal)==0 && normal.z==1 && !api.isClearLineOfSight({0,0,0},{10,0,0}),"actual unloaded native source query defaults");
  api.init();logicLoad(logic,valid,"GeneratedAccepted.map");check(context);
  require(logic.getFirstWaypoint() && logic.getFirstWaypoint()->getLocation()->z==0,"initial source graph uses absent prior visual grid");
  require(visual.heightMap().width()==7 && logic.logicalMetadata().width()==7 && logic.deviceCalls==1,"complete original world plus retained visual admission");
  require(context.data->m_timeOfDay==TIME_OF_DAY_AFTERNOON && visual.heightMap().shadowColor()==0x12345678u &&
    context.data->m_terrainAmbient[0].red==context.data->m_terrainLighting[TIME_OF_DAY_AFTERNOON][0].ambient.red,"source selected active light row and authored shadow color publish with terrain");
  api.getExtent(&region);require(region.lo.x==0 && region.hi.x==50 && region.hi.y==50 && region.lo.z==12.5f && region.hi.z==13.75f,"original boundary and logical cached extrema");
  api.getMaximumPathfindExtent(&region);require(region.hi.x==50 && region.hi.y==50,"maximum authored pathfind boundary");
  region.lo.z=123;region.hi.z=456;api.getExtentIncludingBorder(&region);
  require(region.lo.x==-10 && region.hi.x==60 && region.lo.z==123 && region.hi.z==456,"original border query preserves caller Z");
  const Real oldGround=api.getGroundHeight(10,20);logicLoad(logic,valid,"GeneratedReplacement.map");
  require(logic.getFirstWaypoint()->getLocation()->z==oldGround,"initial source graph uses previously accepted retained ground before replacement");
  require(api.isCliffCell(0,0) && api.getLayerHeight(10,20,LAYER_GROUND)==oldGround &&
    api.getLayerHeight(10,20,static_cast<PathfindLayerEnum>(LAYER_GROUND+1))==oldGround,"actual cliff and absent bridge layer semantics");
  const ICoord2D position{0,0};visual.setRawMapHeight(&position,1);api.getExtent(&region);
  require(region.lo.z==12.5f && region.hi.z==13.75f && api.getGroundHeight(0,0)==0.625f,"deformation leaves original logical cached extent extrema");
  const auto* visualBacking=visual.heightMap().bytes().data();const auto dirty=visual.dirty;
  logicLoad(logic,wire(),"GeneratedQuery.map",TRUE);
  require(visual.heightMap().bytes().data()==visualBacking && visual.filename()=="GeneratedReplacement.map" && visual.dirty==dirty && api.getSourceFilename()=="GeneratedQuery.map","source query mode never loads retained visual terrain");
  bool failed=false;try{api.getLayerHeight(0,0,LAYER_WALL);}catch(ErrorCode){failed=true;}
  require(failed,"wall layer requires actual AI/pathfinder, not a ground-only substitute");
  struct WallAI:AI {void setWallHeight(Real height){m_aiData->m_wallHeight=height;pathfinder()->reset();}};
  struct PublishedWall {
    AI* previous=TheAI;std::unique_ptr<WallAI> owner=std::make_unique<WallAI>();
    PublishedWall(){TheAI=owner.get();owner->init();}
    ~PublishedWall(){owner.reset();TheAI=previous;}
  } wall;
  wall.owner->setWallHeight(40);
  require(api.getLayerHeight(0,0,LAYER_WALL,nullptr,FALSE)==40 &&
    api.getLayerHeight(0,0,LAYER_WALL,nullptr,TRUE)==api.getGroundHeight(0,0),
    "actual source AI wall height and clipped empty-wall ground fallback");
}
void terrainLogicNegative(Context& context){context.load(wire());DeviceFixture visual;LogicFixture logic(visual);const auto valid=providerWorld();logicLoad(logic,valid,"GeneratedAccepted.map");
  auto reject=[&](const std::string& bytes,bool deviceFailure=false){const auto* objects=MapObject::getFirstMapObject();const auto* triggers=PolygonTrigger::getFirstPolygonTrigger();
    const auto* script=context.sides.getSideInfo(0)->getScriptList();const auto* graph=logic.getFirstWaypoint();const auto* backing=visual.heightMap().bytes().data();
    const auto* metadata=logic.logicalMetadata().bytes().data();const auto dirty=visual.dirty;const auto live=AllocationFault::live();const auto id=InspectRead::id();
    const NativeTerrainLightingState lights(*context.data);bool failed=false;logic.failDevice=deviceFailure;
    try{logicLoad(logic,bytes,"GeneratedRejected.map");}catch(ErrorCode){failed=true;}logic.failDevice=false;
    require(failed && MapObject::getFirstMapObject()==objects && PolygonTrigger::getFirstPolygonTrigger()==triggers && context.sides.getSideInfo(0)->getScriptList()==script &&
      logic.getFirstWaypoint()==graph && visual.heightMap().bytes().data()==backing && logic.logicalMetadata().bytes().data()==metadata && visual.dirty==dirty &&
      logic.getSourceFilename()=="GeneratedAccepted.map" && visual.filename()=="GeneratedAccepted.map" && InspectRead::id()==id && AllocationFault::live()==live,"all integrated late owners roll back together");
    require(sameLighting(NativeTerrainLightingState(*context.data),lights) && visual.heightMap().shadowColor()==0x12345678u,"all authored/active terrain light rows and shadow restore after late failure");
    logicLoad(logic,valid,"GeneratedAccepted.map");check(context);};
  reject(wire()); // Complete logical map, but missing mandatory retained blend data.
  reject(providerWorld("ReplacementSide"),true); // Failure after candidate provider and graph publication.
  for(std::size_t n=0;n<valid.size();++n)reject(valid.substr(0,n));
  bool lateCalled=false;
  auto late=[](NativeTerrainLogic& candidate,void* raw){auto& called=*static_cast<bool*>(raw);called=true;
    Region3D bounds;candidate.getExtent(&bounds);require(bounds.hi.x==50 && candidate.getFirstWaypoint(),"late hook observes candidate source dimensions/graph");
    throw ERROR_CORRUPT_FILE_FORMAT;};
  const auto* objects=MapObject::getFirstMapObject();const auto* backing=visual.heightMap().bytes().data();bool failed=false;
  try{logicLoad(logic,providerWorld("ReplacementSide"),"GeneratedLate.map",FALSE,late,&lateCalled);}catch(ErrorCode){failed=true;}
  require(failed && lateCalled && MapObject::getFirstMapObject()==objects && visual.heightMap().bytes().data()==backing,"outer native hook retains complete world/provider rollback");
  logicLoad(logic,valid,"GeneratedAccepted.map");check(context);
}
void terrainLogicFaults(Context& context){context.load(wire());DeviceFixture visual;LogicFixture logic(visual);const auto valid=providerWorld(),replacement=providerWorld("ReplacementSide");
  const AsciiString accepted("GeneratedAccepted.map"),candidate("GeneratedCandidate.map");logicLoad(logic,valid,accepted);ScriptList::IDTransaction stable;
  std::size_t census;
  {ScriptList::IDTransaction start;AllocationFault::arm(SIZE_MAX);try{logicLoad(logic,replacement,candidate);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  logicLoad(logic,valid,accepted);
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){ScriptList::IDTransaction start;const auto live=AllocationFault::live();const PoolUsage pools;
    const auto* objects=MapObject::getFirstMapObject();const auto* graph=logic.getFirstWaypoint();const auto* backing=visual.heightMap().bytes().data();const auto dirty=visual.dirty;
    const NativeTerrainLightingState lights(*context.data);auto* waypointPool=TheMemoryPoolFactory->findMemoryPool("Waypoint");const auto waypointUnits=waypointPool->getUsedBlockCount();
    bool failed=false;AllocationFault::arm(ordinal);try{logicLoad(logic,replacement,candidate);}catch(const std::bad_alloc&){failed=true;}
    catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"integrated original owner failure classification");failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<census),"complete integrated terrain/world manifest and exact terminal");
    if(failed)require(MapObject::getFirstMapObject()==objects && logic.getFirstWaypoint()==graph && visual.heightMap().bytes().data()==backing && visual.dirty==dirty &&
      logic.getSourceFilename()==accepted && visual.filename()==accepted && sameLighting(NativeTerrainLightingState(*context.data),lights) &&
      AllocationFault::live()==live && pools.matches() && waypointPool->getUsedBlockCount()==waypointUnits,"every terrain/world acquisition preserves accepted original owners");
    logicLoad(logic,replacement,candidate);check(context,"ReplacementSide");
    require(context.data->m_timeOfDay==TIME_OF_DAY_NIGHT && visual.heightMap().shadowColor()==0x89abcdefu,"corrected replacement admits distinct lighting/shadow");
    logicLoad(logic,valid,accepted);check(context);require(AllocationFault::live()==live && pools.matches() && waypointPool->getUsedBlockCount()==waypointUnits &&
      sameLighting(NativeTerrainLightingState(*context.data),lights),"every corrected integrated retry retires candidate ownership");
  }std::cout<<"terrain-world prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}
struct PublishedMap {
  AI* previousAI=TheAI;GameLogic* previousSimulation=TheGameLogic;
  TerrainLogic* previousLogic=TheTerrainLogic;TerrainVisual* previousVisual=TheTerrainVisual;
  std::unique_ptr<GameLogic> simulation=std::make_unique<GameLogic>();
  std::unique_ptr<AI> ai=std::make_unique<AI>();
  PublishedMap(LogicFixture& logic,DeviceFixture& visual){TheGameLogic=simulation.get();TheAI=ai.get();TheTerrainLogic=&logic;TheTerrainVisual=&visual;ai->init();}
  ~PublishedMap(){ai.reset();simulation.reset();TheAI=previousAI;TheGameLogic=previousSimulation;TheTerrainLogic=previousLogic;TheTerrainVisual=previousVisual;}
};
void checkPathfindMap(Pathfinder& map){unsigned count=0,cliffs=0,clear=0;
  for(Int x=0;x<5;++x)for(Int y=0;y<5;++y){auto* cell=map.getCell(LAYER_GROUND,x,y);require(cell && cell->getZone(),"actual original source cell classification and zones");++count;
    cliffs+=cell->getType()==PathfindCell::CELL_CLIFF;clear+=cell->getType()==PathfindCell::CELL_CLEAR;}
  require(count==25 && cliffs && clear && !map.getCell(LAYER_GROUND,-1,0) && !map.getCell(LAYER_GROUND,5,0),"actual source mixed terrain and exact grid extent");
}
void pathfinderMap(Context& context){context.load(wire());DeviceFixture visual;LogicFixture logic(visual);logicLoad(logic,providerWorld(),"GeneratedPathfinder.map");
  PublishedMap owner(logic,visual);auto& map=*owner.ai->pathfinder();require(!map.getCell(LAYER_GROUND,0,0),"no grid before actual admission");
  map.newMap();checkPathfindMap(map);const auto* first=map.getCell(LAYER_GROUND,0,0);const auto live=AllocationFault::live();
  AllocationFault::arm(0);try{map.newMap();}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(!hit && attempts==0 && map.getCell(LAYER_GROUND,0,0)==first && AllocationFault::live()==live,"source two-pass reuse has exact zero-allocation terminal");checkPathfindMap(map);
  owner.ai->reset();require(!map.getCell(LAYER_GROUND,0,0),"reset retires admitted grid");map.newMap();checkPathfindMap(map);
}
void pathfinderMapFaults(Context& context){context.load(wire());DeviceFixture visual;LogicFixture logic(visual);logicLoad(logic,providerWorld(),"GeneratedPathfinder.map");
  PublishedMap owner(logic,visual);auto& map=*owner.ai->pathfinder();std::size_t count;
  AllocationFault::arm(SIZE_MAX);try{map.newMap();}catch(...){AllocationFault::disarm();throw;}count=AllocationFault::attempts();AllocationFault::disarm();checkPathfindMap(map);owner.ai->reset();
  const auto baseline=AllocationFault::live();const PoolUsage pools;
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){bool failed=false;AllocationFault::arm(ordinal);
    try{map.newMap();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<count),"complete actual map admission manifest and exact terminal");
    if(failed)require(!map.getCell(LAYER_GROUND,0,0) && AllocationFault::live()==baseline && pools.matches(),"failed classification retires incomplete ground, blocks, layers and zone tables");
    else checkPathfindMap(map);
    map.newMap();checkPathfindMap(map);owner.ai->reset();require(AllocationFault::live()==baseline && pools.matches(),"every corrected actual map retry retires ownership exactly");
  }
  std::cout<<"actual pathfinder map prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void poolFailures(Context& context){const auto valid=wire(),replacement=wire("ReplacementSide");context.load(valid);
  for(const char* name:poolNames){auto* pool=TheMemoryPoolFactory->findMemoryPool(name);
    std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated whole-world pool exhaustion"));
    auto* objects=MapObject::getFirstMapObject();auto* trigger=PolygonTrigger::getFirstPolygonTrigger();
    auto* script=context.sides.getSideInfo(0)->getScriptList();const auto id=InspectRead::id();const auto live=AllocationFault::live();
    const PoolUsage pools;bool rejected=false;
    try{context.load(replacement);}catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"source whole-world pooled failure");rejected=true;}
    require(rejected && pools.matches() && AllocationFault::live()==live && MapObject::getFirstMapObject()==objects &&
      PolygonTrigger::getFirstPolygonTrigger()==trigger && context.sides.getSideInfo(0)->getScriptList()==script &&
      InspectRead::id()==id,"every exhausted owner preserves complete world and retires partial units");check(context);
    for(auto* block:held)pool->freeBlock(block);held.clear();context.load(replacement);check(context,"ReplacementSide");
    context.load(valid);check(context);std::cout<<"world pool "<<name<<" failure and corrected retry\n";
  }
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"world family");initMemoryManager();initialized=true;
    if(std::string_view(argv[1]).starts_with("factory-creation")){
      TheMemoryPoolFactory->createMemoryPool("InactiveBody",sizeof(InactiveBody),32,0);
      TheMemoryPoolFactory->createMemoryPool("VeterancyGainCreate",sizeof(VeterancyGainCreate),32,0);
      TheMemoryPoolFactory->createMemoryPool("Drawable",sizeof(Drawable),32,0);}
    if(std::string_view(argv[1])=="partition-actors")TheMemoryPoolFactory->createMemoryPool("PartitionDataPool",sizeof(PartitionData),8,0);
    if(std::string_view(argv[1])=="pool-failures")fixedPools();
    {Context warm;warm.load(wire());warm.load(wire("ReplacementSide"));
      const auto bytes=wire();Input input(bytes);NativeWaypointGraph graph;
      graph.load(input,MapObject::getFirstMapObject());if(std::string_view(argv[1])=="partition-actors"){partitionActors(warm);partitionActors(warm);}
      if(std::string_view(argv[1])=="partition-ghost-attachment")partitionGhostAttachment(warm);}
    // Module definitions borrow their registry's data: warm complete roots,
    // not a template graph retained after its module-data owner was retired.
    if(std::string_view(argv[1]).starts_with("factory-creation"))for(unsigned repeat=0;repeat<2;++repeat){Context warm;factoryCreation(warm,false,
      std::string_view(argv[1])=="factory-creation-binding",std::string_view(argv[1])=="factory-creation-camera");}
    for(unsigned repeat=0;repeat<3;++repeat){auto live=AllocationFault::live();const PoolUsage pools;
      {Context context;const std::string_view family=argv[1];if(family=="functional")functional(context);
        else if(family=="negative")negative(context);else if(family=="faults"){faults(context);optionalFaults(context);}
        else if(family=="ownership")ownership(context);else if(family=="pool-failures")poolFailures(context);
        else if(family=="continuation")continuation(context);else if(family=="terrain")terrainLogic(context);
        else if(family=="terrain-negative")terrainLogicNegative(context);else if(family=="terrain-faults")terrainLogicFaults(context);
        else if(family=="pathfinder-map")pathfinderMap(context);else if(family=="pathfinder-map-faults")pathfinderMapFaults(context);
        else if(family=="partition-grid")partitionGrid(context,false);else if(family=="partition-grid-faults")partitionGrid(context,true);
        else if(family=="partition-actors")partitionActors(context);
        else if(family=="partition-ghost-attachment")partitionGhostAttachment(context);
        else if(family=="factory-creation")factoryCreation(context,false);else if(family=="factory-creation-faults")factoryCreation(context,true);
        else if(family=="factory-creation-binding")factoryCreation(context,false,true);
        else if(family=="factory-creation-camera")factoryCreation(context,false,false,true);
        else require(false,"unknown world family");}
      require(AllocationFault::live()==live && pools.matches() && !TheThingFactory && !TheSidesList && !TheNameKeyGenerator &&
        !MapObject::getFirstMapObject() && !PolygonTrigger::getFirstPolygonTrigger(),"repeated full-world owner retirement");}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original pool metadata retirement");
    std::cout<<"PASS actual logical world; native startup still pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL source world "<<Int(error)<<'\n';}
  catch(...){std::cerr<<"FAIL source world\n";}AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
