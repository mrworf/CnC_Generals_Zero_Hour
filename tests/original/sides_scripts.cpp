// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original side/script owners and template registry, not simulation acceptance.
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include <algorithm>
#include <bit>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Retire{template<class T>void operator()(T* p)const noexcept{if(p)p->deleteInstance();}};
template<class T>using Owner=std::unique_ptr<T,Retire>;
struct InspectRead:ScriptList{static Int id(){return m_curId;}static Int pending(){return s_numInReadList;}};
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
  if(version>=4){if(key)out.writeAsciiString(key);else out.writeNameKey(TheScriptEngine->getConditionTemplate(type)->m_internalNameKey);}
  out.writeInt(parameters.size());for(auto param:parameters)parameter(out,param);out.closeDataChunk();}
void action(DataChunkOutput& out,ScriptAction::ScriptActionType type,unsigned version=2,
  const std::vector<Parameter::ParameterType>& parameters={},bool negative=false,Int numeric=-1,const char* key=nullptr){
  out.openDataChunk(negative ? "ScriptActionFalse" : "ScriptAction",version);out.writeInt(numeric==-1 ? Int(type) : numeric);
  if(version>=2){if(key)out.writeAsciiString(key);else out.writeNameKey(TheScriptEngine->getActionTemplate(type)->m_internalNameKey);}
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
std::string wire(unsigned version=3,const char* name="AcceptedSide",bool legacy=false,Int builds=2){
  MemoryOutput output;DataChunkOutput out(&output);out.openDataChunk("SidesList",version);out.writeInt(2);
  for(unsigned side=0;side<2;++side){Dict dict;dict.setAsciiString(TheKey_playerName,side==0 ? name : "GeneratedOtherSide");
    dict.setAsciiString(TheKey_playerAllies,"GeneratedOtherSide MissingSide");out.writeDict(dict);out.writeInt(builds);
    for(Int n=0;n<builds;++n){out.writeAsciiString("GeneratedBuilding");out.writeAsciiString("GeneratedTemplate");
      out.writeReal(10+n);out.writeReal(20);out.writeReal(99);out.writeReal(0.5f);out.writeByte(true);out.writeInt(-1);
      if(version>=3){out.writeAsciiString("GeneratedBuildScript");out.writeInt(85);out.writeByte(false);out.writeByte(true);out.writeByte(false);}}}
  if(version>=2){out.writeInt(1);Dict dict;dict.setAsciiString(TheKey_teamName,"GeneratedAuthoredTeam");
    dict.setAsciiString(TheKey_teamOwner,"MissingSide");out.writeDict(dict);}
  out.openDataChunk("PlayerScriptsList",5); // Actual source writer enum value, despite the VERSION_1 name.
  for(unsigned side=0;side<2;++side){out.openDataChunk("ScriptList",1);script(out,"GeneratedDirectScript",legacy);
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
  // Locate the side payload after its actual TOC, without introducing raw enum objects.
  MemoryInput inspect(valid);DataChunkInput input(&inspect);DataChunkVersionType version;
  require(input.openDataChunk(&version)=="SidesList","generated side envelope");const auto offset=inspect.tell();
  for(unsigned count:{0xffffffffu,UnsignedInt(MAX_PLAYER_COUNT+1),0x7fffffffu}){auto changed=valid;overwrite(changed,offset,count);reject(changed);}
}
template<class Operation,class Check>void sweep(const char* label,Operation operation,Check accepted){
  std::size_t census;AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}
  census=AllocationFault::attempts();AllocationFault::disarm();require(census>0 && census<4096,"complete bounded source census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto id=InspectRead::id();bool failed=false;
    AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      AllocationFault::attempts()==(failed ? ordinal+1 : census),"all original-owner acquisition prefixes and exact terminal");
    require(AllocationFault::live()==live && (!failed || InspectRead::id()==id),"partial original tree retires every acquired unit and cursor");accepted(failed);
    if(failed){operation();require(AllocationFault::live()==live,"each corrected original-owner retry retires");}}
  std::cout<<label<<" prefixes [0,"<<census<<"); terminal "<<census<<'\n';}
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
}
void faults(Context& context){for(bool legacy:{false,true})for(Int builds:{2,20}){
  const auto valid=wire(3,"AcceptedSide",legacy,builds),changed=wire(3,"ChangedSide",legacy,builds);load(valid);
  std::size_t census;AllocationFault::arm(SIZE_MAX);try{load(changed);}catch(...){AllocationFault::disarm();throw;}
  census=AllocationFault::attempts();AllocationFault::disarm();load(valid);require(census>0 && census<4096,"complete side parse census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){auto* accepted=context.sides.getSideInfo(0)->getScriptList();
    auto* build=context.sides.getSideInfo(0)->getBuildList();const auto live=AllocationFault::live();const auto id=InspectRead::id();bool failed=false;
    AllocationFault::arm(ordinal);try{load(changed);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      AllocationFault::attempts()==(failed ? ordinal+1 : census),"every complete side/script prefix and exact terminal");
    require(AllocationFault::live()==live && TheSidesList==&context.sides && InspectRead::pending()==0,"candidate side/script retirement");
    if(failed){require(context.sides.getSideInfo(0)->getScriptList()==accepted && context.sides.getSideInfo(0)->getBuildList()==build &&
      InspectRead::id()==id,"failed candidate preserves accepted backing/cursor");check(context,"AcceptedSide",3,legacy,builds);load(changed);}
    check(context,"ChangedSide",3,legacy,builds);load(valid);}
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
    // Retire actual cold providers before repeat-operation baselines.
    {Context warm;load(wire(3,"AcceptedSide",true,20));
      auto* list=warm.sides.getSideInfo(0)->getScriptList();Owner<ScriptList> copy(list->duplicate());
      Owner<ScriptList> qualified(list->duplicateAndQualify("_Q","Faction","Player"));
      SidesInfo side(*warm.sides.getSideInfo(0));Owner<BuildListInfo> builds(side.getBuildList()->duplicate());
      Owner<Condition> predicate(newInstance(Condition)(Condition::COUNTER));
      Owner<ScriptAction> command(newInstance(ScriptAction)(ScriptAction::SET_FLAG));}
    {const std::string family=argv[1];for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      {Context context;if(family=="functional")functional(context);else if(family=="negative")negative(context);
        else if(family=="copies")copies(context);else if(family=="faults")faults(context);else if(family=="setters")setters(context);
        else require(false,"unknown family");}
      require(AllocationFault::live()==live && !TheScriptEngine && !TheSidesList && !TheNameKeyGenerator && !TheWritableGlobalData,
        "whole side/script/native provider retirement");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original pool metadata retirement");
    std::cout<<"PASS actual sides/scripts; complete world/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL source side/script admission "<<Int(error)<<'\n';}
  catch(...){std::cerr<<"FAIL source side/script admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
