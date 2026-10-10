// SPDX-License-Identifier: GPL-3.0-or-later
// All original draw-provider parsers through actual ModuleFactory admission.
#include "PreRTS.h"
#include "W3DAssetInput.h"
#include "Common/W3DDrawProviderData.h"
#include "Common/ModuleFactory.h"
#include "Common/INIException.h"

namespace {
using namespace W3DAssetInput;
struct Factory : ModuleFactory {
  static Module* notPresented(Thing*,const ModuleData*){throw ERROR_BAD_ARG;}
  void init()override{RegistryTransaction transaction(*this);
    for(const auto& provider:nativeDrawProviderData()){
      const AsciiString name(provider.name);
      addModuleInternal(notPresented,provider.create,MODULETYPE_DRAW,name,MODULEINTERFACE_DRAW);
    }transaction.commit();
  }
};
bool model(unsigned n){return n==2 || (n>=4&&n<=6) || n==8 || (n>=10&&n<=14) || n==16;}
std::string fields(unsigned n){
  std::string result;
  if(model(n))result="DefaultConditionState\nModel = Outer\nAnimation = Tree.Walk\nFlags = PRISTINE_BONE_POS_IN_FINAL_FRAME\nWeaponLaunchBone = PRIMARY Muzzle\nEnd\nExtraPublicBone = Arm\n";
  if(n==3)result="NumBeams = 3\nInnerBeamWidth = 2\nOuterBeamWidth = 5\nTexture = GeneratedBeam\nMaxIntensityLifetime = 1000\nFadeLifetime = 500\nTile = Yes\nSegments = 4\n";
  if(n==7)result="Texture = GeneratedStream\nWidth = 2\nTileFactor = 3\nScrollRate = 4\nMaxSegments = 7\n";
  if(n==4||n==13||n==16)result+="TreadAnimationRate = 30\nTreadDriveSpeedFraction = 0.4\n";
  if(n==5||n==8||n==14||n==16)result+="Dust = GeneratedDust\nLeftFrontTireBone = Arm\nTireRotationMultiplier = 2\n";
  if(n==11)result+="SupplyBonePrefix = GeneratedSupply\n";
  if(n==12)result+="AttachToBoneInContainer = Arm\n";
  if(n==17)result="ModelName = Outer\nTextureName = GeneratedTree\nMoveOutwardTime = 1000\nInitialVelocityPercent = 25%\nDoTopple = Yes\nSinkTime = 2000\n";
  if(n==18)result="ModelName = Outer\n";
  return result;
}
std::string filename(unsigned n,bool bad=false){return std::string(bad?"bad-":"valid-")+std::to_string(n)+".ini";}
std::vector<std::pair<std::string,Data>> inputs(){std::vector<std::pair<std::string,Data>> values;
  for(unsigned n=0;n<19;++n)for(bool bad:{false,true}){
    const auto text="Draw\n"+fields(n)+(bad?"UnknownGeneratedField = late\n":"")+"End\n";
    values.emplace_back(filename(n,bad),Data(text.begin(),text.end()));
  }return values;
}
Factory* parserFactory=nullptr;
const AsciiString* parserName=nullptr;
const AsciiString* parserTag=nullptr;
const ModuleData* published=nullptr;
void parse(INI* ini){published=parserFactory->newModuleDataFromINI(ini,*parserName,MODULETYPE_DRAW,*parserTag);}
struct ProviderContext {
  FileSystem* previousFiles=TheFileSystem;
  NameKeyGenerator* previousNames=TheNameKeyGenerator;
  W3DAssetInput::Context input{false,inputs()};
  NameKeyGenerator names;
  ProviderContext(){names.init();TheFileSystem=&input.files;TheNameKeyGenerator=&names;}
  ~ProviderContext(){parserFactory=nullptr;parserName=nullptr;parserTag=nullptr;published=nullptr;TheFileSystem=previousFiles;TheNameKeyGenerator=previousNames;}
};
void load(Factory& factory,const AsciiString& name,const AsciiString& tag,unsigned n,bool bad=false){
  const auto file=filename(n,bad);
  parserFactory=&factory;parserName=&name;parserTag=&tag;
  INI ini;const INIBlockDefinition blocks[]={{"Draw",parse}};
  ini.loadBlocks(file.c_str(),INI_LOAD_OVERWRITE,blocks);
}
void check(unsigned n,const ModuleData& data){
  require(data.getModuleTagNameKey()!=NAMEKEY_INVALID,"original factory publishes parsed tag");
  if(model(n)){const auto* definition=data.getAsW3DModelDrawModuleData();require(definition,"exact inherited model definition");
    const auto* condition=definition->findBestInfoUncached(ModelConditionFlags());
    require(condition && condition->m_modelName=="outer","original inherited lowercase model field parsing");
  }else require(!data.getAsW3DModelDrawModuleData(),"independent data is not collapsed into model data");
  if(n==3){const auto* p=dynamic_cast<const W3DLaserDrawModuleData*>(&data);require(p&&p->m_numBeams==3&&p->m_innerBeamWidth==2&&p->m_segments==4&&p->m_maxIntensityFrames==30,"source laser conversions");}
  if(n==7){const auto* p=dynamic_cast<const W3DProjectileStreamDrawModuleData*>(&data);require(p&&p->m_width==2&&p->m_maxSegments==7,"source stream fields");}
  if(n==4||n==13){const auto* p=dynamic_cast<const W3DTankDrawModuleData*>(&data);require(p&&p->m_treadAnimationRate==1&&p->m_treadDriveSpeedFraction==.4f,"source tank inherited fields");}
  if(n==5||n==8||n==14){const auto* p=dynamic_cast<const W3DTruckDrawModuleData*>(&data);require(p&&p->m_dustEffectName=="GeneratedDust"&&p->m_rotationSpeedMultiplier==2,"source truck/PoliceCar inheritance");}
  if(n==16){const auto* p=dynamic_cast<const W3DTankTruckDrawModuleData*>(&data);require(p&&p->m_treadAnimationRate==1&&p->m_rotationSpeedMultiplier==2,"source combined tank/truck fields");}
  if(n==10){const auto* p=dynamic_cast<const W3DScienceModelDrawModuleData*>(&data);require(p&&p->m_requiredScience==SCIENCE_INVALID,"original science default");}
  if(n==11){const auto* p=dynamic_cast<const W3DSupplyDrawModuleData*>(&data);require(p&&p->m_supplyBonePrefix=="GeneratedSupply","source supply data");}
  if(n==12){const auto* p=dynamic_cast<const W3DDependencyModelDrawModuleData*>(&data);require(p&&p->m_attachToDrawableBoneInContainer=="Arm","source dependency attachment");}
  if(n==17){const auto* p=dynamic_cast<const W3DTreeDrawModuleData*>(&data);require(p&&p->m_framesToMoveOutward==30&&p->m_initialVelocityPercent==.25f&&p->m_sinkFrames==60&&p->m_doTopple,"source tree conversions");}
  if(n==18){const auto* p=dynamic_cast<const W3DPropDrawModuleData*>(&data);require(p&&p->m_modelName=="Outer","source prop data");}
}
void functional(ProviderContext& context){Factory factory;factory.init();require(nativeDrawProviderData().size()==19,"complete source provider census");
  const char* expected[]={"W3DDefaultDraw","W3DDebrisDraw","W3DModelDraw","W3DLaserDraw","W3DOverlordTankDraw","W3DOverlordTruckDraw","W3DOverlordAircraftDraw","W3DProjectileStreamDraw","W3DPoliceCarDraw","W3DRopeDraw","W3DScienceModelDraw","W3DSupplyDraw","W3DDependencyModelDraw","W3DTankDraw","W3DTruckDraw","W3DTracerDraw","W3DTankTruckDraw","W3DTreeDraw","W3DPropDraw"};
  Assets assets(context.input.files,"English");
  for(unsigned n=0;n<19;++n){require(std::strcmp(nativeDrawProviderData()[n].name,expected[n])==0,"exact source provider names and registration order");
    std::unique_ptr<ModuleData> defaults(nativeDrawProviderData()[n].create(nullptr));
    require(defaults->getModuleTagNameKey()==NAMEKEY_INVALID,"original unparsed provider tag sentinel");
    if(n==3){const auto* p=dynamic_cast<const W3DLaserDrawModuleData*>(defaults.get());require(p&&p->m_numBeams==1&&p->m_outerBeamWidth==1&&p->m_segments==1&&p->m_tilingScalar==1,"original laser defaults");}
    if(n==13){const auto* p=dynamic_cast<const W3DTankDrawModuleData*>(defaults.get());require(p&&p->m_treadAnimationRate==0&&p->m_treadDebrisNameLeft=="TrackDebrisDirtLeft","original tank defaults");}
    if(n==17){const auto* p=dynamic_cast<const W3DTreeDrawModuleData*>(defaults.get());require(p&&p->m_framesToMoveOutward==1&&p->m_sinkFrames==300&&p->m_sinkDistance==20&&!p->m_doTopple,"original tree defaults");}
    const AsciiString name(nativeDrawProviderData()[n].name),tag("GeneratedProvider");load(factory,name,tag,n);check(n,*published);
    if(model(n)){const auto* data=published->getAsW3DModelDrawModuleData();const auto* condition=data->findBestInfoUncached(ModelConditionFlags());
      const auto query=condition->prepareNativeQueries(assets,1,data->m_extraPublicBones,{});Coord3D point;
      require(query.condition->findPristineBonePos(NAMEKEY("muzzle"),point)&&point.x==20,"derived parsed data reaches real original pristine consumer");
    }
  }
}
void negative(ProviderContext& context){Factory factory;factory.init();
  {NameKeyGenerator* previous=TheNameKeyGenerator;TheNameKeyGenerator=nullptr;
    try{for(const auto& provider:nativeDrawProviderData()){std::unique_ptr<ModuleData> defaults(provider.create(nullptr));require(defaults!=nullptr,"unparsed creation needs no namespace parent");}}
    catch(...){TheNameKeyGenerator=previous;throw;}TheNameKeyGenerator=previous;
  }
  for(unsigned n=0;n<19;++n){const AsciiString name(nativeDrawProviderData()[n].name),tag("GeneratedProvider");load(factory,name,tag,n);const auto* accepted=published;
    const auto live=AllocationFault::live();auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");const auto units=pool->getUsedBlockCount();
    NameKeyType cursor;{NameKeyTransaction proof(context.names);cursor=context.names.nameToKey("provider-production-cursor");}
    bool rejected=false;try{load(factory,name,tag,n,true);}catch(const INIException&){rejected=true;}catch(ErrorCode){rejected=true;}
    require(rejected&&published==accepted&&AllocationFault::live()==live&&pool->getUsedBlockCount()==units,"late derived parser rejection preserves accepted factory owners");
    {NameKeyTransaction proof(context.names);require(context.names.nameToKey("provider-production-cursor")==cursor,"production namespace rollback before fixture rollback");}
    load(factory,name,tag,n);check(n,*published);
  }
}
void faults(ProviderContext& context,unsigned first,unsigned last,bool seeded){
  for(unsigned n=first;n<last;++n){const AsciiString name(nativeDrawProviderData()[n].name),tag("GeneratedProvider");
    std::size_t terminal=0;
    {NameKeyTransaction lease(context.names);Factory factory;factory.init();if(seeded)load(factory,name,tag,n);
      AllocationFault::arm(SIZE_MAX);try{load(factory,name,tag,n);terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}
    }
    for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){NameKeyTransaction lease(context.names);Factory factory;factory.init();published=nullptr;if(seeded)load(factory,name,tag,n);const auto* accepted=published;
      const auto live=AllocationFault::live();auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");const auto units=pool->getUsedBlockCount();NameKeyType cursor;
      {NameKeyTransaction proof(context.names);cursor=context.names.nameToKey("provider-production-cursor");}
      bool failed=false;AllocationFault::arm(ordinal);try{load(factory,name,tag,n);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal)&&failed==hit&&attempts==(failed?ordinal+1:terminal),"exact derived factory prefix terminal");
      if(failed){require(published==accepted&&AllocationFault::live()==live&&pool->getUsedBlockCount()==units,"late failed provider candidate and factory backing retire exactly");
        {NameKeyTransaction proof(context.names);require(context.names.nameToKey("provider-production-cursor")==cursor,"production cursor rolls back before fixture lease");}
      }load(factory,name,tag,n);check(n,*published);
    }
    std::cout<<"provider "<<n<<' '<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
  }
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"provider data family");initMemoryManager();initialized=true;
  {ProviderContext warm;functional(warm);negative(warm);}
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();{ProviderContext context;const std::string_view family=argv[1];
    if(family=="functional")functional(context);else if(family=="negative")negative(context);
    else if(family=="cold-base")faults(context,0,4,false);else if(family=="cold-vehicles")faults(context,4,10,false);else if(family=="cold-special")faults(context,10,19,false);
    else if(family=="seeded-base")faults(context,0,4,true);else if(family=="seeded-vehicles")faults(context,4,10,true);else if(family=="seeded-special")faults(context,10,19,true);else throw GeneratedFixtureFailure("unknown provider family");
  }require(AllocationFault::live()==live,"complete provider/registry/data/namespace/VFS owner retirement");}
  shutdownMemoryManager();initialized=false;std::cout<<"PASS all original draw-provider data; concrete runtime modules pending\n";return 0;
 }catch(const GeneratedFixtureFailure& e){std::cerr<<"FAIL "<<e.what()<<'\n';}catch(const std::exception&){std::cerr<<"FAIL generated draw-provider data\n";}catch(ErrorCode){std::cerr<<"FAIL source draw-provider data\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
