// SPDX-License-Identifier: GPL-3.0-or-later
// Real source player/cohort ownership; not startup or registered-actor acceptance.
#include "AllocationFault.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Team.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/Squad.h"
#include <array>
#include <climits>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
constexpr std::array<const char*,5> pools{"PlayerRelationMapPool","TeamRelationMapPool","Squad","BuildListInfo","KindOfPercentProductionChange"};
std::array<Int,5> usage(){std::array<Int,5> result{};for(unsigned n=0;n<pools.size();++n){auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[n]);require(pool,"actual player pool exists");result[n]=pool->getUsedBlockCount();}return result;}
struct Context {
  NameKeyGenerator names;RankInfoStore ranks;
  NameKeyGenerator* previousNames=TheNameKeyGenerator;RankInfoStore* previousRanks=TheRankInfoStore;
  Context(){names.init();ranks.init();TheNameKeyGenerator=&names;TheRankInfoStore=&ranks;}
  ~Context(){TheRankInfoStore=previousRanks;TheNameKeyGenerator=previousNames;}
};
struct Published {
  PlayerList* previous=ThePlayerList;
  std::unique_ptr<PlayerList> owner=std::make_unique<PlayerList>();
  Published(){ThePlayerList=owner.get();}
  ~Published(){ThePlayerList=owner.get();owner.reset();ThePlayerList=previous;}
};
void defaults(Player& player,Int index){
  require(player.getPlayerIndex()==index && player.getRankLevel()==1 && player.getSkillPoints()==0 && player.getSkillPointsLevelUp()==INT_MAX,"source neutral index/rank defaults with actual empty rank store");
  require(player.getCanBuildUnits() && player.getCanBuildBase() && !player.isLogicalRetaliationModeEnabled(),"source neutral capability defaults");
  for(Int n=0;n<NUM_HOTKEY_SQUADS;++n)require(player.getHotkeySquad(n) && !player.getHotkeySquad(n)->getSizeOfGroup(),"all source hotkey squads acquired");
  require(!player.getHotkeySquad(-1) && !player.getHotkeySquad(NUM_HOTKEY_SQUADS),"invalid squad index does not expose backing");
}
void populate(Player& player){
  auto* first=newInstance(BuildListInfo);player.setBuildList(first);
  auto* second=newInstance(BuildListInfo);first->setNextBuildList(second);
  first->setDesiredGatherers(3);second->setDesiredGatherers(4);
  KindOfMaskType kind;kind.set(KINDOF_STRUCTURE);
  player.addKindOfProductionCostChange(kind,0.75f);
  player.addKindOfProductionCostChange(kind,0.75f); // Source reference count, not a second owner.
  player.getHotkeySquad(0)->addObjectID(ObjectID(123));
  require(player.getBuildList()->getNext()==second && player.getHotkeySquad(0)->getSizeOfGroup()==1,"actual linked build and selection behavior");
}
void functional(){
  {Player player(3);defaults(player,3);populate(player);player.init(nullptr);defaults(player,3);require(!player.getBuildList(),"explicit init retires owned build chain");populate(player);}
  {Published players;require(players.owner->getPlayerCount()==1 && players.owner->getNeutralPlayer()==players.owner->getLocalPlayer(),"actual cohort initial neutral publication");
    for(Int n=0;n<MAX_PLAYER_COUNT;++n){auto* player=players.owner->getNthPlayer(n);require(player,"complete source cohort");defaults(*player,n);populate(*player);}
    players.owner->init();
    for(Int n=0;n<MAX_PLAYER_COUNT;++n){defaults(*players.owner->getNthPlayer(n),n);populate(*players.owner->getNthPlayer(n));}
    require(!players.owner->getNthPlayer(-1) && !players.owner->getNthPlayer(MAX_PLAYER_COUNT),"cohort bounds reject");
    auto* other=players.owner->getNthPlayer(1);players.owner->setLocalPlayer(other);require(players.owner->getLocalPlayer()==other,"actual source local selection");players.owner->setLocalPlayer(players.owner->getNeutralPlayer());
  }
}
template<class Operation>void manifest(const char* label,Operation operation){
  std::size_t count;
  {AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}count=AllocationFault::attempts();AllocationFault::disarm();}
  for(std::size_t n=0;n<=count;++n){const auto live=AllocationFault::live();const auto units=usage();bool failed=false;
    AllocationFault::arm(n);try{operation();}catch(const std::bad_alloc&){failed=true;}catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"original acquisition error class");failed=true;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(n<count),"complete failure prefixes and exact terminal");
    require(AllocationFault::live()==live && usage()==units,"whole failed/terminal player ownership retirement");operation();
    require(AllocationFault::live()==live && usage()==units,"every corrected player retry retires ownership");
  }
  std::cout<<label<<" prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void faults(){
  manifest("actual player",[]{Player player(3);defaults(player,3);populate(player);});
  manifest("actual player cohort",[]{Published players;for(Int n=0;n<MAX_PLAYER_COUNT;++n)populate(*players.owner->getNthPlayer(n));});
  manifest("player reset whole owner",[]{Player player(3);populate(player);player.init(nullptr);defaults(player,3);});
  Player player(3);KindOfMaskType kind;kind.set(KINDOF_STRUCTURE);
  manifest("production-change registration",[&]{player.addKindOfProductionCostChange(kind,0.5f);player.removeKindOfProductionCostChange(kind,0.5f);});
}
void negative(){
  auto* previous=TheRankInfoStore;TheRankInfoStore=nullptr;bool rejected=false;
  try{Player player(3);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}catch(...){TheRankInfoStore=previous;throw;}
  TheRankInfoStore=previous;require(rejected,"missing rank authority rejects actual construction");
  Player player(3);auto* accepted=player.getHotkeySquad(0);accepted->addObjectID(ObjectID(123));TheRankInfoStore=nullptr;rejected=false;
  try{player.init(nullptr);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}catch(...){TheRankInfoStore=previous;throw;}
  TheRankInfoStore=previous;require(rejected && player.getHotkeySquad(0)==accepted && accepted->getSizeOfGroup()==1,"missing rank rejects reset before accepted selection mutation");
  // Destructors must make no standard allocation, including the whole cohort.
  auto owner=std::make_unique<PlayerList>();const auto live=AllocationFault::live();
  AllocationFault::arm(0);owner.reset();const auto count=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(!count && !hit && AllocationFault::live()<live,"cohort destruction never allocates initialization");
}
void poolFailures(){
  for(unsigned selected=0;selected<3;++selected){auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[selected]);std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire {MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated player pool exhaustion"));
    const auto live=AllocationFault::live();const auto units=usage();bool failed=false;
    try{Player player(3);}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed && AllocationFault::live()==live && usage()==units,"actual typed exhaustion unwinds prior pooled acquisitions");
    for(auto* block:held)pool->freeBlock(block);held.clear();{Player retry(3);defaults(retry,3);}
    require(usage()[selected]==0,"corrected player retry retires exhausted pool");
    std::cout<<pools[selected]<<" actual exhaustion and corrected retry\n";
  }
  // Every partial replacement acquisition preserves accepted selection/contents.
  for(Int available=0;available<=NUM_HOTKEY_SQUADS+1;++available){
    Player player(3);auto* accepted=player.getHotkeySquad(0);accepted->addObjectID(ObjectID(123));
    auto* pool=TheMemoryPoolFactory->findMemoryPool("Squad");std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire {MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
    while(pool->getFreeBlockCount()>available)held.push_back(pool->allocateBlock("generated partial squad exhaustion"));
    const auto units=usage();const auto live=AllocationFault::live();bool failed=false;
    try{player.init(nullptr);}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed==(available<NUM_HOTKEY_SQUADS+1) && usage()==units,"exact eleven-squad pooled terminal");
    if(failed)require(AllocationFault::live()==live && player.getHotkeySquad(0)==accepted && accepted->getSizeOfGroup()==1,"partial eleven-squad acquisition preserves accepted backing and pooled units");
    else defaults(player,3);
    for(auto* block:held)pool->freeBlock(block);held.clear();player.init(nullptr);defaults(player,3);
  }
  std::cout<<"squad replacement pooled prefixes [0,11); terminal 11; every retry\n";
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"player family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool(pools[0],sizeof(PlayerRelationMap),64,0);
    TheMemoryPoolFactory->createMemoryPool(pools[1],sizeof(TeamRelationMap),64,0);
    TheMemoryPoolFactory->createMemoryPool(pools[2],sizeof(Squad),512,0);
    TheMemoryPoolFactory->createMemoryPool(pools[3],sizeof(BuildListInfo),128,0);
    TheMemoryPoolFactory->createMemoryPool(pools[4],sizeof(KindOfPercentProductionChange),64,0);
    {Context context;
      functional(); // Warm actual name keys before the fault/resource census.
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto units=usage();const std::string_view family=argv[1];
        if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();else if(family=="pool-failures")poolFailures();else require(false,"unknown family");
        require(AllocationFault::live()==live && usage()==units,"three actual player/cohort owners retire exactly");
      }
    }
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original memory manager retirement");std::cout<<"PASS actual player/cohort ownership; actor/root integration pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL player "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
