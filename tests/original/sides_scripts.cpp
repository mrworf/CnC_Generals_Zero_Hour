// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original side/script owners and template registry, not simulation acceptance.
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/KindOf.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include <algorithm>
#include <bit>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Retire{template<class T>void operator()(T* p)const noexcept{if(p)p->deleteInstance();}};
template<class T>using Owner=std::unique_ptr<T,Retire>;
struct InspectRead:ScriptList{static Int id(){return m_curId;}static Int pending(){return s_numInReadList;}
  static ScriptList* first(){return s_readLists[0];}};
constexpr const char* poolNames[]{"BuildListInfo","ScriptList","ScriptGroup","Script","OrCondition","Condition","ScriptAction","Parameter","NameKeyBucketPool"};
struct PoolUsage {
  std::array<Int,std::size(poolNames)> used{};
  PoolUsage(){for(std::size_t i=0;i<used.size();++i){auto* pool=TheMemoryPoolFactory->findMemoryPool(poolNames[i]);
    require(pool,"original fixture pool exists");used[i]=pool->getUsedBlockCount();}}
  bool matches()const{for(std::size_t i=0;i<used.size();++i)
    if(TheMemoryPoolFactory->findMemoryPool(poolNames[i])->getUsedBlockCount()!=used[i])return false;return true;}
};
struct Tree{std::filesystem::path path;
  Tree(){char pattern[]="/tmp/zh-side-scripts-XXXXXX";const auto* created=::mkdtemp(pattern);require(created,"generated empty root");path=created;}
  ~Tree(){std::error_code error;std::filesystem::remove_all(path,error);}
};
struct Context {
  Tree tree;FileSystem files;NameKeyGenerator names;SidesList sides;
  std::unique_ptr<GlobalData> data;std::unique_ptr<ScriptEngine> engine;
  FileSystem* previousFiles=TheFileSystem;NameKeyGenerator* previousNames=TheNameKeyGenerator;
  GlobalData* previousData=TheWritableGlobalData;ScriptEngine* previousEngine=TheScriptEngine;SidesList* previousSides=TheSidesList;
  ScriptList::ReadState previousRead;
  Context(){files.mountReadOnly({tree.path.string()});names.init();TheFileSystem=&files;TheNameKeyGenerator=&names;
    TheSidesList=&sides;ScriptList::exchangeReadState(previousRead);
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();
      engine=std::make_unique<ScriptEngine>();TheScriptEngine=engine.get();engine->init();}
    catch(...){sides.clear();engine.reset();data.reset();restore();throw;}}
  void restore()noexcept{ScriptList::exchangeReadState(previousRead);TheSidesList=previousSides;TheScriptEngine=previousEngine;
    TheWritableGlobalData=previousData;TheNameKeyGenerator=previousNames;TheFileSystem=previousFiles;}
  ~Context(){sides.clear();engine.reset();data.reset();restore();}
};
struct MemoryInput:ChunkInputStream{const std::string& bytes;UnsignedInt position=0;
  explicit MemoryInput(const std::string& input):bytes(input){}
  Int read(void* out,Int count)override{if(count<0 || (!out && count))throw ERROR_BAD_ARG;
    const auto size=std::min<std::size_t>(count,bytes.size()-position);if(size)std::memcpy(out,bytes.data()+position,size);position+=size;return size;}
  UnsignedInt tell()override{return position;}
  Bool absoluteSeek(UnsignedInt value)override{if(value>bytes.size())return FALSE;position=value;return TRUE;}
  Bool eof()override{return position==bytes.size();}
};
struct MemoryOutput:OutputStream{std::string bytes;Int write(const void* data,Int size)override{bytes.append(static_cast<const char*>(data),size);return size;}};
void raw(std::string& out,UnsignedInt value,unsigned width=4){for(unsigned n=0;n<width;++n)out+=char(value>>(8*n));}
void overwrite(std::string& out,std::size_t at,UnsignedInt value){for(unsigned n=0;n<4;++n)out.at(at+n)=char(value>>(8*n));}
void parameter(DataChunkOutput& out,Parameter::ParameterType type,const char* text="GeneratedParameter",Int value=1){
  out.writeInt(type);if(type==Parameter::COORD3D){out.writeReal(1);out.writeReal(2);out.writeReal(3);}
  else{out.writeInt(value);out.writeReal(0.5f);out.writeAsciiString(text);}}
void condition(DataChunkOutput& out,Condition::ConditionType type,unsigned version=4,
  const std::vector<Parameter::ParameterType>& parameters={},Int numeric=-1,const char* key=nullptr){
  out.openDataChunk("Condition",version);out.writeInt(numeric==-1 ? Int(type) : numeric);
  if(version>=4){out.writeNameKey(key ? TheNameKeyGenerator->nameToKey(key) : TheScriptEngine->getConditionTemplate(type)->m_internalNameKey);}
  out.writeInt(parameters.size());for(auto param:parameters)parameter(out,param);out.closeDataChunk();}
void action(DataChunkOutput& out,ScriptAction::ScriptActionType type,unsigned version=2,
  const std::vector<Parameter::ParameterType>& parameters={},bool negative=false,Int numeric=-1,const char* key=nullptr){
  out.openDataChunk(negative ? "ScriptActionFalse" : "ScriptAction",version);out.writeInt(numeric==-1 ? Int(type) : numeric);
  if(version>=2){out.writeNameKey(key ? TheNameKeyGenerator->nameToKey(key) : TheScriptEngine->getActionTemplate(type)->m_internalNameKey);}
  out.writeInt(parameters.size());for(auto param:parameters)parameter(out,param);out.closeDataChunk();}
void script(DataChunkOutput& out,const char* name="GeneratedScript",bool legacy=false,unsigned version=2){
  out.openDataChunk("Script",version);out.writeAsciiString(name);out.writeAsciiString("GeneratedComment");
  out.writeAsciiString("ConditionComment");out.writeAsciiString("ActionComment");
  for(Bool value:{true,false,true,false,true,false})out.writeByte(value);
  if(version>=2)out.writeInt(3);
  out.openDataChunk("OrCondition",1);
  condition(out,legacy ? Condition::TEAM_INSIDE_AREA_PARTIALLY : Condition::FLAG,legacy ? 1 : 4,
    legacy ? std::vector{Parameter::TEAM,Parameter::TRIGGER_AREA} : std::vector{Parameter::FLAG,Parameter::BOOLEAN});
  condition(out,Condition::CONDITION_TRUE,legacy ? 3 : 4);out.closeDataChunk();
  action(out,legacy ? ScriptAction::TEAM_FOLLOW_WAYPOINTS : ScriptAction::SET_FLAG,legacy ? 1 : 2,
    legacy ? std::vector{Parameter::TEAM,Parameter::WAYPOINT_PATH} : std::vector{Parameter::FLAG,Parameter::BOOLEAN});
  action(out,ScriptAction::SET_COUNTER,legacy ? 1 : 2,{Parameter::COUNTER,Parameter::INT},true);
  out.closeDataChunk();}
using ScriptWriter=std::function<void(DataChunkOutput&)>;
void customScript(DataChunkOutput& out,const ScriptWriter& body){out.openDataChunk("Script",2);
  for(const char* value:{"GeneratedDirectScript","GeneratedComment","ConditionComment","ActionComment"})out.writeAsciiString(value);
  for(Bool value:{true,false,true,false,true,false})out.writeByte(value);out.writeInt(3);body(out);out.closeDataChunk();}
std::string wire(unsigned version=3,const char* name="AcceptedSide",bool legacy=false,Int builds=2,const ScriptWriter& custom={}){
  MemoryOutput output;DataChunkOutput out(&output);out.openDataChunk("SidesList",version);out.writeInt(2);
  for(unsigned side=0;side<2;++side){Dict dict;dict.setAsciiString(TheKey_playerName,side==0 ? name : "GeneratedOtherSide");
    dict.setAsciiString(TheKey_playerAllies,"GeneratedOtherSide MissingSide");out.writeDict(dict);out.writeInt(builds);
    for(Int n=0;n<builds;++n){out.writeAsciiString("GeneratedBuilding");out.writeAsciiString("GeneratedTemplate");
      out.writeReal(10+n);out.writeReal(20);out.writeReal(99);out.writeReal(0.5f);out.writeByte(true);out.writeInt(-1);
      if(version>=3){out.writeAsciiString("GeneratedBuildScript");out.writeInt(85);out.writeByte(false);out.writeByte(true);out.writeByte(false);}}}
  if(version>=2){out.writeInt(1);Dict dict;dict.setAsciiString(TheKey_teamName,"GeneratedAuthoredTeam");
    dict.setAsciiString(TheKey_teamOwner,"MissingSide");out.writeDict(dict);}
  out.openDataChunk("PlayerScriptsList",5); // Actual source writer enum value, despite the VERSION_1 name.
  for(unsigned side=0;side<2;++side){out.openDataChunk("ScriptList",1);
    if(custom)customScript(out,custom);else script(out,"GeneratedDirectScript",legacy);
    out.openDataChunk("ScriptGroup",2);out.writeAsciiString("GeneratedGroup");out.writeByte(true);out.writeByte(true);
    script(out,"GeneratedGroupedScript",legacy,1);out.closeDataChunk();out.closeDataChunk();}
  out.closeDataChunk();out.closeDataChunk();out.finish();return output.bytes;}
void load(const std::string& bytes){MemoryInput stream(bytes);DataChunkInput input(&stream);
  input.registerParser("SidesList",AsciiString::TheEmptyString,SidesList::ParseSidesDataChunk);
  if(!input.isValidFileType() || !input.parse() || !stream.eof())throw ERROR_CORRUPT_FILE_FORMAT;}
void check(Context& context,const char* name="AcceptedSide",unsigned version=3,bool legacy=false,Int builds=2){
  require(TheSidesList==&context.sides && context.sides.getNumSides()==3 &&
    context.sides.getSideInfo(0)->getDict()->getAsciiString(TheKey_playerName)==name,"accepted original side/neutral publication");
  require(context.sides.findSideInfo("") && context.sides.findTeamInfo("team") &&
    context.sides.getNumTeams()==(version>=2 ? 4 : 3),"original neutral/default-team repair");
  auto* build=context.sides.getSideInfo(0)->getBuildList();require(build && build->getLocation()->z==0 &&
    build->getLocation()->x==10 && build->getNumRebuilds()==0xffffffffu && build->getAngle()==0.5f &&
    build->getHealth()==(version>=3 ? 85 : 100) && build->getUnsellable()==(version>=3),"source build values/defaults and fixed-width rebuild sentinel");
  Int count=0;for(auto* node=build;node;node=node->getNext())++count;require(count==builds,"complete original build list");
  auto* list=context.sides.getSideInfo(0)->getScriptList();require(list && list->getScript() && list->getScriptGroup(),"actual nonempty source scripts/groups");
  auto* current=list->getScript();auto* predicate=current->getOrCondition()->getFirstAndCondition();
  require(current->getName()=="GeneratedDirectScript" && current->getDelayEvalSeconds()==3 &&
    current->isActive() && !current->isOneShot() && !current->isNormal() &&
    current->getFalseAction()->getActionType()==ScriptAction::SET_COUNTER,"source script metadata and false-action branch");
  require(predicate->getConditionType()==(legacy ? Condition::TEAM_INSIDE_AREA_PARTIALLY : Condition::FLAG) &&
    predicate->getNumParameters()==(legacy ? 3 : 2) && predicate->getNext()->getConditionType()==Condition::CONDITION_TRUE,
    "real registry parameters and ordered AND predicates");
  if(legacy)require(predicate->getParameter(2)->getParameterType()==Parameter::SURFACES_ALLOWED &&
    predicate->getParameter(2)->getInt()==3 && current->getAction()->getNumParameters()==3 &&
    current->getAction()->getParameter(2)->getInt()==1,"original v1 condition/action repair defaults");
  auto* group=list->getScriptGroup();require(group->isActive() && group->isSubroutine() &&
    group->getScript()->getDelayEvalSeconds()==0,"group and old script defaults");
  require(InspectRead::pending()==0,"source script admission drained without static leftovers");
}
void functional(Context& context){
  for(unsigned version=1;version<=3;++version){load(wire(version));check(context,"AcceptedSide",version);
    MemoryOutput output;DataChunkOutput out(&output);SidesList::WriteSidesDataChunk(out);out.finish();
    load(output.bytes);require(context.sides.getNumSides()==3 && context.sides.getSideInfo(0)->getScriptList()->getScript()->getName()==
      "GeneratedDirectScript","actual source writer/reader retains scripts");}
  load(wire(3,"AcceptedSide",true));check(context,"AcceptedSide",3,true);
  SidesList detached;auto* list=context.sides.getSideInfo(0)->getScriptList();AllocationFault::arm(0);
  context.sides.swapContents(detached);context.sides.swapContents(detached);const auto attempted=AllocationFault::attempts();AllocationFault::disarm();
  require(!attempted && context.sides.getSideInfo(0)->getScriptList()==list,"whole side content exchange preserves backing without allocation");
}
void negative(Context& context){const auto valid=wire();load(valid);
  auto reject=[&](const std::string& bytes){auto* list=context.sides.getSideInfo(0)->getScriptList();auto* build=context.sides.getSideInfo(0)->getBuildList();
    const auto live=AllocationFault::live();const auto id=InspectRead::id();bool failed=false;
    try{load(bytes);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"malformed side/script class");failed=true;}
    require(failed && TheSidesList==&context.sides && context.sides.getSideInfo(0)->getScriptList()==list &&
      context.sides.getSideInfo(0)->getBuildList()==build && InspectRead::id()==id && AllocationFault::live()==live,
      "late nested rejection preserves accepted world owners/ID/namespace");check(context);load(valid);};
  for(std::size_t prefix=0;prefix<valid.size();++prefix)reject(valid.substr(0,prefix));
  reject(wire(0));reject(wire(4));
  auto malformed=[&](const ScriptWriter& body){reject(wire(3,"AcceptedSide",false,2,body));};
  for(Int count:{-1,MAX_PARMS+1,std::numeric_limits<Int>::max()}){
    malformed([&](DataChunkOutput& out){out.openDataChunk("OrCondition",1);out.openDataChunk("Condition",4);
      out.writeInt(Condition::FLAG);out.writeNameKey(TheScriptEngine->getConditionTemplate(Condition::FLAG)->m_internalNameKey);
      out.writeInt(count);out.closeDataChunk();out.closeDataChunk();});
    malformed([&](DataChunkOutput& out){out.openDataChunk("ScriptAction",2);out.writeInt(ScriptAction::SET_FLAG);
      out.writeNameKey(TheScriptEngine->getActionTemplate(ScriptAction::SET_FLAG)->m_internalNameKey);
      out.writeInt(count);out.closeDataChunk();});}
  for(Int rawType:{-1,Int(Parameter::NUM_ITEMS),std::numeric_limits<Int>::max()})
    malformed([&](DataChunkOutput& out){out.openDataChunk("ScriptAction",2);out.writeInt(ScriptAction::SET_FLAG);
      out.writeNameKey(TheScriptEngine->getActionTemplate(ScriptAction::SET_FLAG)->m_internalNameKey);
      out.writeInt(1);out.writeInt(rawType);out.writeInt(0);out.writeReal(0);out.writeAsciiString("");out.closeDataChunk();});
  for(unsigned version:{0,5})malformed([&](DataChunkOutput& out){out.openDataChunk("OrCondition",1);
    condition(out,Condition::CONDITION_TRUE,version);out.closeDataChunk();});
  for(unsigned version:{0,3})malformed([&](DataChunkOutput& out){action(out,ScriptAction::NO_OP,version);});
  malformed([](DataChunkOutput& out){out.openDataChunk("OrCondition",1);out.openDataChunk("Condition",4);
    out.writeInt(Condition::CONDITION_TRUE);out.writeNameKey(TheScriptEngine->getConditionTemplate(Condition::CONDITION_TRUE)->m_internalNameKey);
    out.writeInt(0);out.writeByte(77);out.closeDataChunk();out.closeDataChunk();});
  // Locate the side payload after its actual TOC, without introducing raw enum objects.
  MemoryInput inspect(valid);DataChunkInput input(&inspect);DataChunkVersionType version;
  require(input.openDataChunk(&version)=="SidesList","generated side envelope");const auto offset=inspect.tell();
  for(unsigned count:{0xffffffffu,UnsignedInt(MAX_PLAYER_COUNT+1),0x7fffffffu}){auto changed=valid;overwrite(changed,offset,count);reject(changed);}
}
template<class Operation,class Check>void sweep(const char* label,Operation operation,Check accepted){
  // Measure the same group-ID state each time. Generated default names cross
  // std::string's small-buffer boundary as IDs grow, changing heap demand.
  auto run=[&]{ScriptList::IDTransaction ids;operation();};
  std::size_t census;AllocationFault::arm(SIZE_MAX);try{run();}catch(...){AllocationFault::disarm();throw;}
  census=AllocationFault::attempts();AllocationFault::disarm();require(census<4096,"complete bounded source census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto id=InspectRead::id();const PoolUsage pools;bool failed=false;
    AllocationFault::arm(ordinal);try{run();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
    const bool exact=failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      AllocationFault::attempts()==(failed ? ordinal+1 : census);
    if(!exact)std::cerr<<label<<" ordinal "<<ordinal<<" census "<<census<<" attempts "<<AllocationFault::attempts()
      <<" failed "<<failed<<" hit "<<AllocationFault::triggered()<<'\n';
    require(exact,"all original-owner acquisition prefixes and exact terminal");
    require(AllocationFault::live()==live && pools.matches() && (!failed || InspectRead::id()==id),"partial original tree retires every acquired unit and cursor");accepted(failed);
    if(failed){run();require(AllocationFault::live()==live && pools.matches(),"each corrected original-owner retry retires");}}
  std::cout<<label<<" prefixes [0,"<<census<<"); terminal "<<census<<'\n';}
std::vector<Parameter::ParameterType> actionParameters(ScriptAction::ScriptActionType type,Int count){
  const auto* metadata=TheScriptEngine->getActionTemplate(type);std::vector<Parameter::ParameterType> result;
  for(Int i=0;i<count;++i)result.push_back(metadata->getParameterType(i));return result;}
struct LegacyAction{ScriptAction::ScriptActionType before,after;Int oldCount;};
constexpr LegacyAction legacyActions[]{
  {ScriptAction::TEAM_FOLLOW_WAYPOINTS,ScriptAction::TEAM_FOLLOW_WAYPOINTS,2},
  {ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT,ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK,1},
  {ScriptAction::NAMED_SET_ATTITUDE,ScriptAction::NAMED_SET_ATTITUDE,2},
  {ScriptAction::TEAM_SET_ATTITUDE,ScriptAction::TEAM_SET_ATTITUDE,2},
  {ScriptAction::MAP_REVEAL_AT_WAYPOINT,ScriptAction::MAP_REVEAL_AT_WAYPOINT,2},
  {ScriptAction::MAP_SHROUD_AT_WAYPOINT,ScriptAction::MAP_SHROUD_AT_WAYPOINT,2},
  {ScriptAction::MAP_REVEAL_ALL,ScriptAction::MAP_REVEAL_ALL,0},
  {ScriptAction::MAP_REVEAL_ALL_PERM,ScriptAction::MAP_REVEAL_ALL_PERM,0},
  {ScriptAction::MAP_REVEAL_ALL_UNDO_PERM,ScriptAction::MAP_REVEAL_ALL_UNDO_PERM,0},
  {ScriptAction::MAP_SHROUD_ALL,ScriptAction::MAP_SHROUD_ALL,0},
  {ScriptAction::SPEECH_PLAY,ScriptAction::SPEECH_PLAY,1},
  {ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM,ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM,1},
  {ScriptAction::CAMERA_MOD_SET_FINAL_PITCH,ScriptAction::CAMERA_MOD_SET_FINAL_PITCH,1},
  {ScriptAction::MOVE_CAMERA_TO,ScriptAction::MOVE_CAMERA_TO,3},
  {ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH,ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH,3},
  {ScriptAction::CAMERA_LOOK_TOWARD_OBJECT,ScriptAction::CAMERA_LOOK_TOWARD_OBJECT,3},
  {ScriptAction::RESET_CAMERA,ScriptAction::RESET_CAMERA,2},
  {ScriptAction::ZOOM_CAMERA,ScriptAction::ZOOM_CAMERA,2},
  {ScriptAction::PITCH_CAMERA,ScriptAction::PITCH_CAMERA,2},
  {ScriptAction::ROTATE_CAMERA,ScriptAction::ROTATE_CAMERA,2},
  {ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT,ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT,2},
  {ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT,ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT,4},
  {ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST,ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST,1}
};
constexpr Condition::ConditionType legacyConditions[]{Condition::TEAM_INSIDE_AREA_PARTIALLY,Condition::TEAM_INSIDE_AREA_ENTIRELY,
  Condition::TEAM_OUTSIDE_AREA_ENTIRELY,Condition::TEAM_ENTERED_AREA_PARTIALLY,Condition::TEAM_ENTERED_AREA_ENTIRELY,
  Condition::TEAM_EXITED_AREA_ENTIRELY,Condition::TEAM_EXITED_AREA_PARTIALLY};
void formats(Context& context){
  const auto rich=wire(3,"AcceptedSide",false,2,[](DataChunkOutput& out){
    out.openDataChunk("OrCondition",1);
    for(auto type:legacyConditions){const auto* metadata=TheScriptEngine->getConditionTemplate(type);
      condition(out,type,1,{metadata->getParameterType(0),metadata->getParameterType(1)});}
    condition(out,Condition::SKIRMISH_SPECIAL_POWER_READY,3,{Parameter::SPECIAL_POWER});
    condition(out,Condition::FLAG,4,{Parameter::FLAG,Parameter::BOOLEAN},std::numeric_limits<Int>::max());
    condition(out,Condition::FLAG,4,{Parameter::FLAG,Parameter::BOOLEAN},0,"UnregisteredGeneratedPredicate");
    condition(out,Condition::FLAG,4);out.closeDataChunk();
    for(auto item:legacyActions){auto parameters=item.before==ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT
        ? std::vector{Parameter::BOOLEAN} : actionParameters(item.after,item.oldCount);
      if(item.before==ScriptAction::NAMED_SET_ATTITUDE || item.before==ScriptAction::TEAM_SET_ATTITUDE)parameters[1]=Parameter::INT;
      if(item.before==ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST)parameters={Parameter::SPECIAL_POWER};
      action(out,item.before,1,parameters);}
    action(out,ScriptAction::SET_FLAG,2,{Parameter::FLAG,Parameter::BOOLEAN},false,std::numeric_limits<Int>::max());
    action(out,ScriptAction::SET_FLAG,2,{Parameter::FLAG,Parameter::BOOLEAN},false,0,"UnregisteredGeneratedCommand");
    action(out,ScriptAction::SET_FLAG,2);});
  auto verify=[&]{auto* script=context.sides.getSideInfo(0)->getScriptList()->getScript();
    auto* predicate=script->getOrCondition()->getFirstAndCondition();
    for(auto type:legacyConditions){require(predicate && predicate->getConditionType()==type &&
      predicate->getNumParameters()==3 && predicate->getParameter(2)->getInt()==3,"all original surface condition repairs");predicate=predicate->getNext();}
    require(predicate && predicate->getConditionType()==Condition::SKIRMISH_SPECIAL_POWER_READY && predicate->getNumParameters()==2 &&
      predicate->getParameter(0)->getString()==THIS_PLAYER,"original special-power predicate inserts player before old parameter");predicate=predicate->getNext();
    require(predicate && predicate->getConditionType()==Condition::FLAG && predicate->getNumParameters()==2,"modern predicate name rematches invalid raw numeric");
    for(unsigned i=0;i<2;++i){predicate=predicate->getNext();require(predicate && predicate->getConditionType()==Condition::CONDITION_FALSE &&
      predicate->getNumParameters()==0,"unknown or arity-mismatched predicate retains source false fallback");}
    auto* command=script->getAction();for(auto item:legacyActions){const auto* metadata=TheScriptEngine->getActionTemplate(item.after);
      require(command && command->getActionType()==item.after && command->getNumParameters()==metadata->getNumParameters(),"all original action repairs and arities");
      for(Int i=0;i<command->getNumParameters();++i)require(command->getParameter(i)->getParameterType()==metadata->getParameterType(i),"repaired parameter type order");
      if(item.after==ScriptAction::TEAM_FOLLOW_WAYPOINTS || item.after==ScriptAction::SPEECH_PLAY)
        require(command->getParameter(command->getNumParameters()-1)->getInt()==1,"original true repair default");
      if(item.after==ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST)require(command->getParameter(0)->getString()==THIS_PLAYER,"inserted player ownership");
      command=command->getNext();}
    require(command && command->getActionType()==ScriptAction::SET_FLAG,"modern action name rematches invalid raw numeric");
    for(unsigned i=0;i<2;++i){command=command->getNext();require(command && command->getActionType()==ScriptAction::NO_OP &&
      command->getNumParameters()==0,"unknown or arity-mismatched action retains source no-op fallback");}
    require(!command->getNext(),"complete repaired command chain");};
  load(rich);verify();
  // Fixed-size scene replacement has equal ownership before/after. Every
  // physical allocation prefix includes the legacy multi-acquisition repairs.
  sweep("all-format-repairs",[&]{load(rich);verify();},[&](bool){verify();});
}
std::string parameterWire(Parameter::ParameterType type,const char* text,Int value=1){MemoryOutput output;DataChunkOutput out(&output);
  out.openDataChunk("GeneratedParameter",1);parameter(out,type,text,value);out.closeDataChunk();out.finish();return output.bytes;}
Owner<Parameter> readParameter(const std::string& bytes){MemoryInput stream(bytes);DataChunkInput input(&stream);DataChunkVersionType version;
  require(input.openDataChunk(&version)=="GeneratedParameter","actual parameter transport");
  Owner<Parameter> result(Parameter::ReadParameter(input));if(!input.atEndOfChunk())throw ERROR_CORRUPT_FILE_FORMAT;return result;}
void parameters(Context& context){load(wire());
  const std::string oldName="Fundamentalist"+std::string(600,'A'),expected="GLA"+std::string(600,'A');
  const auto migrated=parameterWire(Parameter::OBJECT_TYPE,oldName.c_str());
  auto checkMigration=[&]{auto value=readParameter(migrated);require(value->getString()==expected.c_str(),"unbounded source object-prefix migration");};
  checkMigration();sweep("long-parameter-migration",checkMigration,[&](bool){check(context);});
  for(const char* old:{"Upgrade_AmericaRangerCaptureBuilding","Upgrade_ChinaRedguardCaptureBuilding","Upgrade_GLARebelCaptureBuilding"}){
    auto value=readParameter(parameterWire(Parameter::UPGRADE,old));require(value->getString()=="Upgrade_InfantryCaptureBuilding","all source obsolete capture upgrades");}
  auto status=readParameter(parameterWire(Parameter::OBJECT_STATUS,ObjectStatusMaskType::getBitNames()[0]));
  require(status->getStatus()==MAKE_OBJECT_STATUS_MASK(0),"original status name conversion");status.reset();
  for(Int index:{0,Int(KINDOF_COUNT-1)}){auto value=readParameter(parameterWire(Parameter::KIND_OF_PARAM,"",index));
    require(value->getInt()==index && value->getString()==KindOfMaskType::getBitNames()[index],"empty kind name derives valid source index");}
  auto named=readParameter(parameterWire(Parameter::KIND_OF_PARAM,KindOfMaskType::getBitNames()[1],0));
  require(named->getInt()==1,"named kind derives original index");named.reset();
  for(Int index:{-1,Int(KINDOF_COUNT),std::numeric_limits<Int>::max()}){const auto bytes=parameterWire(Parameter::KIND_OF_PARAM,"",index);
    const auto live=AllocationFault::live();const PoolUsage pools;bool failed=false;
    try{auto value=readParameter(bytes);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"malformed kind index class");failed=true;}
    require(failed && AllocationFault::live()==live && pools.matches(),"out-of-range kind index rejects and retires");}
  for(std::size_t size=0;size<migrated.size();++size){bool failed=false;const auto live=AllocationFault::live();const PoolUsage pools;
    try{auto value=readParameter(migrated.substr(0,size));}catch(ErrorCode){failed=true;}catch(const std::runtime_error&){failed=true;}
    require(failed && AllocationFault::live()==live && pools.matches(),"every physical Parameter prefix rejects and retires");}
}
std::string scriptsWire(Int count){MemoryOutput output;DataChunkOutput out(&output);out.openDataChunk("PlayerScriptsList",5);
  for(Int i=0;i<count;++i){out.openDataChunk("ScriptList",1);script(out);out.closeDataChunk();}out.closeDataChunk();out.finish();return output.bytes;}
void loadScripts(const std::string& bytes){MemoryInput stream(bytes);DataChunkInput input(&stream);
  input.registerParser("PlayerScriptsList",AsciiString::TheEmptyString,ScriptList::ParseScriptsDataChunk);
  if(!input.isValidFileType() || !input.parse() || !stream.eof())throw ERROR_CORRUPT_FILE_FORMAT;}
void publication(Context& context){load(wire());const auto all=scriptsWire(MAX_PLAYER_COUNT),excess=scriptsWire(MAX_PLAYER_COUNT+1);
  // Warm simultaneous accepted/candidate list capacity before the rejection
  // baseline. A legitimately grown source pool retains backing until shutdown.
  loadScripts(all);loadScripts(all);require(InspectRead::pending()==MAX_PLAYER_COUNT,"actual full source read-array capacity admitted");
  auto* borrowed=InspectRead::first();
  const auto live=AllocationFault::live();const PoolUsage pools;bool failed=false;
  try{loadScripts(excess);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"excess read count class");failed=true;}
  require(failed && InspectRead::pending()==MAX_PLAYER_COUNT && InspectRead::first()==borrowed &&
    AllocationFault::live()==live && pools.matches(),"late excess read preserves pending trees");
  load(wire());require(context.sides.getNumSides()==3 && context.sides.getSideInfo(0)->getScriptList()->getScript()->getName()=="GeneratedDirectScript" &&
    InspectRead::pending()==MAX_PLAYER_COUNT && InspectRead::first()==borrowed,"side transaction admits its own scripts and restores borrowed pending reads");
  std::array<ScriptList*,MAX_PLAYER_COUNT> accepted{};require(ScriptList::getReadScripts(accepted.data())==MAX_PLAYER_COUNT,"drain full pending owner array");
  for(auto* list:accepted){require(list && list->getScript(),"all source pending scripts retained");list->deleteInstance();}
  require(InspectRead::pending()==0,"pending owner withdrawal complete");
  const auto invalid=wire(3,"AcceptedSide",false,2,[](DataChunkOutput& out){out.openDataChunk("ScriptAction",0);out.closeDataChunk();});
  loadScripts(scriptsWire(1));const auto before=InspectRead::id();failed=false;
  try{load(invalid);}catch(ErrorCode){failed=true;}require(failed && InspectRead::pending()==1 && InspectRead::id()==before,"failed side transaction restores borrowed read owner and cursor");
  accepted.fill(nullptr);require(ScriptList::getReadScripts(accepted.data())==1,"borrowed singleton pending retained");accepted[0]->deleteInstance();
}
void copies(Context& context){load(wire());auto* list=context.sides.getSideInfo(0)->getScriptList();
  auto checkCopy=[&](ScriptList* copy,bool qualified){require(copy!=list && copy->getScript()!=list->getScript() &&
    copy->getScript()->getAction()!=list->getScript()->getAction() && copy->getScript()->getName()==
      (qualified ? "GeneratedDirectScript_Q" : "GeneratedDirectScript"),"real independent deep/qualified tree");
    require(copy->getScript()->getAction()->getParameter(0)->getString()==(qualified ? "GeneratedParameter_Q" : "GeneratedParameter"),
      "original parameter qualification");};
  sweep("script-deep",[&]{Owner<ScriptList> copy(list->duplicate());checkCopy(copy.get(),false);},[&](bool){check(context);});
  sweep("script-qualified",[&]{Owner<ScriptList> copy(list->duplicateAndQualify("_Q","Faction","Player"));checkCopy(copy.get(),true);},[&](bool){check(context);});
  auto* source=context.sides.getSideInfo(0);
  sweep("side-copy",[&]{SidesInfo copy(*source);require(copy.getBuildList()!=source->getBuildList() &&
    copy.getScriptList()!=source->getScriptList(),"actual side copy owns independent lists");},[&](bool){check(context);});
  sweep("build-copy",[&]{Owner<BuildListInfo> copy(source->getBuildList()->duplicate());require(copy->getNext() &&
    copy->getNext()!=source->getBuildList()->getNext(),"original linked build copy");},[&](bool){check(context);});
  SidesInfo target(*source);ScriptList* beforeScripts=nullptr;BuildListInfo* beforeBuilds=nullptr;
  sweep("side-assignment",[&]{beforeScripts=target.getScriptList();beforeBuilds=target.getBuildList();target=*source;
    require(target.getScriptList()!=source->getScriptList() && target.getBuildList()!=source->getBuildList(),"assigned side owns independent trees");},
    [&](bool failed){if(failed)require(target.getScriptList()==beforeScripts && target.getBuildList()==beforeBuilds,"failed assignment preserves accepted side identity");check(context);});
  TeamsInfoRec teams;for(Int i=0;i<context.sides.getNumTeams();++i)teams.addTeam(context.sides.getTeamInfo(i)->getDict());
  sweep("team-copy",[&]{TeamsInfoRec copy(teams);require(copy.getNumTeams()==teams.getNumTeams() &&
    copy.getTeamInfo(0)!=teams.getTeamInfo(0),"actual independent team array copy");},[&](bool){check(context);});
  TeamsInfoRec teamTarget(teams);TeamsInfo* beforeTeam=nullptr;
  sweep("team-assignment",[&]{beforeTeam=teamTarget.getTeamInfo(0);teamTarget=teams;},[&](bool failed){
    require(teamTarget.getNumTeams()==teams.getNumTeams() && (!failed || teamTarget.getTeamInfo(0)==beforeTeam),"strong accepted team assignment identity");});
}
void fixedPools(){
  TheMemoryPoolFactory->createMemoryPool("BuildListInfo",sizeof(BuildListInfo),128,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptList",sizeof(ScriptList),64,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptGroup",sizeof(ScriptGroup),64,0);
  TheMemoryPoolFactory->createMemoryPool("Script",sizeof(Script),128,0);
  TheMemoryPoolFactory->createMemoryPool("OrCondition",sizeof(OrCondition),128,0);
  TheMemoryPoolFactory->createMemoryPool("Condition",sizeof(Condition),128,0);
  TheMemoryPoolFactory->createMemoryPool("ScriptAction",sizeof(ScriptAction),256,0);
  TheMemoryPoolFactory->createMemoryPool("Parameter",sizeof(Parameter),512,0);
}
void poolFailures(Context& context){const auto valid=wire();load(valid);
  for(std::size_t index=0;index<std::size(poolNames)-1;++index){auto* pool=TheMemoryPoolFactory->findMemoryPool(poolNames[index]);
    std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Release{MemoryPool* pool;std::vector<void*>& held;~Release(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated side/script exhaustion"));
    const PoolUsage pools;const auto live=AllocationFault::live();const auto id=InspectRead::id();
    auto* accepted=context.sides.getSideInfo(0)->getScriptList();auto* build=context.sides.getSideInfo(0)->getBuildList();bool failed=false;
    try{load(valid);}catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"explicit pooled acquisition failure class");failed=true;}
    require(failed && pools.matches() && AllocationFault::live()==live && InspectRead::id()==id &&
      context.sides.getSideInfo(0)->getScriptList()==accepted && context.sides.getSideInfo(0)->getBuildList()==build,"each source pool exhaustion retires partial candidate and preserves accepted world");
    if(index==7){
      Owner<Condition> predicate(newInstance(Condition)(Condition::CONDITION_TRUE));
      Owner<ScriptAction> command(newInstance(ScriptAction)(ScriptAction::NO_OP));
      auto partial=[&](auto operation,bool expectFailure){const PoolUsage before;const auto heap=AllocationFault::live();
        const auto freeBefore=pool->getFreeBlockCount();bool rejected=false;
        try{operation();}catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"partial typed pool failure class");rejected=true;}
        require(rejected==expectFailure && before.matches() && AllocationFault::live()==heap &&
          predicate->getConditionType()==Condition::CONDITION_TRUE && predicate->getNumParameters()==0 &&
          command->getActionType()==ScriptAction::NO_OP && command->getNumParameters()==0,"partial typed constructor/setter preserves accepted values and all acquired units");
        if(rejected){for(auto* value:held)pool->freeBlock(value);held.clear();const PoolUsage retryBefore;
          operation();require(retryBefore.matches() && AllocationFault::live()==heap &&
            predicate->getConditionType()==Condition::CONDITION_TRUE && command->getActionType()==ScriptAction::NO_OP,
            "each partial typed failure gets a corrected same-process retry");
          while(pool->getFreeBlockCount()>freeBefore)held.push_back(pool->allocateBlock("generated partial typed capacity"));
          require(before.matches(),"restore exact partial capacity for the next independent factory path");}
      };
      for(unsigned available=0;available<=3;++available){
        if(available){pool->freeBlock(held.back());held.pop_back();}
        partial([]{Owner<Condition> value(newInstance(Condition)(Condition::COUNTER));},available<3);
        partial([]{Owner<ScriptAction> value(newInstance(ScriptAction)(ScriptAction::SET_FLAG));},available<2);
        partial([&]{predicate->setConditionType(Condition::COUNTER);predicate->setConditionType(Condition::CONDITION_TRUE);},available<3);
        partial([&]{command->setActionType(ScriptAction::SET_FLAG);command->setActionType(ScriptAction::NO_OP);},available<2);
      }
      std::cout<<"partial typed pool prefixes condition 3 action 2; all failure/retry pairs and terminals\n";
    }
    for(auto* value:held)pool->freeBlock(value);held.clear();load(valid);check(context);
    std::cout<<"pool-exhaustion "<<index<<" failure and corrected retry\n";
  }
}
void faults(Context& context){for(bool legacy:{false,true})for(Int builds:{2,20}){
  auto replace=[&](const std::string& bytes){ScriptList::IDTransaction ids;const auto before=InspectRead::id();load(bytes);
    require(InspectRead::id()==before+2,"successful source admission commits both generated group IDs");};
  const auto valid=wire(3,"AcceptedSide",legacy,builds),changed=wire(3,"ChangedSide",legacy,builds);replace(valid);
  std::size_t census;AllocationFault::arm(SIZE_MAX);try{replace(changed);}catch(...){AllocationFault::disarm();throw;}
  census=AllocationFault::attempts();AllocationFault::disarm();replace(valid);require(census>0 && census<4096,"complete side parse census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){auto* accepted=context.sides.getSideInfo(0)->getScriptList();
    auto* build=context.sides.getSideInfo(0)->getBuildList();const auto live=AllocationFault::live();const auto id=InspectRead::id();const PoolUsage pools;bool failed=false;
    AllocationFault::arm(ordinal);try{replace(changed);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
    const bool exact=failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      AllocationFault::attempts()==(failed ? ordinal+1 : census);
    if(!exact)std::cerr<<"side-parse legacy "<<legacy<<" builds "<<builds<<" ordinal "<<ordinal<<" census "<<census
      <<" attempts "<<AllocationFault::attempts()<<" failed "<<failed<<" hit "<<AllocationFault::triggered()<<'\n';
    require(exact,"every complete side/script prefix and exact terminal");
    require(AllocationFault::live()==live && pools.matches() && TheSidesList==&context.sides && InspectRead::pending()==0,"candidate side/script retirement");
    if(failed){require(context.sides.getSideInfo(0)->getScriptList()==accepted && context.sides.getSideInfo(0)->getBuildList()==build &&
      InspectRead::id()==id,"failed candidate preserves accepted backing/cursor");check(context,"AcceptedSide",3,legacy,builds);replace(changed);}
    check(context,"ChangedSide",3,legacy,builds);replace(valid);}
  std::cout<<"side-parse legacy "<<legacy<<" builds "<<builds<<" prefixes [0,"<<census<<"); terminal "<<census<<'\n';}}
void setters(Context& context){load(wire());
  sweep("condition-constructor",[]{Owner<Condition> value(newInstance(Condition)(Condition::COUNTER));
    require(value->getNumParameters()==3,"actual typed condition factory");},[&](bool){check(context);});
  sweep("action-constructor",[]{Owner<ScriptAction> value(newInstance(ScriptAction)(ScriptAction::SET_FLAG));
    require(!value->getNext() && !value->hasWarnings() && value->getNumParameters()==2,"complete typed action initialization");},[&](bool){check(context);});
  Owner<Condition> predicate(newInstance(Condition)(Condition::CONDITION_TRUE));
  Owner<ScriptAction> command(newInstance(ScriptAction)(ScriptAction::NO_OP));
  sweep("condition-setter",[&]{predicate->setConditionType(Condition::COUNTER);predicate->setConditionType(Condition::CONDITION_TRUE);},
    [&](bool failed){if(failed)require(predicate->getConditionType()==Condition::CONDITION_TRUE && predicate->getNumParameters()==0,"strong condition setter rollback");});
  sweep("action-setter",[&]{command->setActionType(ScriptAction::SET_FLAG);command->setActionType(ScriptAction::NO_OP);},
    [&](bool failed){if(failed)require(command->getActionType()==ScriptAction::NO_OP && command->getNumParameters()==0,"strong action setter rollback");});
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"side/script family");initMemoryManager();initialized=true;
    if(std::string_view(argv[1])=="pool-failures")fixedPools();
    // Retire actual cold providers before repeat-operation baselines.
    {Context warm;load(wire(3,"AcceptedSide",true,20));
      auto* list=warm.sides.getSideInfo(0)->getScriptList();Owner<ScriptList> copy(list->duplicate());
      Owner<ScriptList> qualified(list->duplicateAndQualify("_Q","Faction","Player"));
      SidesInfo side(*warm.sides.getSideInfo(0));Owner<BuildListInfo> builds(side.getBuildList()->duplicate());
      Owner<Condition> predicate(newInstance(Condition)(Condition::COUNTER));
      Owner<ScriptAction> command(newInstance(ScriptAction)(ScriptAction::SET_FLAG));
      if(std::string_view(argv[1])=="publication")publication(warm);}
    {const std::string family=argv[1];for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const PoolUsage pools;
      {Context context;if(family=="functional")functional(context);else if(family=="negative")negative(context);
        else if(family=="copies")copies(context);else if(family=="faults")faults(context);else if(family=="setters")setters(context);
        else if(family=="formats")formats(context);else if(family=="parameters")parameters(context);else if(family=="publication")publication(context);
        else if(family=="pool-failures")poolFailures(context);
        else require(false,"unknown family");}
      require(AllocationFault::live()==live && pools.matches() && !TheScriptEngine && !TheSidesList && !TheNameKeyGenerator && !TheWritableGlobalData,
        "whole side/script/native provider retirement");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original pool metadata retirement");
    std::cout<<"PASS actual sides/scripts; complete world/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL source side/script admission "<<Int(error)<<'\n';}
  catch(...){std::cerr<<"FAIL source side/script admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
