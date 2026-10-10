// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "W3DAssetInput.h"
#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/NativeParticleSystemAdmission.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/DrawGroupInfo.h"
#include <array>
#include <atomic>
#include <thread>

namespace {
using namespace W3DAssetInput;
Data bytes(const std::string& text){return {text.begin(),text.end()};}
std::string definition(const char* name,const char* slave="",const char* delay="0"){
  std::string out="ParticleSystem ";out+=name;out+="\nShader = ADDITIVE\nType = PARTICLE\nParticleName = generated.dds\nVelocityType = ORTHO\nVolumeType = LINE\nVolLineStart = X:0 Y:0 Z:0\nVolLineEnd = X:0 Y:0 Z:0\n";
  for(const char* field:{"AngleZ","AngularRateZ","AngularDamping","VelocityDamping","Lifetime","Size","StartSizeRate","SizeRate","SizeRateDamping","ColorScale","BurstDelay","BurstCount","VelOrthoX","VelOrthoY","VelOrthoZ"})out+=std::string(field)+" = 0 0 CONSTANT\n";
  out+="InitialDelay = ";out+=delay;out+=' ';out+=delay;out+=" CONSTANT\n";
  for(unsigned i=1;i<=8;++i){out+="Alpha"+std::to_string(i)+" = 0 0 0\n";out+="Color"+std::to_string(i)+" = R:0 G:0 B:0 0\n";}
  if(*slave)out+=std::string("SlaveSystem = ")+slave+'\n';out+="End\n";return out;
}
std::vector<std::pair<std::string,Data>> inputs(){auto text=
  definition("Primary","Child")+definition("Child","Leaf")+definition("Leaf")+
  definition("Missing","Absent")+definition("CycleA","CycleB")+definition("CycleB","CycleA")+
  definition("Negative","","-1")+definition("Overflow","","4294967296");
  for(unsigned i=0;i<16;++i){const auto name="Long"+std::to_string(i),next=i+1<16?"Long"+std::to_string(i+1):"";text+=definition(name.c_str(),next.c_str());}
  return {{"Data/INI/ParticleSystem.ini",bytes(text)}};}
struct Manager:ParticleSystemManager {
  Int getOnScreenParticleCount()override{return m_onScreenParticleCount;}
  void doParticles(RenderInfoClass&)override{throw ERROR_BAD_ARG;}
  void queueParticleRender()override{throw ERROR_BAD_ARG;}
  ParticleSystemID cursor()const{return m_uniqueSystemID;}
};
struct Fixture {
  Context input{false,inputs()};NameKeyGenerator names;
  FileSystem* previousFiles=TheFileSystem;NameKeyGenerator* previousNames=TheNameKeyGenerator;
  ParticleSystemManager* previousManager=TheParticleSystemManager;GameClient* previousClient=TheGameClient;
  std::unique_ptr<Manager> manager;std::unique_ptr<NativeHeadlessGameClient> client;
  Fixture(){names.init();TheFileSystem=&input.files;TheNameKeyGenerator=&names;
    try{client=std::make_unique<NativeHeadlessGameClient>();TheGameClient=client.get();
      manager=std::make_unique<Manager>();TheParticleSystemManager=manager.get();manager->init();
    }catch(...){retire();throw;}}
  void retire()noexcept{TheParticleSystemManager=manager.get();manager.reset();TheParticleSystemManager=previousManager;
    TheGameClient=client.get();client.reset();TheGameClient=previousClient;TheFileSystem=previousFiles;TheNameKeyGenerator=previousNames;}
  ~Fixture(){retire();}
  const ParticleSystemTemplate* source(const char* name){const AsciiString parsed(name);auto* value=manager->findTemplate(parsed);require(value,"actual parsed source particle template");return value;}
};
std::array<int,3> cursor(){GameClientRandomTransaction probe;return {GameClientRandomValue(0,INT32_MAX),GameClientRandomValue(0,INT32_MAX),GameClientRandomValue(0,INT32_MAX)};}
struct BorrowedClientReplacement {
  GameClient* previousClient=TheGameClient;DrawGroupInfo* previousGroup=TheDrawGroupInfo;
  std::unique_ptr<NativeHeadlessGameClient> value;
  BorrowedClientReplacement(){TheDrawGroupInfo=nullptr;
    try{value=std::make_unique<NativeHeadlessGameClient>();TheGameClient=value.get();}
    catch(...){TheGameClient=previousClient;TheDrawGroupInfo=previousGroup;throw;}}
  ~BorrowedClientReplacement(){TheGameClient=value.get();value.reset();TheGameClient=previousClient;TheDrawGroupInfo=previousGroup;}
};
template<class F>void rejects(F action){bool failed=false;try{action();}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}require(failed,"defined particle admission rejection");}
ParticleSystem* create(Fixture& fixture,unsigned route){
  const auto* source=fixture.source("Primary");
  if(route==1)return source->createSlaveSystem();
  return fixture.manager->createParticleSystem(source,route!=2);
}
void verify(Fixture& fixture,ParticleSystem* primary,unsigned route,ParticleSystemID prior){
  const unsigned count=route==0?3:route==1?2:1;
  require(primary&&fixture.manager->getParticleSystemCount()==count&&fixture.manager->getAllParticleSystems().size()==count,"complete actual particle chain registration");
  auto* current=primary;for(unsigned i=0;i<count;++i){require(current&&current->getSystemID()==ParticleSystemID(UnsignedInt(prior)+i+1)&&current->getStartFrame()==fixture.client->getFrame(),"parent-first source IDs and frame identity");
    if(i+1<count)require(current->getSlave()->getMaster()==current,"source reciprocal master/slave links");else require(!current->getSlave(),"terminal source slave");current=current->getSlave();}
  require(fixture.manager->getAllParticleSystems().back()==primary,"source child-first registry order");
}
void functional(Fixture& fixture){GameClientRandomTransaction lease;
  for(unsigned route=0;route<3;++route){const auto prior=fixture.manager->cursor();std::array<Real,3> expectedWind{};
    {GameClientRandomTransaction expected;for(unsigned i=0;i<(route==0?3:route==1?2:1);++i){const Real start=GameClientRandomValueReal(0,PI/4),end=GameClientRandomValueReal(TWO_PI-PI/4,TWO_PI);expectedWind[i]=GameClientRandomValueReal(start,end);}}
    auto* system=create(fixture,route);verify(fixture,system,route,prior);unsigned index=0;
    for(auto* node=system;node;node=node->getSlave())require(node->getWindAngle()==expectedWind[index++],"exact parent-first source wind RNG draws");
    system->setSaveable(FALSE);for(auto* node=system;node;node=node->getSlave())require(!node->isSaveable(),"source saveability propagates to complete chain");
    fixture.manager->destroyParticleSystemByID(system->getSystemID());require(fixture.manager->getParticleSystemCount()==index,"accepted destroy defers list retirement");
    for(auto* node=system;node;node=node->getSlave())require(node->isDestroyed(),"accepted destroy marks all source slaves");fixture.manager->reset();
  }
  const auto* source=fixture.source("Missing");auto* missing=fixture.manager->createParticleSystem(source);require(missing&&!missing->getSlave(),"missing named slave retains source leaf admission");fixture.manager->reset();
  auto* longChain=fixture.manager->createParticleSystem(fixture.source("Long0"));require(fixture.manager->getParticleSystemCount()==16,"iterative chain has no small arbitrary depth cap");
  longChain->setSaveable(FALSE);longChain->destroy();for(auto* node=longChain;node;node=node->getSlave())require(node->isDestroyed()&&!node->isSaveable(),"iterative accepted-chain propagation preserves source semantics");fixture.manager->reset();
  AllocationFault::arm(0);auto* absent=fixture.manager->createParticleSystem(nullptr);const auto attempts=AllocationFault::attempts();AllocationFault::disarm();require(!absent&&attempts==0,"null source preserves exact zero standard demand");
}
void negative(Fixture& fixture){GameClientRandomTransaction lease;
  for(const char* name:{"CycleA","Negative","Overflow"}){const auto* source=fixture.source(name);const auto before=cursor();const auto next=fixture.manager->cursor();const auto live=AllocationFault::live();
    rejects([&]{fixture.manager->createParticleSystem(source);});require(cursor()==before&&fixture.manager->cursor()==next&&fixture.manager->getParticleSystemCount()==0&&AllocationFault::live()==live,"cycle/unsigned-delay rejection restores complete production cursor and ownership");}
  const auto* source=fixture.source("Primary");auto* owner=TheParticleSystemManager;TheParticleSystemManager=nullptr;
  rejects([&]{fixture.manager->createParticleSystem(source);});TheParticleSystemManager=owner;
  auto* client=TheGameClient;TheGameClient=nullptr;rejects([&]{fixture.manager->createParticleSystem(source);});TheGameClient=client;
  {NativeParticleSystemAdmission outer(*fixture.manager);
    {BorrowedClientReplacement replacement;rejects([&]{fixture.manager->createParticleSystem(source);});}outer.commit();}
  std::atomic<bool> workerRejected=false;std::thread worker([&]{try{fixture.manager->createParticleSystem(source);}catch(ErrorCode error){workerRejected=error==ERROR_BAD_ARG;}});worker.join();require(workerRejected,"worker rejected before source registry or cursor access");
  const auto next=fixture.manager->cursor();const auto before=cursor();const auto live=AllocationFault::live();
  {NativeParticleSystemAdmission outer(*fixture.manager);(void)create(fixture,0);}
  require(fixture.manager->getParticleSystemCount()==0&&fixture.manager->cursor()==next&&cursor()==before&&AllocationFault::live()==live,"uncommitted outer tracker admission immediately retires nested committed systems");
  {NativeParticleSystemAdmission outer(*fixture.manager);auto* primary=create(fixture,0);verify(fixture,primary,0,next);outer.commit();}fixture.manager->reset();
  // Keep a nested savepoint's late failure distinct from aborting the root.
  // Committing a root after a caught nested failure must not admit a prefix.
  std::size_t demand;
  {NativeParticleSystemAdmission outer(*fixture.manager);AllocationFault::arm(SIZE_MAX);
    try{(void)create(fixture,0);}catch(...){AllocationFault::disarm();throw;}demand=AllocationFault::attempts();AllocationFault::disarm();}
  const auto acceptedID=fixture.manager->cursor();const auto acceptedCursor=cursor();const auto acceptedLive=AllocationFault::live();
  {NativeParticleSystemAdmission outer(*fixture.manager);bool failed=false;AllocationFault::arm(demand-1);
    try{(void)create(fixture,0);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();require(failed&&fixture.manager->getParticleSystemCount()==0&&fixture.manager->cursor()==acceptedID&&cursor()==acceptedCursor,"caught late nested admission failure rolls back before outer commit");outer.commit();}
  require(AllocationFault::live()==acceptedLive,"caught nested failure retires outer ledger backing exactly");
  auto* retry=create(fixture,0);verify(fixture,retry,0,acceptedID);fixture.manager->reset();
}
void poolFailures(Fixture& fixture){GameClientRandomTransaction lease;
  auto* accepted=create(fixture,2);auto* pool=TheMemoryPoolFactory->findMemoryPool("ParticleSystemPool");require(pool,"source system pool before typed exhaustion");
  for(unsigned available=0;available<3;++available){std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
    struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* value:held)pool->freeBlock(value);}} retire{pool,held};
    while(pool->getFreeBlockCount()>Int(available))held.push_back(pool->allocateBlock("generated particle chain exhaustion"));
    const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();const auto next=fixture.manager->cursor();const auto before=cursor();bool failed=false;
    try{(void)create(fixture,0);}catch(ErrorCode error){failed=error==ERROR_OUT_OF_MEMORY;}
    require(failed&&fixture.manager->getParticleSystemCount()==1&&fixture.manager->getAllParticleSystems().front()==accepted&&fixture.manager->cursor()==next&&cursor()==before&&pool->getUsedBlockCount()==units&&AllocationFault::live()==live,"each partial typed-chain exhaustion preserves accepted owner and retires every candidate");
    for(auto* value:held)pool->freeBlock(value);held.clear();
    {NativeParticleSystemAdmission scope(*fixture.manager);(void)create(fixture,0);}
    require(fixture.manager->getParticleSystemCount()==1&&fixture.manager->getAllParticleSystems().front()==accepted&&fixture.manager->cursor()==next,"corrected typed pool retry retires under original accepted owner");
  }
  std::cout<<"particle chain pooled prefixes [0,3); terminal 3; every failure/retry\n";
}
void faults(unsigned route){std::size_t terminal;
  {Fixture measured;GameClientRandomTransaction lease;AllocationFault::arm(SIZE_MAX);try{(void)create(measured,route);}catch(...){AllocationFault::disarm();throw;}terminal=AllocationFault::attempts();AllocationFault::disarm();}
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){Fixture fixture;GameClientRandomTransaction lease;const auto before=cursor();const auto next=fixture.manager->cursor();
    auto* pool=TheMemoryPoolFactory->findMemoryPool("ParticleSystemPool");require(pool,"actual source particle system pool");const auto units=pool->getUsedBlockCount();
    const auto* source=fixture.source("Primary");(void)source;const auto live=AllocationFault::live();ParticleSystem* candidate=nullptr;bool failed=false;
    AllocationFault::arm(ordinal);try{candidate=create(fixture,route);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(failed==(ordinal<terminal)&&failed==hit&&attempts==(failed?ordinal+1:terminal),"particle chain exact standard failure prefixes and terminal");
    if(failed){require(fixture.manager->getParticleSystemCount()==0&&fixture.manager->getAllParticleSystems().empty()&&fixture.manager->cursor()==next&&cursor()==before&&AllocationFault::live()==live&&pool->getUsedBlockCount()==units,"each particle chain prefix retires list, standard/pooled ownership and original client/ID cursor");
      AllocationFault::arm(SIZE_MAX);try{candidate=create(fixture,route);}catch(...){AllocationFault::disarm();throw;}
      const auto retryDemand=AllocationFault::attempts();AllocationFault::disarm();require(retryDemand==terminal,"every corrected retry retains exact cold template-cache admission demand");}
    verify(fixture,candidate,route,next);
  }std::cout<<"particle admission route "<<route<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every failure/retry\n";
}
}
int main(int argc,char** argv){bool initialized=false;try{require(argc==2,"particle admission family");initMemoryManager();initialized=true;
  // Explicit public allocator configuration: production's growing pool cannot
  // serve as an exhaustion fixture. Keep growth tests and bounded acquisition
  // failures distinct without changing the production startup profile.
  TheMemoryPoolFactory->createMemoryPool("ParticleSystemPool",sizeof(ParticleSystem),64,0);
  {Fixture warm;functional(warm);negative(warm);poolFailures(warm);}
  for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const std::string_view family=argv[1];
    if(family=="functional"){Fixture fixture;functional(fixture);}else if(family=="negative"){Fixture fixture;negative(fixture);}else if(family=="pool-faults"){Fixture fixture;poolFailures(fixture);}else if(family=="chain-faults")faults(0);else if(family=="slave-faults")faults(1);else if(family=="leaf-faults")faults(2);else throw GeneratedFixtureFailure("unknown particle admission family");
    require(AllocationFault::live()==live,"whole parsed particle/client/manager/root cohort retirement");
  }shutdownMemoryManager();initialized=false;std::cout<<"PASS original particle chain admission; concrete model/rendering pending\n";return 0;
 }catch(const GeneratedFixtureFailure& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(const std::exception&){std::cerr<<"FAIL generated particle admission\n";}catch(ErrorCode){std::cerr<<"FAIL source particle admission\n";}
 AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
