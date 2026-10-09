// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/SidesList.h"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
struct InfoInspection:PathfindCellInfo {
  static std::size_t owners(){return s_owners;}
  static std::size_t used(){return s_used;}
  static bool backing(){return s_infoArray!=nullptr;}
};
struct InspectAI:AI {
  TAiData* data(){return m_aiData;}
  void overrideData(){newOverride();}
  UnsignedInt cursor(){return m_nextGroupID;}
  std::size_t groups(){return m_groupList.size();}
};
// Retire the actual AI while its legitimate singleton publication remains live.
struct Published {
  AI* previous=TheAI;
  std::unique_ptr<InspectAI> owner=std::make_unique<InspectAI>();
  Published(){TheAI=owner.get();owner->init();}
  ~Published(){owner.reset();TheAI=previous;}
};
constexpr std::array<const char*,4> poolNames{"AISideInfo","AISideBuildList","BuildListInfo","AIGroupPool"};
std::array<Int,4> usage(){std::array<Int,4> result{};for(unsigned n=0;n<4;++n){auto* pool=TheMemoryPoolFactory->findMemoryPool(poolNames[n]);require(pool,"original pool exists");result[n]=pool->getUsedBlockCount();}return result;}
void populate(InspectAI& ai){
  for(unsigned n=0;n<2;++n){auto* side=newInstance(AISideInfo);side->m_side=n?"SecondSide":"FirstSide";side->m_easy=3+n;ai.data()->addSideInfo(side);}
  auto* list=newInstance(AISideBuildList)("BuiltSide");ai.data()->addFactionBuildList(list);
  for(unsigned n=0;n<2;++n){auto* node=newInstance(BuildListInfo);node->setDesiredGatherers(7+n);list->addInfo(node);}
  ai.data()->addFactionBuildList(newInstance(AISideBuildList)("EmptySide"));
}
void cloned(InspectAI& ai,TAiData* base){auto* copy=ai.data();require(copy!=base && copy->m_next==base,"override published only after complete cloning");
  require(copy->m_sideInfo!=base->m_sideInfo && copy->m_sideInfo->m_side=="FirstSide" && copy->m_sideInfo->m_next->m_side=="SecondSide","original prepend clone order and independent side nodes");
  require(copy->m_sideInfo->m_skillSet1.m_numSkills==0 && copy->m_sideInfo->m_skillSet5.m_numSkills==0,"omitted skill sets are inactive, defined defaults");
  auto* build=copy->m_sideBuildLists;require(build->m_side=="BuiltSide" && build->m_next->m_side=="EmptySide" && !build->m_next->m_buildList,"source list order and valid empty build list");
  require(build->m_buildList!=base->m_sideBuildLists->m_next->m_buildList && build->m_buildList->getDesiredGatherers()==7,"independent nonempty build node clone");
}
void functional(){
  const ICoord2D pos{2,3};PathfindCell cell;
  {
    auto first=std::make_unique<Pathfinder>();require(InfoInspection::owners()==1,"first owner");
    auto* retained=PathfindCellInfo::getACellInfo(&cell,pos);require(retained,"acquired info");
    {Pathfinder second;require(InfoInspection::owners()==2 && InfoInspection::used()==1,"second owner preserves first live cell");second.reset();second.reset();}
    first.reset();require(!InfoInspection::owners() && InfoInspection::used()==1 && InfoInspection::backing(),"last-owner release retains acquired cell backing");
    require(!PathfindCellInfo::getACellInfo(&cell,pos),"new cells require a live owner");
    {Pathfinder reacquired;require(InfoInspection::owners()==1 && InfoInspection::used()==1,"reacquire retained backing without eviction");}
    PathfindCellInfo::releaseACellInfo(retained);require(!InfoInspection::backing(),"last acquired unit retires orphan backing");
  }
  {Pathfinder owner;std::vector<PathfindCellInfo*> held;held.reserve(30000);
    for(unsigned n=0;n<30000;++n){auto* info=PathfindCellInfo::getACellInfo(&cell,pos);require(info,"all source info units available");held.push_back(info);}
    require(!PathfindCellInfo::getACellInfo(&cell,pos) && InfoInspection::used()==30000,"exact source capacity exhaustion");
    auto* released=held.back();held.pop_back();PathfindCellInfo::releaseACellInfo(released);
    auto* reused=PathfindCellInfo::getACellInfo(&cell,pos);require(reused==released,"source free-list reuse");held.push_back(reused);
    for(auto* info:held)PathfindCellInfo::releaseACellInfo(info);
    require(cell.allocateInfo(pos),"actual cell acquisition");cell.reset();cell.reset();require(!InfoInspection::used(),"actual cell repeated reset retires acquired info");
  }
  {Published published;auto& ai=*published.owner;populate(ai);auto* base=ai.data();ai.overrideData();cloned(ai,base);
    auto* first=ai.createGroup();auto* second=ai.createGroup();require(first->getID()==1 && second->getID()==2 && ai.findGroup(2)==second,"source IDs and actual registration");
    ai.destroyGroup(first);require(!ai.findGroup(1) && ai.groups()==1,"explicit group retirement");ai.reset();require(ai.data()==base && !ai.groups() && !ai.cursor(),"reset retires overrides and registered groups");
    ai.createGroup();ai.createGroup(); // Destructor must retire these without a caller reset.
  }
}
void negative(){Pathfinder owner;PathfindCell cell;const ICoord2D pos{0,0};PathfindCellInfo foreign;
  auto reject=[&](PathfindCellInfo* info){const auto live=AllocationFault::live();bool failed=false;try{PathfindCellInfo::releaseACellInfo(info);}catch(ErrorCode code){failed=code==ERROR_BAD_ARG;}
    require(failed && !InfoInspection::used() && AllocationFault::live()==live,"invalid release rejected before mutation");};
  reject(nullptr);reject(&foreign);auto* info=PathfindCellInfo::getACellInfo(&cell,pos);PathfindCellInfo::releaseACellInfo(info);reject(info);
  AI unpublished;bool failed=false;try{unpublished.createGroup();}catch(ErrorCode code){failed=code==ERROR_BAD_ARG;}require(failed,"group acquisition requires legitimate source singleton");
}
template<class Operation,class Verify> void manifest(const char* label,Operation operation,Verify verify){
  AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}
  const auto count=AllocationFault::attempts();AllocationFault::disarm();verify();
  for(std::size_t n=0;n<=count;++n){const auto live=AllocationFault::live();const auto pools=usage();bool failed=false;
    AllocationFault::arm(n);try{operation();}catch(const std::bad_alloc&){failed=true;}catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"source acquisition error class");failed=true;}
    const auto triggered=AllocationFault::triggered();AllocationFault::disarm();require(triggered==failed && failed==(n<count),"complete failure prefixes and exact terminal");verify();
    require(AllocationFault::live()==live && usage()==pools,"failed operation or terminal retires every ownership unit");operation();verify();
    require(AllocationFault::live()==live && usage()==pools,"every corrected retry retires every ownership unit");
  }
  std::cout<<label<<" prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void faults(){
  manifest("cold AI",[]{AI owner;owner.init();owner.reset();},[]{require(!InfoInspection::owners() && !InfoInspection::used() && !InfoInspection::backing(),"failed or completed cold owner leaves no backing");});
  Published published;auto& ai=*published.owner;populate(ai);auto* base=ai.data();
  manifest("warm AI",[]{AI owner;owner.init();owner.reset();},[]{require(InfoInspection::owners()==1 && !InfoInspection::used(),"warm failure preserves published owner");});
  manifest("override",[&]{ai.overrideData();cloned(ai,base);ai.reset();},[&]{require(ai.data()==base && !ai.data()->m_next && !ai.cursor(),"override failure preserves accepted identity and cursors");});
  manifest("group",[&]{auto* group=ai.createGroup();require(group->getID()==1,"retry preserves source ID");ai.reset();},[&]{require(!ai.groups() && !ai.cursor(),"group rollback preserves registry and ID");});
}
void poolFailures(){Published published;auto& ai=*published.owner;populate(ai);auto* base=ai.data();
  for(unsigned selected=0;selected<4;++selected){auto* pool=TheMemoryPoolFactory->findMemoryPool(poolNames[selected]);std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated AI pool exhaustion"));
    const auto before=usage();const auto live=AllocationFault::live();bool failed=false;
    try{if(selected==3)ai.createGroup();else ai.overrideData();}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed && usage()==before && AllocationFault::live()==live && ai.data()==base && !ai.groups() && !ai.cursor(),"actual typed pool exhaustion rolls back whole operation");
    for(auto* block:held)pool->freeBlock(block);held.clear();
    if(selected==3){auto* group=ai.createGroup();require(group->getID()==1,"pool retry preserves ID");}else{ai.overrideData();cloned(ai,base);}ai.reset();
    std::cout<<poolNames[selected]<<" actual exhaustion and corrected retry\n";
  }
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"AI/pathfinder family");initMemoryManager();initialized=true;
    if(std::string_view(argv[1])=="pool-failures"){
      TheMemoryPoolFactory->createMemoryPool("AISideInfo",sizeof(AISideInfo),64,0);
      TheMemoryPoolFactory->createMemoryPool("AISideBuildList",sizeof(AISideBuildList),64,0);
      TheMemoryPoolFactory->createMemoryPool("BuildListInfo",sizeof(BuildListInfo),64,0);
      TheMemoryPoolFactory->createMemoryPool("AIGroupPool",sizeof(AIGroup),64,0);
    }
    {Published warm;populate(*warm.owner);warm.owner->overrideData();warm.owner->createGroup();}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto pools=usage();const std::string_view family=argv[1];
      if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();else if(family=="pool-failures")poolFailures();else require(false,"unknown family");
      require(AllocationFault::live()==live && usage()==pools && !InfoInspection::owners() && !InfoInspection::used() && !InfoInspection::backing(),"three whole AI/pathfinder owners retire exactly");
    }
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original memory manager retirement");std::cout<<"PASS actual AI/pathfinder ownership; classified map/root integration pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL AI/pathfinder "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
