// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/NativeUserStorage.h"
#include "Common/NativeDataFile.h"
#include "Common/NativeSourceStrings.h"
#include <cerrno>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
namespace {
void require(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
template <class Action> void rejects(Action action) {
  bool failed = false;
  try {
    action();
  } catch (ErrorCode) {
    failed = true;
  } catch (const std::exception &) {
    failed = true;
  }
  require(failed, "generated storage operation rejected");
}
struct Tree {
  std::filesystem::path path;
  Tree() {
    char pattern[] = "/tmp/zh-user-storage-XXXXXX";
    const auto *created = ::mkdtemp(pattern);
    require(created, "generated storage root");
    path = created;
  }
  ~Tree() {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
  }
  std::string read(const std::filesystem::path &relative) const {
    std::ifstream input(path / relative, std::ios::binary);
    require(bool(input), "generated storage read");
    return std::string(std::istreambuf_iterator<char>(input), {});
  }
  NativeUserPaths paths() const {
    return NativeUserPaths::resolve(path.string(), (path / "data").string(),
                                    (path / "cache").string());
  }
};
unsigned descriptors() {
  DIR *dir = ::opendir("/proc/self/fd");
  require(dir, "storage fd baseline");
  unsigned count = 0;
  while (::readdir(dir))
    ++count;
  ::closedir(dir);
  return count;
}
bool noTemporary(const std::filesystem::path &directory) {
  if (!std::filesystem::exists(directory))
    return true;
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(directory))
    if (entry.path().filename().string().starts_with(".zh-write-"))
      return false;
  return true;
}
void write(NativeUserStorage &storage, const char *name,
           const std::string &bytes) {
  auto candidate = storage.beginWrite(NativeUserArea::Data, name);
  candidate->write(bytes.data(), static_cast<Int>(bytes.size()));
  require(candidate->commit() == NativeCommitResult::Durable,
          "generated durable user commit");
}
void paths() {
  const auto fallback =
      NativeUserPaths::resolve("/generated-home", "relative", "");
  require(fallback.data ==
                  "/generated-home/.local/share/cnc-generals-zero-hour" &&
              fallback.cache == "/generated-home/.cache/cnc-generals-zero-hour",
          "absolute XDG selection and home fallbacks");
  const auto selected =
      NativeUserPaths::resolve("", "/generated-data/", "/generated-cache");
  require(selected.data == "/generated-data/cnc-generals-zero-hour" &&
              selected.cache == "/generated-cache/cnc-generals-zero-hour",
          "explicit XDG without home");
  rejects([] { NativeUserPaths::resolve("relative", "", ""); });
  Tree assets, user;
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(), files);
  storage.validateRootOwnership();
  require(!std::filesystem::exists(user.path/"data"),
          "ownership admission does not create directories");
  for (const char *invalid :
       {"", "/absolute", "../escape", "a//b", "a/../b", "a/", "a\\b", "C:bad"})
    rejects([&] { storage.beginWrite(NativeUserArea::Data, invalid); });
  NativeUserStorage forbidden(NativeUserPaths::resolve(user.path.string(),
                                                       assets.path.string(),
                                                       assets.path.string()),
                              files);
  rejects([&] { forbidden.beginWrite(NativeUserArea::Data, "never"); });
  rejects([&] { forbidden.validateRootOwnership(); });
  require(std::filesystem::is_empty(assets.path),
          "asset root never receives directories or output");
  std::filesystem::create_directory_symlink(assets.path, user.path / "alias");
  NativeUserStorage alias(
      NativeUserPaths::resolve(user.path.string(),
                               (user.path / "alias").string(),
                               (user.path / "alias").string()),
      files);
  rejects([&] { alias.beginWrite(NativeUserArea::Data, "never"); });
  rejects([&] { alias.validateRootOwnership(); });
  require(std::filesystem::is_empty(assets.path),
          "resolved asset aliases are rejected before mkdir");
  write(storage, "baseline", "baseline");
  std::filesystem::create_directory_symlink(
      assets.path, user.path / "data/cnc-generals-zero-hour/child");
  rejects([&] { storage.beginWrite(NativeUserArea::Data, "child/never"); });
  require(std::filesystem::is_empty(assets.path),
          "nested aliases cannot redirect output");
}
struct FileCloser {
  void operator()(File* file) const noexcept { if(file) file->close(); }
};
using OwnedFile=std::unique_ptr<File,FileCloser>;
void namespaceFiles(bool faults) {
  Tree assets,user;
  FileSystem files;files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(),files);
  files.attachUserStorage(&storage);
  write(storage,"Maps/Generated/Generated.map","authored-map");
  write(storage,"Maps/Generated/map.str","authored-text");
  const auto root=user.paths().data;
  if(!faults) {
    require(files.createDirectory(AsciiString((root+"/Maps/Empty/Nested").c_str())),
            "explicit attached user directory creation");
    require(std::filesystem::is_directory(root+"/Maps/Empty/Nested"),"protected native directory exists");
    require(!files.createDirectory("Maps/AssetWrite"),"logical supplied namespace never writable");
    require(!files.createDirectory(AsciiString((assets.path.string()+"/forbidden").c_str())),
            "outside user root directory rejected");
    require(!std::filesystem::exists(assets.path/"forbidden"),"supplied directory unchanged");
    // Restore the established discovery fixture before its cardinality checks.
    std::filesystem::remove_all(root+"/Maps/Empty");
  }
  // Warm the exact source file pool before residual baselines.
  {OwnedFile warm(storage.openReadFile(NativeUserArea::Data,"Maps/Generated/Generated.map"));}
  auto* pool=TheMemoryPoolFactory->findMemoryPool("NativeDataFile");
  const auto units=pool->getUsedBlockCount();
  if(faults) {
    write(storage,"accepted","accepted");
    for(unsigned operation=0;operation<6;++operation) {
      const auto run=[&] {
        if(operation==0) {OwnedFile view(storage.openReadFile(NativeUserArea::Data,"Maps/Generated/Generated.map"));}
        else if(operation==1) {auto entries=storage.list(NativeUserArea::Data,"Maps",true,3);}
        else if(operation==2) storage.validateRootOwnership();
        else if(operation==3) storage.copy(NativeUserArea::Data,"Maps/Generated/Generated.map","accepted");
        else if(operation==4) {
          FileInfo output{};files.getFileInfo(AsciiString((root+"/Maps/Generated/Generated.map").c_str()),&output);
        } else {
          FilenameList output;
          files.getFileListInDirectory(AsciiString((root+"/Maps").c_str()),AsciiString("*"),output,TRUE);
        }
      };
      AllocationFault::arm(SIZE_MAX);
      try {run();} catch(...) {AllocationFault::disarm();throw;}
      const auto expected=AllocationFault::attempts();
      AllocationFault::disarm();
      require(expected<128,"complete namespace census bounded before sweep");
      if(operation==3) write(storage,"accepted","accepted");
      bool terminal=false;
      for(std::size_t ordinal=0;ordinal<=expected;++ordinal) {
        const auto baseline=AllocationFault::live(),fds=std::size_t(descriptors());
        bool failed=false;
        AllocationFault::arm(ordinal);
        try {
          run();
        } catch(const std::bad_alloc&) { failed=true; }
        catch(...) {AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        require(AllocationFault::live()==baseline && descriptors()==fds && pool->getUsedBlockCount()==units,
                "user namespace exact std/descriptor/source-pool rollback");
        if(!AllocationFault::triggered()) {
          require(!failed && ordinal==expected && AllocationFault::attempts()==expected,
                  "namespace exact independently calibrated terminal succeeds");
          std::cout<<"namespace operation "<<operation<<" terminal "<<ordinal<<'\n';
          terminal=true;break;
        }
        require(failed,"namespace faults propagate without partial publication");
        if(operation==3)
          require(user.read("data/cnc-generals-zero-hour/accepted")=="accepted" && noTemporary(user.path),
                  "failed copy preserves accepted destination and retires scratch");
        if(operation==3) {
          storage.copy(NativeUserArea::Data,"Maps/Generated/Generated.map","accepted");
          require(user.read("data/cnc-generals-zero-hour/accepted")=="authored-map","copy corrected retry");
          write(storage,"accepted","accepted");
        }
        {OwnedFile retry(storage.openReadFile(NativeUserArea::Data,"Maps/Generated/Generated.map"));
         require(retry && retry->size()==12,"same-owner complete read retry");}
        require(storage.list(NativeUserArea::Data,"Maps",true,3).size()==3,
                "same-owner complete directory retry");
        storage.validateRootOwnership();
      }
      require(terminal,"namespace allocation census bounded");
    }
    return;
  }
  {
    auto entries=storage.list(NativeUserArea::Data,"Maps",true,3);
    require(entries.size()==3 && entries[0].directory && entries[1].bytes==12 && entries[2].bytes==13,
            "bounded directory discovery preserves source spelling and byte sizes");
    require(storage.list(NativeUserArea::Data,"Maps",false,1).size()==1,
            "nonrecursive discovery consumes directory entry only");
    require(storage.list(NativeUserArea::Data,"missing",true,0).empty(),"missing user directories optional");
    rejects([&]{storage.list(NativeUserArea::Data,"Maps",true,2);});
    require(storage.relativeDataPath(root+"/Maps\\Generated\\Generated.map")==
                std::optional<std::string>("Maps/Generated/Generated.map"),
            "captured-root lowering preserves root case and source separators");
    require(!storage.relativeDataPath(root+"-other/map") && !storage.relativeDataPath("/unrelated/map"),
            "outside and sibling prefix cannot become user authority");
    rejects([&]{storage.relativeDataPath(root+"/../map");});
    require(files.doesFileExist((root+"/Maps/Generated/Generated.map").c_str()),
            "original consumer admits only attached user owner");
    {OwnedFile view(files.openFile((root+"/Maps/Generated/Generated.map").c_str(),File::READ|File::BINARY));
     require(view && view->size()==12 && view->getName()==AsciiString((root+"/Maps/Generated/Generated.map").c_str()),
             "original consumer returns exact source File and captured filename");}
    FileInfo metadata{17,18,19,20};
    require(files.getFileInfo(AsciiString((root+"/Maps/Generated/Generated.map").c_str()),&metadata) &&
                metadata.sizeHigh==0 && metadata.sizeLow==12 && metadata.timestampHigh!=19,
            "user FileInfo uses original size and timestamp representation");
    const FileInfo prior=metadata;
    require(!files.getFileInfo(AsciiString((root+"/missing").c_str()),&metadata) &&
                metadata.sizeLow==prior.sizeLow && metadata.timestampLow==prior.timestampLow,
            "missing metadata preserves accepted output");
    FilenameList discovered;
    files.getFileListInDirectory(AsciiString((root+"/Maps").c_str()),AsciiString("*.MAP"),discovered,TRUE);
    require(discovered.size()==1 && *discovered.begin()==AsciiString((root+"/Maps/Generated/Generated.map").c_str()),
            "original user discovery preserves physical spelling and source wildcard case");
    rejects([&]{files.mountReadOnly({assets.path.string()});});
    rejects([&]{files.openFile((root+"-other/map").c_str());});
    rejects([&]{files.openFile((root+"/Maps/Generated/Generated.map").c_str(),File::WRITE);});
    FileSystem unrelated;unrelated.mountReadOnly({assets.path.string()});
    rejects([&]{unrelated.attachUserStorage(&storage);});
    rejects([&]{unrelated.openFile((root+"/Maps/Generated/Generated.map").c_str());});
  }
  const auto fds=descriptors();
  OwnedFile retained;
  {
    NativeUserStorage temporary(user.paths(),files);
    retained.reset(temporary.openReadFile(NativeUserArea::Data,"Maps/Generated/Generated.map"));
  }
  char payload[12]{};
  require(retained && retained->read(payload,12)==12 && std::string(payload,12)=="authored-map",
          "original File view survives storage owner retirement");
  retained.reset();
  require(descriptors()==fds && pool->getUsedBlockCount()==units,"retained view exact teardown");
  require(!storage.openReadFile(NativeUserArea::Data,"missing"),"missing read optional");
  write(storage,"accepted","accepted");
  storage.copy(NativeUserArea::Data,"Maps/Generated/Generated.map","accepted");
  require(user.read("data/cnc-generals-zero-hour/accepted")=="authored-map",
          "source-owned File copy publishes complete destination atomically");
  storage.copy(NativeUserArea::Data,"accepted","accepted");
  require(user.read("data/cnc-generals-zero-hour/accepted")=="authored-map",
          "self copy keeps admitted input alive through replacement");
  require(storage.removeFile(NativeUserArea::Data,"accepted") &&
              !storage.removeFile(NativeUserArea::Data,"accepted"),
          "exact scratch retirement is idempotent");
  rejects([&]{storage.removeFile(NativeUserArea::Data,"Maps");});
  rejects([&]{storage.openReadFile(NativeUserArea::Data,"../escape");});
  rejects([&]{storage.openReadFile(NativeUserArea::Data,"Maps");});
  const auto directory=std::filesystem::path(root)/"Maps/Generated";
  std::ofstream(directory/"MAP.STR")<<"ambiguous";
  rejects([&]{storage.list(NativeUserArea::Data,"Maps",true,4);});
  {
    FilenameList accepted;accepted.insert(AsciiString("accepted"));
    rejects([&]{files.getFileListInDirectory(AsciiString((root+"/Maps").c_str()),AsciiString("*"),accepted,TRUE);});
    require(accepted.size()==1 && *accepted.begin()==AsciiString("accepted"),
            "late ambiguous directory preserves original consumer output");
  }
  std::filesystem::remove(directory/"MAP.STR");
  std::filesystem::create_symlink(directory/"map.str",directory/"link");
  rejects([&]{storage.openReadFile(NativeUserArea::Data,"Maps/Generated/link");});
  rejects([&]{storage.removeFile(NativeUserArea::Data,"Maps/Generated/link");});
  rejects([&]{storage.list(NativeUserArea::Data,"Maps",true,4);});
  std::filesystem::remove(directory/"link");
  require(::mkfifo((directory/"pipe").c_str(),0600)==0,"generated special-file fixture");
  rejects([&]{storage.openReadFile(NativeUserArea::Data,"Maps/Generated/pipe");});
  rejects([&]{storage.list(NativeUserArea::Data,"Maps",true,4);});
  std::filesystem::remove(directory/"pipe");
  {
    const auto oversized=directory/"oversized";
    int descriptor=::open(oversized.c_str(),O_CREAT|O_WRONLY|O_CLOEXEC,0600);
    require(descriptor>=0,"generated oversize owner");
    const int resized=::ftruncate(descriptor,off_t(INT32_MAX)+1);
    ::close(descriptor);require(resized==0,"generated sparse boundary");
    rejects([&]{storage.openReadFile(NativeUserArea::Data,"Maps/Generated/oversized");});
    rejects([&]{storage.list(NativeUserArea::Data,"Maps",true,4);});
    std::filesystem::remove(oversized);
  }
  // A failed backing constructor does not adopt or close the caller's fd.
  int rejected=::open((directory/"map.str").c_str(),O_WRONLY|O_CLOEXEC);
  require(rejected>=0,"generated non-read-only backing");
  rejects([&]{NativeDataBacking candidate(rejected,NativeDataBacking::AdmittedDescriptor{});});
  require(::fcntl(rejected,F_GETFD)>=0,"failed backing retains caller descriptor");
  ::close(rejected);
  require(descriptors()==fds && pool->getUsedBlockCount()==units,"negative namespace exact teardown");
}
void atomic() {
  Tree assets, user;
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(), files);
  const auto target =
      std::filesystem::path("data/cnc-generals-zero-hour/accepted");
  write(storage, "accepted", "accepted");
  const auto fds = descriptors();
  {
    auto candidate = storage.beginWrite(NativeUserArea::Data, "accepted");
    candidate->write("candidate", 9);
    require(user.read(target) == "accepted", "candidate bytes are unpublished");
  }
  require(descriptors() == fds && noTemporary(user.path) &&
              user.read(target) == "accepted",
          "abandonment releases descriptors and candidate only");
  {
    auto candidate = storage.beginWrite(NativeUserArea::Data, "accepted");
    candidate->write("candidate", 9);
    rejects([&] { candidate->write(nullptr, 1); });
    rejects([&] { candidate->commit(); });
  }
  require(noTemporary(user.path) && user.read(target) == "accepted",
          "commit-ready candidate poisoned");
  {
    auto candidate = storage.beginWrite(NativeUserArea::Data, "accepted");
    DataChunkOutput chunk(candidate.get());
    chunk.openDataChunk("Generated", 1);
    chunk.writeInt(17);
    chunk.closeDataChunk();
    chunk.finish();
    require(user.read(target) == "accepted",
            "actual chunk finish does not imply file publication");
    candidate->commit();
    rejects([&] { candidate->write("x", 1); });
    rejects([&] { candidate->commit(); });
  }
  require(user.read(target).starts_with("CkMp") && noTemporary(user.path) &&
              descriptors() == fds,
          "actual original writer atomic publication and teardown");
  write(storage, "accepted", "retry");
  require(user.read(target) == "retry", "fresh corrected writer");
  {
    auto candidate=storage.beginWrite(NativeUserArea::Data,"accepted");
    candidate->write("0000payload",11);
    const auto end=candidate->position();
    candidate->seek(0);candidate->write("0007",4);candidate->seek(end);
    require(candidate->position()==11 && user.read(target)=="retry","backpatch remains offside");
    candidate->commit();
  }
  require(user.read(target)=="0007payload","complete seek/backpatch publication");
  {
    auto candidate=storage.beginWrite(NativeUserArea::Data,"accepted");
    candidate->write("candidate",9);
    rejects([&]{candidate->seek(10);});
    rejects([&]{candidate->commit();});
  }
  require(user.read(target)=="0007payload" && noTemporary(user.path),
          "late-invalid seek poisons commit-ready owner and preserves accepted bytes");
  std::filesystem::create_directory(
      user.path / "data/cnc-generals-zero-hour/directory-target");
  {
    auto candidate =
        storage.beginWrite(NativeUserArea::Data, "directory-target");
    candidate->write("x", 1);
    rejects([&] { candidate->commit(); });
  }
  require(noTemporary(user.path) &&
              std::filesystem::is_directory(
                  user.path / "data/cnc-generals-zero-hour/directory-target"),
          "native failed rename does not replace target or retain candidate");
}
struct FaultIO : NativeStorageIO {
  enum class Phase { None, Open, Write, Seek, FileSync, Publish, DirectorySync };
  Phase phase = Phase::None;
  bool throwing = false, shortWrite = false;
  unsigned syncCount = 0;
  bool fault(Phase selected) {
    if (phase != selected)
      return false;
    if (throwing)
      throw NativeStorageError();
    errno = EIO;
    return true;
  }
  int openFile(int dir, const char *name, int flags, unsigned mode) override {
    if (fault(Phase::Open))
      return -1;
    return NativeStorageIO::openFile(dir, name, flags, mode);
  }
  Int writeFile(int fd, const void *bytes, Int count) override {
    if (fault(Phase::Write))
      return -1;
    return NativeStorageIO::writeFile(
        fd, bytes, shortWrite ? std::min<Int>(count, 2) : count);
  }
  int sync(int fd) override {
    if (fault(++syncCount == 1 ? Phase::FileSync : Phase::DirectorySync))
      return -1;
    return NativeStorageIO::sync(fd);
  }
  std::int64_t seekFile(int fd,std::uint64_t offset) override {
    if(fault(Phase::Seek)) return -1;
    return NativeStorageIO::seekFile(fd,offset);
  }
  void preparePublish() override {
    if (fault(Phase::Publish))
      throw NativeStorageError();
  }
};
void ioFaults() {
  Tree assets, user;
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  FaultIO io;
  NativeUserStorage storage(user.paths(), files, &io);
  const auto target =
      std::filesystem::path("data/cnc-generals-zero-hour/accepted");
  write(storage, "accepted", "accepted");
  for (bool throwing : {false, true})
    for (auto phase :
         {FaultIO::Phase::Open, FaultIO::Phase::Write, FaultIO::Phase::Seek, FaultIO::Phase::FileSync,
          FaultIO::Phase::Publish, FaultIO::Phase::DirectorySync}) {
      const auto baseline = AllocationFault::live(),
                 fds = std::size_t(descriptors());
      io.phase = phase;
      io.throwing = throwing;
      io.syncCount = 0;
      bool rejected = false;
      try {
        auto candidate = storage.beginWrite(NativeUserArea::Data, "accepted");
        try {
          candidate->write("candidate", 9);
          candidate->seek(0);
          const auto result = candidate->commit();
          require(phase == FaultIO::Phase::DirectorySync &&
                      result == NativeCommitResult::PublishedDurabilityUnknown,
                  "only post-publication sync has durability-unknown outcome");
        } catch (const NativeStorageError &) {
          rejected = true;
          rejects([&] { candidate->commit(); });
        }
      } catch (const NativeStorageError &) {
        rejected = true;
      }
      require(rejected == (phase != FaultIO::Phase::DirectorySync),
              "native I/O phase result");
      require(AllocationFault::live() == baseline && descriptors() == fds &&
                  noTemporary(user.path),
              "I/O throw/status exact candidate retirement");
      require(user.read(target) == (phase == FaultIO::Phase::DirectorySync
                                        ? "candidate"
                                        : "accepted"),
              "truthful accepted storage at each fault boundary");
      io.phase = FaultIO::Phase::None;
      io.syncCount = 0;
      write(storage, "accepted", "accepted");
    }
  io.shortWrite = true;
  io.syncCount = 0;
  write(storage, "accepted", "partial writes complete");
  require(user.read(target) == "partial writes complete",
          "native partial-write loop");
  io.shortWrite=false;
  write(storage,"source","source payload");
  {OwnedFile warm(storage.openReadFile(NativeUserArea::Data,"source"));}
  for(bool throwing:{false,true})
    for(auto phase:{FaultIO::Phase::Open,FaultIO::Phase::Write,FaultIO::Phase::FileSync,
                   FaultIO::Phase::Publish,FaultIO::Phase::DirectorySync}) {
      io.phase=FaultIO::Phase::None;io.syncCount=0;write(storage,"accepted","accepted");
      const auto baseline=AllocationFault::live(),fds=std::size_t(descriptors());
      io.phase=phase;io.throwing=throwing;io.syncCount=0;
      bool failed=false;
      try {
        const auto result=storage.copy(NativeUserArea::Data,"source","accepted");
        require(phase==FaultIO::Phase::DirectorySync && result==NativeCommitResult::PublishedDurabilityUnknown,
                "copy reports only post-publication durability uncertainty");
      } catch(const NativeStorageError&) {failed=true;}
      require(failed==(phase!=FaultIO::Phase::DirectorySync) && descriptors()==fds &&
                  AllocationFault::live()==baseline && noTemporary(user.path),
              "copy I/O fault retires all source and candidate owners");
      require(user.read(target)==(failed?"accepted":"source payload"),"copy truthful rollback/publication");
      io.phase=FaultIO::Phase::None;io.syncCount=0;
      storage.copy(NativeUserArea::Data,"source","accepted");
      require(user.read(target)=="source payload","same-owner copy retry after I/O fault");
    }
  const auto scratch=std::filesystem::path("data/cnc-generals-zero-hour/Save/extracted.map");
  for(bool throwing:{false,true})
    for(auto phase:{FaultIO::Phase::Open,FaultIO::Phase::Write,FaultIO::Phase::FileSync,
                   FaultIO::Phase::Publish,FaultIO::Phase::DirectorySync}) {
      io.phase=FaultIO::Phase::None;io.syncCount=0;write(storage,"Save/extracted.map","accepted");
      const auto baseline=AllocationFault::live(),fds=std::size_t(descriptors());
      io.phase=phase;io.throwing=throwing;io.syncCount=0;
      bool failed=false;
      try {
        auto candidate=storage.beginScratchWrite("Save/extracted.map");
        candidate->write("candidate",9);
        const auto result=candidate->commit();
        require(phase==FaultIO::Phase::DirectorySync && result==NativeCommitResult::PublishedDurabilityUnknown,
                "scratch reports post-publication durability uncertainty");
      } catch(const NativeStorageError&) {failed=true;}
      require(failed==(phase!=FaultIO::Phase::DirectorySync) && descriptors()==fds &&
                  AllocationFault::live()==baseline && noTemporary(user.path),
              "scratch I/O faults retain exact retirement ownership");
      require(failed?user.read(scratch)=="accepted":!std::filesystem::exists(user.path/scratch),
              "failed scratch preserves accepted target; published scratch retires exactly");
      io.phase=FaultIO::Phase::None;io.syncCount=0;
      {auto retry=storage.beginScratchWrite("Save/extracted.map");retry->write("retry",5);retry->commit();
       require(user.read(scratch)=="retry","same-owner scratch I/O retry");}
      require(!std::filesystem::exists(user.path/scratch),"corrected scratch retires exact backing");
    }
}
struct DirectoryFileIO : NativeStorageIO {
  int openFile(int directory,const char*,int,unsigned) override {
    return ::fcntl(directory,F_DUPFD_CLOEXEC,0);
  }
};
void scratchFiles(bool faults) {
  Tree assets,user;
  FileSystem files;files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(),files);
  const auto target=std::filesystem::path("data/cnc-generals-zero-hour/Save/extracted.map");
  write(storage,"Save/extracted.map","accepted");
  const auto run=[&] {
    auto candidate=storage.beginScratchWrite("Save/extracted.map");
    candidate->write("scratch",7);candidate->commit();
  };
  if(faults) {
    AllocationFault::arm(SIZE_MAX);
    try {run();} catch(...) {AllocationFault::disarm();throw;}
    const auto expected=AllocationFault::attempts();AllocationFault::disarm();
    require(expected<128,"complete scratch census bounded");
    write(storage,"Save/extracted.map","accepted");
    for(std::size_t ordinal=0;ordinal<=expected;++ordinal) {
      const auto baseline=AllocationFault::live(),fds=std::size_t(descriptors());
      bool failed=false;
      AllocationFault::arm(ordinal);
      try {run();} catch(const std::bad_alloc&) {failed=true;}
      catch(...) {AllocationFault::disarm();throw;}
      AllocationFault::disarm();
      require(AllocationFault::live()==baseline && descriptors()==fds && noTemporary(user.path),
              "scratch exact ownership retirement at every ordinal");
      if(!AllocationFault::triggered()) {
        require(!failed && ordinal==expected && AllocationFault::attempts()==expected &&
                    !std::filesystem::exists(user.path/target),"scratch exact terminal and successful retirement");
        std::cout<<"scratch allocation terminal "<<expected<<'\n';return;
      }
      require(failed && user.read(target)=="accepted","failed scratch cannot retire accepted prior target");
      {auto retry=storage.beginScratchWrite("Save/extracted.map");
       retry->write("retry",5);retry->commit();require(user.read(target)=="retry","same-owner scratch retry");}
      require(!std::filesystem::exists(user.path/target),"retry retains exact scratch lifetime");
      write(storage,"Save/extracted.map","accepted");
    }
    throw std::runtime_error("scratch exact terminal missing");
  }
  const auto baseline=AllocationFault::live(),fds=std::size_t(descriptors());
  {
    auto inactive=storage.beginScratchWrite("Save/extracted.map");
    inactive->write("candidate",9);
  }
  require(user.read(target)=="accepted" && noTemporary(user.path),"inactive scratch preserves accepted target");
  {
    auto prior=storage.beginScratchWrite("Save/extracted.map");
    prior->write("prior",5);prior->commit();
    auto replacement=storage.beginScratchWrite("Save/extracted.map");
    replacement->write("replacement",11);replacement->commit();
    AllocationFault::arm(0);prior.reset();
    const bool allocated=AllocationFault::triggered();AllocationFault::disarm();
    require(!allocated && user.read(target)=="replacement",
            "allocation-free prior retirement cannot delete replacement identity");
    replacement.reset();
  }
  require(!std::filesystem::exists(user.path/target),"current scratch identity removed exactly");
  {
    auto candidate=storage.beginScratchWrite("Save/extracted.map");
    candidate->write("owned",5);candidate->commit();
    write(storage,"Save/extracted.map","unrelated replacement");
    candidate.reset();
    require(user.read(target)=="unrelated replacement","scratch retirement preserves external replacement");
  }
  {
    std::unique_ptr<NativeScratchOutput> retained;
    {
      FileSystem temporaryFiles;temporaryFiles.mountReadOnly({assets.path.string()});
      NativeUserStorage temporary(user.paths(),temporaryFiles);
      temporaryFiles.attachUserStorage(&temporary);
      retained=temporary.beginScratchWrite("Save/extracted.map");
      retained->write("retained",8);retained->commit();
    }
    require(user.read(target)=="retained","scratch record outlives storage and asset provider");
    AllocationFault::arm(0);retained.reset();
    const bool allocated=AllocationFault::triggered();AllocationFault::disarm();
    require(!allocated && !std::filesystem::exists(user.path/target),"captured scratch teardown allocation-free");
  }
  require(AllocationFault::live()==baseline && descriptors()==fds && noTemporary(user.path),
          "scratch lifecycle exact process residuals");
  {
    DirectoryFileIO malformed;
    NativeUserStorage rejected(user.paths(),files,&malformed);
    const auto live=AllocationFault::live(),before=std::size_t(descriptors());
    rejects([&]{rejected.beginScratchWrite("Save/extracted.map");});
    require(AllocationFault::live()==live && descriptors()==before && noTemporary(user.path),
            "failed scratch constructor guards duplicated retirement and backing descriptors");
  }
  // Successful attachment is withdrawn when its storage owner leaves, before
  // the same asset provider is safely remounted for a fresh owner.
  {
    NativeUserStorage temporary(user.paths(),files);files.attachUserStorage(&temporary);
  }
  files.mountReadOnly({assets.path.string()});
}
void mapIdentities(bool faults) {
  Tree assets,user;
  FileSystem files;files.mountReadOnly({assets.path.string()});
  auto paths=NativeUserPaths::resolve(user.path.string(),(user.path/"UpperRoot").string(),
                                     (user.path/"CacheRoot").string());
  NativeUserStorage storage(paths,files);files.attachUserStorage(&storage);
  write(storage,"Maps/Generated/Generated.map","CkMp generated map");
  write(storage,"Maps/Generated/Map.INI","authored definitions");
  write(storage,"Save/Extracted.map","CkMp extracted map");
  std::string identity=paths.data+"/maps/generated/generated.map";
  for(char& byte:identity) if(byte>='A' && byte<='Z') byte=char(byte-'A'+'a');
  const auto expected=paths.data+"/Maps/Generated/Generated.map";
  const auto expectedCompanion=paths.data+"/Maps/Generated/Map.INI";
  const auto run=[&] {
    require(storage.mapIdentityPath(identity)==expected,"folded map identity resolves captured physical owner");
    require(nativeMapCompanionPath(identity,"map.ini",&storage)==expectedCompanion,
            "map companion preserves authored native backing spelling");
  };
  if(faults) {
    AllocationFault::arm(SIZE_MAX);
    try {run();} catch(...) {AllocationFault::disarm();throw;}
    const auto count=AllocationFault::attempts();AllocationFault::disarm();
    require(count<512,"complete map identity census bounded");
    for(std::size_t ordinal=0;ordinal<=count;++ordinal) {
      const auto live=AllocationFault::live(),fds=std::size_t(descriptors());
      bool failed=false;
      AllocationFault::arm(ordinal);
      try {run();} catch(const std::bad_alloc&) {failed=true;}
      catch(...) {AllocationFault::disarm();throw;}
      AllocationFault::disarm();
      require(AllocationFault::live()==live && descriptors()==fds && noTemporary(user.path),
              "map lowering failure preserves owner and exact resources");
      if(!AllocationFault::triggered()) {
        require(!failed && ordinal==count && AllocationFault::attempts()==count,"exact map identity terminal");
        std::cout<<"map identity allocation terminal "<<count<<'\n';return;
      }
      require(failed,"map lowering fault propagates before publication");run();
    }
    throw std::runtime_error("map identity terminal missing");
  }
  run();
  std::string backslashes=identity;
  for(char& byte:backslashes) if(byte=='/') byte='\\';
  require(storage.mapIdentityPath(backslashes)==expected,"original backslash identity retains native root");
  require(!storage.relativeDataPath(identity),"map key lowering does not broaden ordinary absolute admission");
  require(nativePathLeaf("a/b\\map.map")=="map.map" && nativePathLeaf("leaf")=="leaf" &&
              nativePathLeaf("path/").empty(),"source leaf selection supports both separators without null arithmetic");
  require(nativeMapCompanionPath(identity,"map.str",&storage)==paths.data+"/Maps/Generated/map.str",
          "missing optional companion still has captured user owner");
  {
    const auto prior=TheFileSystem;
    TheFileSystem=&files;
    try {
      CachedFileInputStream stream(&storage);
      require(stream.open(AsciiString(identity.c_str())),"actual cached map stream resolves folded identity");
      char bytes[18]{};require(stream.read(bytes,18)==18 && std::string(bytes,18)=="CkMp generated map",
                              "actual source stream preserves map bytes through optional cache");
      std::string saved=paths.data+"/save/extracted.map";
      for(char& byte:saved) if(byte>='A' && byte<='Z') byte=char(byte-'A'+'a');
      require(stream.open(AsciiString(saved.c_str())),"actual saved-map source owner resolves Save identity");
    } catch(...) {TheFileSystem=prior;throw;}
    TheFileSystem=prior;
  }
  for(const auto& invalid:{paths.data+"-other/Maps/map.map",paths.data+"/Maps/../escape.map",
                          paths.data+"/Options.ini",std::string("/unrelated/Maps/map.map")})
    rejects([&]{storage.mapIdentityPath(invalid);});
  rejects([&]{nativeMapCompanionPath(identity,"../escape",&storage);});
  rejects([&]{nativeMapCompanionPath(identity,"map.ini");});
  write(storage,"Maps/Generated/MAP.INI","ambiguous");
  rejects([&]{storage.mapIdentityPath(identity);});
  require(storage.removeFile(NativeUserArea::Data,"Maps/Generated/MAP.INI"),"retire generated collision only");
  run();
}
void allocationFaults() {
  Tree assets, user;
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(), files);
  write(storage, "accepted", "accepted");
  const auto target =
      std::filesystem::path("data/cnc-generals-zero-hour/accepted");
  for (std::size_t ordinal = 0; ordinal < 128; ++ordinal) {
    const auto baseline = AllocationFault::live(),
               fds = std::size_t(descriptors());
    bool failed = false;
    AllocationFault::arm(ordinal);
    try {
      auto candidate = storage.beginWrite(NativeUserArea::Data, "accepted");
      candidate->write("candidate", 9);
      candidate->commit();
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    require(AllocationFault::live() == baseline && descriptors() == fds &&
                noTemporary(user.path),
            "allocation exact candidate retirement");
    if (!failed) {
      require(!AllocationFault::triggered() && ordinal == 9 &&
                  user.read(target) == "candidate",
              "atomic exact terminal");
      std::cout << "atomic allocation ordinals [0," << ordinal << "); terminal "
                << ordinal << '\n';
      return;
    }
    require(user.read(target) == "accepted",
            "allocation preserves accepted file");
    write(storage, "accepted", "candidate");
    require(user.read(target) == "candidate", "atomic corrected retry");
    write(storage, "accepted", "accepted");
  }
  throw std::runtime_error("atomic allocation manifest exceeded");
}
struct AdmissionFixture : NativeCacheAdmission {
  unsigned rejectPhase = 0;
  bool throwing = true;
  bool enter(unsigned phase) {
    if (phase != rejectPhase)
      return true;
    if (throwing)
      throw NativeStorageError();
    return false;
  }
  bool header(std::span<const unsigned char>, std::uint64_t) override {
    return enter(1);
  }
  void beginPayload() override { (void)enter(2); }
  void payload(std::span<const unsigned char>) override { (void)enter(3); }
  bool finishPayload() override { return enter(4); }
};
void cache(bool faults, bool miss = false) {
  Tree assets, user;
  FileSystem files;
  files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage(user.paths(), files);
  const std::vector<unsigned char> source(257, 42);
  const std::vector<unsigned char> expected(257, 43);
  unsigned calls = 0;
  NativeConverter convert = [&](std::span<const unsigned char> input) {
    ++calls;
    std::vector<unsigned char> result(input.begin(), input.end());
    for (auto &value : result)
      ++value;
    return result;
  };
  const auto run = [&](NativeConversionKey key,
                       std::span<const unsigned char> input) {
    return loadOrConvertOriginalData(storage, input, key, 1024, convert);
  };
  auto first = run({1, 1}, source);
  require(!first.hit && first.persisted && first.bytes == expected &&
              calls == 1,
          "missing cache invokes actual converter then publishes");
  first.bytes.clear();
  first.bytes.shrink_to_fit();
  const auto directory = user.path / "cache/cnc-generals-zero-hour";
  const auto file = std::filesystem::directory_iterator(directory)->path();
  require(file.filename() == "06e785cb52f7fa19e2d65c37e7d0fb760716ac5da533af44e"
                             "03a1a0e9256b520.zhc",
          "independent hashlib SHA256 source/kind/version key oracle");
  if (!faults) {
    for (unsigned phase = 1; phase <= 4; ++phase) {
      AdmissionFixture admission;
      admission.rejectPhase = phase;
      const auto baseline = AllocationFault::live(),
                 fds = std::size_t(descriptors());
      rejects([&] {
        storage.readCache(file.filename().string(), 1024, admission);
      });
      require(AllocationFault::live() == baseline && descriptors() == fds,
              "cache admission callback throws retire exact physical owner");
      admission.rejectPhase = 0;
      {
        auto retry =
            storage.readCache(file.filename().string(), 1024, admission);
        require(retry &&
                    retry->size() == expected.size() + NativeCacheHeaderBytes,
                "same physical cache admission retry");
      }
    }
    for (unsigned phase : {1u, 4u}) {
      AdmissionFixture admission;
      admission.rejectPhase = phase;
      admission.throwing = false;
      require(
          !storage.readCache(file.filename().string(), 1024, admission),
          "cache header/payload status rejection before backing publication");
    }
    auto hit = run({1, 1}, source);
    require(hit.hit && hit.bytes == expected && calls == 1,
            "validated cache hit");
    require(!run({1, 2}, source).hit && !run({2, 1}, source).hit,
            "converter version and kind separate cache owners");
    auto changed = source;
    changed[0] = 43;
    auto replacement = run({1, 1}, changed);
    require(!replacement.hit && replacement.bytes[0] == 44,
            "same-length source change invalidates cache identity");
    for (const std::string &corrupt :
         {std::string("ZHC1"), std::string(400, char(0xff))}) {
      std::ofstream output(file, std::ios::binary | std::ios::trunc);
      output << corrupt;
      output.close();
      const auto before = calls;
      auto rebuilt = run({1, 1}, source);
      require(!rebuilt.hit && rebuilt.bytes == expected && calls == before + 1,
              "corrupt cache rebuild");
    }
    {
      std::fstream output(file,
                          std::ios::binary | std::ios::in | std::ios::out);
      output.seekp(84);
      output.put(char(0));
    }
    require(!run({1, 1}, source).hit, "payload checksum rejection");
    for (unsigned offset : {0u, 4u, 8u, 12u, 44u, 52u}) {
      {
        std::fstream output(file,
                            std::ios::binary | std::ios::in | std::ios::out);
        output.seekp(offset);
        output.put(char(0xff));
      }
      require(!run({1, 1}, source).hit,
              "every envelope field is independently validated");
    }
    rejects([&] {
      loadOrConvertOriginalData(storage, source, {1, 1}, 1, convert);
    });
    rejects([&] {
      loadOrConvertOriginalData(storage, source, {0, 1}, 1024, convert);
    });
    require(run({1, 1}, source).hit,
            "converter/bound rejection preserves accepted cache");
    Tree blocked;
    std::ofstream(blocked.path / "not-directory") << "blocked";
    NativeUserStorage unavailable(
        {user.paths().data, (blocked.path / "not-directory").string()}, files);
    unavailable.validateRootOwnership();
    auto memory =
        loadOrConvertOriginalData(unavailable, source, {1, 1}, 1024, convert);
    require(!memory.hit && !memory.persisted && memory.bytes == expected,
            "unwritable cache falls back to memory");
    NativeConverter failure = [](auto) -> std::vector<unsigned char> {
      throw std::runtime_error("generated converter failure");
    };
    rejects([&] {
      loadOrConvertOriginalData(storage, source, {3, 1}, 1024, failure);
    });
    require(noTemporary(user.path),
            "converter rejection has no unpublished files");
    return;
  }
  // Cache-only allocation faults may be handled as misses. Enumerate through
  // the first *untriggered* success, not merely the first successful fallback.
  for (std::size_t ordinal = 0; ordinal < 128; ++ordinal) {
    if (miss) {
      std::ofstream reset(file, std::ios::binary | std::ios::trunc);
      reset << "generated invalid cache";
    }
    const auto baseline = AllocationFault::live(),
               fds = std::size_t(descriptors());
    bool failed = false;
    AllocationFault::arm(ordinal);
    try {
      auto result = run({1, 1}, source);
      require(result.bytes == expected, "cache hit/fallback bytes");
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      AllocationFault::disarm();
      throw;
    }
    AllocationFault::disarm();
    require(AllocationFault::live() == baseline && descriptors() == fds &&
                noTemporary(user.path),
            "cache allocation exact retirement");
    if (!AllocationFault::triggered()) {
      require(!failed && ordinal == (miss ? 21 : 11),
              "untriggered cache exact terminal");
      std::cout << (miss ? "cache miss" : "cache hit")
                << " allocation ordinals [0," << ordinal << "); terminal "
                << ordinal << '\n';
      return;
    }
    require(run({1, 1}, source).bytes == expected, "cache corrected retry");
  }
  throw std::runtime_error("cache allocation manifest exceeded");
}
} // namespace
int main(int argc, char **argv) {
  bool initialized = false;
  try {
    require(argc == 2, "storage family required");
    initMemoryManager();
    initialized = true;
    const std::string family = argv[1];
    for (unsigned repeat = 0; repeat < 3; ++repeat) {
      if (family == "paths")
        paths();
      else if (family == "atomic")
        atomic();
      else if(family=="namespace")
        namespaceFiles(false);
      else if(family=="namespace-faults")
        namespaceFiles(true);
      else if(family=="scratch")
        scratchFiles(false);
      else if(family=="scratch-faults")
        scratchFiles(true);
      else if(family=="map-identities")
        mapIdentities(false);
      else if(family=="map-identity-faults")
        mapIdentities(true);
      else if (family == "io-faults")
        ioFaults();
      else if (family == "allocation-faults")
        allocationFaults();
      else if (family == "cache")
        cache(false);
      else if (family == "cache-faults")
        cache(true);
      else if (family == "cache-miss-faults")
        cache(true, true);
      else
        throw std::runtime_error("unknown storage family");
    }
    shutdownMemoryManager();
    initialized = false;
    std::cout << "PASS: generated original user storage " << family
              << " (GameLogic pending)\n";
    return 0;
  } catch (ErrorCode) {
    std::cerr << "FAIL: generated source storage admission\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
  }
  if (initialized)
    shutdownMemoryManager();
  return 1;
}
