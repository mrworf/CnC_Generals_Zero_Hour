// SPDX-License-Identifier: GPL-3.0-or-later
// Actual collector/output owner with generated synchronous inputs, not GameLogic.
#include "AllocationFault.h"
#include "Common/StatsCollector.h"
#include "Common/NativeUserStorage.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/MessageStream.h"
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
namespace {
void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
template<class Action> void rejects(Action action) {
  bool rejected=false;try {action();}catch(ErrorCode) {rejected=true;}
  require(rejected,"generated statistics input rejects");
}
struct Tree {
  std::filesystem::path path;
  Tree(){char pattern[]="/tmp/zh-statistics-XXXXXX";const char* p=::mkdtemp(pattern);
    require(p,"generated statistics root");path=p;}
  ~Tree(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
std::size_t descriptors() {
  DIR* directory=::opendir("/proc/self/fd");require(directory,"generated descriptor census");
  std::size_t count=0;while(::readdir(directory)) ++count;::closedir(directory);return count;
}
struct Source:NativeStatsSource {
  UnsignedInt current=0;Int seconds=2;
  NativeStatsIdentity description;
  NativeStatsSample sampleValue{3,4,1000};
  bool failSample=false;
  Source(){description.map="Maps\\Generated\\Generated.map";description.side="USA";description.sessionName="Generated";}
  UnsignedInt frame() const override{return current;}
  Int intervalSeconds() const override{return seconds;}
  Int localPlayerIndex() const override{return 7;}
  NativeStatsIdentity identity() const override{return description;}
  NativeStatsSample sample() const override{if(failSample) throw ERROR_BAD_ARG;return sampleValue;}
  std::time_t timestamp() const override{return 1700000000;}
};
struct FaultIO:NativeStorageIO {
  enum Phase {None,OpenRead,OpenWrite,Write,FileSync,DirectorySync,Publish};
  Phase phase=None;unsigned syncs=0;bool throwing=false,shortWrites=false;
  bool fail(Phase value){if(phase!=value) return false;if(throwing) throw NativeStorageError();errno=EIO;return true;}
  int openFile(int directory,const char* name,int flags,unsigned mode) override {
    if(fail((flags&O_WRONLY)?OpenWrite:OpenRead)) return -1;
    return NativeStorageIO::openFile(directory,name,flags,mode);
  }
  Int writeFile(int descriptor,const void* bytes,Int count) override {
    if(fail(Write)) return -1;
    return NativeStorageIO::writeFile(descriptor,bytes,shortWrites?std::min<Int>(count,3):count);
  }
  int sync(int descriptor) override {
    if(fail(++syncs==1?FileSync:DirectorySync)) return -1;
    return NativeStorageIO::sync(descriptor);
  }
  void preparePublish() override {if(fail(Publish)) throw NativeStorageError();}
};
struct Context {
  Tree assets,user;
  FileSystem files;
  FaultIO io;
  NativeUserStorage storage;
  Source source;
  StatsCollector owner;
  Context():storage({(user.path/"data").string(),(user.path/"cache").string()},files,&io),
    owner(storage,source) {files.mountReadOnly({assets.path.string()});files.attachUserStorage(&storage);}
  std::string read(const char* name="Stats/Generated.txt") const {
    const auto bytes=storage.readFile(NativeUserArea::Data,name,INT32_MAX);
    require(bool(bytes),"complete protected statistics file");return std::string(bytes->begin(),bytes->end());
  }
  bool clean() const {
    for(const auto& entry:std::filesystem::recursive_directory_iterator(user.path))
      if(entry.path().filename().string().starts_with(".zh-write-")) return false;
    return true;
  }
  void prepare(unsigned operation) {
    owner.reset();require(!owner.outputFailed(),"accepted initial header");
    owner.incrementBuildCount();owner.incrementMoveCount();owner.incrementAttackCount();
    if(operation==0) source.description.side="Candidate";
    source.current=operation==2?30:60;
  }
  void run(unsigned operation) {
    io.syncs=0;
    if(operation==0) owner.reset();else if(operation==1) owner.update();else owner.writeFileEnd();
  }
};
void functional() {
  Context context;context.owner.reset();
  const auto initial=context.read();
  require(initial.find("Map:\tMaps\\Generated\\Generated.map\nSide:\tUSA\n")!=initial.npos &&
      initial.ends_with("0\t0\t0\t0\t0\t0\t0\t1000\t3\t4\n"),"original header and initial unit/money row");
  using MessageOwner=std::unique_ptr<GameMessage,void(*)(GameMessage*)>;
  const auto message=[&](GameMessage::Type type,Int player) {
    MessageOwner value(newInstance(GameMessage)(type,player),[](GameMessage* p){p->deleteInstance();});
    context.owner.collectMsgStats(value.get());
  };
  message(GameMessage::MSG_QUEUE_UNIT_CREATE,7);message(GameMessage::MSG_DOZER_CONSTRUCT,7);
  message(GameMessage::MSG_DOZER_CONSTRUCT_LINE,7);message(GameMessage::MSG_QUEUE_UNIT_CREATE,8);
  message(GameMessage::MSG_FRAME_TICK,7);
  context.owner.incrementMoveCount();context.owner.incrementAttackCount();context.owner.startScrollTime();
  context.source.current=59;context.owner.update();require(context.read()==initial,"interval boundary has no early row");
  context.source.current=60;context.owner.update();
  require(context.read().ends_with("2\t3\t1\t1\t1\t2\t0\t1000\t3\t4\n"),"source command filtering and exact sampled row");
  context.source.current=90;context.owner.endScrollTime();context.owner.writeFileEnd();
  const auto ended=context.read();
  require(ended.find("3\t0\t0\t0\t0\t1\t0\t1000\t3\t4\n---------------------------------------------------\nEnd Time:")!=ended.npos,
      "final units and scroll captured before one ordered footer");
  context.owner.writeFileEnd();context.source.current=120;context.owner.update();
  require(context.read()==ended,"end publication idempotent, no later sample rows");
  context.owner.reset();require(context.read()==initial,"repeated reset retires all previous counters/scroll state");
  context.source.current=UINT32_MAX-29;context.owner.reset();context.owner.incrementBuildCount();
  context.source.current=30;context.owner.update();
  require(context.read().ends_with("2\t1\t0\t0\t0\t0\t0\t1000\t3\t4\n"),"frame rollover preserves elapsed interval");
  context.source.seconds=INT32_MAX;context.source.current=31;const auto before=context.read();
  context.owner.update();require(context.read()==before,"large interval multiplication is defined, not an early sample");
  context.source.description.benchmarkSeconds=2;context.source.current=60;context.owner.writeFileEnd();
  require(context.read().find("*** BENCHMARK MODE STATS ***")!=std::string::npos,"retained benchmark footer fields");
  require(context.clean() && std::filesystem::is_empty(context.assets.path),"statistics never write supplied assets");
}
void negative() {
  Context context;context.owner.reset();const auto accepted=context.read();
  rejects([&]{context.owner.collectMsgStats(nullptr);});
  for(const char* map:{"","not-map",".map","bad\n.map"}) {
    context.source.description.sessionName="";context.source.description.map=map;
    rejects([&]{context.owner.reset();});require(context.read()==accepted,"malformed names keep accepted output");
  }
  context.source.description.map="Maps/Colon:Percent%.map";context.owner.reset();
  const auto names=context.storage.list(NativeUserArea::Data,"Stats",false,10);
  bool encoded=false;for(const auto& name:names) if(name.relative.find("Colon%3APercent%25_")!=std::string::npos) encoded=true;
  require(encoded,"unsafe physical leaf bytes encoded reversibly");
  context.source.description.sessionName="Generated";context.source.description.map="Generated.map";
  context.source.description.directory="../escape";context.owner.reset();
  require(context.owner.outputFailed() && context.read()==accepted,"directory traversal rejected with truthful status");
  context.source.description.directory=context.assets.path.string().c_str();
  rejects([&]{context.owner.reset();});require(std::filesystem::is_empty(context.assets.path),"asset namespace never becomes stats output");
  context.source.description.directory="Stats";
  context.owner.reset();require(context.owner.outputReady(),"corrected reset restores output admission");
  context.source.description.directory="/";rejects([&]{context.owner.reset();});
  context.source.description.directory="Stats";context.owner.reset();
  context.source.current=60;context.source.seconds=0;rejects([&]{context.owner.update();});
  context.source.seconds=-1;rejects([&]{context.owner.update();});
  context.source.seconds=2;context.source.failSample=true;rejects([&]{context.owner.update();});
  context.source.failSample=false;context.owner.update();require(!context.owner.outputFailed(),"same-owner corrected sample retry");
}
void ioFaults() {
  for(unsigned operation=0;operation<3;++operation)
    for(bool throwing:{false,true})
      for(auto phase:{FaultIO::OpenRead,FaultIO::OpenWrite,FaultIO::Write,FaultIO::FileSync,FaultIO::Publish,FaultIO::DirectorySync}) {
        if(operation==0 && phase==FaultIO::OpenRead) continue;
        Context context;context.prepare(operation);const auto before=context.read();
        const auto live=AllocationFault::live(),fds=descriptors();
        context.io.phase=phase;context.io.throwing=throwing;context.run(operation);
        context.io.phase=FaultIO::None;
        require(AllocationFault::live()==live && descriptors()==fds && context.clean(),"every storage phase retires exact backing");
        const auto after=context.read();
        if(phase==FaultIO::DirectorySync) {
          require(!context.owner.outputFailed() && !context.owner.outputDurable() && after!=before,"published durability-unknown is never rollback");
          if(operation==1 || operation==2) {context.run(operation);require(context.read()==after,"published rows/end never append twice");}
        } else {
          require(context.owner.outputFailed() && after==before,"optional storage failure retains accepted output/state");
          context.run(operation);require(!context.owner.outputFailed() && context.read()!=before,"every storage rejection has a corrected retry");
          if(operation==1) require(context.read().ends_with("2\t1\t1\t1\t0\t0\t0\t1000\t3\t4\n"),"failed row retained each counter exactly once");
        }
      }
  Context context;context.io.shortWrites=true;context.prepare(1);context.run(1);
  require(!context.owner.outputFailed() && context.read().ends_with("2\t1\t1\t1\t0\t0\t0\t1000\t3\t4\n"),"short writes publish complete row");
  Context renamed;renamed.prepare(0);const auto accepted=renamed.read();
  renamed.source.description.sessionName="Other";renamed.io.phase=FaultIO::Publish;renamed.run(0);
  renamed.io.phase=FaultIO::None;
  require(renamed.owner.outputFailed() && renamed.read()==accepted &&
      !renamed.storage.readFile(NativeUserArea::Data,"Stats/Other.txt",INT32_MAX),"rejected reset does not publish a new filename");
  renamed.owner.update();renamed.owner.writeFileEnd();
  require(!renamed.owner.outputReady() && renamed.read()==accepted,
      "failed new-session reset never appends new-world samples into the old report");
  renamed.run(0);
  require(renamed.read("Stats/Other.txt").find("Side:\tCandidate\n")!=std::string::npos,
      "same-owner corrected reset publishes new identity only on success");
}
void faults(unsigned operation) {
  std::size_t census=0;
  {Context context;context.prepare(operation);AllocationFault::arm(SIZE_MAX);
    try {context.run(operation);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<256,"bounded complete statistics manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    Context context;context.prepare(operation);const auto before=context.read();
    const auto live=AllocationFault::live(),fds=descriptors();bool failed=false;
    AllocationFault::arm(ordinal);
    try {context.run(operation);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();
    require(failed==(ordinal<census) && AllocationFault::triggered()==failed &&
        (failed || AllocationFault::attempts()==census),"every allocation failure and exact terminal");
    require(descriptors()==fds && context.clean(),"all candidates retire descriptors and temporary files");
    if(failed) {
      require(AllocationFault::live()==live && context.read()==before,"failed publication preserves complete owner/file backing");
      context.run(operation);
    }
    require(!context.owner.outputFailed() && context.read()!=before,"every allocation pair retries in the same collector");
    if(operation==1) require(context.read().ends_with("2\t1\t1\t1\t0\t0\t0\t1000\t3\t4\n"),"retry retains source counters without duplication");
  }
  std::cout<<"statistics operation "<<operation<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"statistics family required");initMemoryManager();initialized=true;
    // Warm source pooled readers/messages and timezone setup before baselines.
    {Context warm;warm.prepare(1);warm.run(1);
      auto* message=newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0);message->deleteInstance();}
    const auto live=AllocationFault::live(),fds=descriptors();
    const std::string family=argv[1];
    for(unsigned repeat=0;repeat<3;++repeat) {
      if(family=="functional") functional();else if(family=="negative") negative();
      else if(family=="io-faults") ioFaults();else if(family=="reset-faults") faults(0);
      else if(family=="update-faults") faults(1);else if(family=="end-faults") faults(2);
      else throw std::runtime_error("unknown statistics family");
      require(AllocationFault::live()==live && descriptors()==fds,"whole repeated collector/storage ownership retires exactly");
    }
    shutdownMemoryManager();initialized=false;std::cout<<"PASS actual statistics ownership (GameLogic sampling pending)\n";return 0;
  }catch(ErrorCode){std::cerr<<"FAIL generated statistics admission\n";}
  catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
