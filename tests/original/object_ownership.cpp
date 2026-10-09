// SPDX-License-Identifier: GPL-3.0-or-later
// Actual registered actor admission/retirement; not full startup/simulation.
#include "AllocationFault.h"
#include "Common/Dict.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/ModuleFactory.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/ControlBar.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/AI.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Module/ObjectSMCHelper.h"
#include "GameLogic/Module/ObjectDefectionHelper.h"
#include "GameLogic/Module/StatusDamageHelper.h"
#include "GameLogic/Module/SubdualDamageHelper.h"
#include "GameLogic/Module/DeletionUpdate.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct UI:InGameUI {
  void draw()override{} // Only presentation is absent; retain every logical callback.
  View* createView()override{throw ERROR_BAD_ARG;}
};
struct RadarOwner:Radar {
  void draw(Int,Int,Int,Int)override{}
  void clearShroud()override{}
  void setShroudLevel(Int,Int,CellShroudStatus)override{}
};
struct Logic:GameLogic {
  void defaults(){setObjectIDCounter(ObjectID(1));}
  void flush(){processDestroyList();}
  void clear(){destroyAllObjectsImmediate();}
  std::size_t updates()const{return getNumberSleepyUpdates();}
};
// Expose the original protected ID operation on a genuinely constructed
// derived actor; never cast an original base object to a fictitious subtype.
struct InspectObject:Object {using Object::Object;using Object::setID;};
static_assert(sizeof(InspectObject)==sizeof(Object));
template<class Base,class Concrete=Base>struct Published {
  Base*& slot;Base* previous;std::unique_ptr<Concrete> owner;
  explicit Published(Base*& value):slot(value),previous(value){}
  void create(){owner=std::make_unique<Concrete>();slot=owner.get();}
  void retire(){if(owner){slot=owner.get();owner.reset();slot=previous;}}
  ~Published(){retire();}
};
struct Tree {
  std::filesystem::path root;FileSystem files;FileSystem* previous=TheFileSystem;
  Tree(){char pattern[]="/tmp/zh-actor-ownership-XXXXXX";auto* value=::mkdtemp(pattern);require(value,"generated actor root");root=value;
    try{std::ofstream out(root/"actors.ini");
      out<<"Object GeneratedActor\n RadarPriority = UNIT\n Geometry = BOX\n GeometryMajorRadius = 5\n GeometryMinorRadius = 5\n End\n";
      out<<"Object GeneratedManyUpdates\n RadarPriority = UNIT\n";
      for(unsigned n=0;n<270;++n)out<<" Behavior = DeletionUpdate GeneratedTag"<<n<<"\n MinLifetime = 10000\n MaxLifetime = 10000\n End\n";
      out<<"End\n";out.close();require(bool(out),"complete generated actual object definitions");
      files.mountReadOnly({root.string()});TheFileSystem=&files;
    }catch(...){std::error_code error;std::filesystem::remove_all(root,error);throw;}}
  ~Tree(){TheFileSystem=previous;std::error_code error;std::filesystem::remove_all(root,error);}
};
struct ControlLease {ControlBar* previous=TheControlBar;ControlLease(){TheControlBar=nullptr;}~ControlLease(){TheControlBar=previous;}};
struct Context {
  Tree tree;ControlLease control;
  Published<NameKeyGenerator> names{TheNameKeyGenerator};
  Published<GlobalData> data{TheWritableGlobalData};
  Published<RankInfoStore> ranks{TheRankInfoStore};
  Published<ScriptEngine> scripts{TheScriptEngine};
  Published<AI> ai{TheAI};
  Published<InGameUI,UI> ui{TheInGameUI};
  Published<Radar,RadarOwner> radar{TheRadar};
  Published<ModuleFactory> modules{TheModuleFactory};
  Published<ThingFactory> templates{TheThingFactory};
  Published<PartitionManager> partition{ThePartitionManager};
  Published<PlayerList> players{ThePlayerList};
  Published<TeamFactory> teams{TheTeamFactory};
  Published<GameLogic,Logic> logic{TheGameLogic};
  Context(){try{names.create();names.owner->init();data.create();ranks.create();ranks.owner->init();
    scripts.create();ai.create();ai.owner->init();logic.create();logic.owner->defaults();
    ui.create();radar.create();modules.create();modules.owner->init();templates.create();templates.owner->init();
    INI ini;ini.load("actors.ini",INI_LOAD_OVERWRITE,nullptr);
    partition.create();partition.owner->init();players.create();teams.create();teams.owner->init();
    Dict definition;teams.owner->initTeam("team","",TRUE,&definition);players.owner->getNeutralPlayer()->setDefaultTeam();
    }catch(...){retire();throw;}}
  void retire(){
    // Two phases: actor callbacks require all parents; radar cleanup itself
    // consults GameLogic even after its last radar node has retired.
    if(logic.owner)logic.owner->clear();
    ui.retire();radar.retire();partition.retire();teams.retire();players.retire();
    ai.retire();scripts.retire();templates.retire();modules.retire();logic.retire();
    ranks.retire();data.retire();names.retire();
  }
  ~Context(){retire();}
  Object* make(bool many=false){return logic.owner->friend_createObject(templates.owner->findTemplate(many?"GeneratedManyUpdates":"GeneratedActor",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);}
  InspectObject* inspectable(){return new (Object::ObjectMagicEnum(0)) InspectObject(templates.owner->findTemplate("GeneratedActor",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);}
  void retire(Object* object){logic.owner->destroyObject(object);logic.owner->flush();}
};
constexpr std::array<const char*,8> pools{"ObjectPool","SightingInfo","ObjectSMCHelperPool","StatusDamageHelper","SubdualDamageHelper","ObjectDefectionHelperPool","ExperienceTrackerPool","RadarObject"};
std::array<Int,8> usage(){std::array<Int,8> result{};for(unsigned n=0;n<pools.size();++n){auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[n]);require(pool,"actual actor pool exists");result[n]=pool->getUsedBlockCount();}return result;}
void functional(Context& context){auto& logic=*context.logic.owner;auto* first=context.make();auto* second=context.make(true);
  const auto firstID=first->getID(),secondID=second->getID();
  require(firstID+1==secondID && logic.getObjectIDCounter()==UnsignedInt(secondID)+1 && logic.findObjectByID(firstID)==first && logic.getFirstObject()==second,"source cursor/lookup/intrusive admission");
  require(first->getTeam()==context.players.owner->getNeutralPlayer()->getDefaultTeam() && first->friend_getRadarData() && first->areModulesReady(),"actual team/radar/module participants");
  const auto count=logic.updates();require(count>270,"actual stock helper and update cardinality exceeds old cleanup cap");
  for(unsigned pass=0;pass<3;++pass){unsigned n=0;
    for(auto** module=second->getBehaviorModules();*module;++module)
      if(auto* update=dynamic_cast<DeletionUpdate*>(*module)){const auto delay=UnsignedInt(pass==1?20000-n%19:2+n%17);update->setLifetimeRange(delay,delay);
        require(update->getDieFrame()==logic.getFrame()+delay,"actual stock wake-frame updates preserve source timing");++n;}
    require(n==270 && logic.updates()==count,"both heap directions retain every actual scheduler owner");}
  context.retire(second);require(logic.updates()==4 && !logic.findObjectByID(secondID) && logic.findObjectByID(firstID)==first && logic.getFirstObject()==first,"all many-module scheduler owners retire and survivor remains");
  context.retire(first);require(!logic.updates() && !logic.getFirstObject(),"complete source destruction queue and registry retirement");
}
template<class Action>void rejects(Action action){bool rejected=false;try{action();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}require(rejected,"invalid actor admission rejects");}
void negative(Context& context){auto& logic=*context.logic.owner;auto* first=context.inspectable();auto* second=context.make();const auto id=first->getID();const auto live=AllocationFault::live();const auto units=usage();const auto updates=logic.updates();
  rejects([&]{first->setID(second->getID());});rejects([&]{first->setID(INVALID_ID);});rejects([&]{logic.registerObject(first);});
  rejects([&]{logic.friend_createObject(nullptr,OBJECT_STATUS_MASK_NONE,nullptr);});
  require(first->getID()==id && logic.findObjectByID(id)==first && logic.updates()==updates && usage()==units && AllocationFault::live()==live,"rejection preserves accepted actor identities and units");
  require(!logic.findObjectByID(ObjectID(std::numeric_limits<UnsignedInt>::max())),"unsigned malformed lookup remains bounded");
  context.retire(second);context.retire(first);
}
void faults(Context& context){auto& logic=*context.logic.owner;
  const auto start=logic.getObjectIDCounter();std::size_t count;
  AllocationFault::arm(SIZE_MAX);Object* discovered=nullptr;try{discovered=context.make();}catch(...){AllocationFault::disarm();throw;}
  count=AllocationFault::attempts();AllocationFault::disarm();context.retire(discovered);logic.setObjectIDCounter(start);
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();const auto units=usage();Object* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=context.make();}catch(const std::bad_alloc&){failed=true;}catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"actual actor allocation error class");failed=true;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<count),"all actor failure prefixes and exact terminal");
    if(failed)require(!logic.getFirstObject() && !logic.findObjectByID(ObjectID(start)) && logic.getObjectIDCounter()==start && !logic.updates() && usage()==units && AllocationFault::live()==live,"partial actor rolls back every ownership/registration and cursor");
    else{require(candidate->getID()==ObjectID(start) && logic.getObjectIDCounter()==start+1,"production cursor advancement before lease restoration");context.retire(candidate);}
    candidate=context.make();require(candidate->getID()==ObjectID(start+(failed?0:1)),"corrected retry uses source rollback or advance");context.retire(candidate);
    require(!logic.updates() && !logic.getFirstObject() && usage()==units && AllocationFault::live()==live,"every corrected actor retry retires exactly");logic.setObjectIDCounter(start);
  }
  // Queue allocation failure must leave a live, unmarked actor eligible for retry.
  auto* object=context.make();const auto id=object->getID();AllocationFault::arm(0);bool failed=false;
  try{logic.destroyObject(object);}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
  require(failed && !object->isDestroyed() && logic.findObjectByID(id)==object,"queue-node failure precedes destructive transitions");context.retire(object);
  std::cout<<"actor prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; queue failure/retry\n";
}
void poolFailures(Context& context){auto& logic=*context.logic.owner;
  for(unsigned selected=0;selected<pools.size();++selected){auto* pool=TheMemoryPoolFactory->findMemoryPool(pools[selected]);std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
    while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated actor exhaustion"));
    const auto start=logic.getObjectIDCounter();const auto live=AllocationFault::live();const auto units=usage();bool failed=false;
    try{context.make();}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed && logic.getObjectIDCounter()==start && !logic.getFirstObject() && !logic.updates() && usage()==units && AllocationFault::live()==live,"actual pooled exhaustion retires partial actor and registrations");
    for(auto* value:held)pool->freeBlock(value);held.clear();auto* object=context.make();require(object->getID()==ObjectID(start),"pooled retry preserves source cursor");context.retire(object);
    std::cout<<pools[selected]<<" actual exhaustion/retry\n";
  }
}
void parentFaults(Context& context){
  std::size_t count=0;
  {Published<InGameUI,UI> candidate{TheInGameUI};AllocationFault::arm(SIZE_MAX);
    try{candidate.create();}catch(...){AllocationFault::disarm();throw;}
    count=AllocationFault::attempts();AllocationFault::disarm();}
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();bool failed=false;
    {Published<InGameUI,UI> candidate{TheInGameUI};AllocationFault::arm(ordinal);
      try{candidate.create();}catch(const std::bad_alloc&){failed=true;}
      const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<count),"complete original UI constructor manifest");}
    require(TheInGameUI==context.ui.owner.get() && AllocationFault::live()==live,"UI partial placement backing and publication retire exactly");
    {Published<InGameUI,UI> retry{TheInGameUI};retry.create();}
    require(TheInGameUI==context.ui.owner.get() && AllocationFault::live()==live,"corrected UI retry retires exactly");
  }
  const auto prior=context.data.owner->m_maxLineBuildObjects;context.data.owner->m_maxLineBuildObjects=-1;
  struct Restore{Int& slot;Int value;~Restore(){slot=value;}} restore{context.data.owner->m_maxLineBuildObjects,prior};
  rejects([]{UI candidate;});
  std::cout<<"UI prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void updatePoolFailures(Context& context,unsigned begin,unsigned end){auto& logic=*context.logic.owner;
  auto* pool=TheMemoryPoolFactory->findMemoryPool("DeletionUpdate");require(pool,"actual stock update pool");
  for(unsigned available=begin;available<end;++available){const auto start=logic.getObjectIDCounter();std::vector<void*> held;
    held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
    while(pool->getFreeBlockCount()>Int(available))held.push_back(pool->allocateBlock("generated update prefix exhaustion"));
    const auto live=AllocationFault::live();const auto units=usage();const auto before=pool->getUsedBlockCount();bool failed=false;Object* candidate=nullptr;
    try{candidate=context.make(true);}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed==(available<270),"all stock module acquisition prefixes and exact pooled terminal");
    if(failed)require(!logic.getFirstObject() && !logic.updates() && logic.getObjectIDCounter()==start,"partial stock modules never leave scheduler/actor publication");
    else{require(candidate->getID()==ObjectID(start) && logic.getObjectIDCounter()==start+1,"many-module production cursor advance");context.retire(candidate);}
    require(pool->getUsedBlockCount()==before && usage()==units && AllocationFault::live()==live,"every acquired stock module prefix retires");
    for(auto* value:held)pool->freeBlock(value);held.clear();candidate=context.make(true);
    require(candidate->getID()==ObjectID(start+(failed?0:1)),"every stock module prefix has corrected source cursor retry");context.retire(candidate);
    require(pool->getUsedBlockCount()==0 && usage()==units && AllocationFault::live()==live,"corrected full stock-module retry retires all units");logic.setObjectIDCounter(start);
  }
  std::cout<<"stock update available ["<<begin<<','<<end<<"); every failure/retry; terminal270\n";
}
void idGrowth(){
  // Each candidate starts from the source's initial 8192-entry lookup. A
  // fresh whole owner keeps every failure prefix and terminal comparable.
  for(unsigned ordinal=0;ordinal<=1;++ordinal){const auto outside=AllocationFault::live();
    {Context context;auto& logic=*context.logic.owner;auto* actor=context.inspectable();const auto old=actor->getID();bool failed=false;
      AllocationFault::arm(ordinal);try{actor->setID(ObjectID(8192));}catch(const std::bad_alloc&){failed=true;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal==0) && attempts==1,"ID growth exact one-allocation terminal");
      if(failed)require(actor->getID()==old && logic.findObjectByID(old)==actor && !logic.findObjectByID(ObjectID(8192)),"ID growth failure preserves old lookup identity");
      actor->setID(ObjectID(8192));require(actor->getID()==ObjectID(8192) && logic.findObjectByID(ObjectID(8192))==actor && !logic.findObjectByID(old),"corrected strong ID replacement retires old entry");
      context.retire(actor);}
    require(AllocationFault::live()==outside,"whole ID growth root retires retained capacity");
  }
}
void coldFaults(bool schedulerGrowth){
  auto prepare=[&](Context& context){functional(context);context.logic.retire();context.logic.create();context.logic.owner->defaults();
    return schedulerGrowth?context.make():nullptr;};
  std::size_t count=0;
  {Context context;auto* accepted=prepare(context);AllocationFault::arm(SIZE_MAX);Object* candidate=nullptr;
    try{candidate=context.make(schedulerGrowth);}catch(...){AllocationFault::disarm();throw;}
    count=AllocationFault::attempts();AllocationFault::disarm();context.retire(candidate);if(accepted)context.retire(accepted);}
  // Short template-name lookup adds one allocation; the long name adds two.
  require(count==4,"complete caller plus array/lookup/scheduler cold manifest");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto outside=AllocationFault::live();
    {Context context;auto* accepted=prepare(context);auto& logic=*context.logic.owner;const auto start=logic.getObjectIDCounter();
      const auto units=usage();const auto updates=logic.updates();Object* candidate=nullptr;bool failed=false;
      AllocationFault::arm(ordinal);try{candidate=context.make(schedulerGrowth);}catch(const std::bad_alloc&){failed=true;}
      const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(ordinal<count),"every cold actor prefix and exact terminal");
      if(failed)require(logic.getFirstObject()==accepted && !logic.findObjectByID(ObjectID(start)) && logic.getObjectIDCounter()==start && logic.updates()==updates && usage()==units,"cold failure withdraws actor/radar/scheduler and preserves accepted participants");
      else{require(candidate->getID()==ObjectID(start) && logic.getObjectIDCounter()==start+1,"cold terminal production cursor advance");context.retire(candidate);}
      candidate=context.make(schedulerGrowth);require(candidate->getID()==ObjectID(start+(failed?0:1)),"every cold failure/terminal has cursor-correct retry");context.retire(candidate);
      require(usage()==units && logic.updates()==updates && logic.getFirstObject()==accepted,"cold retry retires all acquired actor units");
      if(accepted){require(logic.findObjectByID(accepted->getID())==accepted && accepted->friend_getRadarData(),"accepted survivor retains lookup and radar");context.retire(accepted);}}
    require(AllocationFault::live()==outside,"whole cold owner retires all legitimately retained registry backing");
  }
  std::cout<<(schedulerGrowth?"late scheduler":"cold lookup")<<" prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"actor family");initMemoryManager();initialized=true;
    const std::array<std::size_t,8> sizes{sizeof(Object),sizeof(SightingInfo),sizeof(ObjectSMCHelper),sizeof(StatusDamageHelper),sizeof(SubdualDamageHelper),sizeof(ObjectDefectionHelper),sizeof(ExperienceTracker),sizeof(RadarObject)};
    for(unsigned n=0;n<pools.size();++n)TheMemoryPoolFactory->createMemoryPool(pools[n],sizes[n],n==1?128:64,0);
    TheMemoryPoolFactory->createMemoryPool("DeletionUpdate",sizeof(DeletionUpdate),512,0);
    {Context context;functional(context);
      {Context nested;functional(nested);} // Warm shared Squad arenas for two live cohorts.
      const auto units=usage();const auto live=AllocationFault::live();
      for(unsigned repeat=0;repeat<3;++repeat){const std::string_view family=argv[1];
        if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else if(family=="pool-failures")poolFailures(context);else if(family=="parent-faults")parentFaults(context);else if(family=="id-growth")idGrowth();
        else if(family=="pool-updates-early")updatePoolFailures(context,0,90);else if(family=="pool-updates-middle")updatePoolFailures(context,90,180);else if(family=="pool-updates-late")updatePoolFailures(context,180,271);
        else if(family=="cold-faults")coldFaults(false);else if(family=="scheduler-faults")coldFaults(true);else require(false,"unknown actor family");
        require(usage()==units && AllocationFault::live()==live,"repeated actor retirement preserves complete accepted parents");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original actor parents and manager retire");
    std::cout<<"PASS actual registered actor ownership; full root/simulation pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL actor "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
