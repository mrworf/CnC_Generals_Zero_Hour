// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/FileSystem.h"
#include "Common/NameKeyGenerator.h"
#include "Common/WellKnownKeys.h"
#include "Common/NativeUserStorage.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <dirent.h>
namespace {
void require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
unsigned descriptors() {
  DIR* directory=::opendir("/proc/self/fd");require(directory,"descriptor census");unsigned count=0;
  while(::readdir(directory))++count;::closedir(directory);return count;
}
struct Context {
  std::filesystem::path root;
  FileSystem files;
  NameKeyGenerator names;
  std::unique_ptr<GameTextInterface> text;
  MapCache cache;
  Context() {
    char name[]="/tmp/zh-metadata-XXXXXX";const auto* path=::mkdtemp(name);require(path,"generated root");root=path;
    try {
      put("Data/Generals.str","MAP:Label\n\"Localized\"\nEnd\n");
      put("map.str","MAP:Active\n\"Gameplay\"\nEnd\n");
      put("accepted.ini","MapCache Maps/Example/Example.map\nnumPlayers = 2\nfileSize = 17\nfileCRC = 19\nEnd\n");
      put("candidate.ini","MapCache Maps/Example/Example.map\nnumPlayers = 8\nfileSize = 31\nfileCRC = 23\n"
        "timestampLo = -2147483648\ntimestampHi = -1\n"
        "extentMax = X:80 Y:90 Z:10\nnameLookupTag = MAP:Label\n"
        "Player_8_Start = X:7 Y:8 Z:9\nsupplyPosition = X:1 Y:2 Z:3\n"
        "supplyPosition = X:4 Y:5 Z:6\ntechPosition = X:11 Y:12 Z:13\nEnd\n");
      files.mountReadOnly({root.string()});TheFileSystem=&files;TheNameKeyGenerator=&names;names.init();
      text.reset(CreateGameTextInterface());text->init();text->initMapStringFile("map.str");TheGameText=text.get();
      TheMapCache=&cache;load("accepted.ini");
    } catch(...) {withdraw();std::error_code ignored;std::filesystem::remove_all(root,ignored);throw;}
  }
  void withdraw() noexcept {TheMapCache=nullptr;TheGameText=nullptr;TheNameKeyGenerator=nullptr;TheFileSystem=nullptr;}
  ~Context() {withdraw();std::error_code ignored;std::filesystem::remove_all(root,ignored);}
  void put(const char* name,const char* bytes) {
    const auto path=root/name;std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path,std::ios::binary);output<<bytes;require(bool(output),"generated input");
  }
  void load(const char* filename) {
    const INIBlockDefinition blocks[]{{"MapCache",INI::parseMapCacheDefinition}};
    INI ini;ini.loadBlocks(filename,INI_LOAD_OVERWRITE,blocks);
  }
  const MapMetaData& entry() const {require(cache.size()==1,"one accepted metadata entry");return cache.begin()->second;}
};
void accepted(const Context& context) {
  const auto& md=context.entry();
  require(md.m_numPlayers==2 && md.m_filesize==17 && md.m_CRC==19 && md.m_displayName.compare(L"Example.map (2)")==0,
    "accepted metadata unchanged");
  require(context.text->fetch("MAP:Active").compare(L"Gameplay")==0,"active gameplay text unchanged");
}
void candidate(const Context& context) {
  const auto& md=context.entry();
  require(md.m_numPlayers==8 && md.m_waypoints.m_numStartSpots==8 && md.m_displayName.compare(L"Localized (8)")==0,
    "actual selected INI provider prepares localized eight-slot metadata");
  require(md.m_waypoints.size()==9 && md.m_waypoints.at("Player_8_Start").z==9 &&
      md.m_waypoints.at("Player_7_Start").x==0 && md.m_waypoints.at("Player_7_Start").y==0 &&
      md.m_waypoints.at("Player_7_Start").z==0,"every absent fixed-capacity slot initialized");
  require(md.m_supplyPositions.size()==2 && md.m_supplyPositions.front().x==1 &&
      md.m_supplyPositions.back().x==4 && md.m_techPositions.front().z==13,"source coordinate ordering");
  require(md.m_timestamp.m_lowTimeStamp==0x80000000u && md.m_timestamp.m_highTimeStamp==0xffffffffu,
    "signed source timestamp words retain exact unsigned backing");
}
void values() {
  alignas(MapMetaData) std::array<unsigned char,sizeof(MapMetaData)> bytes;
  bytes.fill(0xa5);auto* value=new(bytes.data()) MapMetaData;
  require(value->m_numPlayers==0 && !value->m_isOfficial && !value->m_isMultiplayer && value->m_filesize==0 &&
    value->m_CRC==0 && value->m_timestamp.m_lowTimeStamp==0 && value->m_timestamp.m_highTimeStamp==0 &&
    value->m_extent.lo.x==0 && value->m_extent.lo.y==0 && value->m_extent.lo.z==0 &&
    value->m_extent.hi.x==0 && value->m_extent.hi.y==0 && value->m_extent.hi.z==0 &&
    value->m_waypoints.m_numStartSpots==1,"fresh metadata from nonzero backing");
  MapMetaData copy=*value;copy.m_numPlayers=8;copy.m_waypoints.m_numStartSpots=8;copy.m_displayName=L"changed";
  AllocationFault::arm(0);value->swap(copy);AllocationFault::disarm();
  require(!AllocationFault::triggered() && value->m_numPlayers==8 && value->m_waypoints.m_numStartSpots==8 &&
    copy.m_numPlayers==0 && copy.m_waypoints.m_numStartSpots==1,"complete metadata swap allocation-free");
  MapMetaData reset;value->swap(reset);require(value->m_extent.hi.z==0 && value->m_displayName.isEmpty(),"reset metadata");
  value->~MapMetaData();
}
void rejection() {
  Context context;
  for(const char* body:{"numPlayers = -1\n","numPlayers = 9\n","fileSize = 2147483648\n",
      "extentMin = X:10 Y:0 Z:0\nextentMax = X:1 Y:0 Z:0\n",
      "extentMax = X:nan Y:1 Z:1\n","extentMax = X:inf Y:1 Z:1\n",
      "numPlayers = 8\nsupplyPosition = X:1 Y:2 Z:3\nUnknown = late\n"}) {
    std::string input="MapCache Maps/Example/Example.map\n";input+=body;input+="End\n";
    context.put("bad.ini",input.c_str());context.files.mountReadOnly({context.root.string()});
    const auto live=AllocationFault::live();const auto fds=descriptors();bool rejected=false;
    try {context.load("bad.ini");}catch(ErrorCode){rejected=true;}catch(const INIException&){rejected=true;}
    require(rejected,"malformed/range/late-field metadata rejected");accepted(context);
    require(AllocationFault::live()==live && descriptors()==fds,"semantic rejection retires exact backing");
    context.load("candidate.ini");candidate(context);context.load("accepted.ini");accepted(context);
  }
}
template<class Action,class Rollback,class Restore>
void sweep(Action action,Rollback rollback,Restore restore) {
  action();restore();
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());action();
  const auto census=AllocationFault::attempts();AllocationFault::disarm();restore();
  require(census>0 && census<2048,"bounded complete operation census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    const auto live=AllocationFault::live();const auto fds=descriptors();
    const Int pooled=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
    AllocationFault::arm(ordinal);bool failed=false;
    try {action();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();
    require(failed==(ordinal<census) && AllocationFault::triggered()==failed,"disjoint complete census and terminal");
    if(failed) {
      require(AllocationFault::live()==live && descriptors()==fds &&
        TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==pooled,"exact rejected owner residuals");
      rollback();action();
    }
    restore();
  }
  std::cout<<"ordinals [0,"<<census<<") complete; terminal "<<census<<'\n';
}
void faults(bool insertion) {
  Context context;
  if(insertion)context.cache.clear();
  sweep([&]{context.load("candidate.ini");},
    [&]{if(insertion)require(context.cache.empty(),"failed insertion publishes no entry");else accepted(context);},
    [&]{candidate(context);if(insertion)context.cache.clear();else context.load("accepted.ini");});
}
void serialization(bool fault) {
  Context context;context.load("candidate.ini");
  const AsciiString dir("Maps");const auto expected=context.cache.serializeCacheINI(dir);
  require(expected.find("numPlayers = 8\n")!=expected.npos && expected.find("nameLookupTag = MAP:Label\n")!=expected.npos &&
    expected.find("supplyPosition = X:1.00 Y:2.00 Z:3.00\n")!=expected.npos,"original cache grammar");
  require(expected.find("Localized")==expected.npos,"cache omits localized display text");
  context.put("roundtrip.ini",expected.c_str());context.files.mountReadOnly({context.root.string()});
  context.load("accepted.ini");context.load("roundtrip.ini");candidate(context);
  if(fault)sweep([&]{require(context.cache.serializeCacheINI(dir)==expected,"complete serialized candidate");},
    [&]{candidate(context);},[]{});
}
struct UserRoot {
  std::filesystem::path path;
  UserRoot() {char name[]="/tmp/zh-metadata-output-XXXXXX";const auto* made=::mkdtemp(name);require(made,"generated output root");path=made;}
  ~UserRoot() {std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
struct WriterIO : NativeStorageIO {
  enum Phase {None,Open,Write,Partial,FileSync,Prepare,DirectorySync};
  Phase phase=None;
  bool throwing=false,partialWritten=false;
  unsigned syncs=0;
  void select(Phase selected,bool throws) {phase=selected;throwing=throws;partialWritten=false;syncs=0;}
  int fail() {if(throwing)throw NativeStorageError();errno=EIO;return -1;}
  int openFile(int directory,const char* name,int flags,unsigned mode) override {
    if(phase==Open)return fail();return NativeStorageIO::openFile(directory,name,flags,mode);
  }
  Int writeFile(int descriptor,const void* bytes,Int count) override {
    if(phase==Write)return fail();
    if(phase==Partial) {
      if(partialWritten)return fail();partialWritten=true;
      return NativeStorageIO::writeFile(descriptor,bytes,std::max(1,count/2));
    }
    return NativeStorageIO::writeFile(descriptor,bytes,count);
  }
  int sync(int descriptor) override {
    const auto ordinal=syncs++;
    if((phase==FileSync && ordinal==0) || (phase==DirectorySync && ordinal==1))return fail();
    return NativeStorageIO::sync(descriptor);
  }
  void preparePublish() override {if(phase==Prepare)throw NativeStorageError();}
};
void persistence(bool allocationFaults) {
  Context context;context.load("candidate.ini");const auto serialized=context.cache.serializeCacheINI("Maps");
  UserRoot user;WriterIO io;
  const NativeUserPaths paths{(user.path/"data").string(),(user.path/"cache").string()};
  NativeUserStorage storage(paths,context.files,&io);
  const std::string old="accepted cache";
  auto restore=[&] {io.select(WriterIO::None,false);require(MapCache::persistCacheINI(storage,old),"restore accepted cache");};
  auto read=[&] {
    const auto bytes=storage.readFile(NativeUserArea::Data,"Maps/MapCache.ini",65536);
    require(bytes.has_value(),"published cache reachable");return std::string(bytes->begin(),bytes->end());
  };
  auto noTemporary=[&] {
    for(const auto& entry:std::filesystem::directory_iterator(user.path/"data/Maps"))
      require(!entry.path().filename().string().starts_with(".zh-write-"),"temporary metadata output retired");
  };
  restore();require(MapCache::persistCacheINI(storage,serialized) && read()==serialized,"actual optional cache publication");
  restore();
  if(allocationFaults) {
    sweep([&]{require(MapCache::persistCacheINI(storage,serialized),"complete prepared cache publication");},
      [&]{require(read()==old,"cache allocation rejection preserves accepted file");noTemporary();},
      [&]{require(read()==serialized,"successful cache candidate bytes");noTemporary();restore();});
  } else {
    for(auto phase:{WriterIO::Open,WriterIO::Write,WriterIO::Partial,WriterIO::FileSync,WriterIO::Prepare,WriterIO::DirectorySync})
      for(bool throwing:{false,true}) {
        restore();const auto live=AllocationFault::live();const auto fds=descriptors();io.select(phase,throwing);
        const bool published=MapCache::persistCacheINI(storage,serialized);io.select(WriterIO::None,false);
        require(published==(phase==WriterIO::DirectorySync),"optional fallback versus irreversible publication");
        require(AllocationFault::live()==live && descriptors()==fds,"writer exact residuals");
        require(read()==(published?serialized:old),"I/O rejection preserves file; post-rename uncertainty retains published bytes");
        noTemporary();require(MapCache::persistCacheINI(storage,serialized) && read()==serialized,"same-owner writer retry");
      }
    std::ofstream(user.path/"blocked")<<"not a directory";
    NativeUserStorage blocked({(user.path/"blocked").string(),paths.cache},context.files);
    require(!MapCache::persistCacheINI(blocked,serialized),"unavailable optional output fallback");
    NativeUserStorage forbidden({context.root.string(),paths.cache},context.files);
    require(!MapCache::persistCacheINI(forbidden,serialized),"supplied asset root cannot receive cache output");
    require(!std::filesystem::exists(context.root/"Maps/MapCache.ini"),"asset root unchanged");
  }
  candidate(context);require(context.text->fetch("MAP:Active").compare(L"Gameplay")==0,"writer cannot mutate metadata/text owners");
}
void optionalCache(bool allocationFaults) {
  Context context;
  require(context.cache.loadCacheINI("candidate.ini"),"optional actual metadata complete load");candidate(context);
  context.load("accepted.ini");
  if(allocationFaults) {
    sweep([&]{require(context.cache.loadCacheINI("candidate.ini"),"optional valid cache admitted");},
      [&]{require(TheMapCache==&context.cache,"offside callback publication restored on allocation failure");accepted(context);},
      [&]{require(TheMapCache==&context.cache,"offside callback publication restored on success");candidate(context);context.load("accepted.ini");});
    return;
  }
  for(const char* bad:{
      "MapCache Maps/Example/Example.map\nnumPlayers = 8\nEnd\nMapCache Maps/Second/Second.map\nUnknown = late\nEnd\n",
      "MapCache Maps/Example/Example.map\nnumPlayers = 8\nEnd\nWebpageURL Foreign\nURL = ignored\nEnd\n",
      "MapCache Maps/Example/Example.map\nnumPlayers = 9\nEnd\n",
      "MapCache _zz\nnumPlayers = 2\nEnd\n",
      "MapCache _00\nnumPlayers = 2\nEnd\n",
      "MapCache Maps/Example/Example.map\nnameLookupTag = _zz\nEnd\n"}) {
    context.put("bad-cache.ini",bad);context.files.mountReadOnly({context.root.string()});
    const auto live=AllocationFault::live();const auto fds=descriptors();
    require(!context.cache.loadCacheINI("bad-cache.ini"),"malformed optional cache returns rescan fallback");
    require(TheMapCache==&context.cache && AllocationFault::live()==live && descriptors()==fds,
      "whole-file rejection retires offside metadata and restores actual publication");accepted(context);
    require(context.cache.loadCacheINI("candidate.ini"),"optional corrected retry");candidate(context);context.load("accepted.ini");
  }
  require(!context.cache.loadCacheINI("missing-cache.ini"),"missing optional cache fallback");accepted(context);
  TheMapCache=nullptr;bool rejected=false;
  try {context.cache.loadCacheINI("candidate.ini");}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG;}
  TheMapCache=&context.cache;require(rejected,"invalid required owner is not corrupt-cache fallback");accepted(context);
  context.put("cold-bad-cache.ini","MapCache Maps/Example/Example.map\nnumPlayers = 8\nEnd\nForeign Late\nEnd\n");
  context.files.mountReadOnly({context.root.string()});context.names.reset();
  const auto live=AllocationFault::live();
  const auto units=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount();
  require(!context.cache.loadCacheINI("cold-bad-cache.ini") && context.names.keyToName(NameKeyType(1)).isEmpty(),
    "first-use shared key retires on whole-file rejection");
  require(AllocationFault::live()==live && TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount()==units,
    "cold metadata namespace exact residuals");
  require(context.names.nameToKey("changed ordinal order")==1 && context.cache.loadCacheINI("candidate.ini"),
    "cold metadata retry after different ordinal acquisition");
  require(context.entry().m_waypoints.contains("InitialCameraPosition") &&
    !context.entry().m_waypoints.contains("changed ordinal order"),"static-key cache invalidated after rejection");candidate(context);
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"metadata family");initMemoryManager();initialized=true;const std::string family=argv[1];
    for(int repeat=0;repeat<3;++repeat) {
      if(family=="values")values();
      else if(family=="parsing"){Context context;accepted(context);context.load("candidate.ini");candidate(context);}
      else if(family=="rejection")rejection();
      else if(family=="fault-insert")faults(true);
      else if(family=="fault-replace")faults(false);
      else if(family=="serialization")serialization(false);
      else if(family=="fault-serialization")serialization(true);
      else if(family=="persistence")persistence(false);
      else if(family=="fault-persistence")persistence(true);
      else if(family=="optional-cache")optionalCache(false);
      else if(family=="fault-optional-cache")optionalCache(true);
      else throw std::runtime_error("unknown metadata family");
    }
    shutdownMemoryManager();initialized=false;std::cout<<"PASS: actual metadata provider (whole MapCache process acceptance pending)\n";return 0;
  }catch(ErrorCode error){std::cerr<<"FAIL: source error "<<static_cast<unsigned>(error)<<'\n';}
  catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';}
  AllocationFault::disarm();TheFileSystem=nullptr;TheNameKeyGenerator=nullptr;TheGameText=nullptr;TheMapCache=nullptr;
  if(initialized)shutdownMemoryManager();return 1;
}
