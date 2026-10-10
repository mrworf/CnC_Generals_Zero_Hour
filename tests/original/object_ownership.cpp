// SPDX-License-Identifier: GPL-3.0-or-later
// Actual registered actor admission/retirement; not full startup/simulation.
#include "AllocationFault.h"
#include "AbsentAudioFixture.h"
#include "Common/Dict.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/INI.h"
#include "Common/ModuleFactory.h"
#include "Common/MessageStream.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/ControlBar.h"
#include "GameClient/InGameUI.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "GameLogic/AI.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/ScriptConditions.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/Module/ObjectSMCHelper.h"
#include "GameLogic/Module/ObjectDefectionHelper.h"
#include "GameLogic/Module/StatusDamageHelper.h"
#include "GameLogic/Module/SubdualDamageHelper.h"
#include "GameLogic/Module/DeletionUpdate.h"
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/DestroyModule.h"
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
unsigned destroyFaultPhase=0,destroyCallbackCalls=0,deleteCallbackCalls=0;
struct DestroyAdmissionProbe:BehaviorModule,DestroyModuleInterface {
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(DestroyAdmissionProbe,"DestroyAdmissionProbe",8,0)
public:
  DestroyAdmissionProbe(Thing* thing,const ModuleData* data):BehaviorModule(thing,data){}
  NameKeyType getModuleNameKey()const override{static const StaticNameKey key("DestroyAdmissionProbe");return key.key();}
  DestroyModuleInterface* getDestroy()override{return this;}
  void onDestroy()override{
    ++destroyCallbackCalls;require(!getObject()->isDestroyed(),"source destroy callback precedes mark");
    if(destroyFaultPhase==1)throw ERROR_BAD_ARG;
    if(destroyFaultPhase==2){const auto calls=destroyCallbackCalls;
      TheGameLogic->destroyObject(getObject());require(destroyCallbackCalls==calls,"same-actor reentry does not repeat callbacks");}
  }
  void onDelete()override{
    ++deleteCallbackCalls;require(getObject()->isDestroyed(),"source delete callback follows mark and queue");
    if(destroyFaultPhase==3)throw ERROR_BAD_ARG;
  }
};
DestroyAdmissionProbe::~DestroyAdmissionProbe()=default;
struct ActorModules:ModuleFactory {
  void registerProbe(){addModuleInternal(
    [](Thing* thing,const ModuleData* data)->Module*{return newInstance(DestroyAdmissionProbe)(thing,data);},
    [](INI* ini)->ModuleData*{auto data=std::make_unique<BehaviorModuleData>();
      if(ini)ini->initFromINIMultiProc(data.get(),BehaviorModuleData::buildFieldParse);return data.release();},
    MODULETYPE_BEHAVIOR,"DestroyAdmissionProbe",MODULEINTERFACE_DESTROY);}
};
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
struct Conditions:ScriptConditions {using ScriptConditions::evaluateUnitHasEmptied;};
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
      out<<"Object GeneratedCarrier\n RadarPriority = UNIT\n Behavior = OpenContain GeneratedContainer\n ContainMax = 8\n End\n End\n";
      out<<"Object GeneratedDestroyCallbacks\n RadarPriority = UNIT\n Behavior = DestroyAdmissionProbe GeneratedDestroy\n End\n End\n";
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
  Published<ModuleFactory,ActorModules> modules{TheModuleFactory};
  Published<ThingFactory> templates{TheThingFactory};
  Published<PartitionManager> partition{ThePartitionManager};
  Published<PlayerList> players{ThePlayerList};
  Published<TeamFactory> teams{TheTeamFactory};
  Published<GameLogic,Logic> logic{TheGameLogic};
  explicit Context(bool initializeUI=true){try{names.create();names.owner->init();data.create();ranks.create();ranks.owner->init();
    scripts.create();ai.create();ai.owner->init();logic.create();logic.owner->defaults();
    if(initializeUI)ui.create();radar.create();modules.create();modules.owner->init();modules.owner->registerProbe();templates.create();templates.owner->init();
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
  // Queue ownership moved into construction: prove zero-demand terminal and
  // repeated cleanup, rather than exempting the new ownership unit from faults.
  auto* object=context.make();const auto id=object->getID();AllocationFault::arm(0);
  logic.destroyObject(object);const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(!hit && attempts==0 && object->isDestroyed() && logic.findObjectByID(id)==object,"reserved queue node transfers without allocation");
  logic.destroyObject(object);logic.flush();require(!logic.findObjectByID(id),"duplicate destruction queues actor once");
  std::cout<<"actor prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; destroy terminal0\n";
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
void destroyCallbacks(Context& context){auto& logic=*context.logic.owner;
  auto make=[&]{return logic.friend_createObject(context.templates.owner->findTemplate("GeneratedDestroyCallbacks",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);};
  auto* object=make();auto* callbackPool=TheMemoryPoolFactory->findMemoryPool("DestroyAdmissionProbe");
  require(callbackPool && callbackPool->getUsedBlockCount()==1,"actual callback module owns one typed unit");
  const auto id=object->getID();const auto units=usage();const auto live=AllocationFault::live();
  destroyFaultPhase=1;rejects([&]{logic.destroyObject(object);});destroyFaultPhase=0;
  require(!object->isDestroyed() && logic.findObjectByID(id)==object && usage()==units && callbackPool->getUsedBlockCount()==1 && AllocationFault::live()==live,"pre-mark callback rejection restores exact reserved admission");
  destroyFaultPhase=2;AllocationFault::arm(0);logic.destroyObject(object);
  const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();destroyFaultPhase=0;
  require(!hit && attempts==0 && object->isDestroyed(),"corrected recursive retry transfers admission without allocation");logic.flush();
  object=make();const auto lateID=object->getID();destroyFaultPhase=3;
  rejects([&]{logic.destroyObject(object);});destroyFaultPhase=0;
  require(object->isDestroyed() && logic.findObjectByID(lateID)==object,"late callback rejection retains marked queued actor");
  logic.destroyObject(object);logic.flush();require(!logic.findObjectByID(lateID) && !logic.updates(),"late rejection cannot strand or double-queue actor");
  object=make();context.retire(object);require(!logic.getFirstObject() && !callbackPool->getUsedBlockCount(),"corrected late retry retires all callback units normally");
}
void audioConstructorFaults(){
  const auto outside=AllocationFault::live();const auto* borrowed=TheAudio;
  auto verify=[](const AbsentAudioFixture& audio){
    require(audio.getAudioSettings() && audio.getMiscAudio() && audio.getValidSilentAudioEvent(),"all source audio constructor candidates acquired");
  };
  std::size_t count=0;
  {AllocationFault::arm(SIZE_MAX);std::unique_ptr<AbsentAudioFixture> audio;
    try{audio=std::make_unique<AbsentAudioFixture>();}catch(...){AllocationFault::disarm();throw;}
    count=AllocationFault::attempts();AllocationFault::disarm();verify(*audio);}
  require(count>=4 && AllocationFault::live()==outside,"whole source audio constructor census retires before baselines");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){
    std::unique_ptr<AbsentAudioFixture> audio;bool failed=false;
    AllocationFault::arm(ordinal);
    try{audio=std::make_unique<AbsentAudioFixture>();}catch(const std::bad_alloc&){failed=true;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count) && attempts==(failed?ordinal+1:count),"every source audio constructor failure prefix and exact terminal");
    if(audio)verify(*audio);audio.reset();
    require(TheAudio==borrowed && AllocationFault::live()==outside,"source audio constructor rollback retires all candidates without publication");
    {auto retry=std::make_unique<AbsentAudioFixture>();verify(*retry);
      auto independent=std::make_unique<AbsentAudioFixture>();verify(*independent);
      require(retry->getAudioSettings()!=independent->getAudioSettings() && retry->getMiscAudio()!=independent->getMiscAudio() && retry->getValidSilentAudioEvent()!=independent->getValidSilentAudioEvent(),"source audio owners never share candidates");}
    require(TheAudio==borrowed && AllocationFault::live()==outside,"every corrected source audio retry and independent owner retire exactly");
  }
  std::cout<<"audio constructor prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; no device acceptance\n";
}
void transportCacheOwners(Context& context){
  auto* first=context.make();auto* second=context.make();
  const AsciiString firstName("GeneratedTransportOne"),secondName("GeneratedTransportTwo"),missingName("GeneratedTransportMissing");
  first->setName(firstName);second->setName(secondName);
  context.scripts.owner->addObjectToCache(first);context.scripts.owner->addObjectToCache(second);
  auto* firstParam=newInstance(Parameter)(Parameter::UNIT);MemoryPoolObjectHolder firstLease(firstParam);firstParam->friend_setString(firstName);
  auto* secondParam=newInstance(Parameter)(Parameter::UNIT);MemoryPoolObjectHolder secondLease(secondParam);secondParam->friend_setString(secondName);
  auto* missingParam=newInstance(Parameter)(Parameter::UNIT);MemoryPoolObjectHolder missingLease(missingParam);missingParam->friend_setString(missingName);
  Conditions one;one.init();one.reset();Conditions two;two.init();
  require(!one.evaluateUnitHasEmptied(missingParam),"unknown source named unit does not acquire transport status");
  require(!one.evaluateUnitHasEmptied(firstParam),"first source sample returns false");
  auto* pool=TheMemoryPoolFactory->findMemoryPool("TransportStatus");require(pool && pool->getUsedBlockCount()==1,"first evaluator owns actual transport pool unit");
  const auto warmed=AllocationFault::live();
  AllocationFault::arm(0);
  require(!two.evaluateUnitHasEmptied(firstParam) && !one.evaluateUnitHasEmptied(secondParam) && !two.evaluateUnitHasEmptied(secondParam),"independent source owners sample both actor IDs");
  const auto sampleAttempts=AllocationFault::attempts();const auto sampleHit=AllocationFault::triggered();AllocationFault::disarm();
  require(!sampleHit && sampleAttempts==0 && pool->getUsedBlockCount()==4 && AllocationFault::live()==warmed,"warmed source transport cache exact standard-allocation terminal zero");
  AllocationFault::arm(0);one.reset();one.reset();
  const auto resetAttempts=AllocationFault::attempts();const auto resetHit=AllocationFault::triggered();AllocationFault::disarm();
  require(!resetHit && resetAttempts==0 && pool->getUsedBlockCount()==2,"repeated source reset retires only its own complete chain without allocation");
  require(!two.evaluateUnitHasEmptied(firstParam) && pool->getUsedBlockCount()==2,"other evaluator retains both source cache identities");
  // Exhaust the real typed backing. Growth, if configured, is rejected by
  // the standard fault; cache publication must remain empty for a retry.
  std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
  struct PoolLease{MemoryPool* pool;std::vector<void*>& held;~PoolLease(){for(auto* value:held)pool->freeBlock(value);}} poolLease{pool,held};
  while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated transport exhaustion"));
  const auto used=pool->getUsedBlockCount();const auto live=AllocationFault::live();bool rejected=false;
  AllocationFault::arm(0);
  try{one.evaluateUnitHasEmptied(firstParam);}catch(const std::bad_alloc&){rejected=true;}catch(ErrorCode code){rejected=code==ERROR_OUT_OF_MEMORY;}
  AllocationFault::disarm();require(rejected && pool->getUsedBlockCount()==used && AllocationFault::live()==live,"typed transport exhaustion leaves independent accepted chain and rejected owner unchanged");
  for(auto* value:held)pool->freeBlock(value);held.clear();
  require(!one.evaluateUnitHasEmptied(firstParam) && pool->getUsedBlockCount()==3,"corrected same-owner retry admits one transport unit");
  {Conditions nested;nested.init();require(!nested.evaluateUnitHasEmptied(firstParam) && pool->getUsedBlockCount()==4,"nested independent source owner acquires separate cache");}
  require(pool->getUsedBlockCount()==3,"actual source destructor retires only nested cache");
  one.reset();two.reset();require(pool->getUsedBlockCount()==0,"all source transport cache units retire");
  context.retire(second);context.retire(first);
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
  require(count==5,"complete caller plus admission/array/lookup/scheduler cold manifest");
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
struct Root:GameEngine {
  Root(){m_serviceOwners.create(TheSubsystemList,[]{return new SubsystemInterfaceList;});}
  template<class Base,class Concrete>void admit(Published<Base,Concrete>& value){
    Base* acquired=value.owner.get();
    const SubsystemPublication publication{&value.slot,value.previous,
      [](void* slot,void* prior)noexcept{*static_cast<Base**>(slot)=static_cast<Base*>(prior);},acquired};
    TheSubsystemList->adoptInitializedSubsystem(acquired,publication);
    value.owner.release();
  }
  void quiesce(){quiesceOwnedWorld();}
  GameLogic* createGameLogic()override{throw ERROR_BAD_ARG;}
  GameClient* createGameClient()override{throw ERROR_BAD_ARG;}
  ModuleFactory* createModuleFactory()override{throw ERROR_BAD_ARG;}
  ThingFactory* createThingFactory()override{throw ERROR_BAD_ARG;}
  FunctionLexicon* createFunctionLexicon()override{throw ERROR_BAD_ARG;}
  Radar* createRadar()override{throw ERROR_BAD_ARG;}
  WebBrowser* createWebBrowser()override{throw ERROR_BAD_ARG;}
  ParticleSystemManager* createParticleSystemManager()override{throw ERROR_BAD_ARG;}
  AudioManager* createAudioManager()override{throw ERROR_BAD_ARG;}
};
void rootRetirement(bool explicitQuiescence){
  const auto outside=AllocationFault::live();
  const auto outsideUnits=usage();
  {Context context(false);
    Published<AudioManager,AbsentAudioFixture> audio{TheAudio};audio.create();
    Published<MessageStream> messages{TheMessageStream};messages.create();
    Published<GameClient,NativeHeadlessGameClient> client{TheGameClient};
    // This fixture uses the explicit bare client, not an accepted whole-root
    // startup. It exercises actual root retirement of source actors/effects.
    client.owner=std::make_unique<NativeHeadlessGameClient>(NativeHeadlessGameClient::Composition::DrawablesOnly);
    TheGameClient=client.owner.get();
    { // The borrowed outer UI remains owned by its outer fixture. No actors
      // exist here yet; the source client initializes before this cohort's UI.
      struct BeforeUI{InGameUI* prior=TheInGameUI;BeforeUI(){TheInGameUI=nullptr;}~BeforeUI(){TheInGameUI=prior;}} beforeUI;
      client.owner->init();
    }
    context.ui.create();
    auto* logic=context.logic.owner.get();auto* sourceClient=client.owner.get();
    // Original init order deliberately registers players after teams. The
    // root must quiesce actors, then target teams before reverse retirement.
    {Root root;root.admit(client);root.admit(context.ui);root.admit(context.logic);
      root.admit(context.teams);root.admit(context.players);root.admit(context.radar);
      auto make=[&](const char* name){return logic->friend_createObject(context.templates.owner->findTemplate(name,FALSE),OBJECT_STATUS_MASK_NONE,nullptr);};
      // Latest actor is the list head: the carrier must be visited before its
      // still-live children, proving the stock recursive onDelete path.
      auto* first=make("GeneratedActor");auto* second=make("GeneratedActor");auto* carrier=make("GeneratedCarrier");
      auto* contain=dynamic_cast<OpenContain*>(carrier->getContain());require(contain,"actual stock recursive container");
      contain->addToContain(first);contain->addToContain(second);
      require(first->getContainedBy()==carrier && second->getContainedBy()==carrier,"source child admission");
      const auto carrierID=carrier->getID(),firstID=first->getID(),secondID=second->getID();
      auto* effect=sourceClient->friend_createDrawable(context.templates.owner->findTemplate("GeneratedActor",FALSE),DRAWABLE_STATUS_NONE);
      const auto effectID=effect->getID();
      // Prove acquired owner selection even when current slots are borrowed.
      TheGameLogic=context.logic.previous;TheTeamFactory=context.teams.previous;
      ThePlayerList=context.players.previous;TheGameClient=client.previous;
      if(explicitQuiescence){
        AllocationFault::arm(0);
        root.quiesce();const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
        require(!hit && attempts==0,"actual root quiescence zero-demand terminal");
        require(!logic->findObjectByID(carrierID) && !logic->findObjectByID(firstID) && !logic->findObjectByID(secondID) && !logic->getFirstObject() && !logic->updates(),"all actual recursive actors and scheduler units retire");
        require(!sourceClient->findDrawableByID(effectID) && !sourceClient->firstDrawable(),"client-only source effects retire before UI");
        root.quiesce();
      }
      // Destruction itself runs under the zero-demand fault. It is the source
      // GameEngine destructor, not Context's manually ordered cleanup.
      AllocationFault::arm(0);
    }
    const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(!hit && attempts==0,"actual root shutdown acquires no standard storage");
    require(TheGameLogic==context.logic.previous && TheTeamFactory==context.teams.previous && ThePlayerList==context.players.previous && TheInGameUI==context.ui.previous && TheRadar==context.radar.previous && TheGameClient==client.previous && !TheSubsystemList,"root restores complete borrowed publications after legitimate cleanup");
  }
  require(AllocationFault::live()==outside,"whole source retirement cohort releases standard ownership");
  require(usage()==outsideUnits && TheMemoryPoolFactory->findMemoryPool("OpenContain")->getUsedBlockCount()==0 && TheMemoryPoolFactory->findMemoryPool("Drawable")->getUsedBlockCount()==0,"whole root releases actor/container/drawable typed units");
}
void rootNegative(Context& context){
  auto* actor=context.make();const auto id=actor->getID();
  {Root root;const auto live=AllocationFault::live();const auto units=usage();
    rejects([&]{root.reset();});root.quiesce();
    require(TheGameLogic==context.logic.owner.get() && context.logic.owner->findObjectByID(id)==actor && !actor->isDestroyed() && usage()==units && AllocationFault::live()==live,"unowned root reset rejects and quiescence preserves borrowed actors/parents");}
  require(context.logic.owner->findObjectByID(id)==actor && TheInGameUI==context.ui.owner.get(),"empty root destruction does not retire borrowed world");context.retire(actor);
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"actor family");initMemoryManager();initialized=true;
    const std::array<std::size_t,8> sizes{sizeof(Object),sizeof(SightingInfo),sizeof(ObjectSMCHelper),sizeof(StatusDamageHelper),sizeof(SubdualDamageHelper),sizeof(ObjectDefectionHelper),sizeof(ExperienceTracker),sizeof(RadarObject)};
    for(unsigned n=0;n<pools.size();++n)TheMemoryPoolFactory->createMemoryPool(pools[n],sizes[n],n==1?128:64,0);
    TheMemoryPoolFactory->createMemoryPool("DeletionUpdate",sizeof(DeletionUpdate),512,0);
    TheMemoryPoolFactory->createMemoryPool("OpenContain",sizeof(OpenContain),8,0);
    TheMemoryPoolFactory->createMemoryPool("Drawable",sizeof(Drawable),8,0);
    TheMemoryPoolFactory->createMemoryPool("DestroyAdmissionProbe",sizeof(DestroyAdmissionProbe),8,0);
    {Context context;functional(context);
      {Context nested;functional(nested);} // Warm shared Squad arenas for two live cohorts.
      const std::string_view selected=argv[1];
      if(selected=="root-retirement" || selected=="root-quiescence")rootRetirement(selected=="root-quiescence");
      if(selected=="destroy-callbacks")destroyCallbacks(context);
      if(selected=="transport-cache")transportCacheOwners(context); // Warm the complete source cohort before baselines.
      const auto units=usage();const auto live=AllocationFault::live();
      for(unsigned repeat=0;repeat<3;++repeat){const std::string_view family=argv[1];
        if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else if(family=="pool-failures")poolFailures(context);else if(family=="parent-faults")parentFaults(context);else if(family=="id-growth")idGrowth();
        else if(family=="pool-updates-early")updatePoolFailures(context,0,90);else if(family=="pool-updates-middle")updatePoolFailures(context,90,180);else if(family=="pool-updates-late")updatePoolFailures(context,180,271);
        else if(family=="cold-faults")coldFaults(false);else if(family=="scheduler-faults")coldFaults(true);
        else if(family=="root-retirement")rootRetirement(false);else if(family=="root-quiescence")rootRetirement(true);
        else if(family=="destroy-callbacks")destroyCallbacks(context);else if(family=="root-negative")rootNegative(context);
        else if(family=="audio-constructor")audioConstructorFaults();else if(family=="transport-cache")transportCacheOwners(context);else require(false,"unknown actor family");
        require(usage()==units && AllocationFault::live()==live,"repeated actor retirement preserves complete accepted parents");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original actor parents and manager retire");
    std::cout<<"PASS actual registered actor ownership; full root/simulation pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL actor "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
