// SPDX-License-Identifier: GPL-3.0-or-later
#include "W3DAssetInput.h"
#include "Common/W3DModelDrawData.h"
#include "Common/INIException.h"
#include "GameClient/ParticleSys.h"
#include <span>

namespace {
using namespace W3DAssetInput;
const char* valid=R"(Draw
IgnoreConditionStates = NIGHT
DefaultConditionState
  Model = Outer
  Animation = Tree.Walk 12 2
  AnimationMode = ONCE
  TransitionKey = idle
  Flags = PRISTINE_BONE_POS_IN_FINAL_FRAME
  WeaponLaunchBone = PRIMARY Muzzle
  WeaponFireFXBone = PRIMARY Muzzle
  WeaponRecoilBone = PRIMARY Arm
  Turret = Arm
  TurretArtAngle = 90
  HideSubObject = Body.Part
  ParticleSysBone = Muzzle Spark
  AnimationSpeedFactorRange = 0.8 1.2
End
ConditionState = MOVING
  Animation = Tree.Walk 6 3
  TransitionKey = moving
  HideSubObject = None
  ShowSubObject = Body.Part
End
AliasConditionState = ATTACKING
ConditionState = DAMAGED
  Model = None
  Animation = None
End
TransitionState = idle moving
  Animation = Tree.Walk
  AnimationMode = ONCE
End
ExtraPublicBone = Muzzle
TrackMarks = Example
End
)";
Data bytes(std::string_view text){return Data(text.begin(),text.end());}
std::vector<std::pair<std::string,Data>> inputs(){return {
  {"draw.ini",bytes(valid)},
  {"Data/INI/ParticleSystem.ini",bytes("ParticleSystem Spark\nEnd\n")},
  {"alias.ini",bytes("Draw\nAliasConditionState = MOVING\nEnd\n")},
  {"first.ini",bytes("Draw\nConditionState = MOVING\nModel = Outer\nEnd\nEnd\n")},
  {"default.ini",bytes("Draw\nDefaultConditionState\nModel = Outer\nEnd\nDefaultConditionState\nModel = Outer\nEnd\nEnd\n")},
  {"ignored.ini",bytes("Draw\nIgnoreConditionStates = MOVING\nDefaultConditionState\nModel = Outer\nEnd\nConditionState = MOVING\nModel = Outer\nEnd\nEnd\n")},
  {"duplicate.ini",bytes("Draw\nConditionState = NONE\nModel = Outer\nEnd\nAliasConditionState = NONE\nEnd\n")},
  {"missing.ini",bytes("Draw\nConditionState = NONE\nAnimation = Tree.Walk\nEnd\nEnd\n")},
  {"transition.ini",bytes("Draw\nDefaultConditionState\nModel = Outer\nTransitionKey = idle\nEnd\nTransitionState = idle moving\nAnimationMode = LOOP\nEnd\nEnd\n")},
  {"particle.ini",bytes("Draw\nConditionState = NONE\nModel = Outer\nParticleSysBone = Muzzle Missing\nEnd\nEnd\n")},
  {"idle.ini",bytes("Draw\nConditionState = NONE\nModel = Outer\nIdleAnimation = Tree.Walk 5\nEnd\nEnd\n")},
  {"late.ini",bytes(std::string(valid).replace(std::string(valid).rfind("End\n"),4,"UnknownField = late\nEnd\n"))}
};}
thread_local std::unique_ptr<W3DModelDrawModuleData>* destination=nullptr;
void parse(INI* ini){auto candidate=W3DModelDrawModuleData::createFromINI(ini);destination->swap(candidate);}
// Only abstract presentation methods are fixture seams. Real source template
// initialization, lookup and retirement execute on the actual manager.
struct FixtureParticles : ParticleSystemManager {
  Int getOnScreenParticleCount()override{return m_onScreenParticleCount;}
  void doParticles(RenderInfoClass&)override{throw ERROR_BAD_ARG;}
  void queueParticleRender()override{throw ERROR_BAD_ARG;}
};
struct Fixture {
  Context input{false,inputs()};
  NameKeyGenerator keys;
  FixtureParticles particles;
  std::unique_ptr<W3DModelDrawModuleData> accepted;
  FileSystem* previousFiles=TheFileSystem;
  NameKeyGenerator* previousKeys=TheNameKeyGenerator;
  ParticleSystemManager* previousParticles=TheParticleSystemManager;
  Fixture(){keys.init();TheFileSystem=&input.files;TheNameKeyGenerator=&keys;TheParticleSystemManager=&particles;destination=&accepted;
    try{particles.init();}catch(...){withdraw();throw;}}
  void withdraw()noexcept{destination=nullptr;TheFileSystem=previousFiles;TheNameKeyGenerator=previousKeys;TheParticleSystemManager=previousParticles;}
  ~Fixture(){accepted.reset();withdraw();}
  void load(const char* name="draw.ini"){const INIBlockDefinition blocks[]={{"Draw",parse}};INI ini;ini.loadBlocks(name,INI_LOAD_OVERWRITE,blocks);}
};
void verify(Fixture& context){auto& data=*context.accepted;
  require(data.m_defaultState==0 && data.m_conditionStates.size()==3 && data.m_transitionMap.size()==1,"complete original default/state/alias/transition parse");
  const auto& base=data.m_conditionStates[0];const auto& moving=data.m_conditionStates[1];const auto& absent=data.m_conditionStates[2];
  require(base.m_animations.size()==2 && moving.m_animations.size()==3 && moving.m_animations[0].getDistanceCovered()==6 && absent.m_animations.empty() && absent.m_modelName.isEmpty(),"original inherited animation replacement, weights and None");
  require(base.m_publicBones.size()==2 && base.m_particleSysBones.size()==1 && base.m_particleSysBones[0].particleSystemTemplate==context.particles.findTemplate(AsciiString("Spark")),"original public bone and real particle parent lookup");
  require(base.m_hideShowVec.size()==1 && base.m_hideShowVec[0].hide && moving.m_hideShowVec.size()==1 && !moving.m_hideShowVec[0].hide,"original inherited hide/show and None clearing");
  require(moving.m_conditionsYesVec.size()==2 && data.m_extraPublicBones.size()==1 && data.m_trackFile=="example","original alias/global draw fields");
  ModelConditionFlags bits;bits.set(MODELCONDITION_MOVING);bits.set(MODELCONDITION_NIGHT);
  require(data.findBestInfo(bits)==&moving && data.getBestModelNameForWB(bits)=="outer","real module selector removes ignored flags");
  bits.clear();bits.set(MODELCONDITION_ATTACKING);require(data.findBestInfo(bits)==&moving,"original module aliases select preceding state");
  const auto& transition=data.m_transitionMap.begin()->second;
  require(recoverSrcState(transition.m_transitionSig)==base.m_transitionKey && recoverDstState(transition.m_transitionSig)==moving.m_transitionKey && transition.m_transitionKey==NAMEKEY_INVALID,"original fixed-width transition signature and inherited key clearing");
  Assets assets(context.input.files,"English");auto binding=data.findBestInfo(bits)->prepareNativeBinding(assets,1);
  require(binding.pose.bone(2).transform[3]==20,"original INI/module selector reaches owned final-animation muzzle query");
}
void warm(Fixture& context){NameKeyTransaction lease(context.keys);context.load();verify(context);context.accepted.reset();}
struct TemplateOwner {
  ParticleSystemManager* previous=TheParticleSystemManager;
  std::unique_ptr<FixtureParticles> owner=std::make_unique<FixtureParticles>();
  TemplateOwner(){TheParticleSystemManager=owner.get();try{owner->init();}catch(...){owner.reset();TheParticleSystemManager=previous;throw;}}
  ~TemplateOwner(){owner.reset();TheParticleSystemManager=previous;}
};
void templateFaults(){AsciiString name("Candidate-template-with-owned-key"),lookup("Spark");std::size_t terminal;
  {TemplateOwner measured;AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try{(void)measured.owner->newTemplate(name);terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}}
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto outer=AllocationFault::live();
    {TemplateOwner holder;auto& owner=*holder.owner;auto* accepted=owner.findTemplate(lookup);const auto live=AllocationFault::live();
      auto* pool=TheMemoryPoolFactory->findMemoryPool("ParticleSystemTemplatePool");require(pool,"source named template pool initialized");
      const auto units=pool->getUsedBlockCount();bool failed=false;
      AllocationFault::arm(ordinal);try{(void)owner.newTemplate(name);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"exact original particle-template insertion prefixes and terminal");
      if(failed)require(!owner.findTemplate(name) && owner.findTemplate(lookup)==accepted && AllocationFault::live()==live && pool->getUsedBlockCount()==units,"failed insertion retires pooled template and candidate map node without replacing prior template");
      auto* retry=owner.newTemplate(name);require(retry && owner.findTemplate(name)==retry && owner.newTemplate(name)==retry,"same-owner corrected template retry and existing identity");
    }require(AllocationFault::live()==outer,"whole actual template manager retires registry backing");
  }std::cout<<"particle-template prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
void positive(Fixture& context){NameKeyTransaction lease(context.keys);context.load();verify(context);
  {struct Restore {NameKeyGenerator* prior;~Restore(){TheNameKeyGenerator=prior;}} restore{TheNameKeyGenerator};
    TheNameKeyGenerator=nullptr;auto blank=W3DModelDrawModuleData::createFromINI(nullptr);
    require(blank->m_defaultState==-1 && blank->m_conditionStates.empty(),"unparsed source module data does not acquire an unused name parent");}
  auto copy=*context.accepted;ModelConditionFlags bits;bits.set(MODELCONDITION_ATTACKING);
  require(copy.findBestInfo(bits)==&copy.m_conditionStates[1],"copied original module matcher resolves copied backing");context.accepted.reset();}
void negative(Fixture& context){NameKeyTransaction lease(context.keys);context.load();const auto* accepted=context.accepted.get();
  for(const char* name:{"alias.ini","first.ini","default.ini","ignored.ini","duplicate.ini","missing.ini","transition.ini","particle.ini","idle.ini","late.ini"}){
    const auto live=AllocationFault::live();bool failed=false;try{context.load(name);}catch(ErrorCode){failed=true;}catch(const INIException&){failed=true;}
    require(failed && context.accepted.get()==accepted && AllocationFault::live()==live,"whole original parse rejection retains accepted module and retires candidate");verify(context);
  }TheParticleSystemManager=nullptr;bool missingParent=false;try{context.load();}catch(ErrorCode){missingParent=true;}catch(const INIException&){missingParent=true;}
  TheParticleSystemManager=&context.particles;require(missingParent && context.accepted.get()==accepted,"particle lookup requires its actual parent before publication");context.accepted.reset();
}
void faults(Fixture& context,bool seeded=false){std::size_t terminal;
  {NameKeyTransaction lease(context.keys);if(seeded){context.load();verify(context);}AllocationFault::arm(std::numeric_limits<std::size_t>::max());try{context.load();terminal=AllocationFault::attempts();AllocationFault::disarm();}catch(...){AllocationFault::disarm();throw;}context.accepted.reset();}
  require(terminal>0 && terminal<512,"bounded whole draw-definition census");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto outer=AllocationFault::live();
    {NameKeyTransaction lease(context.keys);if(seeded){context.load();verify(context);}else context.accepted=W3DModelDrawModuleData::createFromINI(nullptr);
      const auto* retained=context.accepted.get();NameKeyType expectedNext;
      {NameKeyTransaction proof(context.keys);expectedNext=context.keys.nameToKey("cursor-proof");}
      const auto live=AllocationFault::live();const auto nameUnits=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount();bool failed=false;
      AllocationFault::arm(ordinal);try{context.load();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      const auto hit=AllocationFault::triggered();const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
      require(failed==(ordinal<terminal) && failed==hit && attempts==(failed?ordinal+1:terminal),"every draw parse allocation prefix and exact terminal");
      if(failed){require(context.accepted.get()==retained && AllocationFault::live()==live,"failed original parse preserves accepted owner and candidate backing");
        require(TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount()==nameUnits,"failed parse retires every source name pool unit");
        {NameKeyTransaction proof(context.keys);require(context.keys.nameToKey("cursor-proof")==expectedNext,"failed parse restores production name cursor before outer lease");}
        if(seeded)verify(context);}
      context.load();verify(context);context.accepted.reset();
    }require(AllocationFault::live()==outer,"whole draw parse/retry and key lease retire standard backing");
  }std::cout<<"draw-definition "<<(seeded?"seeded":"cold")<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"draw-data family");initMemoryManager();initialized=true;
  {Fixture context;warm(context);templateFaults();for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
    const std::string_view family=argv[1];if(family=="values")positive(context);else if(family=="negative")negative(context);else if(family=="faults")faults(context);else if(family=="seeded_faults")faults(context,true);else if(family=="template_faults")templateFaults();else throw std::runtime_error("unknown draw-data family");
    require(AllocationFault::live()==live,"repeated whole draw/particle/name cohort retirement");}}
  shutdownMemoryManager();initialized=false;std::cout<<"PASS original draw definition; Drawable/full startup pending\n";return 0;
 }catch(const GeneratedFixtureFailure& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(const std::exception&){std::cerr<<"FAIL original draw definition\n";}catch(ErrorCode){std::cerr<<"FAIL source draw data\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;}
