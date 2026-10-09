// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original LOD/INI/storage owner; generated probes test legacy profile
// rules, not native calibration, renderer performance or gameplay acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/GameLOD.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/NativeUserStorage.h"
#include "Common/INIException.h"
#include "Common/crc.h"
#include "GameClient/ParticleSys.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>
#include <limits>
#include <dirent.h>
#include <unistd.h>
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F action){bool failed=false;try{action();}catch(ErrorCode){failed=true;}
  catch(const INIException&){failed=true;}catch(const NativeLODCalibrationUnavailable&){failed=true;}
  require(failed,"invalid original LOD admission rejects");}
unsigned descriptors(){DIR* directory=::opendir("/proc/self/fd");require(directory,"descriptor census");unsigned count=0;
  while(::readdir(directory))++count;::closedir(directory);return count;}
struct Tree {
  std::filesystem::path root;
  Tree(){char name[]="/tmp/zh-lod-XXXXXX";const char* result=::mkdtemp(name);require(result,"generated tree");root=result;}
  ~Tree(){std::error_code ignored;std::filesystem::remove_all(root,ignored);}
};
const char* definitions="StaticGameLOD Low\nTextureReductionFactor = 2\nEnd\n"
  "StaticGameLOD Medium\nTextureReductionFactor = 1\nEnd\n"
  "StaticGameLOD High\nTextureReductionFactor = 0\nMaxParticleCount = 4321\nEnd\n"
  "DynamicGameLOD Low\nMinimumFPS = 0\nParticleSkipMask = -2147483648\nDebrisSkipMask = 3\nSlowDeathScale = 0.5\nEnd\n"
  "DynamicGameLOD Medium\nMinimumFPS = 10\nSlowDeathScale = 0.75\nEnd\n"
  "DynamicGameLOD High\nMinimumFPS = 20\nSlowDeathScale = 0.8\nEnd\n"
  "DynamicGameLOD VeryHigh\nMinimumFPS = 30\nSlowDeathScale = 1.0\nEnd\n";
const char* presets="LODPreset High P4 2000 PS20 512\nBenchProfile P4 2000 2 3 4\nReallyLowMHz 400\n";
struct Probe:NativeLODProbe {
  NativeLODHardware facts{std::uint64_t{16}<<30,XX,0,FALSE};
  std::optional<NativeLODLegacyScores> scores;
  std::optional<ChipsetType> chip;
  Bool failHardware=FALSE,failScores=FALSE,failChip=FALSE;
  NativeLODHardware hardware() override {if(failHardware)throw ERROR_BAD_ARG;return facts;}
  std::optional<NativeLODLegacyScores> legacyScores() override {if(failScores)throw ERROR_BAD_ARG;return scores;}
  std::optional<ChipsetType> chipset() override {if(failChip)throw ERROR_BAD_ARG;return chip;}
};
struct IO:NativeStorageIO {
  std::size_t ordinal=SIZE_MAX,calls=0;Bool armed=FALSE,triggered=FALSE;
  void arm(std::size_t value){ordinal=value;calls=0;armed=TRUE;triggered=FALSE;}
  void disarm(){armed=FALSE;}
  void step(){if(armed && calls++==ordinal){armed=FALSE;triggered=TRUE;throw NativeStorageError();}}
  int openFile(int directory,const char* name,int flags,unsigned mode) override {step();return NativeStorageIO::openFile(directory,name,flags,mode);}
  Int writeFile(int descriptor,const void* bytes,Int size) override {step();return NativeStorageIO::writeFile(descriptor,bytes,size);}
  int sync(int descriptor) override {step();return NativeStorageIO::sync(descriptor);}
  void preparePublish() override {step();NativeStorageIO::preparePublish();}
};
struct Context {
  Tree assets,user;FileSystem files;IO io;
  NativeUserStorage storage;
  std::unique_ptr<GlobalData> data;
  FileSystem* previousFiles=TheFileSystem;GlobalData* previousData=TheWritableGlobalData;
  Context():storage(NativeUserPaths::resolve(user.root.string(),(user.root/"data").string(),(user.root/"cache").string()),files,&io){
    put("Data/INI/GameLOD.ini",definitions);put("Data/INI/GameLODPresets.ini",presets);
    TheFileSystem=&files;
    try{data=std::make_unique<GlobalData>();data->m_enableStaticLOD=TRUE;data->m_textureReductionFactor=1;
      TheWritableGlobalData=data.get();}
    catch(...){TheFileSystem=previousFiles;throw;}
  }
  ~Context(){TheWritableGlobalData=data.get();data.reset();TheWritableGlobalData=previousData;TheFileSystem=previousFiles;}
  void put(const char* name,const std::string& bytes){const auto path=assets.root/name;std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path,std::ios::binary);output.write(bytes.data(),bytes.size());output.close();require(bool(output),"generated INI output");files.mountReadOnly({assets.root.string()});}
  void output(const char* name,const char* bytes){auto target=storage.beginWrite(NativeUserArea::Data,name);target->write(bytes,std::strlen(bytes));target->commit();}
  std::string read(const char* name){const auto bytes=storage.readFile(NativeUserArea::Data,name,4096);if(!bytes)return {};
    return {bytes->begin(),bytes->end()};}
};
struct Owner:GameLODManager {
  using GameLODManager::GameLODManager;
  UnsignedInt state() const {
    CRC crc;auto add=[&](const auto& value){crc.computeCRC(&value,sizeof(value));};
    for(const auto& p:m_staticGameLODInfo){add(p.m_minFPS);add(p.m_minProcessorFPS);add(p.m_sampleCount2D);add(p.m_sampleCount3D);add(p.m_streamCount);
      add(p.m_maxParticleCount);add(p.m_useShadowVolumes);add(p.m_useShadowDecals);add(p.m_useCloudMap);add(p.m_useLightMap);add(p.m_showSoftWaterEdge);
      add(p.m_maxTankTrackEdges);add(p.m_maxTankTrackOpaqueEdges);add(p.m_maxTankTrackFadeDelay);add(p.m_useBuildupScaffolds);add(p.m_useTreeSway);
      add(p.m_useEmissiveNightMaterials);add(p.m_useHeatEffects);add(p.m_textureReduction);add(p.m_useFpsLimit);add(p.m_enableDynamicLOD);add(p.m_useTrees);}
    for(const auto& p:m_dynamicGameLODInfo){add(p.m_minFPS);add(p.m_dynamicParticleSkipMask);add(p.m_dynamicDebrisSkipMask);add(p.m_slowDeathScale);
      add(p.m_minDynamicParticlePriority);add(p.m_minDynamicParticleSkipPriority);}
    for(const auto& level:m_lodPresets)for(const auto& p:level){add(p.m_cpuType);add(p.m_mhz);add(p.m_cpuPerfIndex);add(p.m_videoType);add(p.m_memory);}
    for(const auto& p:m_benchProfiles){add(p.m_cpuType);add(p.m_mhz);add(p.m_intBenchIndex);add(p.m_floatBenchIndex);add(p.m_memBenchIndex);}
    for(const auto count:m_numLevelPresets)add(count);
    add(m_numBenchProfiles);add(m_currentStaticLOD);add(m_currentDynamicLOD);add(m_numParticleGenerations);add(m_dynamicParticleSkipMask);
    add(m_numDebrisGenerations);add(m_dynamicDebrisSkipMask);add(m_slowDeathScale);add(m_minDynamicParticlePriority);add(m_minDynamicParticleSkipPriority);
    add(m_videoPassed);add(m_cpuPassed);add(m_memPassed);add(m_idealDetailLevel);add(m_videoChipType);add(m_cpuType);add(m_numRAM);add(m_cpuFreq);
    add(m_intBenchIndex);add(m_floatBenchIndex);add(m_memBenchIndex);add(m_compositeBenchIndex);add(m_currentTextureReduction);add(m_reallyLowMHz);
    add(m_frequencyKnown);add(m_calibrated);add(m_reportStatus);add(m_recommendationStatus);return crc.get();
  }
  void seed(){m_staticGameLODInfo[STATIC_GAME_LOD_HIGH].m_maxParticleCount=777;m_numParticleGenerations=123;m_numDebrisGenerations=456;}
  void rollover(){m_numParticleGenerations=std::numeric_limits<UnsignedInt>::max();m_numDebrisGenerations=std::numeric_limits<UnsignedInt>::max();}
};
struct Publication {
  GameLODManager* previous=TheGameLODManager;
  Publication(GameLODManager& owner){TheGameLODManager=&owner;}
  ~Publication(){TheGameLODManager=previous;}
};
void functional(){
  Context context;Probe probe;Owner owner(probe,&context.storage);Publication publication(owner);owner.init();
  require(owner.physicalRAMBytes()==probe.facts.ramBytes && owner.didMemPass(),"physical RAM beyond signed32 remains actual wide bytes");
  require(!owner.isReallyLowMHz() && !owner.hasLegacyCalibration(),"unknown frequency/calibration is not a slow or equivalent legacy CPU");
  const auto prior=owner.state();require(owner.findStaticLODLevel()==STATIC_GAME_LOD_UNKNOWN && owner.getRecommendedTextureReduction()==1 && owner.state()==prior,
      "unjustified recommendation preserves existing settings and does not publish a false profile");
  require(context.read("Options.ini").empty(),"unknown hardware never persists pretend ideal detail");
  require(owner.getSlowDeathScale()==1.0f && owner.getMinDynamicParticlePriority()==PARTICLE_PRIORITY_LOWEST,
      "initial source scale is unchanged and source priority default is defined");
  require(owner.findDynamicLODLevel(20)==DYNAMIC_GAME_LOD_MEDIUM && owner.findDynamicLODLevel(20.99f)==DYNAMIC_GAME_LOD_MEDIUM &&
      owner.findDynamicLODLevel(21)==DYNAMIC_GAME_LOD_HIGH,"original truncation and strict threshold ordering");
  require(owner.setDynamicLODLevel(DYNAMIC_GAME_LOD_LOW) && owner.getSlowDeathScale()==0.5f,"actual source transition updates configured scale");
  owner.rollover();require(owner.isParticleSkipped() && owner.isDebrisSkipped(),"defined 32-bit wrapping retains exact original masks");
  require(owner.setDynamicLODLevel(DYNAMIC_GAME_LOD_HIGH) && owner.getSlowDeathScale()==0.8f,"subsequent source scale transition retained");
  probe.facts={std::uint64_t{8}<<30,P4,2000,TRUE};probe.chip=DC_GENERIC_PIXEL_SHADER_2_0;owner.init();
  require(owner.findStaticLODLevel()==STATIC_GAME_LOD_HIGH && owner.recommendationPersistenceStatus()==NativeLODReportStatus::Published,
      "generated established legacy class selects exact source preset and protected preferences");
  require(context.read("Options.ini").find("IdealStaticGameLOD = High")!=std::string::npos,"original preference spelling and value retained");
  require(owner.setStaticLODLevel(STATIC_GAME_LOD_HIGH) && context.data->m_maxParticleCount==4321 && context.data->m_textureReductionFactor==1,
      "actual source manual quality application retains unchanged texture-reduction comparison");
  probe.facts.frequencyMHz=399;owner.init();require(owner.isReallyLowMHz(),"actual known frequency below original audio cutoff");
  probe.facts.frequencyMHz=400;owner.init();require(!owner.isReallyLowMHz(),"original strict frequency boundary");
}
void negative(){
  Context context;Probe probe;Owner owner(probe,&context.storage);Publication publication(owner);owner.init();owner.seed();
  auto rejectUnchanged=[&]{const auto state=owner.state();const auto live=AllocationFault::live();rejects([&]{owner.init();});
    require(owner.state()==state && AllocationFault::live()==live && TheGameLODManager==&owner,"failed admission retains complete old owner and legitimate publication");};
  probe.failHardware=TRUE;rejectUnchanged();probe.failHardware=FALSE;
  probe.facts.ramBytes=0;rejectUnchanged();probe.facts.ramBytes=std::uint64_t{16}<<30;
  probe.facts.frequencyMHz=1;rejectUnchanged();probe.facts.frequencyMHz=0;
  probe.failScores=TRUE;rejectUnchanged();probe.failScores=FALSE;
  context.data->m_forceBenchmark=TRUE;rejectUnchanged();context.data->m_forceBenchmark=FALSE;
  for(Real invalid:{0.0f,-1.0f,std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN()}){
    probe.scores=NativeLODLegacyScores{invalid,3,4};rejectUnchanged();
  }probe.scores.reset();
  for(const char* invalid:{"LODPreset Custom P4 2000 PS20 512\n","LODPreset Missing P4 2000 PS20 512\n",
      "LODPreset High P4 0 PS20 512\n","LODPreset High P4 2000 PS20 0\n","BenchProfile P4 2000 0 3 4\n"}){
    context.put("Data/INI/GameLODPresets.ini",invalid);rejectUnchanged();
  }
  context.put("Data/INI/GameLODPresets.ini",presets);
  for(Real invalid:{-1.0f,std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::max()})
    rejects([&]{owner.findDynamicLODLevel(invalid);});
  rejects([&]{owner.setDynamicLODLevel(DYNAMIC_GAME_LOD_COUNT);});rejects([&]{owner.setStaticLODLevel(STATIC_GAME_LOD_COUNT);});
  rejects([&]{owner.getLevelTextureReduction(STATIC_GAME_LOD_UNKNOWN);});
  Owner bounds(probe,&context.storage);
  rejects([&]{bounds.newLODPreset(STATIC_GAME_LOD_CUSTOM);});
  for(unsigned i=0;i<MAX_LOD_PRESETS_PER_LEVEL;++i)require(bounds.newLODPreset(STATIC_GAME_LOD_HIGH),"source preset capacity");
  rejects([&]{bounds.newLODPreset(STATIC_GAME_LOD_HIGH);});
  for(unsigned i=0;i<MAX_BENCH_PROFILES;++i)require(bounds.newBenchProfile(),"source benchmark capacity");
  rejects([&]{bounds.newBenchProfile();});
  owner.init();require(owner.m_staticGameLODInfo[STATIC_GAME_LOD_HIGH].m_maxParticleCount==4321,"corrected same-owner full retry");
}
void native(){const auto live=AllocationFault::live();const auto fds=descriptors();
  const auto hardware=nativeLODProbe().hardware();require(hardware.ramBytes && hardware.cpu==XX &&
      ((!hardware.frequencyKnown && hardware.frequencyMHz==0) || (hardware.frequencyKnown && hardware.frequencyMHz>0)),"actual redacted POSIX hardware admission");
  require(!nativeLODProbe().legacyScores() && !nativeLODProbe().chipset(),"unmeasured calibration/renderer classification remains explicitly unavailable");
  require(AllocationFault::live()==live && descriptors()==fds,"native hardware query retires acquired descriptor exactly");
}
void faults(const std::string& family){
  Context context;Probe probe;Owner owner(probe,&context.storage);Publication publication(owner);owner.init();
  if(family=="fault-report"){
    context.data->m_forceBenchmark=TRUE;probe.scores=NativeLODLegacyScores{2,3,4};context.output("Benchmark.txt","retained report");
  }
  std::size_t census;
  {AllocationFault::arm(SIZE_MAX);try{owner.init();}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<4096,"complete bounded actual LOD allocation manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){
    owner.seed();if(family=="fault-report")context.output("Benchmark.txt","retained report");
    const auto state=owner.state();const auto live=AllocationFault::live();const auto fds=descriptors();bool failed=false;
    AllocationFault::arm(ordinal);try{owner.init();}catch(const std::bad_alloc&){failed=true;}
    AllocationFault::disarm();require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
        (failed || AllocationFault::attempts()==census),"every allocation failure/retry pair and exact success terminal");
    require(AllocationFault::live()==live && descriptors()==fds && TheGameLODManager==&owner,"candidate standard/pool/descriptor retirement and parent restoration");
    if(failed){require(owner.state()==state,"failed candidate preserves complete source metadata");
      if(family=="fault-report")require(context.read("Benchmark.txt")=="retained report","failed acquisition leaves prior protected report");
      owner.init();}
    require(owner.m_staticGameLODInfo[STATIC_GAME_LOD_HIGH].m_maxParticleCount==4321,"corrected original owner metadata accepted");
    if(family=="fault-report")require(owner.benchmarkReportStatus()==NativeLODReportStatus::Published &&
        context.read("Benchmark.txt").starts_with("BenchProfile = XX 0 2.000000 3.000000 4.000000"),"complete source-compatible protected report");
  }std::cout<<family<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
void ioFaults(){
  Context context;Probe probe;probe.scores=NativeLODLegacyScores{2,3,4};context.data->m_forceBenchmark=TRUE;
  Owner owner(probe,&context.storage);Publication publication(owner);
  // Establish the same protected namespace before discovery and every retry:
  // otherwise the first absent data directory skips the preference-file open.
  context.output("Benchmark.txt","retained report");
  context.io.arm(SIZE_MAX);owner.init();const auto census=context.io.calls;context.io.disarm();require(census>0 && census<32,"complete protected report I/O manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){context.output("Benchmark.txt","retained report");const auto live=AllocationFault::live();const auto fds=descriptors();
    const auto state=owner.state();bool failed=false;
    context.io.arm(ordinal);try{owner.init();}catch(const NativeStorageError&){failed=true;}context.io.disarm();
    if(context.io.triggered!=(ordinal<census) || (!context.io.triggered && context.io.calls!=census))
      std::cerr<<"storage manifest ordinal "<<ordinal<<" calls "<<context.io.calls<<" census "<<census<<'\n';
    require(context.io.triggered==(ordinal<census) && (context.io.triggered || context.io.calls==census),"each report I/O failure and exact terminal");
    require(AllocationFault::live()==live && descriptors()==fds,"all protected publication backing retires");
    const auto result=context.read("Benchmark.txt");
    require(result=="retained report" || result.starts_with("BenchProfile = XX 0"),"old or complete published output, never partial");
    if(failed){require(owner.state()==state && result=="retained report" && TheGameLODManager==&owner,
        "required preference-read failure retains complete prior owner, report and publication");}
    else if(owner.benchmarkReportStatus()==NativeLODReportStatus::Unavailable)require(result=="retained report","unavailable prepublication keeps prior report");
    else require(result.starts_with("BenchProfile = XX 0"),"post-rename durability failure remains complete published output");
    owner.init();require(owner.benchmarkReportStatus()==NativeLODReportStatus::Published,"same-owner corrected protected report retry");
  }std::cout<<"report storage ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){bool initialized=false;const auto initial=AllocationFault::live();
  try {require(argc==2,"LOD family required");initMemoryManager();initialized=true;
    {Context warm;Probe probe;Owner owner(probe,&warm.storage);Publication publication(owner);owner.init();}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto fds=descriptors();const std::string family=argv[1];
      if(family=="functional")functional();else if(family=="negative")negative();else if(family=="native")native();
      else if(family=="fault-storage")ioFaults();else if(family=="fault-init" || family=="fault-report")faults(family);else throw std::runtime_error("unknown LOD family");
      require(AllocationFault::live()==live && descriptors()==fds && !TheGameLODManager && !TheWritableGlobalData && !TheFileSystem,"complete original LOD/context retirement");
    }shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original pool metadata retires exactly");
    std::cout<<"PASS actual LOD owner; native legacy calibration and full startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL original LOD admission\n";}catch(const INIException&){std::cerr<<"FAIL generated LOD definition\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
