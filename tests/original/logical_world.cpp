// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/ThingFactory.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/NativeLogicalWorld.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
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
                 Real angle=0.5f,Int weather=WEATHER_SNOWY,unsigned omit=0,unsigned duplicate=0,bool heightLast=false){
  Output output;DataChunkOutput out(&output);
  if(!(omit&1) && !heightLast)height(out,heightVersion);
  if(!(omit&2)){out.openDataChunk("WorldInfo",1);Dict dict;dict.setInt(TheKey_weather,weather);out.writeDict(dict);out.closeDataChunk();}
  if(!(omit&8))sides(out,name);
  if(!(omit&4))objects(out,objectVersion,angle);
  if(!(omit&16))triggers(out);
  if(duplicate==1)height(out,heightVersion);if(duplicate==2){out.openDataChunk("WorldInfo",1);out.writeDict(Dict());out.closeDataChunk();}
  if(duplicate==4)objects(out,objectVersion,angle);if(duplicate==8)sides(out,name);if(duplicate==16)triggers(out);
  if(!(omit&1) && heightLast)height(out,heightVersion);
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
    if(std::string_view(argv[1])=="pool-failures")fixedPools();
    {Context warm;warm.load(wire());warm.load(wire("ReplacementSide"));}
    for(unsigned repeat=0;repeat<3;++repeat){auto live=AllocationFault::live();const PoolUsage pools;
      {Context context;const std::string_view family=argv[1];if(family=="functional")functional(context);
        else if(family=="negative")negative(context);else if(family=="faults"){faults(context);optionalFaults(context);}
        else if(family=="ownership")ownership(context);else if(family=="pool-failures")poolFailures(context);else require(false,"unknown world family");}
      require(AllocationFault::live()==live && pools.matches() && !TheThingFactory && !TheSidesList && !TheNameKeyGenerator &&
        !MapObject::getFirstMapObject() && !PolygonTrigger::getFirstPolygonTrigger(),"repeated full-world owner retirement");}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original pool metadata retirement");
    std::cout<<"PASS actual logical world; native startup still pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL source world "<<Int(error)<<'\n';}
  catch(...){std::cerr<<"FAIL source world\n";}AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
