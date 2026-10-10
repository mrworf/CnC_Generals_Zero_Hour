// SPDX-License-Identifier: GPL-3.0-or-later
// Actual registered Drawable query dispatch; presentation is explicitly absent.
#include "PreRTS.h"
#include "W3DAssetInput.h"
#include "GameClient/NativeModelDrawCurrentQueries.h"
#include "GameClient/NativeModelDrawAnimationProducer.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/NativeHeadlessDisplay.h"
#include "NativeCameraViewFixture.h"
#include "GameLogic/ScriptEngine.h"
#include "WWMath/obbox.h"
#include "GameClient/Drawable.h"
#include "Common/ModuleFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "GameLogic/GameLogic.h"
#include <cfenv>
#include <array>
#include <cstdio>

namespace {
using namespace W3DAssetInput;
Assets* fixtureAssets=nullptr;
Assets& selectedAssets(){if(!fixtureAssets)throw ERROR_BAD_ARG;return *fixtureAssets;}
struct QueryModule : NativeModelDrawAnimationProducer {
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(QueryModule,"GeneratedNativeQueryModule",8,0)
public:
  QueryModule(Thing* thing,const ModuleData* data):NativeModelDrawAnimationProducer(thing,data,selectedAssets()){}
  bool producerEnabled=false;
  void initializeProducer(){initializeNativeAnimationProducer();producerEnabled=true;}
  void producerStep(const Matrix3D& root){advanceNativeConditionAnimation(root);}
  const ModelConditionInfo* selected()const{return nativeSelectedCondition();}
  const ModelConditionInfo* pending()const{return nativePendingCondition();}
  Int selectedAnimation()const{return nativeSelectedAnimation();}
  Bool paused()const{return nativeAnimationPaused();}
  UnsignedInt pendingDuration()const{return nativePendingDuration();}
  W3DModelDrawModuleData& inputData(){return const_cast<W3DModelDrawModuleData&>(nativeModelDrawData());}
  void capture(unsigned bone){m_current->binding.pose.capture(bone);}
  void control(unsigned bone,const NativeW3DPose::Matrix& value){m_current->binding.pose.control(bone,value,false);}
  static Module* create(Thing* thing,const ModuleData* data){return newInstance(QueryModule)(thing,data);}
  static ModuleData* createData(INI* ini){return W3DModelDrawModuleData::createFromINI(ini).release();}
  NameKeyType getModuleNameKey()const override{static const StaticNameKey name("W3DModelDraw");return name.key();}
  const ModelConditionInfo* cached()const{return nativeQueryData();}
  void current(std::string_view model,std::string_view clip,const Matrix3D& root,Real frame){replaceNativeCurrentModel(model,clip,root,frame);}
  void pose(const Matrix3D& root,Real frame){updateNativeCurrentPose(root,frame);}
  void timed(const Matrix3D& root,Real frame,NativeModelAnimationClock::Mode mode,std::uint32_t now){replaceNativeTimedModel("Outer","Tree.Walk",root,frame,mode,now);}
  void advance(const Matrix3D& root,std::uint32_t now){advanceNativeCurrentPose(root,now);}
  void multiplier(Real value){setNativeAnimationMultiplier(value);}
  const NativeModelAnimationClock* clock()const{return nativeAnimationClock();}
  const NativeW3DAssets::Binding* currentBinding()const{return nativeCurrentBinding();}
  void objectScale(Real value){setNativeCurrentObjectScale(value);}
  void updateProjectileClipStatus(UnsignedInt,UnsignedInt,WeaponSlotType)override{throw ERROR_BAD_ARG;}
  void updateDrawModuleSupplyStatus(Int,Int)override{throw ERROR_BAD_ARG;}
  void notifyDrawModuleDependencyCleared()override{throw ERROR_BAD_ARG;}
  void setHidden(Bool)override{} void replaceModelConditionState(const ModelConditionFlags& value)override{
    if(producerEnabled)NativeModelDrawAnimationProducer::replaceModelConditionState(value);
  }
  void replaceIndicatorColor(Color)override{} void setSelectable(Bool)override{}
  Bool handleWeaponFireFX(WeaponSlotType,Int,const FXList*,Real,const Coord3D*,Real)override{throw ERROR_BAD_ARG;}
  Int getBarrelCount(WeaponSlotType)const override{throw ERROR_BAD_ARG;}
  void setAnimationLoopDuration(UnsignedInt value)override{if(!producerEnabled)throw ERROR_BAD_ARG;NativeModelDrawAnimationProducer::setAnimationLoopDuration(value);}
  void setAnimationCompletionTime(UnsignedInt value)override{if(!producerEnabled)throw ERROR_BAD_ARG;NativeModelDrawAnimationProducer::setAnimationCompletionTime(value);}
  Bool updateBonesForClientParticleSystems()override{throw ERROR_BAD_ARG;}
  void setAnimationFrame(int value)override{if(!producerEnabled)throw ERROR_BAD_ARG;NativeModelDrawAnimationProducer::setAnimationFrame(value);}
  void setPauseAnimation(Bool value)override{if(!producerEnabled)throw ERROR_BAD_ARG;NativeModelDrawAnimationProducer::setPauseAnimation(value);}
  void updateSubObjects()override{throw ERROR_BAD_ARG;}
  void showSubObject(const AsciiString&,Bool)override{throw ERROR_BAD_ARG;}
  void doDrawModule(const Matrix3D*)override{throw ERROR_BAD_ARG;}
  void setFullyObscuredByShroud(Bool)override{}
  void reactToTransformChange(const Matrix3D*,const Coord3D*,Real)override{}
  void reactToGeometryChange()override{}
  void setShadowsEnabled(Bool)override{} void releaseShadows()override{} void allocateShadows()override{throw ERROR_BAD_ARG;}
};
QueryModule::~QueryModule()=default;
struct Factory:ModuleFactory {
  void init()override{RegistryTransaction transaction(*this);
    addModuleInternal(QueryModule::create,QueryModule::createData,MODULETYPE_DRAW,AsciiString("W3DModelDraw"),MODULEINTERFACE_DRAW);transaction.commit();}
};
struct Definition:ThingTemplate {~Definition()override=default;};
struct Logic:GameLogic {
  template<class F>auto phase(F action){LogicQueryPhase scope(*this);return action();}
};
Data bytes(std::string_view value){return {value.begin(),value.end()};}
Data numberedClientTree(){Data header,pivots;word(header,0x40001);text(header,"NumberedTree");word(header,71);for(unsigned n=0;n<3;++n)real(header,0);
  for(unsigned n=0;n<71;++n){char name[16]="Root";if(n)std::snprintf(name,sizeof(name),"Bone%02u",n);text(pivots,name);word(pivots,n?0:UINT32_MAX);
    real(pivots,float(n));for(unsigned k=0;k<8;++k)real(pivots,0);real(pivots,1);
  }return chunk(0x100,join({chunk(0x101,header),chunk(0x102,pivots)}),true);
}
std::vector<std::pair<std::string,Data>> inputs(){
  const std::string parent="Draw\nIgnoreConditionStates = NIGHT\nDefaultConditionState\nModel = Outer\nAnimation = Tree.Walk\nFlags = PRISTINE_BONE_POS_IN_FINAL_FRAME\nEnd\nConditionState = MOVING\nFlags = None\nEnd\nAliasConditionState = ATTACKING\nExtraPublicBone = Arm\nExtraPublicBone = Muzzle\nEnd\n";
  const std::string child="Draw\nDefaultConditionState\nModel = Outer\nAnimation = Tree.Walk\nFlags = PRISTINE_BONE_POS_IN_FINAL_FRAME\nWeaponFireFXBone = PRIMARY Muzzle\nWeaponLaunchBone = PRIMARY Muzzle\nWeaponRecoilBone = PRIMARY Arm\nTurret = Arm\nTurretArtAngle = 90\nEnd\nConditionState = MOVING\nFlags = None\nEnd\nAliasConditionState = ATTACKING\nAttachToBoneInAnotherModule = Arm\nEnd\n";
  return {{"parent.ini",bytes(parent)},{"child.ini",bytes(child)},{"definition.ini",bytes("Definition\nScale = 1\nEnd\n")},
    {"Art/W3D/NumberedTree.w3d",numberedClientTree()},{"Art/W3D/Numbered.w3d",hlod("Numbered","NumberedTree",{{"NULL",0}})},
    {"Art/W3D/BBox.w3d",join({box("BBox.BOUNDINGBOX"),box("BBox.Other",2)})},
    {"Art/W3D/Bounded.w3d",hlod("Bounded","Tree",{{"BBox.BOUNDINGBOX",2}})},
    {"Art/W3D/Ordinary.w3d",hlod("Ordinary","Tree",{{"BBox.Other",2}})},
    {"Art/W3D/Second.w3d",box("Second.BoundingBox",1,1,1)},
    {"Art/W3D/Reverse.w3d",hlod("Reverse","Tree",{{"BBox.BOUNDINGBOX",2},{"Second.BoundingBox",1}})},
    {"Art/W3D/DuplicateBox.w3d",join({box("DuplicateBox.Shape",0xf002,7,2),box("DuplicateBox.Shape",1,99,9)})}};
}
Factory* parserFactory=nullptr;Definition* parserDefinition=nullptr;const char* parserTag=nullptr;
void parseDefinition(INI* ini){ini->initFromINI(parserDefinition,parserDefinition->getFieldParse());}
void parse(INI* ini){AsciiString name("W3DModelDraw"),tag(parserTag);
  const auto* data=parserFactory->newModuleDataFromINI(ini,name,MODULETYPE_DRAW,tag);
  auto& modules=const_cast<ModuleInfo&>(parserDefinition->getDrawModuleInfo());
  modules.addModuleInfo(parserDefinition,name,tag,data,MODULEINTERFACE_DRAW,FALSE);
}
struct Fixture {
  FileSystem* previousFiles=TheFileSystem;NameKeyGenerator* previousNames=TheNameKeyGenerator;
  GlobalData* previousData=TheWritableGlobalData;GameLogic* previousLogic=TheGameLogic;
  ModuleFactory* previousFactory=TheModuleFactory;GameClient* previousClient=TheGameClient;
  MessageStream* previousMessages=TheMessageStream;
  Context input{false,inputs()};NameKeyGenerator names;MessageStream messages;
  Assets assets{input.files,"English"};
  std::unique_ptr<GlobalData> data;std::unique_ptr<Logic> logic;
  std::unique_ptr<Factory> factory;std::optional<Definition> definition;
  std::unique_ptr<NativeHeadlessGameClient> client;Drawable* draw=nullptr;
  Fixture(){names.init();TheNameKeyGenerator=&names;TheFileSystem=&input.files;TheMessageStream=&messages;fixtureAssets=&assets;
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();data->m_useDrawModuleLOD=FALSE;
      data->m_standardPublicBones={AsciiString("Arm"),AsciiString("Muzzle"),AsciiString("Part")};
      logic=std::make_unique<Logic>();TheGameLogic=logic.get();factory=std::make_unique<Factory>();TheModuleFactory=factory.get();factory->init();
      definition.emplace();parserFactory=factory.get();parserDefinition=&*definition;
      {const INIBlockDefinition blocks[]={{"Definition",parseDefinition}};INI ini;ini.loadBlocks("definition.ini",INI_LOAD_OVERWRITE,blocks);}
      const INIBlockDefinition blocks[]={{"Draw",parse}};
      {INI ini;parserTag="GeneratedParent";ini.loadBlocks("parent.ini",INI_LOAD_OVERWRITE,blocks);}
      {INI ini;parserTag="GeneratedChild";ini.loadBlocks("child.ini",INI_LOAD_OVERWRITE,blocks);}
      client=std::make_unique<NativeHeadlessGameClient>(NativeHeadlessGameClient::Composition::DrawablesOnly);TheGameClient=client.get();client->init();
      create();
    }catch(...){retire();throw;}
  }
  void retire()noexcept{
    TheGameClient=client.get();client.reset();draw=nullptr;TheGameClient=previousClient;
    definition.reset();TheModuleFactory=factory.get();factory.reset();TheModuleFactory=previousFactory;
    TheGameLogic=logic.get();logic.reset();TheGameLogic=previousLogic;
    TheWritableGlobalData=data.get();data.reset();TheWritableGlobalData=previousData;
    fixtureAssets=nullptr;parserFactory=nullptr;parserDefinition=nullptr;parserTag=nullptr;
    TheNameKeyGenerator=previousNames;TheFileSystem=previousFiles;TheMessageStream=previousMessages;
  }
  ~Fixture(){retire();}
  void create(){draw=client->friend_createDrawable(&*definition,DRAWABLE_STATUS_NONE);}
  void fresh(){if(draw)client->destroyDrawable(draw);draw=nullptr;assets.clear();create();}
  QueryModule& module(unsigned index){auto* value=dynamic_cast<QueryModule*>(draw->getDrawModules()[index]);require(value,"actual registered native query module");return *value;}
};
template<class F>void rejects(F action){bool failed=false;try{action();}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}require(failed,"defined invalid query admission");}
void verify(Fixture& fixture){fixture.logic->phase([&]{
  Coord3D positions[2];Matrix3D transforms[2];
  const auto count=fixture.draw->getPristineBonePositions("MUZZLE",0,positions,transforms,2);
  require(count==2 && positions[0].x==20 && positions[1].x==20,"actual Drawable walks both original query modules");
  Matrix3D launch(true);Coord3D rotation,pitch;
  require(fixture.draw->getProjectileLaunchOffset(PRIMARY_WEAPON,-1,&launch,TURRET_MAIN,&rotation,&pitch) && std::abs(launch.Get_X_Translation())<.001f && std::abs(launch.Get_Y_Translation()-20)<.001f && rotation.x==32 && pitch.x==16,"actual projectile selection, fallback barrel, art rotation and cross-module attachment offsets");
});}
void functional(Fixture& fixture){verify(fixture);fixture.logic->phase([&]{
  Coord3D widePositions[130]{};Matrix3D wideTransforms[130];for(auto& matrix:wideTransforms)matrix.Make_Identity();wideTransforms[2].Set_Translation(Vector3(77,0,0));
  require(fixture.draw->getPristineBonePositions("Muzzle",0,widePositions,wideTransforms,130)==2 && widePositions[0].x==20 && widePositions[1].x==20 && wideTransforms[2].Get_X_Translation()==77,"multi-module wide staging publishes only actually written transforms");
  AllocationFault::arm(SIZE_MAX);Matrix3D launch(true);Coord3D rotation;
  bool valid;try{valid=fixture.draw->getProjectileLaunchOffset(PRIMARY_WEAPON,99,&launch,TURRET_MAIN,&rotation);AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}
  require(valid && AllocationFault::attempts()==0,"fully warmed actual projectile/attachment dispatch has zero standard demand");
  Matrix3D missing(true);missing.Set_Translation(Vector3(91,92,93));Coord3D unchanged{71,72,73};
  require(!fixture.draw->getPristineBonePositions("Missing",0,&unchanged,&missing,1) && missing.Get_X_Translation()==0 && unchanged.x==71,"original missing pristine fallback does not count or publish a position");
  require(fixture.draw->getPristineBonePositions("Muzzle",0,nullptr,nullptr,2)==2,"count-only query remains real lookup");
  std::fesetround(FE_DOWNWARD);fixture.draw->getPristineBonePositions("Muzzle",0,nullptr,nullptr,1);require(std::fegetround()==FE_TONEAREST,"native logic queries restore original FPU mode");
});
  fixture.draw->setModelConditionState(MODELCONDITION_MOVING);
  fixture.logic->phase([&]{Coord3D position;require(fixture.draw->getPristineBonePositions("Muzzle",0,&position,nullptr,1)==1 && position.x==16,"logic condition reselects first frame rather than retaining prior state");});
  fixture.draw->clearModelConditionState(MODELCONDITION_MOVING);verify(fixture);
}
void negative(Fixture& fixture){verify(fixture);Matrix3D output(true);output.Set_Translation(Vector3(99,0,0));
  rejects([&]{fixture.draw->getPristineBonePositions("Muzzle",0,nullptr,&output,1);});
  require(output.Get_X_Translation()==99 && !fixture.logic->isInGameLogicUpdate(),"out-of-phase query leaves outputs and actual phase intact");
  require(fixture.draw->getPristineBonePositions(nullptr,-1,nullptr,&output,0)==0 && output.Get_X_Translation()==99,"zero capacity does not enter a query or touch output");
  fixture.logic->phase([&]{
    rejects([&]{fixture.draw->getPristineBonePositions(nullptr,0,nullptr,&output,1);});
    rejects([&]{fixture.draw->getPristineBonePositions("Muzzle",-1,nullptr,&output,1);});
    rejects([&]{fixture.draw->getPristineBonePositions("Muzzle",100,nullptr,&output,1);});
    rejects([&]{fixture.draw->getProjectileLaunchOffset(WeaponSlotType(UINT32_MAX),0,&output,TURRET_MAIN,nullptr);});
    rejects([&]{fixture.draw->getProjectileLaunchOffset(PRIMARY_WEAPON,0,&output,WhichTurretType(INT32_MAX),nullptr);});
  });
  bool workerRejected=false;std::thread worker([&]{try{fixture.draw->getPristineBonePositions("Muzzle",0,nullptr,&output,1);}catch(ErrorCode error){workerRejected=error==ERROR_BAD_ARG;}});worker.join();
  require(workerRejected && output.Get_X_Translation()==99,"worker query rejects before reading mutable simulation parents or output");
  rejects([&]{fixture.logic->phase([&]{throw ERROR_BAD_ARG;});});require(!fixture.logic->isInGameLogicUpdate(),"production phase guard restores its latch on exception");
}
void faults(Fixture& fixture,bool seeded,bool wide=false){std::size_t terminal;
  auto setup=[&]{fixture.fresh();if(seeded)verify(fixture);};
  auto operation=[&]{return fixture.logic->phase([&]{if(wide){Coord3D positions[130];Matrix3D transforms[130];return fixture.draw->getPristineBonePositions("Muzzle",0,positions,transforms,130)==2;}
    Matrix3D launch(true);Coord3D rotation;return bool(fixture.draw->getProjectileLaunchOffset(PRIMARY_WEAPON,0,&launch,TURRET_MAIN,&rotation));});};
  {NameKeyTransaction lease(fixture.names);setup();AllocationFault::arm(SIZE_MAX);
    try{require(operation(),"query discovery");terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}
    fixture.client->destroyDrawable(fixture.draw);fixture.draw=nullptr;fixture.assets.clear();
  }
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){NameKeyTransaction lease(fixture.names);setup();
    const auto* parent=fixture.module(0).cached();const auto* child=fixture.module(1).cached();const auto counts=fixture.assets.counts();
    NameKeyType cursor;{NameKeyTransaction proof(fixture.names);cursor=fixture.names.nameToKey("query-production-cursor");}
    auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");const auto units=pool->getUsedBlockCount();const auto live=AllocationFault::live();
    Matrix3D launch(true);launch.Set_Translation(Vector3(99,0,0));Coord3D rotation{91,92,93};bool failed=false;
    Coord3D positions[130];Matrix3D transforms[130];for(unsigned i=0;i<130;++i){positions[i]={91,92,93};transforms[i].Make_Identity();transforms[i].Set_Translation(Vector3(99,0,0));}
    AllocationFault::arm(ordinal);try{fixture.logic->phase([&]{if(wide)fixture.draw->getPristineBonePositions("Muzzle",0,positions,transforms,130);else fixture.draw->getProjectileLaunchOffset(PRIMARY_WEAPON,0,&launch,TURRET_MAIN,&rotation);});}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"every whole Drawable query prefix and exact terminal");
    if(failed){require(fixture.module(0).cached()==parent && fixture.module(1).cached()==child && fixture.assets.counts()==counts && AllocationFault::live()==live && pool->getUsedBlockCount()==units && launch.Get_X_Translation()==99 && rotation.x==91,"late multi-module/recursive failure restores every owner and caller output");
      for(unsigned i=0;i<130;++i)require(positions[i].x==91 && transforms[i].Get_X_Translation()==99,"late wide sweep rejection preserves the complete caller arrays");
      {NameKeyTransaction proof(fixture.names);require(fixture.names.nameToKey("query-production-cursor")==cursor,"production namespace rollback precedes fixture rollback");}
    }
    require(operation(),"same-owner complete corrected projectile retry");verify(fixture);
    fixture.client->destroyDrawable(fixture.draw);fixture.draw=nullptr;fixture.assets.clear();
  }fixture.create();std::cout<<"actual Drawable query "<<(wide?"wide":"projectile")<<' '<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void constructorFaults(Fixture& fixture){auto* accepted=fixture.draw;const auto acceptedID=accepted->getID();
  auto* pool=TheMemoryPoolFactory->findMemoryPool("GeneratedNativeQueryModule");require(pool,"actual native query-module pool");
  auto create=[&]{return fixture.client->friend_createDrawable(&*fixture.definition,DRAWABLE_STATUS_NONE);};
  AllocationFault::arm(SIZE_MAX);Drawable* measured=nullptr;
  try{measured=create();}catch(...){AllocationFault::disarm();throw;}
  const auto terminal=AllocationFault::attempts();AllocationFault::disarm();fixture.client->destroyDrawable(measured);
  auto intact=[&]{require(fixture.client->firstDrawable()==accepted && fixture.client->findDrawableByID(acceptedID)==accepted && pool->getUsedBlockCount()==2,"accepted actual Drawable/module pool graph stays intact");};
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();const auto next=fixture.client->getDrawableIDCounter();Drawable* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=create();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"actual query-module constructor standard prefixes and exact terminal");
    if(failed){intact();require(AllocationFault::live()==live && fixture.client->getDrawableIDCounter()==next,"failed constructor retires arrays/module prefix and rolls back production Drawable cursor");candidate=create();}
    fixture.client->destroyDrawable(candidate);intact();require(AllocationFault::live()==live,"complete constructor retry retirement");
  }
  for(unsigned prefix=0;prefix<2;++prefix){std::vector<void*> held;held.reserve(6);for(unsigned i=0;i<6-prefix;++i)held.push_back(pool->allocateBlock("generated fixed native module capacity"));
    const auto used=pool->getUsedBlockCount();const auto live=AllocationFault::live();const auto next=fixture.client->getDrawableIDCounter();bool failed=false;
    try{auto* unexpected=create();fixture.client->destroyDrawable(unexpected);}catch(ErrorCode error){failed=error==ERROR_OUT_OF_MEMORY;}
    require(failed && pool->getUsedBlockCount()==used && AllocationFault::live()==live && fixture.client->getDrawableIDCounter()==next,"each pooled native module acquisition failure retires the candidate prefix exactly");
    for(auto* unit:held)pool->freeBlock(unit);auto* retry=create();fixture.client->destroyDrawable(retry);intact();
  }
  std::cout<<"actual native draw construction prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; pooled prefixes [0,2); terminal 2\n";
}
Matrix3D clientRoot(){Matrix3D root(true);root.Rotate_Z(1.5707963267948966f);root.Set_Translation(Vector3(100,200,300));return root;}
bool close(Real a,Real b){return std::abs(a-b)<.001f;}
void seedClient(Fixture& fixture){const auto root=clientRoot();fixture.module(0).current("Outer","Tree.Walk",root,0);
  auto second=root;second.Set_Translation(Vector3(400,500,600));fixture.module(1).current("Outer","Tree.Walk",second,1);}
void verifyClient(Fixture& fixture){Matrix3D world(true);
  require(fixture.draw->getCurrentWorldspaceClientBonePositions("MUZZLE",world)&&close(world.Get_X_Translation(),100)&&close(world.Get_Y_Translation(),216)&&close(world.Get_Z_Translation(),300),"actual Drawable current world pose is first valid interface, not final pristine pose");
  Coord3D positions[2];Matrix3D transforms[2];
  require(fixture.draw->getCurrentClientBonePositions("Muzzle",0,positions,transforms,2)==2&&close(positions[0].x,16)&&close(positions[1].x,20)&&close(positions[0].y,0),"actual current model-space traversal preserves world inverse and each module's live frame");
}
void clientFunctional(Fixture& fixture){seedClient(fixture);verifyClient(fixture);verify(fixture);
  fixture.logic->phase([&]{fixture.module(0).current("Outer","Tree.Walk",clientRoot(),1);
    Coord3D point;require(fixture.draw->getPristineBonePositions("Muzzle",0,&point,nullptr,1)==1&&point.x==20,"source logic condition publication does not replace independent pristine query selection");});
  seedClient(fixture);verifyClient(fixture);
  const AsciiString bone("Muzzle");const auto* di=fixture.draw->getDrawModules()[0]->getObjectDrawInterface();Matrix3D output(true);
  require(di->clientOnly_getRenderObjBoneTransform(bone,&output)&&close(output.Get_Y_Translation(),216),"real object draw interface uses current world-space client transform");
  auto shifted=clientRoot();shifted.Set_Translation(Vector3(7,8,9));AllocationFault::arm(SIZE_MAX);
  try{fixture.module(0).pose(shifted,1);fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output);fixture.draw->getCurrentClientBonePositions("Muzzle",0,nullptr,nullptr,2);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
  require(attempts==0&&close(output.Get_X_Translation(),7)&&close(output.Get_Y_Translation(),28),"warmed whole current pose update and actual reads have exact zero standard demand");
  Matrix3D missing(true);missing.Set_Translation(Vector3(99,0,0));const AsciiString rootName("Root");
  require(!di->clientOnly_getRenderObjBoneTransform(rootName,&missing)&&missing.Get_X_Translation()==0,"client bone sentinel writes defined identity when a model exists");
  missing.Set_Translation(Vector3(99,0,0));require(!fixture.draw->getCurrentWorldspaceClientBonePositions("Root",missing)&&missing.Get_X_Translation()==99,"world-space miss leaves output untouched");
  Coord3D unchanged{91,92,93};require(!fixture.draw->getCurrentClientBonePositions("Missing",0,&unchanged,&missing,1)&&unchanged.x==91&&missing.Get_X_Translation()==99,"current sweep has no pristine missing fallback");
  fixture.draw->setInstanceScale(2);fixture.module(0).current("Outer","Tree.Walk",clientRoot(),0);
  Coord3D scaled;require(fixture.module(0).getCurrentBonePositions("Muzzle",0,&scaled,nullptr,1)==1&&close(scaled.x,264)&&close(scaled.y,-100)&&close(scaled.z,300),"source inverse.Scale changes axes, not translation, and retains Drawable tree scaling");
  fixture.draw->setInstanceScale(1);seedClient(fixture);verifyClient(fixture);
  fixture.module(0).current("","",clientRoot(),0);missing.Set_Translation(Vector3(99,0,0));
  require(!di->clientOnly_getRenderObjBoneTransform(bone,&missing)&&missing.Get_X_Translation()==99,"absent model differs from missing bone identity output");
  fixture.module(0).current("Static.Mesh","",clientRoot(),0);require(!di->clientOnly_getRenderObjBoneTransform(bone,&missing)&&missing.Get_X_Translation()==0,"real static mesh exists but has no named bone");
  fixture.module(0).current("Outer","Tree.Walk",clientRoot(),0);verifyClient(fixture);
}
template<class F>void clientRejects(F action){bool rejected=false;try{action();}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG||error==ERROR_CORRUPT_FILE_FORMAT;}catch(const NativeW3DAssetFailure&){rejected=true;}require(rejected,"defined current-client rejection");}
void clientNegative(Fixture& fixture){seedClient(fixture);const auto* accepted=fixture.module(0).currentBinding();const auto counts=fixture.assets.counts();Matrix3D output(true);output.Set_Translation(Vector3(99,0,0));
  require(fixture.draw->getCurrentClientBonePositions(nullptr,-1,nullptr,&output,0)==0&&output.Get_X_Translation()==99,"zero current capacity does not validate or publish");
  clientRejects([&]{fixture.draw->getCurrentClientBonePositions(nullptr,0,nullptr,&output,1);});
  clientRejects([&]{fixture.draw->getCurrentClientBonePositions("Muzzle",100,nullptr,&output,1);});
  clientRejects([&]{fixture.module(0).clientOnly_getRenderObjBoneTransform(AsciiString("Muzzle"),nullptr);});
  fixture.logic->phase([&]{clientRejects([&]{fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output);});});
  bool rejected=false;auto& module=fixture.module(0);std::thread worker([&]{try{module.getCurrentWorldspaceClientBonePositions("Muzzle",output);}catch(ErrorCode e){rejected=e==ERROR_BAD_ARG;}});worker.join();require(rejected&&output.Get_X_Translation()==99,"current worker/logic access cannot publish client output");
  auto invalid=clientRoot();invalid[1][2]=std::numeric_limits<Real>::quiet_NaN();clientRejects([&]{module.pose(invalid,1);});
  invalid=clientRoot();invalid[0][0]=std::numeric_limits<Real>::max();clientRejects([&]{module.pose(invalid,1);});
  clientRejects([&]{module.current("Outer","Tree.Walk",clientRoot(),std::numeric_limits<Real>::infinity());});
  clientRejects([&]{module.current("NoFile","",clientRoot(),0);});
  clientRejects([&]{module.current("Outer","OtherTree.OtherWalk",clientRoot(),0);});
  require(module.currentBinding()==accepted&&fixture.assets.counts()==counts,"late/nonfinite/wrong hierarchy rejection preserves accepted current model and asset identities");verifyClient(fixture);
  fixture.draw->setInstanceScale(std::numeric_limits<Real>::quiet_NaN());
  clientRejects([&]{module.current("Outer","Tree.Walk",clientRoot(),0);});
  clientRejects([&]{fixture.draw->getCurrentClientBonePositions("Muzzle",0,nullptr,&output,1);});
  require(module.currentBinding()==accepted&&output.Get_X_Translation()==99,"nonfinite Drawable scale preserves current state and caller output");fixture.draw->setInstanceScale(1);verifyClient(fixture);
  auto distant=Matrix3D(true);distant.Set_Translation(Vector3(std::numeric_limits<Real>::max(),0,0));fixture.module(1).pose(distant,1);fixture.draw->setInstanceScale(2);
  Coord3D positions[2]={{91,92,93},{91,92,93}};Matrix3D transforms[2];for(auto& t:transforms){t.Make_Identity();t.Set_Translation(Vector3(99,0,0));}
  clientRejects([&]{fixture.draw->getCurrentClientBonePositions("Muzzle",0,positions,transforms,2);});
  require(positions[0].x==91&&positions[1].x==91&&transforms[0].Get_X_Translation()==99&&transforms[1].Get_X_Translation()==99,"late second-module current query overflow preserves all caller outputs");
  fixture.draw->setInstanceScale(1);auto second=clientRoot();second.Set_Translation(Vector3(400,500,600));fixture.module(1).pose(second,1);verifyClient(fixture);
}
void clientNumbered(Fixture& fixture){const Matrix3D identity(true);fixture.module(0).current("Numbered","",identity,0);fixture.module(1).current("Numbered","",identity,0);
  Coord3D positions[130];Matrix3D transforms[130];for(unsigned i=0;i<130;++i){positions[i]={91,92,93};transforms[i].Make_Identity();transforms[i].Set_Translation(Vector3(99,0,0));}
  AllocationFault::arm(SIZE_MAX);Int count;
  try{count=fixture.draw->getCurrentClientBonePositions("bOnE",1,positions,transforms,130);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();require(count==128&&attempts==0,"actual two-module numbered sweep caps64 per module with zero standard demand");
  for(unsigned i=0;i<128;++i)require(positions[i].x==float(i%64+1)&&transforms[i].Get_X_Translation()==float(i%64+1),"every actual numbered transform is published in source module/index order");
  require(positions[128].x==91&&transforms[128].Get_X_Translation()==99,"numbered sweep never publishes uncounted tail");
  require(fixture.module(0).getCurrentBonePositions("Bone",65,positions,transforms,64)==6&&positions[0].x==65&&positions[5].x==70,"numbered current sweep stops at first gap before99");
  positions[0]={91,92,93};transforms[0].Set_Translation(Vector3(99,0,0));
  require(!fixture.draw->getCurrentClientBonePositions("Bone",0,positions,transforms,130)&&positions[0].x==91&&transforms[0].Get_X_Translation()==99,"current start0 is exact, not an implicit numbered sweep");
}
void clientFaults(Fixture& fixture,bool seeded){const auto root=clientRoot();auto setup=[&]{fixture.fresh();if(seeded)fixture.module(0).current("Static.Mesh","",root,0);};
  auto operation=[&]{fixture.module(0).timed(root,0,NativeModelAnimationClock::Mode::Once,100);};std::size_t terminal;
  {NameKeyTransaction lease(fixture.names);setup();AllocationFault::arm(SIZE_MAX);try{operation();terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}
    fixture.client->destroyDrawable(fixture.draw);fixture.draw=nullptr;fixture.assets.clear();}
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){NameKeyTransaction lease(fixture.names);setup();const auto* accepted=fixture.module(0).currentBinding();const auto counts=fixture.assets.counts();const auto live=AllocationFault::live();
    auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");const auto units=pool->getUsedBlockCount();NameKeyType cursor;{NameKeyTransaction proof(fixture.names);cursor=fixture.names.nameToKey("current-production-cursor");}
    bool failed=false;AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal<terminal)&&failed==hit&&attempts==(failed?ordinal+1:terminal),"complete current replacement exact terminal and every prefix");
    if(failed){require(fixture.module(0).currentBinding()==accepted&&fixture.assets.counts()==counts&&AllocationFault::live()==live&&pool->getUsedBlockCount()==units,"current candidate and late asset-cache admissions unwind exactly");
      {NameKeyTransaction proof(fixture.names);require(fixture.names.nameToKey("current-production-cursor")==cursor,"current production namespace proof precedes fixture rollback");}
    }operation();Matrix3D output(true);require(fixture.module(0).clock()&&fixture.module(0).clock()->frame()==0&&fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output)&&close(output.Get_Y_Translation(),216),"actual timed current-query corrected retry");
    fixture.client->destroyDrawable(fixture.draw);fixture.draw=nullptr;fixture.assets.clear();
  }fixture.create();std::cout<<"current model "<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void animationClock(){using Clock=NativeModelAnimationClock;using Mode=Clock::Mode;
  auto make=[](Mode mode,float frame=0,std::uint32_t now=100){return Clock::start(5,10,frame,mode,now);};
  require(make(Mode::Manual,2).advanced(10000).frame()==2,"manual timing is retained");
  auto once=make(Mode::Once).advanced(200);require(close(once.frame(),1)&&once.previousFrame()==0&&!once.complete(),"once progresses but completes only retained endpoint");
  require(once.advanced(200).previousFrame()==0,"duplicate sync time preserves previous event frame");
  require(once.advanced(1000).frame()==4&&once.advanced(1000).complete(),"once endpoint clamps and completes");
  require(close(make(Mode::Loop,3).advanced(300).frame(),1)&&make(Mode::Loop).advanced(1100).frame()==0,"loop wraps once then resets rather than modulo");
  require(close(make(Mode::LoopBackwards,1).advanced(300).frame(),3)&&make(Mode::LoopBackwards).advanced(1100).frame()==4,"backwards loop adds once then resets to endpoint");
  require(make(Mode::OnceBackwards,4).advanced(1100).complete(),"backwards once clamps0");
  auto ping=make(Mode::PingPong,3).advanced(300);require(close(ping.frame(),3)&&ping.direction()==-1,"forward pingpong reflects once");
  ping=ping.advanced(700);require(close(ping.frame(),1)&&ping.direction()==1,"backward pingpong reflects and reverses");
  require(make(Mode::PingPong).advanced(1100).frame()==-2,"source forward oversized pingpong retains negative result, not invented modulo");
  auto backwards=make(Mode::PingPong,3).advanced(300).advanced(1300);require(backwards.frame()==0&&backwards.direction()==1,"source oversized backward reflection resets0");
  auto paused=make(Mode::Loop,2);paused.setMultiplier(0);require(paused.advanced(10000).frame()==2,"zero multiplier retains source frame");
  auto wrap=Clock::start(5,10,0,Mode::Once,UINT32_MAX-49).advanced(50);require(close(wrap.frame(),1),"unsigned32 sync time wrap");
  require(Clock::start(1,30,0,Mode::Once,0).advanced(100).complete(),"single-frame once endpoint");
  auto accelerated=make(Mode::Once);accelerated.setMultiplier(2);require(close(accelerated.advanced(200).frame(),2),"source rate multiplier");
  clientRejects([&]{Clock::start(0,10,0,Mode::Once,0);});clientRejects([&]{Clock::start(5,0,0,Mode::Once,0);});
  clientRejects([&]{Clock::start(5,10,0,static_cast<Mode>(UINT32_MAX),0);});
  clientRejects([&]{paused.setMultiplier(std::numeric_limits<Real>::infinity());});require(paused.multiplier()==0,"rejected multiplier preserves state");
  accelerated.setMultiplier(std::numeric_limits<Real>::max());clientRejects([&]{accelerated.advanced(10000);});require(accelerated.frame()==0,"nonfinite timing arithmetic cannot commit");
}
void animationPose(Fixture& fixture){using Mode=NativeModelAnimationClock::Mode;auto& module=fixture.module(0);const auto root=clientRoot();
  module.timed(root,0,Mode::Once,100);Matrix3D output(true);AllocationFault::arm(SIZE_MAX);
  try{module.advance(root,134);fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
  require(attempts==0&&module.clock()->complete()&&close(output.Get_Y_Translation(),220),"actual timed pose advances from source30fps with zero standard demand");
  auto invalid=root;invalid[0][0]=std::numeric_limits<Real>::max();
  module.timed(root,0,Mode::Once,100);clientRejects([&]{module.advance(invalid,134);});
  require(module.clock()->frame()==0&&!module.clock()->complete(),"late pose failure preserves frame, sync and completion");
  module.advance(root,134);require(module.clock()->complete(),"corrected same-time pose retry advances source clock");
  module.timed(root,1,Mode::OnceBackwards,100);module.advance(root,134);require(module.clock()->complete(),"actual backward current pose");
  module.timed(root,0,Mode::PingPong,100);const auto* identity=module.currentBinding();module.advance(root,1000);
  Matrix3D negativeOutput(true);
  require(module.currentBinding()==identity&&module.clock()->frame()<0&&module.clock()->direction()==-1&&
    fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",negativeOutput)&&close(negativeOutput.Get_Y_Translation(),214),"source oversized negative raw frame publishes bounded default-channel pose without rewriting clock");
  module.advance(root,1020);require(module.clock()->frame()==0&&module.clock()->direction()==1,"source reverse reflection fallback remains coupled to accepted raw pose");
  module.current("Outer","Tree.Walk",root,-1);require(fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",negativeOutput)&&close(negativeOutput.Get_Y_Translation(),214),"source maintain-frame -1 admits raw default pose through actual Drawable");
  clientRejects([&]{module.pose(root,2147483648.f);});
  require(fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",negativeOutput)&&close(negativeOutput.Get_Y_Translation(),214),"undefined signed conversion rejection preserves accepted whole current pose");
  module.timed(root,0,Mode::Once,100);
  fixture.logic->phase([&]{clientRejects([&]{module.advance(root,121);});});
  module.multiplier(0);const auto frame=module.clock()->frame();module.advance(root,10000);require(module.clock()->frame()==frame,"explicit zero rate multiplier retains frame; not source pause callback");
  module.pose(root,0);require(!module.clock(),"explicit manual frame supersedes timed advancement");
  module.pose(root,1);module.timed(root,0,Mode::Once,100);require(module.clock()->previousFrame()==1,"manual-to-timed replacement retains prior source frame");
}
void displayTime(Fixture& fixture){
  NativePresentationClock fast;require(fast.advance(0,33,false,true).cameraOnly&&fast.accumulated()==33&&fast.published()==0,"fast clock retains accumulated but unpublished time");
  require(!fast.advance(0,33,false,true).cameraOnly&&fast.published()==33,"repeated fast frame freezes and publishes previously deferred time");
  fast.advance(1,33,true,true);require(fast.accumulated()==33&&fast.published()==33,"freeze takes precedence over fast time");
  fast.advance(2,33,false,true);fast.advance(3,33,false,false);require(fast.published()==99,"next ordinary publication includes fast-time accumulation");
  NativePresentationClock wrap(UINT32_MAX-9,UINT32_MAX-9);wrap.advance(0,20,false,false);require(wrap.published()==10,"fixed-step time preserves unsigned32 wrap");
  const auto oldScripts=TheScriptEngine;const auto oldDisplay=TheDisplay;const auto oldView=TheTacticalView;
  struct Restore{ScriptEngine* scripts;Display* display;View* view;~Restore(){TheScriptEngine=scripts;TheDisplay=display;TheTacticalView=view;}}restore{oldScripts,oldDisplay,oldView};
  auto scripts=std::make_unique<ScriptEngine>();TheScriptEngine=scripts.get();scripts->init();
  auto display=std::make_unique<NativeHeadlessDisplay>();TheDisplay=display.get();
  auto view=std::make_unique<NativeCameraViewFixture>();view->setFrameLengthMilliseconds(33);
  auto* primary=view.get();display->attachView(view.get());view.release();TheTacticalView=primary;
  fixture.client->setFrame(0);fixture.module(0).timed(clientRoot(),0,NativeModelAnimationClock::Mode::Once,display->animationSyncTime());
  AllocationFault::arm(SIZE_MAX);try{display->advanceLogicalFrame();fixture.module(0).advance(clientRoot(),display->animationSyncTime());}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();Matrix3D output(true);
  require(attempts==0&&display->animationSyncTime()==33&&fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output)&&close(output.Get_Y_Translation(),220),"actual display-owned source33ms tick reaches current Drawable with zero standard demand");
  display->advanceLogicalFrame();require(display->animationSyncTime()==33,"actual repeated client frame does not advance display time");
  scripts->doFreezeTime();fixture.client->setFrame(1);display->advanceLogicalFrame();require(display->animationSyncTime()==33,"actual source script freeze retains display time");
  scripts->doUnfreezeTime();display->advanceLogicalFrame();require(display->animationSyncTime()==33,"same-frame unfreeze retains source suppression");
  fixture.client->setFrame(2);primary->rotateCamera(1,330,0,0);primary->cameraModFreezeTime();display->advanceLogicalFrame();require(display->animationSyncTime()==33,"unfinished source frozen camera prevents animation-time advancement");
  const auto time=display->animationSyncTime();TheScriptEngine=nullptr;clientRejects([&]{display->advanceLogicalFrame();});TheScriptEngine=scripts.get();
  TheTacticalView=nullptr;clientRejects([&]{display->advanceLogicalFrame();});TheTacticalView=primary;
  fixture.logic->phase([&]{clientRejects([&]{display->advanceLogicalFrame();});});
  bool rejected=false;std::thread worker([&]{try{display->advanceLogicalFrame();}catch(ErrorCode e){rejected=e==ERROR_BAD_ARG;}});worker.join();
  require(rejected&&display->animationSyncTime()==time,"worker and missing/phase parent rejection leave accepted time unchanged");
  clientRejects([&]{display->draw();});
  delete primary;TheTacticalView=nullptr;
  auto replacement=std::make_unique<NativeCameraViewFixture>();replacement->setFrameLengthMilliseconds(17);primary=replacement.get();display->attachView(replacement.get());replacement.release();TheTacticalView=primary;
  fixture.client->setFrame(3);display->advanceLogicalFrame();require(display->animationSyncTime()==50,"replaced camera shares configured source step and same-owner retry");
  // Source display reset resets its views, not the retained sync-time owner.
  delete primary;TheTacticalView=nullptr;display->reset();require(display->animationSyncTime()==50,"ordinary display reset preserves monotonically retained sync time");
  display.reset();TheDisplay=oldDisplay;auto fresh=std::make_unique<NativeHeadlessDisplay>();require(fresh->animationSyncTime()==0,"new display owns fresh time instead of process-static state");
  scripts.reset();TheScriptEngine=oldScripts;
}
struct ProducerTimeOwner {
  ScriptEngine* oldScripts=TheScriptEngine;Display* oldDisplay=TheDisplay;View* oldView=TheTacticalView;
  std::unique_ptr<ScriptEngine> scripts;
  std::unique_ptr<NativeHeadlessDisplay> display;
  UnsignedInt frame=0;
  ProducerTimeOwner(){
    try{
      scripts=std::make_unique<ScriptEngine>();TheScriptEngine=scripts.get();scripts->init();
      display=std::make_unique<NativeHeadlessDisplay>();TheDisplay=display.get();
      auto view=std::make_unique<NativeCameraViewFixture>();view->setFrameLengthMilliseconds(33);
      TheTacticalView=view.get();display->attachView(view.get());view.release();
    }catch(...){retire();throw;}
  }
  void retire()noexcept{TheDisplay=display.get();display.reset();TheDisplay=oldDisplay;TheTacticalView=oldView;
    scripts.reset();TheScriptEngine=oldScripts;}
  ~ProducerTimeOwner(){retire();}
  void tick(Fixture& fixture){fixture.client->setFrame(frame++);display->advanceLogicalFrame();}
};
void configureProducer(Fixture& fixture){
  auto& data=fixture.module(0).inputData();
  auto idle=data.m_conditionStates[0];idle.m_flags=1<<MAINTAIN_FRAME_ACROSS_STATES;idle.m_mode=2;
  idle.m_animations.clear();idle.m_animations.emplace_back(AsciiString("Tree.Walk"),FALSE,0);
  idle.m_transitionKey=TheNameKeyGenerator->nameToKey("generatedProducerIdle");
  idle.m_allowToFinishKey=NAMEKEY_INVALID;idle.m_transitionSig=NO_TRANSITION;
  idle.m_animMinSpeedFactor=idle.m_animMaxSpeedFactor=1;
  idle.m_conditionsYesVec={ModelConditionFlags{}};
  auto moving=idle;ModelConditionFlags movingFlags;movingFlags.set(MODELCONDITION_MOVING);
  moving.m_conditionsYesVec={movingFlags};moving.m_transitionKey=TheNameKeyGenerator->nameToKey("generatedProducerMoving");
  auto attacking=idle;ModelConditionFlags attackFlags;attackFlags.set(MODELCONDITION_ATTACKING);
  attacking.m_conditionsYesVec={attackFlags};attacking.m_transitionKey=NAMEKEY_INVALID;
  attacking.m_mode=5;attacking.m_flags=(1<<START_FRAME_FIRST)|(1<<START_FRAME_LAST);
  data.m_conditionStates={idle,moving,attacking};data.m_transitionMap.clear();
}
void producerFunctional(Fixture& fixture){
  ProducerTimeOwner time;configureProducer(fixture);auto& module=fixture.module(0);
  GameClientRandomTransaction randomOwner;module.initializeProducer();
  const ModelConditionFlags empty;ModelConditionFlags moving;moving.set(MODELCONDITION_MOVING);
  ModelConditionFlags attacking;attacking.set(MODELCONDITION_ATTACKING);
  require(module.selected()==&module.inputData().m_conditionStates[0]&&module.clock()->mode()==NativeModelAnimationClock::Mode::Once,"shared original producer initializes idle through actual module");
  const auto model=module.currentBinding()->model;
  module.objectScale(3);module.capture(1);auto control=NativeW3DPose::identity();control[3]=2;module.control(1,control);
  Coord3D position;Real radius;Matrix3D transform;
  module.clientOnly_getRenderObjInfo(&position,&radius,&transform);const auto priorRadius=radius;
  module.replaceModelConditionState(moving);Matrix3D output(true);
  require(module.currentBinding()->model==model&&module.clock()->frame()==-1&&fixture.draw->getCurrentWorldspaceClientBonePositions("Muzzle",output)&&output.Get_X_Translation()==16,"same-model maintain -1 retains source captured controls and raw defaults");
  module.clientOnly_getRenderObjInfo(&position,&radius,&transform);
  require(radius==priorRadius,"same-model source ObjectScale is retained across condition replacement");
  module.setAnimationLoopDuration(10);require(module.clock()->multiplier()<1,"source logic-frame loop duration overrides selected rate");
  module.setPauseAnimation(TRUE);require(module.paused()&&module.clock()->mode()==NativeModelAnimationClock::Mode::Manual&&module.clock()->multiplier()==1,"pause saves mode and resets rate at retained frame");
  module.setAnimationLoopDuration(10);const auto rate=module.clock()->multiplier();
  module.setPauseAnimation(TRUE);require(module.clock()->multiplier()==rate,"duplicate pause does not reset rate/sync");
  module.setPauseAnimation(FALSE);require(!module.paused()&&module.clock()->mode()==NativeModelAnimationClock::Mode::Once&&module.clock()->multiplier()==1,"resume restores saved mode and resets rate");
  module.replaceModelConditionState(attacking);require(module.clock()->frame()==0&&module.clock()->mode()==NativeModelAnimationClock::Mode::OnceBackwards,"FIRST precedes LAST and backwards default");
  const auto accepted=module.selected();const auto frame=module.clock()->frame();
  auto oldDisplay=TheDisplay;TheDisplay=nullptr;clientRejects([&]{module.setAnimationFrame(1);});TheDisplay=oldDisplay;
  require(module.selected()==accepted&&module.clock()->frame()==frame,"missing time owner preserves selected/current state");
  module.setAnimationFrame(1);require(module.clock()->mode()==NativeModelAnimationClock::Mode::Manual&&module.clock()->frame()==1,"actual animation frame callback uses source manual mode");
  clientRejects([&]{module.setAnimationFrame(INT32_MAX);});require(module.clock()->frame()==1,"undefined signed float conversion preserves manual frame");
  module.replaceModelConditionState(empty);time.tick(fixture);
  AllocationFault::arm(0);try{module.producerStep(Matrix3D(true));}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();require(attempts==0,"ordinary producer pose tick has zero allocation demand");
}
void producerTransitions(Fixture& fixture){
  ProducerTimeOwner time;configureProducer(fixture);auto& module=fixture.module(0);auto& data=module.inputData();
  data.m_conditionStates[1].m_allowToFinishKey=data.m_conditionStates[0].m_transitionKey;
  auto transition=data.m_conditionStates[0];transition.m_flags=0;
  transition.m_transitionKey=NAMEKEY_INVALID;transition.m_allowToFinishKey=NAMEKEY_INVALID;
  transition.m_transitionSig=buildTransitionSig(data.m_conditionStates[0].m_transitionKey,data.m_conditionStates[1].m_transitionKey);
  data.m_transitionMap.emplace(transition.m_transitionSig,transition);
  GameClientRandomTransaction randomOwner;module.initializeProducer();
  const ModelConditionFlags empty;ModelConditionFlags moving;moving.set(MODELCONDITION_MOVING);
  module.replaceModelConditionState(moving);
  require(module.selected()==&data.m_conditionStates[0]&&module.pending()==&data.m_conditionStates[1],"AllowToFinish takes precedence over explicit transition");
  module.setAnimationLoopDuration(7);module.replaceModelConditionState(moving);
  require(module.pending()==&data.m_conditionStates[1],"duplicate pending request retains wait target");
  module.replaceModelConditionState(empty);require(!module.pending()&&module.clock()->frame()==0,"request current state while pending cancels and restarts current");
  module.setAnimationFrame(1); // Manual is not complete, so restore Once before explicit transition.
  module.replaceModelConditionState(moving);module.replaceModelConditionState(empty);
  time.tick(fixture);module.producerStep(Matrix3D(true));time.tick(fixture);module.producerStep(Matrix3D(true));
  require(module.clock()->complete(),"accepted current frame reaches retained source completion");
  module.replaceModelConditionState(moving);
  require(module.selected()->m_transitionSig!=NO_TRANSITION&&module.pending()==&data.m_conditionStates[1],"completed wait source enters explicit transition");
  module.setAnimationCompletionTime(10);
  require(module.pendingDuration()==5&&module.clock()->multiplier()<1,"completion-time uses first clip durations and defers target remainder");
  module.replaceModelConditionState(moving);require(module.pendingDuration()==5,"duplicate pending request retains deferred duration");
  for(unsigned n=0;n<8&&module.pending();++n){time.tick(fixture);module.producerStep(Matrix3D(true));}
  require(module.selected()==&data.m_conditionStates[1]&&!module.pending()&&module.pendingDuration()==~UnsignedInt(0)&&module.clock()->multiplier()<1,"transition completion admits target and its deferred duration");
  module.setPauseAnimation(TRUE);
  module.replaceModelConditionState(empty); // Same model keeps paused flag but starts its selected animation.
  require(module.paused()&&module.clock()->mode()==NativeModelAnimationClock::Mode::Once,"same-model selection while paused retains source duplicate-notification semantics");
  module.setPauseAnimation(TRUE);require(module.clock()->mode()==NativeModelAnimationClock::Mode::Once,"duplicate true pause cannot re-pause newly selected reused animation");
}
void producerSelection(Fixture& fixture){
  ProducerTimeOwner time;configureProducer(fixture);auto& module=fixture.module(0);auto& data=module.inputData();
  auto& idle=data.m_conditionStates[0];idle.m_flags=(1<<RANDOMIZE_START_FRAME)|(1<<START_FRAME_FIRST)|(1<<START_FRAME_LAST);
  idle.m_animations.clear();for(unsigned n=0;n<3;++n)idle.m_animations.emplace_back(AsciiString("Tree.Walk"),TRUE,0);
  idle.m_animMinSpeedFactor=.5f;idle.m_animMaxSpeedFactor=1.5f;
  GameClientRandomTransaction randomOwner;
  Int expectedIndex,expectedFrame;Real expectedRate;{
    GameClientRandomTransaction proof;expectedIndex=GameClientRandomValue(0,2);expectedFrame=GameClientRandomValue(0,1);expectedRate=GameClientRandomValueReal(.5f,1.5f);
  }
  module.initializeProducer();require(module.selectedAnimation()==expectedIndex&&module.clock()->frame()==expectedFrame&&module.clock()->multiplier()==expectedRate,"weighted repeated records and RANDOMSTART precedence consume exact original client cursor");
  const auto avoid=module.selectedAnimation();
  // RANDOMSTART can already select the endpoint. Do not restart that clip
  // while trying to establish completion of the retained selection.
  if(!module.clock()->complete()){
    module.multiplier(100);time.tick(fixture);module.producerStep(Matrix3D(true));
  }
  require(module.clock()->complete(),"weighted idle reaches retained completion");
  Int nextIndex,nextFrame;Real nextRate;{
    GameClientRandomTransaction proof;do {nextIndex=GameClientRandomValue(0,2);}while(nextIndex==avoid);
    nextFrame=GameClientRandomValue(0,1);nextRate=GameClientRandomValueReal(.5f,1.5f);
  }
  module.producerStep(Matrix3D(true));
  require(module.selectedAnimation()==nextIndex&&module.selectedAnimation()!=avoid&&module.clock()->frame()==nextFrame&&module.clock()->multiplier()==nextRate,"idle restart rejection-samples a distinct record index without deduplicating clip names");
  bool workerRejected=false;std::thread worker([&]{try{module.setPauseAnimation(TRUE);}catch(ErrorCode e){workerRejected=e==ERROR_BAD_ARG;}});worker.join();
  require(workerRejected,"producer rejects workers before RNG/parent access");
}
std::array<Int,8> probeClientCursor(){
  GameClientRandomTransaction proof;std::array<Int,8> values;
  for(auto& value:values)value=GameClientRandomValue(0,INT32_MAX);
  return values;
}
void producerFaults(Fixture& fixture,bool seeded){
  ProducerTimeOwner time;configureProducer(fixture);
  auto& data=fixture.module(0).inputData();
  auto& selected=data.m_conditionStates[seeded?1:0];
  selected.m_flags=1<<RANDOMIZE_START_FRAME;
  selected.m_animations.clear();for(unsigned n=0;n<3;++n)selected.m_animations.emplace_back(AsciiString("Tree.Walk"),FALSE,0);
  selected.m_animMinSpeedFactor=.5f;selected.m_animMaxSpeedFactor=1.5f;
  ModelConditionFlags moving;moving.set(MODELCONDITION_MOVING);
  auto setup=[&]{fixture.fresh();if(seeded){fixture.module(0).initializeProducer();fixture.module(0).objectScale(3);
    fixture.module(0).capture(1);auto control=NativeW3DPose::identity();control[3]=2;fixture.module(0).control(1,control);}};
  auto operation=[&]{if(seeded)fixture.module(0).replaceModelConditionState(moving);else fixture.module(0).initializeProducer();};
  auto retire=[&]{fixture.client->destroyDrawable(fixture.draw);fixture.draw=nullptr;fixture.assets.clear();};
  std::size_t terminal;
  {NameKeyTransaction keys(fixture.names);GameClientRandomTransaction cursor;setup();
    AllocationFault::arm(SIZE_MAX);try{operation();terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}
    retire();}
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){
    NameKeyTransaction keys(fixture.names);GameClientRandomTransaction cursor;setup();auto& module=fixture.module(0);
    const auto* accepted=module.currentBinding();const auto* source=module.selected();const auto* pending=module.pending();
    const auto index=module.selectedAnimation();const auto clock=module.clock()?std::optional(*module.clock()):std::nullopt;
    const auto counts=fixture.assets.counts();const auto nextID=fixture.client->getDrawableIDCounter();
    auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");const auto units=pool->getUsedBlockCount();
    NameKeyType nextKey;{NameKeyTransaction proof(fixture.names);nextKey=fixture.names.nameToKey("producer-production-cursor");}
    const auto randomBefore=probeClientCursor();Int expectedIndex,expectedFrame;Real expectedRate;
    {GameClientRandomTransaction proof;expectedIndex=GameClientRandomValue(0,2);expectedFrame=GameClientRandomValue(0,1);expectedRate=GameClientRandomValueReal(.5f,1.5f);}
    const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==(ordinal<terminal)&&failed==hit&&attempts==(failed?ordinal+1:terminal),"producer exact terminal and every failure prefix");
    if(failed){
      require(module.currentBinding()==accepted&&module.selected()==source&&module.pending()==pending&&module.selectedAnimation()==index,
        "failed producer candidate preserves accepted binding and selected/pending/index");
      require(bool(module.clock())==bool(clock)&&(!clock||(module.clock()->frame()==clock->frame()&&module.clock()->mode()==clock->mode()&&module.clock()->multiplier()==clock->multiplier())),
        "failed producer candidate preserves accepted clock");
      require(fixture.assets.counts()==counts&&AllocationFault::live()==live&&pool->getUsedBlockCount()==units,
        "failed producer candidate retires all standard and namespace pooled ownership");
      require(probeClientCursor()==randomBefore,"production client cursor preserved before fixture rollback");
      {NameKeyTransaction proof(fixture.names);require(fixture.names.nameToKey("producer-production-cursor")==nextKey,
        "production namespace preserved before fixture rollback");}
      operation();
    }
    require(module.selected()==&selected&&!module.pending()&&module.selectedAnimation()==expectedIndex&&module.clock()->frame()==expectedFrame&&module.clock()->multiplier()==expectedRate,
      "corrected producer retry retains exact source selection/frame/rate cursor");
    require(fixture.client->getDrawableIDCounter()==nextID,"producer never consumes Drawable IDs");
    retire();
  }
  fixture.create();std::cout<<"condition producer "<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every failure/retry\n";
}
void producerLifecycle(Fixture& fixture){
  ProducerTimeOwner time;configureProducer(fixture);auto& module=fixture.module(0);auto& data=module.inputData();
  data.m_conditionStates[0].m_flags=1<<START_FRAME_FIRST;
  data.m_conditionStates[1].m_flags=0;data.m_conditionStates[1].m_animations.clear();
  data.m_conditionStates[2].m_modelName="Static.Mesh";data.m_conditionStates[2].m_flags=1<<RANDOMIZE_START_FRAME;
  data.m_conditionStates[2].m_animMinSpeedFactor=.5f;data.m_conditionStates[2].m_animMaxSpeedFactor=1.5f;
  auto absent=data.m_conditionStates[0];ModelConditionFlags dying;dying.set(MODELCONDITION_DYING);
  absent.m_conditionsYesVec={dying};absent.m_modelName="None";absent.m_flags=1<<RANDOMIZE_START_FRAME;
  absent.m_animations.clear();for(unsigned n=0;n<3;++n)absent.m_animations.emplace_back(AsciiString("Tree.Walk"),FALSE,0);
  data.m_conditionStates.push_back(absent);
  auto bare=data.m_conditionStates[0];ModelConditionFlags damaged;damaged.set(MODELCONDITION_DAMAGED);
  bare.m_conditionsYesVec={damaged};bare.m_animations.clear();data.m_conditionStates.push_back(bare);
  auto unsupported=data.m_conditionStates[0];ModelConditionFlags night;night.set(MODELCONDITION_NIGHT);
  unsupported.m_conditionsYesVec={night};unsupported.m_hideShowVec.push_back({AsciiString("GeneratedPart"),TRUE});data.m_conditionStates.push_back(unsupported);
  // The fixture's parsed ignore mask would otherwise remove this negative state.
  data.m_ignoreConditionStates.clear();
  GameClientRandomTransaction randomOwner;module.initializeProducer();
  ModelConditionFlags moving;moving.set(MODELCONDITION_MOVING);ModelConditionFlags attacking;attacking.set(MODELCONDITION_ATTACKING);
  const auto* clip=module.currentBinding()->animation.get();module.setPauseAnimation(TRUE);module.objectScale(3);
  module.replaceModelConditionState(moving);
  require(module.selectedAnimation()==-1&&module.currentBinding()->animation.get()==clip&&module.clock()->mode()==NativeModelAnimationClock::Mode::Manual&&module.paused(),
    "empty animation list on reused HLOD retains current clip/clock/pause");
  std::array<Int,8> expectedLeafCursor;{GameClientRandomTransaction proof;(void)GameClientRandomValue(0,1);expectedLeafCursor=probeClientCursor();}
  module.replaceModelConditionState(attacking);
  require(module.currentBinding()->model->kind==NativeW3DAssets::Prototype::Kind::Mesh&&!module.clock()&&!module.paused()&&probeClientCursor()==expectedLeafCursor,
    "leaf recreation clears playback/pause but consumes source RANDOMSTART and no rate draw");
  Coord3D position;Real radius;Matrix3D world;
  require(module.clientOnly_getRenderObjInfo(&position,&radius,&world)&&radius==3,"recreated model starts ObjectScale1");
  auto root=Matrix3D(true);root.Set_Translation(Vector3(7,8,9));time.tick(fixture);module.producerStep(root);
  require(module.clientOnly_getRenderObjInfo(&position,&radius,&world)&&position.x==7,"non-HLOD producer still publishes current root");
  const auto* accepted=module.currentBinding();const auto counts=fixture.assets.counts();const auto before=probeClientCursor();
  clientRejects([&]{module.replaceModelConditionState(night);});
  require(module.currentBinding()==accepted&&fixture.assets.counts()==counts&&probeClientCursor()==before,
    "unsupported effects reject without claiming application or changing current owners");
  Int expectedAbsentIndex;std::array<Int,8> expectedAbsentCursor;
  {GameClientRandomTransaction proof;expectedAbsentIndex=GameClientRandomValue(0,2);expectedAbsentCursor=probeClientCursor();}
  module.replaceModelConditionState(dying);
  require(!module.currentBinding()&&module.selectedAnimation()==expectedAbsentIndex&&probeClientCursor()==expectedAbsentCursor,
    "absent model consumes record selection but neither start frame nor speed draw");
  module.replaceModelConditionState(damaged);require(module.currentBinding()&&!module.clock(),"recreated empty-animation HLOD is base pose");
  time.tick(fixture);module.producerStep(root);require(module.currentBinding()&&!module.clock(),"base-pose HLOD does not invent completion or a clock");
}
void producerNegative(Fixture& fixture){
  ProducerTimeOwner time;configureProducer(fixture);auto& module=fixture.module(0);auto& target=module.inputData().m_conditionStates[1];
  target.m_flags=1<<RANDOMIZE_START_FRAME;target.m_animations.clear();for(unsigned n=0;n<3;++n)target.m_animations.emplace_back(AsciiString("Tree.Walk"),FALSE,0);
  target.m_animMinSpeedFactor=.5f;target.m_animMaxSpeedFactor=std::numeric_limits<Real>::infinity();
  GameClientRandomTransaction randomOwner;module.initializeProducer();
  const auto* accepted=module.currentBinding();const auto* selected=module.selected();const auto before=probeClientCursor();const auto counts=fixture.assets.counts();
  ModelConditionFlags moving;moving.set(MODELCONDITION_MOVING);
  bool rateRejected=false;try{module.replaceModelConditionState(moving);}catch(const std::invalid_argument&){rateRejected=true;}
  require(rateRejected,"original random real API rejects the nonfinite factor with its defined exception type");
  require(module.currentBinding()==accepted&&module.selected()==selected&&!module.pending()&&fixture.assets.counts()==counts&&probeClientCursor()==before,
    "late invalid rate rejects after randomized index/frame while preserving whole producer and client cursor");
  target.m_animMaxSpeedFactor=1.5f;Int expectedIndex,expectedFrame;Real expectedRate;
  {GameClientRandomTransaction proof;expectedIndex=GameClientRandomValue(0,2);expectedFrame=GameClientRandomValue(0,1);expectedRate=GameClientRandomValueReal(.5f,1.5f);}
  module.replaceModelConditionState(moving);
  require(module.selectedAnimation()==expectedIndex&&module.clock()->frame()==expectedFrame&&module.clock()->multiplier()==expectedRate,"corrected late rejection retry preserves exact original RNG cursor");
  fixture.logic->phase([&]{clientRejects([&]{module.producerStep(Matrix3D(true));});});
  auto invalid=Matrix3D(true);invalid[0][0]=std::numeric_limits<Real>::infinity();
  clientRejects([&]{module.producerStep(invalid);});require(module.clock()->frame()==expectedFrame,"invalid root/phase cannot consume accepted playback");
}
void modelBounds(Fixture& fixture){auto& module=fixture.module(0);const Matrix3D identity(true);const AsciiString empty("");
  Coord3D position{91,92,93};Real radius=99;Matrix3D world(true);world.Set_Translation(Vector3(99,0,0));OBBoxClass box(Vector3(91,92,93),Vector3(99,99,99));
  module.current("","",identity,0);require(!fixture.draw->clientOnly_getFirstRenderObjInfo(&position,&radius,&world)&&position.x==91&&radius==99&&world.Get_X_Translation()==99,"absent first model leaves every info output untouched");
  require(!module.clientOnly_getRenderObjBoundBox(&box)&&box.Center.X==91,"absent bounds leave output untouched");
  module.current("NULL","",identity,0);require(fixture.draw->clientOnly_getFirstRenderObjInfo(&position,&radius,&world)&&close(radius,.1f),"source NULL has nonzero sphere radius");
  module.current("Static.Mesh","",clientRoot(),0);require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,99)&&close(box.Center.Y,201)&&box.Extent.X==1&&close(box.Basis[0][1],-1),"source object-space AABB becomes oriented world box, not world AABB");
  require(fixture.draw->clientOnly_getFirstRenderObjInfo(&position,&radius,&world)&&position.x==100&&position.y==200&&radius==3,"info returns root position/transform and source mesh sphere, not sphere center");
  fixture.draw->setInstanceScale(2);module.current("Static.Mesh","",identity,0);module.objectScale(2);
  require(module.clientOnly_getRenderObjInfo(&position,&radius,&world)&&radius==12,"geometry scale and separately published ObjectScale both affect normal sphere");
  module.current("BBox.BOUNDINGBOX","",identity,0);require(module.clientOnly_getRenderObjBoundBox(&box)&&box.Center.X==3&&box.Extent.X==2,"box inherits source no-op geometry Scale");
  fixture.draw->setInstanceScale(1);module.current("Bounded","Tree.Walk",identity,0);module.objectScale(9);
  AllocationFault::arm(SIZE_MAX);try{module.clientOnly_getRenderObjBoundBox(&box);module.clientOnly_getRenderObjInfo(&position,&radius,&world);}catch(...){AllocationFault::disarm();throw;}
  auto attempts=AllocationFault::attempts();AllocationFault::disarm();require(attempts==0&&close(box.Center.X,19)&&close(radius,std::sqrt(6.f)),"animated source BOUNDINGBOX override ignores normal ObjectScale and allocates nothing");
  module.pose(identity,1);require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,23),"invisible-independent current override follows selected pose");
  module.current("Ordinary","Tree.Walk",identity,0);module.pose(identity,1);require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,17),"aligned or differently named box uses base hierarchy bounds, not animated pose");
  module.current("Reverse","Tree.Walk",identity,0);require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,13),"reverse last-LOD lookup picks final oriented BOUNDINGBOX");
  module.current("Outer","Tree.Walk",identity,0);module.pose(identity,1);
  require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,12.95f)&&close(box.Extent.X,13.05f)&&close(box.Extent.Y,1.05f),"nested HLOD bounds include every LOD, null and aggregate in pristine hierarchy, not current animation");
  fixture.draw->setInstanceScale(2);module.current("Outer","Tree.Walk",identity,0);
  require(module.clientOnly_getRenderObjBoundBox(&box)&&close(box.Center.X,25.95f)&&close(box.Extent.X,26.05f)&&close(box.Extent.Y,2.05f),"final scaled clone recomputes nested geometry and base hierarchy bounds while null Scale remains no-op");
  fixture.draw->setInstanceScale(1);
  auto duplicate=fixture.assets.model("DuplicateBox.Shape");require(duplicate->kind==NativeW3DAssets::Prototype::Kind::AlignedBox&&duplicate->document->boxes()[duplicate->metadataIndex].center[0]==7,"typed roots preserve first duplicate and unknown flags");
  auto encoded=W3DAssetInput::box("Generated.Shape",0xf001);auto decoded=NativeW3DDocument::decode(encoded);require(decoded.boxes().size()==1&&decoded.boxes()[0].attributes==0xf001&&decoded.boxes()[0].color[3]==0xa5,"BOX fixed fields and RGB padding retained");
  auto malformed=encoded;malformed[8+56+3]=0xbf; // Defined negative extent float bits.
  clientRejects([&]{decoded.replace(malformed);});require(decoded.boxes()[0].extent[0]==2,"malformed box replacement preserves accepted metadata");
  malformed=encoded;std::fill(malformed.begin()+16,malformed.begin()+48,'X');clientRejects([&]{decoded.replace(malformed);});
  malformed=encoded;malformed.resize(malformed.size()-1);clientRejects([&]{decoded.replace(malformed);});
  Data extended(encoded.begin()+8,encoded.end());extended.push_back(0xab);decoded.replace(chunk(0x740,extended));require(decoded.boxes()[0].extent[0]==2,"source fixed box read permits retained trailing chunk bytes");
  fixture.logic->phase([&]{clientRejects([&]{module.clientOnly_getRenderObjInfo(&position,&radius,&world);});});
  clientRejects([&]{module.clientOnly_getRenderObjBoundBox(nullptr);});clientRejects([&]{module.objectScale(std::numeric_limits<Real>::infinity());});
  module.current("Static.Mesh","",identity,0);module.objectScale(std::numeric_limits<Real>::max());position={91,92,93};radius=99;world.Set_Translation(Vector3(99,0,0));
  clientRejects([&]{fixture.draw->clientOnly_getFirstRenderObjInfo(&position,&radius,&world);});require(position.x==91&&radius==99&&world.Get_X_Translation()==99,"late radius overflow preserves all source info outputs");
  module.objectScale(1);require(module.clientOnly_getRenderObjInfo(&position,&radius,&world)&&radius==3,"corrected model info retry");
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"Drawable query family");initMemoryManager();initialized=true;
  {Fixture warm;functional(warm);negative(warm);clientFunctional(warm);clientNegative(warm);clientNumbered(warm);animationClock();animationPose(warm);displayTime(warm);modelBounds(warm);}
  if(std::string_view(argv[1]).starts_with("producer-")){
    {Fixture warm;producerFunctional(warm);}
    {Fixture warm;producerTransitions(warm);}
    {Fixture warm;producerSelection(warm);}
    {Fixture warm;producerLifecycle(warm);}
    {Fixture warm;producerNegative(warm);}
  }
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
    {Fixture fixture;const std::string_view family=argv[1];if(family=="functional")functional(fixture);else if(family=="negative")negative(fixture);else if(family=="faults")faults(fixture,false);else if(family=="seeded_faults")faults(fixture,true);else if(family=="wide_faults")faults(fixture,false,true);else if(family=="wide_seeded_faults")faults(fixture,true,true);else if(family=="constructor_faults")constructorFaults(fixture);else if(family=="client-functional")clientFunctional(fixture);else if(family=="client-negative")clientNegative(fixture);else if(family=="client-numbered")clientNumbered(fixture);else if(family=="client-cold")clientFaults(fixture,false);else if(family=="client-seeded")clientFaults(fixture,true);else if(family=="animation-clock")animationClock();else if(family=="animation-pose")animationPose(fixture);else if(family=="display-time")displayTime(fixture);else if(family=="model-bounds")modelBounds(fixture);else if(family=="producer-functional")producerFunctional(fixture);else if(family=="producer-transitions")producerTransitions(fixture);else if(family=="producer-selection")producerSelection(fixture);else if(family=="producer-cold")producerFaults(fixture,false);else if(family=="producer-seeded")producerFaults(fixture,true);else if(family=="producer-lifecycle")producerLifecycle(fixture);else if(family=="producer-negative")producerNegative(fixture);else throw GeneratedFixtureFailure("unknown Drawable query family");}
    require(AllocationFault::live()==live,"whole original Drawable/client/definition/query/parent cohort retirement");
  }shutdownMemoryManager();initialized=false;std::cout<<"PASS actual Drawable query dispatch; renderer/full simulation pending\n";return 0;
 }catch(const GeneratedFixtureFailure& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(const std::exception&){std::cerr<<"FAIL generated Drawable query\n";}catch(ErrorCode){std::cerr<<"FAIL source Drawable query\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
