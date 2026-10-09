// SPDX-License-Identifier: GPL-3.0-or-later
// Actual default-team and prototype lifetimes, not Object/startup acceptance.
#include "AllocationFault.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Dict.h"
#include "Common/Team.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Squad.h"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
constexpr std::array<const char*,5> pools{"PlayerRelationMapPool","TeamRelationMapPool","Squad","TeamPrototypePool","TeamPool"};
std::array<Int,5> usage(){std::array<Int,5> result{};for(unsigned n=0;n<pools.size();++n){auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[n]);require(pool,"actual team pool exists");result[n]=pool->getUsedBlockCount();}return result;}
struct Context {
  NameKeyGenerator names;RankInfoStore ranks;std::unique_ptr<ScriptEngine> scripts;
  std::unique_ptr<PlayerList> players;
  NameKeyGenerator* previousNames=TheNameKeyGenerator;RankInfoStore* previousRanks=TheRankInfoStore;
  ScriptEngine* previousScripts=TheScriptEngine;PlayerList* previousPlayers=ThePlayerList;
  Context(){names.init();ranks.init();TheNameKeyGenerator=&names;TheRankInfoStore=&ranks;
    try{scripts=std::make_unique<ScriptEngine>();TheScriptEngine=scripts.get();players=std::make_unique<PlayerList>();ThePlayerList=players.get();}
    catch(...){scripts.reset();restore();throw;}}
  void restore()noexcept{ThePlayerList=previousPlayers;TheScriptEngine=previousScripts;TheRankInfoStore=previousRanks;TheNameKeyGenerator=previousNames;}
  ~Context(){players.reset();scripts.reset();restore();}
};
struct Factory {
  TeamFactory* previous=TheTeamFactory;
  std::unique_ptr<TeamFactory> owner=std::make_unique<TeamFactory>();
  Dict definition;
  Factory(){TheTeamFactory=owner.get();owner->init();}
  ~Factory(){TheTeamFactory=owner.get();owner.reset();TheTeamFactory=previous;}
  void add(const char* name,Bool singleton=TRUE){owner->initTeam(name,AsciiString::TheEmptyString,singleton,&definition);}
  TeamPrototype* prototype(const char* name){return owner->findTeamPrototype(name);}
  Team* team(const char* name){return owner->findTeam(name);}
};
void functional(Context& context){Factory factory;factory.add("team");auto* neutral=context.players->getNeutralPlayer();neutral->setDefaultTeam();
  auto* base=factory.team("team");require(neutral->getDefaultTeam()==base && base->isActive() && base->getControllingPlayer()==neutral,"actual source neutral default team and activation");
  require(base->getID()==1 && factory.prototype("team")->getID()==1 && neutral->getRelationship(base)==ALLIES,"fixed source IDs and neutral self alliance");
  require(factory.owner->createTeam("team")==base && factory.owner->createTeamOnPrototype(factory.prototype("team"))==base,"source singleton reuses acquired owner without advancing ID");
  factory.add("GeneratedMultiple",FALSE);auto* proto=factory.prototype("GeneratedMultiple");require(proto->getID()==2 && !proto->getFirstItemIn_TeamInstanceList(),"non-singleton prototype does not invent an instance");
  auto* first=factory.owner->createTeamOnPrototype(proto);auto* second=factory.owner->createInactiveTeam("GeneratedMultiple");
  require(first->getID()==2 && first->isActive() && second->getID()==3 && !second->isActive() && proto->getFirstItemIn_TeamInstanceList()==second,"source instance ID/activation/intrusive prepend order");
  neutral->setTeamRelationship(second,ENEMIES);first->setOverrideTeamRelationship(second->getID(),ENEMIES);
  factory.owner->teamAboutToBeDeleted(second);
  require(!neutral->removeTeamRelationship(second) && !first->removeOverrideTeamRelationship(3),"actual deletion notifications withdraw player/team relationships while identity is live");
  second->deleteInstance();require(proto->getFirstItemIn_TeamInstanceList()==first,"actual member retirement preserves surviving intrusive participant");
  const auto live=AllocationFault::live();AllocationFault::arm(0);factory.owner->clear();
  const auto count=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(!count && !hit && AllocationFault::live()<live && !factory.prototype("team") && !factory.prototype("GeneratedMultiple"),"complete source clear is allocation-free and withdraws registry before callbacks");
  context.players->init(); // Original clear -> player reset phase withdraws default team.
  factory.owner->reset();factory.add("team");neutral->setDefaultTeam();require(factory.team("team")->getID()==1,"explicit source factory reset restarts cursor");
}
template<class Action>void rejects(Action action){bool rejected=false;try{action();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}require(rejected,"invalid team admission rejects");}
void negative(Context& context){Factory factory;factory.add("team");auto* base=factory.team("team");auto* proto=factory.prototype("team");const auto units=usage();const auto live=AllocationFault::live();
  rejects([&]{factory.add("team");});rejects([&]{factory.owner->initTeam("MissingDefinition","",TRUE,nullptr);});
  rejects([&]{factory.owner->createInactiveTeam("MissingPrototype");});rejects([&]{factory.owner->createTeamOnPrototype(nullptr);});
  rejects([]{TeamTemplateInfo missing(nullptr);});
  require(factory.team("team")==base && factory.prototype("team")==proto && usage()==units && AllocationFault::live()==live,"malformed admission preserves accepted identities and ownership");
  {Factory foreign;foreign.add("GeneratedForeign");rejects([&]{foreign.owner->createTeamOnPrototype(proto);});}
  require(TheTeamFactory==factory.owner.get(),"borrowed factory publication restored after legitimate retirement");
}
void manifest(Context& context,const char* label,bool prototypeOperation,bool onPrototype){
  std::size_t count;
  {Factory discovery;discovery.add("team");if(!prototypeOperation)discovery.add("GeneratedMultiple",FALSE);
    AllocationFault::arm(SIZE_MAX);
    try{if(prototypeOperation)discovery.add("GeneratedCandidate");else if(onPrototype)discovery.owner->createTeamOnPrototype(discovery.prototype("GeneratedMultiple"));else discovery.owner->createInactiveTeam("GeneratedMultiple");}
    catch(...){AllocationFault::disarm();throw;}
    count=AllocationFault::attempts();AllocationFault::disarm();
  }
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){Factory factory;factory.add("team");auto* base=factory.team("team");auto* baseProto=factory.prototype("team");
    auto* player=context.players->getNeutralPlayer();player->setDefaultTeam();if(!prototypeOperation)factory.add("GeneratedMultiple",FALSE);
    const auto live=AllocationFault::live();const auto units=usage();bool failed=false;Team* candidate=nullptr;
    AllocationFault::arm(ordinal);
    try{if(prototypeOperation)factory.add("GeneratedCandidate");else if(onPrototype)candidate=factory.owner->createTeamOnPrototype(factory.prototype("GeneratedMultiple"));else candidate=factory.owner->createInactiveTeam("GeneratedMultiple");}
    catch(const std::bad_alloc&){failed=true;}catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"actual team acquisition failure class");failed=true;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<count),"complete team failure prefixes and exact terminal");
    require(factory.team("team")==base && factory.prototype("team")==baseProto && player->getDefaultTeam()==base,"failed or terminal operation retains accepted participants and neutral default team");
    auto retireCandidate=[&]{if(prototypeOperation){auto* proto=factory.prototype("GeneratedCandidate");if(proto)proto->deleteInstance();}else if(candidate){factory.owner->teamAboutToBeDeleted(candidate);candidate->deleteInstance();candidate=nullptr;}};
    if(failed){require(!candidate && (!prototypeOperation || !factory.prototype("GeneratedCandidate")),"failed candidate registration withdrawn");}
    else{if(prototypeOperation){auto* proto=factory.prototype("GeneratedCandidate");require(proto->getID()==2 && factory.team("GeneratedCandidate")->getID()==2,"production cursor advancement before retirement");}else require(candidate->getID()==2,"production team cursor advancement");retireCandidate();}
    require(usage()==units && AllocationFault::live()==live,"failed or terminal candidate retires heap and all pooled units");
    if(prototypeOperation){factory.add("GeneratedCandidate");require(factory.prototype("GeneratedCandidate")->getID()==(failed?2u:3u) && factory.team("GeneratedCandidate")->getID()==(failed?2u:3u),"corrected retry proves both source cursors rollback or advance");}
    else{candidate=onPrototype?factory.owner->createTeamOnPrototype(factory.prototype("GeneratedMultiple")):factory.owner->createInactiveTeam("GeneratedMultiple");require(candidate->getID()==(failed?2u:3u),"corrected retry proves source team cursor rollback or advance");}
    retireCandidate();require(usage()==units && AllocationFault::live()==live,"each corrected retry retires every acquired ownership unit");
  }
  context.players->init();std::cout<<label<<" prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void faults(Context& context){manifest(context,"singleton prototype/member",true,false);manifest(context,"inactive team",false,false);manifest(context,"active team on prototype",false,true);}
void poolFailures(Context& context){
  for(unsigned selected:{0u,1u,3u,4u}){Factory factory;factory.add("team");auto* base=factory.team("team");auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[selected]);
    std::vector<void*> held;held.reserve(pool->getFreeBlockCount());struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated team pool exhaustion"));
    const auto live=AllocationFault::live();const auto units=usage();bool failed=false;
    try{factory.add("GeneratedCandidate");}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed && factory.team("team")==base && !factory.prototype("GeneratedCandidate") && usage()==units && AllocationFault::live()==live,"actual pooled exhaustion rolls back both prototype/player registrations and prior maps");
    for(auto* block:held)pool->freeBlock(block);held.clear();factory.add("GeneratedCandidate");
    require(factory.prototype("GeneratedCandidate")->getID()==2 && factory.team("GeneratedCandidate")->getID()==2,"pooled retry preserves both cursors");
    std::cout<<pools[selected]<<" actual exhaustion and corrected retry\n";
  }
  context.players->init();
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"team family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool(pools[0],sizeof(PlayerRelationMap),64,0);
    TheMemoryPoolFactory->createMemoryPool(pools[1],sizeof(TeamRelationMap),64,0);
    TheMemoryPoolFactory->createMemoryPool(pools[2],sizeof(Squad),512,0);
    TheMemoryPoolFactory->createMemoryPool(pools[3],sizeof(TeamPrototype),32,0);
    TheMemoryPoolFactory->createMemoryPool(pools[4],sizeof(Team),32,0);
    {Context context;
      {Factory warm;warm.add("team");warm.add("GeneratedCandidate");warm.add("GeneratedMultiple",FALSE);warm.owner->createTeamOnPrototype(warm.prototype("GeneratedMultiple"));warm.add("GeneratedForeign");}
      context.names.nameToKey("MissingPrototype");
      functional(context); // Warm player-owned relationship bucket capacity too.
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto units=usage();const std::string_view family=argv[1];
        if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else if(family=="pool-failures")poolFailures(context);else require(false,"unknown family");
        require(AllocationFault::live()==live && usage()==units && !TheTeamFactory,"three whole actual team factories retire exactly");
      }
    }
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original memory manager retirement");std::cout<<"PASS actual default-team/prototype ownership; actors/root pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL team "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
