// SPDX-License-Identifier: GPL-3.0-or-later
// Actual registered Drawable query dispatch; presentation is explicitly absent.
#include "PreRTS.h"
#include "W3DAssetInput.h"
#include "GameClient/NativeModelDrawQueries.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/Drawable.h"
#include "Common/ModuleFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "GameLogic/GameLogic.h"
#include <cfenv>

namespace {
using namespace W3DAssetInput;
Assets* fixtureAssets=nullptr;
Assets& selectedAssets(){if(!fixtureAssets)throw ERROR_BAD_ARG;return *fixtureAssets;}
struct QueryModule : NativeModelDrawQueries {
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(QueryModule,"GeneratedNativeQueryModule",8,0)
public:
  QueryModule(Thing* thing,const ModuleData* data):NativeModelDrawQueries(thing,data,selectedAssets()){}
  static Module* create(Thing* thing,const ModuleData* data){return newInstance(QueryModule)(thing,data);}
  static ModuleData* createData(INI* ini){return W3DModelDrawModuleData::createFromINI(ini).release();}
  NameKeyType getModuleNameKey()const override{static const StaticNameKey name("W3DModelDraw");return name.key();}
  const ModelConditionInfo* cached()const{return nativeQueryData();}
  Bool clientOnly_getRenderObjInfo(Coord3D*,Real*,Matrix3D*)const override{throw ERROR_BAD_ARG;}
  Bool clientOnly_getRenderObjBoundBox(OBBoxClass*)const override{throw ERROR_BAD_ARG;}
  Bool clientOnly_getRenderObjBoneTransform(const AsciiString&,Matrix3D*)const override{throw ERROR_BAD_ARG;}
  Int getCurrentBonePositions(const char*,Int,Coord3D*,Matrix3D*,Int)const override{throw ERROR_BAD_ARG;}
  Bool getCurrentWorldspaceClientBonePositions(const char*,Matrix3D&)const override{throw ERROR_BAD_ARG;}
  void updateProjectileClipStatus(UnsignedInt,UnsignedInt,WeaponSlotType)override{throw ERROR_BAD_ARG;}
  void updateDrawModuleSupplyStatus(Int,Int)override{throw ERROR_BAD_ARG;}
  void notifyDrawModuleDependencyCleared()override{throw ERROR_BAD_ARG;}
  void setHidden(Bool)override{} void replaceModelConditionState(const ModelConditionFlags&)override{}
  void replaceIndicatorColor(Color)override{} void setSelectable(Bool)override{}
  Bool handleWeaponFireFX(WeaponSlotType,Int,const FXList*,Real,const Coord3D*,Real)override{throw ERROR_BAD_ARG;}
  Int getBarrelCount(WeaponSlotType)const override{throw ERROR_BAD_ARG;}
  void setAnimationLoopDuration(UnsignedInt)override{throw ERROR_BAD_ARG;}
  void setAnimationCompletionTime(UnsignedInt)override{throw ERROR_BAD_ARG;}
  Bool updateBonesForClientParticleSystems()override{throw ERROR_BAD_ARG;}
  void setAnimationFrame(int)override{throw ERROR_BAD_ARG;}
  void setPauseAnimation(Bool)override{throw ERROR_BAD_ARG;}
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
std::vector<std::pair<std::string,Data>> inputs(){
  const std::string parent="Draw\nIgnoreConditionStates = NIGHT\nDefaultConditionState\nModel = Outer\nAnimation = Tree.Walk\nFlags = PRISTINE_BONE_POS_IN_FINAL_FRAME\nEnd\nConditionState = MOVING\nFlags = None\nEnd\nAliasConditionState = ATTACKING\nExtraPublicBone = Arm\nExtraPublicBone = Muzzle\nEnd\n";
  const std::string child="Draw\nDefaultConditionState\nModel = Outer\nAnimation = Tree.Walk\nFlags = PRISTINE_BONE_POS_IN_FINAL_FRAME\nWeaponFireFXBone = PRIMARY Muzzle\nWeaponLaunchBone = PRIMARY Muzzle\nWeaponRecoilBone = PRIMARY Arm\nTurret = Arm\nTurretArtAngle = 90\nEnd\nConditionState = MOVING\nFlags = None\nEnd\nAliasConditionState = ATTACKING\nAttachToBoneInAnotherModule = Arm\nEnd\n";
  return {{"parent.ini",bytes(parent)},{"child.ini",bytes(child)},{"definition.ini",bytes("Definition\nScale = 1\nEnd\n")}};
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
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"Drawable query family");initMemoryManager();initialized=true;
  {Fixture warm;functional(warm);negative(warm);}
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
    {Fixture fixture;const std::string_view family=argv[1];if(family=="functional")functional(fixture);else if(family=="negative")negative(fixture);else if(family=="faults")faults(fixture,false);else if(family=="seeded_faults")faults(fixture,true);else if(family=="wide_faults")faults(fixture,false,true);else if(family=="wide_seeded_faults")faults(fixture,true,true);else if(family=="constructor_faults")constructorFaults(fixture);else throw GeneratedFixtureFailure("unknown Drawable query family");}
    require(AllocationFault::live()==live,"whole original Drawable/client/definition/query/parent cohort retirement");
  }shutdownMemoryManager();initialized=false;std::cout<<"PASS actual Drawable query dispatch; renderer/full simulation pending\n";return 0;
 }catch(const GeneratedFixtureFailure& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(const std::exception&){std::cerr<<"FAIL generated Drawable query\n";}catch(ErrorCode){std::cerr<<"FAIL source Drawable query\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
