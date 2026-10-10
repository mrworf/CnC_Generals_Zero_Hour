// SPDX-License-Identifier: GPL-3.0-or-later
// Actual registered actor admission/retirement; not full startup/simulation.
#include "AllocationFault.h"
#include "AbsentAudioFixture.h"
#include "Common/Dict.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/ModuleFactory.h"
#include "Common/MessageStream.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/StateMachine.h"
#include "Common/RandomValue.h"
#include "Common/NativeOverrideAdmission.h"
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
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/TurretAI.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/AIStateMachine.h"
#include <array>
#include <cmath>
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
      out<<"Object GeneratedModelActor\n RadarPriority = UNIT\n Geometry = BOX\n GeometryMajorRadius = 5\n GeometryMinorRadius = 3\n GeometryHeight = 40\n"
           " Behavior = PhysicsBehavior GeneratedModelPhysics\n Mass = 10\n End\n"
           " Behavior = AIUpdateInterface GeneratedModelAI\n Turret\n TurretTurnRate = 45\n ControlledWeaponSlots = PRIMARY\n NaturalTurretAngle = 30\n NaturalTurretPitch = 10\n End\n"
           " AltTurret\n TurretTurnRate = 90\n ControlledWeaponSlots = SECONDARY\n NaturalTurretAngle = -45\n NaturalTurretPitch = 20\n End\n End\n End\n";
      out<<"Object GeneratedCarrier\n RadarPriority = UNIT\n Behavior = OpenContain GeneratedContainer\n ContainMax = 8\n End\n End\n";
      out<<"Object GeneratedDestroyCallbacks\n RadarPriority = UNIT\n Behavior = DestroyAdmissionProbe GeneratedDestroy\n End\n End\n";
      out<<"Object GeneratedManyUpdates\n RadarPriority = UNIT\n";
      for(unsigned n=0;n<270;++n)out<<" Behavior = DeletionUpdate GeneratedTag"<<n<<"\n MinLifetime = 10000\n MaxLifetime = 10000\n End\n";
      out<<"End\n";out.close();require(bool(out),"complete generated actual object definitions");
      {std::ofstream ai(root/"model-ai.ini");ai<<"ModelAI\n Turret\n ControlledWeaponSlots = PRIMARY\n End\n AltTurret\n ControlledWeaponSlots = SECONDARY\n End\n End\n";
        ai.close();require(bool(ai),"complete generated AI data");}
      {std::ofstream ai(root/"model-ai-bad.ini");ai<<"ModelAI\n Turret\n ControlledWeaponSlots = PRIMARY\n End\n AltTurret\n ControlledWeaponSlots = INVALID_GENERATED_SLOT\n End\n End\n";
        ai.close();require(bool(ai),"defined malformed second turret data");}
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
  Published<WeaponStore> weapons{TheWeaponStore};
  Published<AudioManager,AbsentAudioFixture> audio{TheAudio};
  Published<ThingFactory> templates{TheThingFactory};
  Published<PartitionManager> partition{ThePartitionManager};
  Published<PlayerList> players{ThePlayerList};
  Published<TeamFactory> teams{TheTeamFactory};
  Published<GameLogic,Logic> logic{TheGameLogic};
  explicit Context(bool initializeUI=true){try{names.create();names.owner->init();data.create();ranks.create();ranks.owner->init();
    scripts.create();ai.create();ai.owner->init();logic.create();logic.owner->defaults();audio.create();
    if(initializeUI)ui.create();radar.create();modules.create();modules.owner->init();modules.owner->registerProbe();weapons.create();templates.create();templates.owner->init();
    INI ini;ini.load("actors.ini",INI_LOAD_OVERWRITE,nullptr);
    partition.create();partition.owner->init();players.create();teams.create();teams.owner->init();
    Dict definition;teams.owner->initTeam("team","",TRUE,&definition);players.owner->getNeutralPlayer()->setDefaultTeam();
    }catch(...){retire();throw;}}
  void retire(){
    // Two phases: actor callbacks require all parents; radar cleanup itself
    // consults GameLogic even after its last radar node has retired.
    if(logic.owner)logic.owner->clear();
    ui.retire();radar.retire();partition.retire();teams.retire();players.retire();
    ai.retire();scripts.retire();templates.retire();modules.retire();weapons.retire();audio.retire();logic.retire();
    ranks.retire();data.retire();names.retire();
  }
  ~Context(){retire();}
  Object* make(bool many=false){return logic.owner->friend_createObject(templates.owner->findTemplate(many?"GeneratedManyUpdates":"GeneratedActor",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);}
  Object* model(){return logic.owner->friend_createObject(templates.owner->findTemplate("GeneratedModelActor",FALSE),OBJECT_STATUS_MASK_NONE,nullptr);}
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
template<class Action>void rejects(Action action);
MemoryPool* modelPoolCensus=nullptr;
using PoolCohort=std::vector<std::pair<MemoryPool*,Int>>;
PoolCohort modelUnits(){PoolCohort out;require(modelPoolCensus,"complete model parent pool census warmed");
  for(auto* pool=modelPoolCensus;pool;pool=pool->getNextPoolInList())out.emplace_back(pool,pool->getUsedBlockCount());return out;
}
void requireModelUnits(const PoolCohort& before){for(const auto& [pool,units]:before)
  require(pool->getUsedBlockCount()==units,"all warmed model parent/state pools retire their precise units");}
void verifyModelActor(Object& actor){auto* physics=actor.getPhysics();auto* ai=actor.getAIUpdateInterface();
  require(physics && ai && ai->isIdle() && actor.areModulesReady(),"actual parsed physics/AI and initialized full state machine");
  Real yaw=0,pitch=0;
  require(ai->getTurretRotAndPitch(TURRET_MAIN,&yaw,&pitch) && std::abs(yaw-3.14159265358979323846f/6)<.00001f && std::abs(pitch-3.14159265358979323846f/18)<.00001f,"actual main turret authored initial angle/pitch");
  require(ai->getTurretRotAndPitch(TURRET_ALT,&yaw,&pitch) && std::abs(yaw+3.14159265358979323846f/4)<.00001f && std::abs(pitch-3.14159265358979323846f/9)<.00001f,"actual alternate turret authored initial angle/pitch");
  const Coord3D velocity{3,4,0};physics->addVelocityTo(&velocity);
  // addVelocityTo alone does not invalidate the source magnitude cache.
  // The public scrub path establishes the real post-physics cached read.
  physics->scrubVelocity2D(5);
  require(physics->getVelocityMagnitude()==5 && actor.getGeometryInfo().getMaxHeightAbovePosition()==40,"real velocity cache and construction geometry, not fixture-returned values");
  actor.setConstructionPercent(25);require(actor.getConstructionPercent()==25,"actual source construction percentage");
}
void modelParents(Context& context){
  Published<MessageStream> messages{TheMessageStream};messages.create();
  Published<GameClient,NativeHeadlessGameClient> client{TheGameClient};
  client.owner=std::make_unique<NativeHeadlessGameClient>(NativeHeadlessGameClient::Composition::DrawablesOnly);
  TheGameClient=client.owner.get();
  {struct BeforeUI{InGameUI* prior=TheInGameUI;BeforeUI(){TheInGameUI=nullptr;}~BeforeUI(){TheInGameUI=prior;}} beforeUI;client.owner->init();}
  const auto before=GetGameLogicRandomSeedCRC();auto* first=context.model();auto* second=context.model();
  const auto acquired=GetGameLogicRandomSeedCRC();require(acquired!=before,"real AI default-state admission advances source logic RNG");
  verifyModelActor(*first);verifyModelActor(*second);
  require(GetGameLogicRandomSeedCRC()==acquired,"real physics/geometry/turret reads do not consume simulation RNG");
  auto* draw=client.owner->friend_createDrawable(first->getTemplate(),DRAWABLE_STATUS_NONE);
  first->friend_bindToDrawable(draw);draw->friend_bindToObject(first);
  require(first->getDrawable()==draw && draw->getObject()==first && draw->getShouldAnimate(TRUE),"real physics/AI actor and source Drawable paired publication");
  first->setScriptStatus(OBJECT_STATUS_SCRIPT_UNPOWERED);
  require(!draw->getShouldAnimate(TRUE) && draw->getShouldAnimate(FALSE),"actual power status reaches source animation gate");
  first->clearScriptStatus(OBJECT_STATUS_SCRIPT_UNPOWERED);
  const auto id=first->getID();client.owner->destroyDrawable(draw);
  require(!first->getDrawable() && context.logic.owner->findObjectByID(id)==first && first->getPhysics() && first->getAI(),"retiring source Drawable preserves actual actor/physics/AI identity");
  context.retire(second);context.retire(first);
  require(!context.logic.owner->getFirstObject() && !context.logic.owner->updates(),"whole real model-parent actor/state/scheduler retirement");
}
void modelParentFaults(Context& context){auto& logic=*context.logic.owner;
  const auto start=logic.getObjectIDCounter();std::size_t count=0;
  AllocationFault::arm(SIZE_MAX);Object* measured=nullptr;
  try{measured=context.model();count=AllocationFault::attempts();}catch(...){AllocationFault::disarm();throw;}
  AllocationFault::disarm();verifyModelActor(*measured);context.retire(measured);logic.setObjectIDCounter(start);
  require(count>50,"real two-turret and full AI state-map manifest, not geometry-only admission");
  const auto cohort=modelUnits();
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();Object* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);
    try{candidate=context.model();}catch(const std::bad_alloc&){failed=true;}catch(ErrorCode code){require(code==ERROR_OUT_OF_MEMORY,"actual model-parent allocation error");failed=true;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count) && attempts==(failed?ordinal+1:count),"every actual model-parent standard prefix and exact terminal");
    if(failed){require(!logic.getFirstObject() && !logic.updates() && logic.getObjectIDCounter()==start && AllocationFault::live()==live,"failed model-parent prefix withdraws complete actual actor/registrations/storage");requireModelUnits(cohort);}
    else{verifyModelActor(*candidate);require(candidate->getID()==ObjectID(start) && logic.getObjectIDCounter()==start+1,"actual model actor cursor publication before fixture restoration");context.retire(candidate);}
    candidate=context.model();verifyModelActor(*candidate);
    require(candidate->getID()==ObjectID(start+(failed?0:1)),"every corrected model-parent retry preserves source ID semantics");context.retire(candidate);
    require(!logic.getFirstObject() && !logic.updates() && AllocationFault::live()==live,"every corrected model-parent retry retires complete standard ownership");requireModelUnits(cohort);logic.setObjectIDCounter(start);
  }
  std::cout<<"actual physics/two-turret actor prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
void modelParentPools(Context& context){auto& logic=*context.logic.owner;
  for(const auto* name:{"PhysicsBehavior","AIUpdateInterface","TurretAI","TurretStateMachine","TurretAIIdleState","AIStateMachine","AIIdleState"}){
    auto* pool=TheMemoryPoolFactory->findMemoryPool(name);require(pool,"actual model-parent/state pool warmed");
    const unsigned acquisitions=std::string_view(name).starts_with("Turret")?2:1;
    for(unsigned available=0;available<acquisitions;++available){std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
      struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* slot:held)pool->freeBlock(slot);}} retire{pool,held};
      while(pool->getFreeBlockCount()>Int(available))held.push_back(pool->allocateBlock("generated model-parent exhaustion"));
      const auto start=logic.getObjectIDCounter();const auto cohort=modelUnits();const auto live=AllocationFault::live();bool failed=false;
      try{auto* unexpected=context.model();context.retire(unexpected);}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
      require(failed && !logic.getFirstObject() && !logic.updates() && logic.getObjectIDCounter()==start && AllocationFault::live()==live,"real model-parent pooled partial acquisition rejects and retires prefix");requireModelUnits(cohort);
      for(auto* slot:held)pool->freeBlock(slot);held.clear();auto* retry=context.model();verifyModelActor(*retry);context.retire(retry);
      require(pool->getUsedBlockCount()==0 && !logic.getFirstObject() && !logic.updates() && AllocationFault::live()==live,"every model-parent pool retry retires whole actor");logic.setObjectIDCounter(start);
    }
    std::cout<<"model parent "<<name<<" pooled prefixes [0,"<<acquisitions<<"); every retry\n";
  }
}
struct AdmissionMachine:StateMachine {
  using StateMachine::StateMachine;using StateMachine::defineState;using StateMachine::internalGetState;
};
Bool neverTransition(State*,void*){return FALSE;}
void modelStateAdmission(Context& context){auto* actor=context.make();const AsciiString name("GeneratedStateAdmission");
  const StateConditionInfo conditions[]={{neverTransition,1,nullptr},{nullptr,0,nullptr}};
  auto operation=[&](AdmissionMachine& machine){machine.defineState(1,newInstance(ContinueState)(&machine),1,1,conditions);};
  {AdmissionMachine warm(actor,name);operation(warm);} // Retire typed-pool discovery before the manifest.
  std::size_t count=0;
  {AdmissionMachine machine(actor,name);AllocationFault::arm(SIZE_MAX);
    try{operation(machine);count=AllocationFault::attempts();}catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();}
  auto* pool=TheMemoryPoolFactory->findMemoryPool("ContinueState");require(pool && count>=2,"state transition storage and map-node admission census");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();
    {AdmissionMachine machine(actor,name);bool failed=false;AllocationFault::arm(ordinal);
      try{operation(machine);}catch(const std::bad_alloc&){failed=true;}const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal<count) && attempts==(failed?ordinal+1:count),"state map/transition every prefix and exact terminal");
      if(failed){require(pool->getUsedBlockCount()==units && AllocationFault::live()==live && machine.getCurrentStateID()==INVALID_STATE_ID,"unpublished state/transition candidate retires on map or vector failure");operation(machine);}
      auto* accepted=machine.internalGetState(1);machine.initDefaultState();
      rejects([&]{machine.defineState(1,newInstance(ContinueState)(&machine),1,1);});
      rejects([&]{machine.defineState(2,accepted,1,1);});
      {AdmissionMachine foreign(actor,name);rejects([&]{foreign.defineState(2,accepted,2,2);});}
      require(machine.internalGetState(1)==accepted && machine.getCurrentStateID()==1 && pool->getUsedBlockCount()==units+1,"duplicate candidate and admitted-pointer rejection retain exact accepted state");
    }
    require(pool->getUsedBlockCount()==units && AllocationFault::live()==live,"every state admission/retry and whole machine retires exactly");
  }
  const auto cohort=modelPoolCensus?modelUnits():PoolCohort{};const auto live=AllocationFault::live();
  bool rejected=false;try{std::unique_ptr<TurretAI,NativePooledRetire> invalid(newInstance(TurretAI)(actor,nullptr,TURRET_MAIN));}catch(ErrorCode code){rejected=code==INI_INVALID_DATA;}
  require(rejected && AllocationFault::live()==live,"null turret data rejects before initializer dereference and retires pooled candidate");
  if(modelPoolCensus)requireModelUnits(cohort);
  context.retire(actor);std::cout<<"state admission prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
}
std::unique_ptr<AIUpdateModuleData>* modelAIDataDestination=nullptr;
void parseModelAI(INI* ini){auto candidate=std::make_unique<AIUpdateModuleData>();
  ini->initFromINIMultiProc(candidate.get(),AIUpdateModuleData::buildFieldParse);
  *modelAIDataDestination=std::move(candidate);
}
void modelDataAdmission(Context& context){
  auto parse=[&](const char* path){std::unique_ptr<AIUpdateModuleData> candidate;
    struct Destination{std::unique_ptr<AIUpdateModuleData>* prior=modelAIDataDestination;~Destination(){modelAIDataDestination=prior;}} destination;
    modelAIDataDestination=&candidate;const INIBlockDefinition blocks[]={{"ModelAI",parseModelAI}};INI ini;ini.loadBlocks(path,INI_LOAD_OVERWRITE,blocks);
    require(candidate && candidate->m_turretData[0] && candidate->m_turretData[1],"complete actual two-turret parsed data");return candidate;
  };
  {auto warm=parse("model-ai.ini");}
  const auto cohort=modelUnits();const auto outside=AllocationFault::live();bool bad=false;
  try{auto rejected=parse("model-ai-bad.ini");}catch(const INIException&){bad=true;}catch(ErrorCode code){bad=code==INI_INVALID_DATA;}
  require(bad && AllocationFault::live()==outside,"malformed second turret retires preceding and unpublished parsed pooled data");requireModelUnits(cohort);
  std::size_t count=0;
  {AllocationFault::arm(SIZE_MAX);std::unique_ptr<AIUpdateModuleData> measured;
    try{measured=parse("model-ai.ini");count=AllocationFault::attempts();}catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();}
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();bool failed=false;std::unique_ptr<AIUpdateModuleData> candidate;
    AllocationFault::arm(ordinal);try{candidate=parse("model-ai.ini");}catch(const std::bad_alloc&){failed=true;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count) && attempts==(failed?ordinal+1:count),"actual nested turret data parse prefixes and exact terminal");candidate.reset();
    require(AllocationFault::live()==live,"failed or accepted whole nested parser retires standard storage");requireModelUnits(cohort);
    {auto retry=parse("model-ai.ini");}require(AllocationFault::live()==live,"every corrected nested turret parser retry retires");requireModelUnits(cohort);
  }
  std::cout<<"actual nested turret data prefixes [0,"<<count<<"); terminal "<<count<<"; every retry\n";
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
    const bool modelFamily=std::string_view(argv[1]).starts_with("model-");
    if(modelFamily){
      const std::array<const char*,7> names{"PhysicsBehavior","AIUpdateInterface","TurretAI","TurretStateMachine","TurretAIIdleState","AIStateMachine","AIIdleState"};
      const std::array<std::size_t,7> modelSizes{sizeof(PhysicsBehavior),sizeof(AIUpdateInterface),sizeof(TurretAI),sizeof(TurretStateMachine),sizeof(TurretAIIdleState),sizeof(AIStateMachine),sizeof(AIIdleState)};
      for(unsigned n=0;n<names.size();++n)TheMemoryPoolFactory->createMemoryPool(names[n],modelSizes[n],8,0);
    }
    {Context context;functional(context);
      {Context nested;functional(nested);} // Warm shared Squad arenas for two live cohorts.
      const std::string_view selected=argv[1];
      if(selected=="root-retirement" || selected=="root-quiescence")rootRetirement(selected=="root-quiescence");
      if(selected=="destroy-callbacks")destroyCallbacks(context);
      if(selected=="transport-cache")transportCacheOwners(context); // Warm the complete source cohort before baselines.
      if(modelFamily){modelParents(context);modelStateAdmission(context);
        modelPoolCensus=TheMemoryPoolFactory->createMemoryPool("GeneratedModelPoolCensus",16,1,0);modelDataAdmission(context);}
      const auto units=usage();const auto live=AllocationFault::live();
      for(unsigned repeat=0;repeat<3;++repeat){const std::string_view family=argv[1];
        if(family=="functional")functional(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else if(family=="pool-failures")poolFailures(context);else if(family=="parent-faults")parentFaults(context);else if(family=="id-growth")idGrowth();
        else if(family=="pool-updates-early")updatePoolFailures(context,0,90);else if(family=="pool-updates-middle")updatePoolFailures(context,90,180);else if(family=="pool-updates-late")updatePoolFailures(context,180,271);
        else if(family=="cold-faults")coldFaults(false);else if(family=="scheduler-faults")coldFaults(true);
        else if(family=="root-retirement")rootRetirement(false);else if(family=="root-quiescence")rootRetirement(true);
        else if(family=="destroy-callbacks")destroyCallbacks(context);else if(family=="root-negative")rootNegative(context);
        else if(family=="audio-constructor")audioConstructorFaults();else if(family=="transport-cache")transportCacheOwners(context);
        else if(family=="model-functional")modelParents(context);else if(family=="model-faults")modelParentFaults(context);
        else if(family=="model-pools")modelParentPools(context);else if(family=="model-state-admission")modelStateAdmission(context);
        else if(family=="model-data-admission")modelDataAdmission(context);else require(false,"unknown actor family");
        require(usage()==units && AllocationFault::live()==live,"repeated actor retirement preserves complete accepted parents");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original actor parents and manager retire");
    std::cout<<"PASS actual registered actor ownership; full root/simulation pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL actor "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
