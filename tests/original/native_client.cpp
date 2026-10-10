// SPDX-License-Identifier: GPL-3.0-or-later
// Actual client/Drawable owners, not a renderer or full startup acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "NativeCameraViewFixture.h"
#include "DisplayOwnershipFixture.h"
#include "NativeFrameCameraViewFixture.h"
#include "GameClient/InGameUI.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/MessageStream.h"
#include "Common/ThingTemplate.h"
#include "Common/ModuleFactory.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/Module/AnimatedParticleSysBoneClientUpdate.h"
#include "GameClient/Module/BeaconClientUpdate.h"
#include "GameClient/Module/SwayClientUpdate.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "GameClient/RayEffect.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/GhostObject.h"
#include <memory>
#include <iostream>
#include <limits>
#include <array>
namespace {
// Exposes destruction of the actual source template without replacing data,
// parsing, modules or gameplay behavior.
struct Definition:ThingTemplate {~Definition() override=default;};
struct InspectableClient:NativeHeadlessGameClient {using GameClient::updateDrawableState;};
unsigned startupFault=0,startupCreations=0,startupRetirements=0;
bool startupInvalidPublication=false;
struct StartupView:NativeFrameCameraViewFixture {
  using NativeFrameCameraViewFixture::NativeFrameCameraViewFixture;
  ~StartupView()override{++startupRetirements;if(TheTacticalView!=this)startupInvalidPublication=true;}
  void init()override{NativeFrameCameraViewFixture::init();if(startupFault==1)throw ERROR_BAD_ARG;}
  void setWidth(Int width)override{NativeFrameCameraViewFixture::setWidth(width);if(startupFault==2)throw ERROR_BAD_ARG;}
  void setHeight(Int height)override{NativeFrameCameraViewFixture::setHeight(height);if(startupFault==3)throw ERROR_BAD_ARG;}
  void setDefaultView(Real pitch,Real angle,Real height)override{
    ScriptedView::setDefaultView(pitch,angle,height);if(startupFault==4)throw ERROR_BAD_ARG;}
};
struct StartupUI:InGameUI {
  NativeCameraShaker& effects;
  explicit StartupUI(NativeCameraShaker& value):effects(value){}
  using InGameUI::initializeTacticalView;
  void draw()override{throw ERROR_BAD_ARG;}
  View* createView()override{++startupCreations;return startupFault==5?nullptr:new StartupView(effects);}
};
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode){rejected=true;}require(rejected,"invalid native client admission rejects");}
struct Context {
  FileSystem files;MessageStream messages;
  std::unique_ptr<GlobalData> data;
  FileSystem* previousFiles=TheFileSystem;GlobalData* previousData=TheWritableGlobalData;
  MessageStream* previousMessages=TheMessageStream;
  Context(){TheFileSystem=&files;TheMessageStream=&messages;
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();}
    catch(...){TheFileSystem=previousFiles;TheMessageStream=previousMessages;throw;}}
  ~Context(){TheWritableGlobalData=data.get();data.reset();TheWritableGlobalData=previousData;TheMessageStream=previousMessages;TheFileSystem=previousFiles;}
};
struct Client {
  GameClient* previous=TheGameClient;
  std::unique_ptr<InspectableClient> owner;
  Client(){owner=std::make_unique<InspectableClient>();TheGameClient=owner.get();}
  void retire(){TheGameClient=owner.get();owner.reset();TheGameClient=previous;}
  ~Client(){retire();}
  void init(){owner->init();}
};
void tacticalStartup(){
  Context context;context.data->m_cameraHeight=300;context.data->m_cameraPitch=45;
  context.data->m_cameraYaw=0;context.data->m_maxCameraHeight=600;
  NativeCameraShaker effects;NativeCameraViewFixture borrowed;
  auto* previousTerrain=TheTerrainLogic;auto* previousDisplay=TheDisplay;
  auto* previousUI=TheInGameUI;auto* previousView=TheTacticalView;
  struct Restore {TerrainLogic* terrain;Display* display;InGameUI* ui;View* view;
    ~Restore(){startupFault=0;TheTerrainLogic=terrain;TheDisplay=display;TheInGameUI=ui;TheTacticalView=view;}
  } restore{previousTerrain,previousDisplay,previousUI,previousView};
  TheTerrainLogic=nullptr;TheTacticalView=&borrowed;startupInvalidPublication=false;
  auto operation=[&]{DisplayOwnershipFixture display;display.setWidth(800);display.setHeight(600);TheDisplay=&display;
    StartupUI ui(effects);TheInGameUI=&ui;
    ui.initializeTacticalView();auto* view=dynamic_cast<StartupView*>(TheTacticalView);
    require(view && display.getFirstView()==view && view->getWidth()==800 && view->getHeight()==462 &&
      view->transformBuildCount()==2 && !view->constraintsValid(),"actual UI admits native tactical camera before source terrain startup");
    const auto created=startupCreations;rejects([&]{ui.initializeTacticalView();});
    require(startupCreations==created && display.getFirstView()==view,"duplicate tactical initialization preserves admitted View");
    const auto builds=view->transformBuildCount();view->forceCameraConstraintRecalc();view->setZoom(1);
    require(view->transformBuildCount()==builds+2 && !view->constraintsValid(),"pre-terrain invalid constraints retain explicit two-build cadence");
    rejects([&]{view->updateView();});rejects([&]{view->initHeightForMap();});
    require(view->transformBuildCount()==builds+2,"missing frame/map parents do not advance accepted camera effects");
    display.deleteViews();require(!TheTacticalView && !display.getFirstView(),"actual Display cleanup withdraws exact tactical publication");
    TheTacticalView=&borrowed;TheInGameUI=previousUI;TheDisplay=previousDisplay;
  };
  operation();operation();
  for(unsigned phase=1;phase<=5;++phase){DisplayOwnershipFixture display;display.setWidth(800);display.setHeight(600);TheDisplay=&display;
    StartupUI ui(effects);TheInGameUI=&ui;const auto live=AllocationFault::live();const auto views=View::liveViewCount();
    startupFault=phase;rejects([&]{ui.initializeTacticalView();});startupFault=0;
    require(AllocationFault::live()==live && View::liveViewCount()==views && TheTacticalView==&borrowed && !display.getFirstView(),
      "every tactical factory/init/viewport/default rejection retires candidate and restores borrowed publication");
    ui.initializeTacticalView();display.deleteViews();TheTacticalView=&borrowed;
    require(AllocationFault::live()==live && View::liveViewCount()==views,"each source setup rejection permits exact same-owner retry");
    TheInGameUI=previousUI;TheDisplay=previousDisplay;
  }
  AllocationFault::arm(SIZE_MAX);operation();const auto terminal=AllocationFault::attempts();AllocationFault::disarm();
  require(terminal>0 && terminal<64,"complete bounded tactical cohort allocation manifest");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();const auto views=View::liveViewCount();
    AllocationFault::arm(ordinal);bool failed=false;try{operation();}catch(const std::bad_alloc&){failed=true;}
    const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    // Restore test-owned outer publications after complete local owner cleanup.
    TheInGameUI=previousUI;TheDisplay=previousDisplay;TheTacticalView=&borrowed;
    require(failed==(ordinal<terminal) && hit==failed && (failed || attempts==terminal),"every tactical acquisition prefix and exact terminal");
    require(AllocationFault::live()==live && View::liveViewCount()==views,"failed/terminal whole tactical cohort retires exactly");
    if(failed){operation();require(AllocationFault::live()==live && View::liveViewCount()==views,"every tactical allocation failure has complete corrected retry");}
  }
  require(!startupInvalidPublication,"legitimate tactical publication remains valid through all derived candidate/owner cleanup");
}
// Inspection exposes source state only, never replaces registration or creators.
struct Factory:ModuleFactory {std::size_t count() const{return m_moduleTemplateMap.size();}};
constexpr std::size_t registeredCount=198
#ifdef ALLOW_SURRENDER
  +5
#endif
#ifdef ALLOW_DEMORALIZE
  +1
#endif
;
struct Registry {
  NameKeyGenerator names;
  NameKeyGenerator* previousNames=TheNameKeyGenerator;
  ModuleFactory* previousFactory=TheModuleFactory;
  std::unique_ptr<Factory> factory;
  Registry(){names.init();TheNameKeyGenerator=&names;
    try{factory=std::make_unique<Factory>();factory->init();TheModuleFactory=factory.get();}
    catch(...){TheNameKeyGenerator=previousNames;throw;}}
  ~Registry(){TheModuleFactory=factory.get();factory.reset();TheModuleFactory=previousFactory;TheNameKeyGenerator=previousNames;}
};
constexpr std::array<const char*,3> moduleNames{"AnimatedParticleSysBoneClientUpdate","BeaconClientUpdate","SwayClientUpdate"};
void moduleDefinition(Registry& registry,Definition& definition){
  auto& info=const_cast<ModuleInfo&>(definition.getClientUpdateModuleInfo());
  for(std::size_t index=0;index<moduleNames.size();++index){
    const AsciiString name(moduleNames[index]);AsciiString tag;tag.format("GeneratedClientTag%u",unsigned(index));
    const auto* data=registry.factory->newModuleDataFromINI(nullptr,name,MODULETYPE_CLIENT_UPDATE,tag);
    require(data,"actual registered creator data");
    info.addModuleInfo(&definition,name,tag,data,registry.factory->findModuleInterfaceMask(name,MODULETYPE_CLIENT_UPDATE),FALSE);
  }
}
struct LogicPublication {
  GameLogic logic;GameLogic* previous=TheGameLogic;
  LogicPublication(){TheGameLogic=&logic;}
  ~LogicPublication(){TheGameLogic=previous;}
};
void checkModules(Drawable* draw){
  auto** modules=draw->getClientUpdateModules();
  require(modules && dynamic_cast<AnimatedParticleSysBoneClientUpdate*>(modules[0]) &&
    dynamic_cast<BeaconClientUpdate*>(modules[1]) && dynamic_cast<SwayClientUpdate*>(modules[2]) && !modules[3],
    "actual registered nonempty module ordering and terminated complete backing");
  // Stop real sway explicitly: breeze/camera/world integration is not established
  // by a bare GameLogic constructor. The other two source callbacks are executed.
  static_cast<SwayClientUpdate*>(modules[2])->stopSway();
  draw->friend_setSelected();require(draw->isSelected(),"actual source selected state");
  static_cast<BeaconClientUpdate*>(modules[1])->hideBeacon();
  draw->updateDrawable();
  require(draw->isDrawableEffectivelyHidden() && !draw->isSelected(),"actual beacon hides/deselects logical drawable without an invented UI");
  draw->friend_setSelected();draw->setSelectable(FALSE);require(!draw->isSelected(),"parallel selectable transition withdraws actual source selection");
}
void modulesFunctional(){Context context;Registry registry;LogicPublication logic;Definition definition,bad;Client client;client.init();
  require(registry.factory->count()==registeredCount,"complete original conditional registry, including duplicate-name replacement");
  moduleDefinition(registry,definition);moduleDefinition(registry,bad);
  auto* accepted=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);checkModules(accepted);
  const auto acceptedID=accepted->getID();
  const auto* data=definition.getClientUpdateModuleInfo().getNthData(0);
  auto& malformed=const_cast<ModuleInfo&>(bad.getClientUpdateModuleInfo());
  malformed.addModuleInfo(&bad,"MissingGeneratedProvider","GeneratedMissingTag",data,MODULEINTERFACE_CLIENT_UPDATE,FALSE);
  // Append another real provider after the absent one: old code would create it
  // behind an interior null sentinel, hiding that ownership from all walks.
  malformed.addModuleInfo(&bad,moduleNames[0],"GeneratedTrailingTag",data,MODULEINTERFACE_CLIENT_UPDATE,FALSE);
  registry.factory->findModuleInterfaceMask("MissingGeneratedProvider",MODULETYPE_CLIENT_UPDATE); // legitimate warmed namespace
  const auto live=AllocationFault::live();const auto next=client.owner->getDrawableIDCounter();
  rejects([&]{client.owner->friend_createDrawable(&bad,DRAWABLE_STATUS_NONE);});
  require(AllocationFault::live()==live && client.owner->getDrawableIDCounter()==next &&
    client.owner->firstDrawable()==accepted && client.owner->findDrawableByID(acceptedID)==accepted,
    "late absent provider rejects without hidden module suffix or partial registry");
  auto* corrected=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);checkModules(corrected);
  client.owner->destroyDrawable(corrected);
  // Both source module lists must reject data absence before dereference.
  for(bool draw:{false,true}){Definition invalid;
    auto& info=const_cast<ModuleInfo&>(draw?invalid.getDrawModuleInfo():invalid.getClientUpdateModuleInfo());
    info.addModuleInfo(&invalid,moduleNames[0],"GeneratedNullData",nullptr,MODULEINTERFACE_CLIENT_UPDATE,FALSE);
    rejects([&]{client.owner->friend_createDrawable(&invalid,DRAWABLE_STATUS_NONE);});}
  require(AllocationFault::live()==live && client.owner->firstDrawable()==accepted,"both malformed module paths preserve actual accepted owner");
}
void modulesFaults(){Context context;Registry registry;LogicPublication logic;Definition definition;Client client;client.init();
  std::array<MemoryPool*,3> pools{
    TheMemoryPoolFactory->createMemoryPool(moduleNames[0],sizeof(AnimatedParticleSysBoneClientUpdate),4,0),
    TheMemoryPoolFactory->createMemoryPool(moduleNames[1],sizeof(BeaconClientUpdate),4,0),
    TheMemoryPoolFactory->createMemoryPool(moduleNames[2],sizeof(SwayClientUpdate),4,0)};
  moduleDefinition(registry,definition);
  auto* accepted=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);checkModules(accepted);
  auto operation=[&]{return client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);};
  AllocationFault::arm(SIZE_MAX);auto* discovery=operation();const auto census=AllocationFault::attempts();AllocationFault::disarm();
  client.owner->destroyDrawable(discovery);require(census>0 && census<128,"complete bounded nonempty drawable allocation manifest");
  auto intact=[&]{for(auto* pool:pools)require(pool->getUsedBlockCount()==1,"all accepted module pool units retained, candidate prefix retired");
    require(client.owner->firstDrawable()==accepted && client.owner->findDrawableByID(accepted->getID())==accepted,"actual accepted module graph remains published");};
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto next=client.owner->getDrawableIDCounter();
    AllocationFault::arm(ordinal);Drawable* candidate=nullptr;bool failed=false;
    try{candidate=operation();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"nonempty module array allocation prefixes and exact terminal");
    if(failed){intact();require(AllocationFault::live()==live && client.owner->getDrawableIDCounter()==next,"nonempty failed registration exact rollback");candidate=operation();}
    checkModules(candidate);client.owner->destroyDrawable(candidate);intact();require(AllocationFault::live()==live,"nonempty array corrected retry retires exactly");}
  for(std::size_t ordinal=0;ordinal<=pools.size();++ordinal){std::array<void*,3> held{};
    if(ordinal<pools.size())for(auto& unit:held)unit=pools[ordinal]->allocateBlock("generated fixed capacity owner");
    const auto live=AllocationFault::live();const auto next=client.owner->getDrawableIDCounter();Drawable* candidate=nullptr;bool failed=false;
    try{candidate=operation();}catch(ErrorCode error){if(error!=ERROR_OUT_OF_MEMORY)throw;failed=true;}
    require(failed==(ordinal<pools.size()),"each real module pool acquisition prefix and exact success terminal");
    for(auto* unit:held)if(unit)pools[ordinal]->freeBlock(unit);
    if(failed){intact();require(AllocationFault::live()==live && client.owner->getDrawableIDCounter()==next,"failed actual module constructor retires earlier module prefix");candidate=operation();}
    checkModules(candidate);client.owner->destroyDrawable(candidate);intact();require(AllocationFault::live()==live,"actual module pool corrected retry retires exactly");}
  std::cout<<"nonempty drawable allocation prefixes [0,"<<census<<"); terminal "<<census<<"; module pool prefixes [0,3); terminal 3\n";
}
void registryFaults(unsigned group){Context context;
  auto operation=[]{Registry registry;require(registry.factory->count()==registeredCount,"complete original module registration");};
  AllocationFault::arm(SIZE_MAX);operation();const auto census=AllocationFault::attempts();AllocationFault::disarm();
  require(census>0 && census<4096,"complete bounded original module registry acquisition manifest");
  auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");require(pool,"actual registry name pool");
  for(std::size_t ordinal=group;ordinal<census;ordinal+=4){const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();
    AllocationFault::arm(ordinal);bool failed=false;try{operation();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed && AllocationFault::triggered() && AllocationFault::live()==live && pool->getUsedBlockCount()==units &&
      !TheModuleFactory && !TheNameKeyGenerator,"each complete registry acquisition failure restores owners and namespace");
    operation();require(AllocationFault::live()==live && pool->getUsedBlockCount()==units,"complete registry corrected retry exact retirement");}
  AllocationFault::arm(census);operation();AllocationFault::disarm();require(!AllocationFault::triggered() && AllocationFault::attempts()==census,"each shard proves independently discovered exact registry terminal");
  std::cout<<"registry group "<<group<<" of 4, prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
void cameraLifetime(){Context context;Definition definition;Client client;client.init();
  context.data->m_cameraHeight=300;context.data->m_cameraPitch=45;context.data->m_cameraYaw=0;
  context.data->m_disableCameraMovement=FALSE;
  NativeCameraViewFixture first,second,unrelated;first.init();second.init();unrelated.init();
  auto* prior=TheTacticalView;TheTacticalView=&first;
  struct Restore{View* prior;~Restore(){TheTacticalView=prior;}}restore{prior};
  first.setCameraLock(static_cast<ObjectID>(77));second.setCameraLock(static_cast<ObjectID>(78));
  auto* a=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  auto* b=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  first.setCameraLockDrawable(a);second.setCameraLockDrawable(a);unrelated.setCameraLockDrawable(b);
  View::withdrawDrawableBorrows(nullptr);require(first.getCameraLockDrawable()==a,"null withdrawal does not clear legitimate borrow");
  struct FailedView:NativeCameraViewFixture {explicit FailedView(Drawable* draw){setCameraLockDrawable(draw);throw ERROR_BAD_ARG;}};
  const auto viewCount=View::liveViewCount();
  rejects([&]{FailedView candidate(a);}); // Base destruction must retire its live-View link.
  require(View::liveViewCount()==viewCount,"failed derived construction retires its exact live-View registration");
  AllocationFault::arm(0);try{client.owner->destroyDrawable(a);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(!hit && attempts==0 && !first.getCameraLockDrawable() && !second.getCameraLockDrawable() &&
    unrelated.getCameraLockDrawable()==b && first.getCameraLock()==static_cast<ObjectID>(77) && second.getCameraLock()==static_cast<ObjectID>(78),
    "actual destruction withdraws all matching native Views without allocation, dereference or Object-lock loss");
  const auto changed=static_cast<DrawableID>(static_cast<UnsignedInt>(b->getID())+42);
  b->setID(changed);require(unrelated.getCameraLockDrawable()==b && client.owner->findDrawableByID(changed)==b,
    "ordinary lookup rekey preserves a live Drawable camera borrow");
  client.owner->cancelDrawableConstruction(b);require(!unrelated.getCameraLockDrawable() && !client.owner->findDrawableByID(changed),
    "candidate cancellation withdraws camera borrow and lookup together");b->deleteInstance();
  auto* c=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);first.setCameraLockDrawable(c);
  AllocationFault::arm(0);bool failed=false;try{client.owner->reset();}catch(const std::bad_alloc&){failed=true;}
  catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
  require(failed && first.getCameraLockDrawable()==c && client.owner->findDrawableByID(c->getID())==c,
    "failed reset preparation preserves accepted native camera borrow and child");
  client.owner->reset();require(!first.getCameraLockDrawable(),"corrected actual reset withdraws camera borrow");
  auto* direct=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);second.setCameraLockDrawable(direct);
  // Retire registry/master-list admission separately, then exercise the actual
  // Drawable owner path that also protects constructor rollback/direct cleanup.
  client.owner->cancelDrawableConstruction(direct);second.setCameraLockDrawable(direct);direct->deleteInstance();
  require(!second.getCameraLockDrawable(),"direct Drawable owner retirement independently withdraws borrow");
  auto* last=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  first.setCameraLockDrawable(last);second.setCameraLockDrawable(last);client.retire();
  require(!first.getCameraLockDrawable() && !second.getCameraLockDrawable() && TheTacticalView==&first,
    "whole client retirement clears child borrows while preserving legitimate borrowed View publication");
}
void viewOwnership(){Context context;DisplayOwnershipFixture first,second;
  auto* prior=TheTacticalView;struct Restore{View* prior;~Restore(){TheTacticalView=prior;}}restore{prior};
  unsigned destroyed=0;bool identity=false;
  struct Observed:NativeCameraViewFixture {
    unsigned& destroyed;bool& identity;bool expected;
    Observed(unsigned& count,bool& state,bool publication=false):destroyed(count),identity(state),expected(publication){}
    ~Observed(){++destroyed;if(expected)identity=TheTacticalView==this;}
  };
  auto a=std::make_unique<Observed>(destroyed,identity,true),b=std::make_unique<Observed>(destroyed,identity);
  auto* tactical=a.get();first.attachView(a.get());a.release();first.attachView(b.get());auto* other=b.release();TheTacticalView=tactical;
  rejects([&]{first.attachView(nullptr);});rejects([&]{first.attachView(tactical);});rejects([&]{second.attachView(tactical);});
  require(first.getFirstView()==other && first.getNextView(other)==tactical && !first.getNextView(tactical) && !second.getFirstView(),
    "null/duplicate/cross-owner rejection preserves complete acyclic ownership");
  delete other;require(destroyed==1 && first.getFirstView()==tactical && !first.getNextView(tactical),"direct View retirement withdraws exact owned-list link");
  AllocationFault::arm(0);try{first.deleteViews();}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(attempts==0 && !hit && destroyed==2 && identity && !first.getFirstView() && !TheTacticalView,
    "whole owned View retirement has zero allocation and preserves tactical identity through derived cleanup, then withdraws it");
  NativeCameraViewFixture borrowed;TheTacticalView=&borrowed;
  {DisplayOwnershipFixture owner;auto candidate=std::make_unique<Observed>(destroyed,identity);owner.attachView(candidate.get());candidate.release();}
  require(destroyed==3 && TheTacticalView==&borrowed,"Display teardown preserves unrelated borrowed tactical View");
  auto retry=std::make_unique<Observed>(destroyed,identity);second.attachView(retry.get());retry.release();second.deleteViews();
  require(destroyed==4 && !second.getFirstView(),"same-owner corrected attachment and whole retirement retry");
}
void functional(){Context context;Definition definition;Client client;client.init();
  require(client.owner->isHeadless() && TheRayEffects,"explicit absent output retains actual ray-effect owner");
  Drawable* first=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  Drawable* second=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
  const auto firstID=first->getID(),secondID=second->getID();
  require(firstID!=INVALID_DRAWABLE_ID && secondID!=firstID && client.owner->findDrawableByID(firstID)==first &&
    client.owner->findDrawableByID(secondID)==second && client.owner->firstDrawable()==second,"actual source registration/list/lookup distinct identity");
  rejects([&]{first->setID(secondID);});
  rejects([&]{client.owner->registerDrawable(first);});
  require(first->getID()==firstID && client.owner->findDrawableByID(firstID)==first && client.owner->findDrawableByID(secondID)==second,
    "duplicate ID admission retains both actual owners");
  require(!client.owner->findDrawableByID(static_cast<DrawableID>(std::numeric_limits<UnsignedInt>::max())),"raw UInt32 lookup never narrows to a negative index");
  rejects([&]{client.owner->init();});rejects([&]{client.owner->update();});
  require(client.owner->firstDrawable()==second,"incomplete simulation/update rejection never discards logical state");
  client.owner->destroyDrawable(first);require(!client.owner->findDrawableByID(firstID) && client.owner->findDrawableByID(secondID)==second,"exact source registry withdrawal");
  AllocationFault::arm(0);bool failed=false;try{client.owner->reset();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
  require(failed && client.owner->findDrawableByID(secondID)==second && client.owner->firstDrawable()==second,"failed reset preparation preserves accepted drawable and lookup");
  client.owner->reset();require(!client.owner->firstDrawable() && !client.owner->findDrawableByID(secondID),"corrected reset retires original drawable graph");
  const Coord3D start{0,2,4},end{4,6,8};client.owner->createRayEffectByTemplate(&start,&end,&definition);
  Drawable* ray=client.owner->firstDrawable();RayEffectData captured{};client.owner->getRayEffectData(ray,&captured);
  require(ray && captured.draw==ray && captured.startLoc.x==0 && captured.endLoc.z==8 && ray->getPosition()->x==2,
    "actual source ray endpoints and midpoint retained without a synthetic renderer");
  require(!client.owner->friend_createDrawable(nullptr,DRAWABLE_STATUS_NONE),"original null-template factory contract");
}
void frames(){Context context;Definition definition;GameLogic logic;GhostObjectManager ghosts;
  const auto previousLogic=TheGameLogic;const auto previousGhosts=TheGhostObjectManager;
  struct Restore {GameLogic* logic;GhostObjectManager* ghosts;~Restore(){TheGameLogic=logic;TheGhostObjectManager=ghosts;}} restore{previousLogic,previousGhosts};
  TheGameLogic=&logic;TheGhostObjectManager=&ghosts;
  {Client client;client.init();auto* draw=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);draw->fadeOut(2);
    client.owner->setFrame(0);client.owner->updateDrawableState(FALSE,0);client.owner->updateDrawableState(FALSE,0);
    require(draw->getEffectiveOpacity()==1.0f,"source repeated frame does not advance drawable fade twice");
    client.owner->setFrame(1);client.owner->updateDrawableState(FALSE,0);require(draw->getEffectiveOpacity()==0.5f,"original drawable fade transition");
    client.owner->setFrame(2);client.owner->updateDrawableState(TRUE,0);require(draw->getEffectiveOpacity()==0.5f,"source frozen frame retains drawable state");
    client.owner->setFrame(3);client.owner->updateDrawableState(FALSE,0);require(draw->getEffectiveOpacity()==0.0f,"source unfreezing resumes actual drawable update");
    client.owner->setFrame(0);client.owner->updateDrawableState(FALSE,0);}
  {Client replacement;replacement.init();auto* draw=replacement.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);draw->fadeOut(1);
    replacement.owner->setFrame(0);replacement.owner->updateDrawableState(FALSE,0);
    replacement.owner->setFrame(1);replacement.owner->updateDrawableState(FALSE,0);
    require(draw->getEffectiveOpacity()==0.0f,"replaced source owner does not inherit previous owner's last-frame suppression");}
}
void drawFaults(){Context context;Definition definition;Client client;client.init();
  Drawable* accepted=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);const auto acceptedID=accepted->getID();
  std::size_t census;
  {AllocationFault::arm(SIZE_MAX);Drawable* discovery=nullptr;try{discovery=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();client.owner->destroyDrawable(discovery);}
  auto* pool=TheMemoryPoolFactory->findMemoryPool("Drawable");require(pool && census>0 && census<128,"complete bounded source drawable acquisition manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();
    const auto next=client.owner->getDrawableIDCounter();Drawable* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"each actual drawable allocation prefix and exact terminal");
    if(failed){require(client.owner->getDrawableIDCounter()==next && client.owner->firstDrawable()==accepted &&
        pool->getUsedBlockCount()==units && AllocationFault::live()==live,"failed source constructor withdraws identity, arrays and pool unit exactly");
      candidate=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);}
    require(client.owner->findDrawableByID(acceptedID)==accepted && client.owner->findDrawableByID(candidate->getID())==candidate,"accepted source drawable survives candidate/retry");
    client.owner->destroyDrawable(candidate);require(pool->getUsedBlockCount()==units && AllocationFault::live()==live,"complete corrected constructor owner retirement");
  }std::cout<<"drawable prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
void clientFaults(){Context context;
  auto operation=[] {Client client;client.init();};std::size_t census;
  {AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<128,"complete bounded actual client acquisition manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() && (failed || AllocationFault::attempts()==census),"every actual client acquisition failure and exact terminal");
    require(AllocationFault::live()==live && !TheGameClient && !TheRayEffects && !TheDrawGroupInfo,"partial client retires acquired backing/publications");
    if(failed)operation();require(AllocationFault::live()==live,"corrected complete client retry retires exactly");
  }std::cout<<"client prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"native client family required");initMemoryManager();initialized=true;
    {Context context;Definition definition;Client client;client.init();auto* drawable=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);client.owner->destroyDrawable(drawable);}
    {const std::string family=argv[1];
    if(family=="modules" || family=="module-faults" || family.starts_with("registry-faults-")){
      Context context;Registry registry;LogicPublication logic;Definition definition;Client client;client.init();
      TheMemoryPoolFactory->createMemoryPool(moduleNames[0],sizeof(AnimatedParticleSysBoneClientUpdate),4,0);
      TheMemoryPoolFactory->createMemoryPool(moduleNames[1],sizeof(BeaconClientUpdate),4,0);
      TheMemoryPoolFactory->createMemoryPool(moduleNames[2],sizeof(SwayClientUpdate),4,0);
      moduleDefinition(registry,definition);auto* draw=client.owner->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);
      checkModules(draw);client.owner->destroyDrawable(draw);
    }
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto views=View::liveViewCount();
      if(family=="tactical-startup")tacticalStartup();else if(family=="functional")functional();else if(family=="frames")frames();else if(family=="draw-faults")drawFaults();else if(family=="client-faults")clientFaults();
      else if(family=="camera-lifetime")cameraLifetime();else if(family=="view-ownership")viewOwnership();
      else if(family=="modules")modulesFunctional();else if(family=="module-faults")modulesFaults();
      else if(family=="registry-faults-0")registryFaults(0);else if(family=="registry-faults-1")registryFaults(1);
      else if(family=="registry-faults-2")registryFaults(2);else if(family=="registry-faults-3")registryFaults(3);
      else throw std::runtime_error("unknown native client family");
      require(AllocationFault::live()==live && View::liveViewCount()==views && !TheGameClient && !TheRayEffects && !TheWritableGlobalData && !TheFileSystem && !TheMessageStream,"complete source client/context/View retirement");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original metadata retirement");
    std::cout<<"PASS actual headless client/Drawable owner; full startup/modules/output pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL native source client admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
