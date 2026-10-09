// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/FileOwner.h"
#include "Common/GameMemory.h"
#include "Common/NativeUserStorage.h"
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <unistd.h>
namespace {
void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
template<class Action> void rejects(Action action) {
  bool failed=false;
  try {action();} catch(ErrorCode) {failed=true;} catch(const std::exception&) {failed=true;}
  require(failed,"generated mod operation rejected");
}
struct Tree {
  std::filesystem::path path;
  Tree() {char pattern[]="/tmp/zh-mod-fixture-XXXXXX";const char* p=::mkdtemp(pattern);
    require(p,"generated mod root");path=p;}
  ~Tree() {std::error_code ignored;std::filesystem::remove_all(path,ignored);}
  void write(const std::string& name,const std::string& bytes) const {
    const auto file=path/name;std::filesystem::create_directories(file.parent_path());
    std::ofstream output(file,std::ios::binary);output.write(bytes.data(),bytes.size());
    require(bool(output),"generated mod backing");
  }
  std::string read(const std::string& name) const {
    std::ifstream input(path/name,std::ios::binary);
    require(bool(input),"generated physical read");
    return std::string(std::istreambuf_iterator<char>(input),{});
  }
};
void word(std::string& bytes,std::uint32_t value) {
  for(int shift:{24,16,8,0}) bytes+=char(value>>shift);
}
std::string big(const std::vector<std::pair<std::string,std::string>>& entries) {
  std::uint32_t offset=16;
  for(const auto& [name,bytes]:entries) {(void)bytes;offset+=9+name.size();}
  std::string result="BIGF";word(result,0);word(result,entries.size());word(result,offset);
  for(const auto& [name,bytes]:entries) {
    word(result,offset);word(result,bytes.size());result+=name;result+='\0';offset+=bytes.size();
  }
  for(const auto& [name,bytes]:entries) {(void)name;result+=bytes;}
  return result;
}
std::string content(FileSystem& files,const char* name) {
  FileCloseOwner file(files.openFile(name));require(bool(file),"actual mod file reachability");
  std::string result(file->size(),'\0');
  require(file->read(result.data(),result.size())==Int(result.size()),"complete actual mod read");
  return result;
}
std::size_t descriptors() {
  DIR* directory=::opendir("/proc/self/fd");require(directory,"generated fd census");
  std::size_t count=0;while(::readdir(directory)) ++count;::closedir(directory);return count;
}
struct Inputs {
  Tree base,user,mods;
  std::string selected,directory;
  NativeUserPaths paths;
  Inputs():paths{(user.path/"data").string(),(user.path/"cache").string()} {
    base.write("base.big",big({{"Data/Value.ini","base"},{"Data/Loose.ini","archive"}}));
    base.write("Data/Loose.ini","loose");
    mods.write("selected.big",big({{"Data/Value.ini","selected"},{"Data/Loose.ini","mod"}}));
    mods.write("directory/a.big",big({{"Data/Value.ini","first-directory"}}));
    mods.write("directory/nested/z.BIG",big({{"Data/Value.ini","last-directory"}}));
    mods.write("directory/Data/Loose.ini","directory-loose-is-not-mounted");
    selected=(mods.path/"selected.big").string();directory=(mods.path/"directory").string();
  }
};
void precedence() {
  Inputs input;FileSystem files;files.mountReadOnly({input.base.path.string()});
  NativeUserStorage storage(input.paths,files);files.attachUserStorage(&storage);
  files.mountReadOnlyMods(input.selected,{});
  require(content(files,"Data/Value.ini")=="selected","explicit archive overrides base");
  require(content(files,"Data/Loose.ini")=="loose","original loose precedence retained");
  files.mountReadOnlyMods(input.selected,input.directory);
  require(content(files,"Data/Value.ini")=="last-directory","recursive sorted directory last wins");
  require(content(files,"Data/Loose.ini")=="loose","mod directory adds archives only");
  const auto live=AllocationFault::live(),fds=descriptors();
  for(unsigned repeat=0;repeat<100;++repeat) {
    files.mountReadOnlyMods(input.selected,input.directory);
    require(AllocationFault::live()==live && descriptors()==fds,"repeated admitted mounts do not grow backing");
    require(content(files,"Data/Value.ini")=="last-directory","repeated exact namespace selection");
  }
  input.mods.write("broken.big","BIGF");
  for(const auto& bad:{(input.mods.path/"missing.big").string(),(input.mods.path/"broken.big").string(),
      input.selected+std::string("\0suffix",7)}) {
    rejects([&]{files.mountReadOnlyMods(bad,{});});
    require(content(files,"Data/Value.ini")=="last-directory","failed mod retains accepted namespace");
  }
  rejects([&]{files.mountReadOnlyMods(input.selected,(input.user.path).string());});
  require(content(files,"Data/Value.ini")=="last-directory","storage root alias rejects offside");
  auto output=storage.beginWrite(NativeUserArea::Data,"Options.ini");output->write("ok",2);output->commit();
  rejects([&]{files.mountReadOnlyMods(input.selected,{});});output.reset();
  files.mountReadOnlyMods(input.selected,{});
}
void protection() {
  Inputs input;FileSystem files;files.mountReadOnly({input.base.path.string()});
  NativeUserStorage storage(input.paths,files);files.attachUserStorage(&storage);
  const auto bytes=big({{"Data/Value.ini","user-mod"}});
  input.user.write("data/Mods/selected.big",bytes);
  const auto mod=(input.user.path/"data/Mods/selected.big").string();
  auto output=storage.beginWrite(NativeUserArea::Data,"Mods/selected.big");
  output->write("replacement",11);
  rejects([&]{files.mountReadOnlyMods(mod,{});});output.reset();
  auto scratch=storage.beginScratchWrite("Scratch.ini");scratch->write("scratch",7);scratch->commit();
  rejects([&]{files.mountReadOnlyMods(mod,{});});scratch.reset();
  files.mountReadOnlyMods(mod,{});
  require(content(files,"Data/Value.ini")=="user-mod","user-provided mod admission");
  rejects([&]{storage.beginWrite(NativeUserArea::Data,"Mods/selected.big");});
  rejects([&]{storage.removeFile(NativeUserArea::Data,"Mods/selected.big");});
  auto ordinary=storage.beginWrite(NativeUserArea::Data,"Options.ini");ordinary->write("ok",2);
  ordinary->commit();ordinary.reset();
  rejects([&]{storage.copy(NativeUserArea::Data,"Options.ini","Mods/selected.big");});
  require(input.user.read("data/Mods/selected.big")==bytes,"asset mod unchanged by every mutating path");
  input.user.write("data/Directory/a.big",bytes);
  const auto directory=(input.user.path/"data/Directory").string();
  files.mountReadOnlyMods({},directory);
  rejects([&]{storage.beginWrite(NativeUserArea::Data,"Directory/new-file");});
  rejects([&]{storage.removeFile(NativeUserArea::Data,"Directory/a.big");});
  rejects([&]{storage.ensureDirectory(NativeUserArea::Data,"Directory/new-directory");});
  require(input.user.read("data/Directory/a.big")==bytes,"complete mod directory read-only");
  rejects([&]{files.mountReadOnly({input.base.path.string()});});
  {
    auto pending=storage.beginWrite(NativeUserArea::Data,"Pending.ini");
    files.attachUserStorage(nullptr);
    rejects([&]{files.mountReadOnly({input.base.path.string()});});
    files.attachUserStorage(&storage);
  }
  std::unique_ptr<NativeScratchOutput> retired;
  {
    FileSystem other;other.mountReadOnly({input.base.path.string()});
    NativeUserStorage captured(input.paths,other);
    retired=captured.beginScratchWrite("RetiredOwner.ini");retired->write("captured",8);retired->commit();
  }
  require(input.user.read("data/RetiredOwner.ini")=="captured","scratch survives complete namespace owner retirement");
  retired.reset();
  require(!std::filesystem::exists(input.user.path/"data/RetiredOwner.ini"),"captured scratch cleanup needs no retired owner callback");
}
void faults() {
  Inputs input;std::size_t terminal=0;
  {
    FileSystem files;files.mountReadOnly({input.base.path.string()});
    NativeUserStorage storage(input.paths,files);files.attachUserStorage(&storage);
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try {files.mountReadOnlyMods(input.selected,input.directory);} catch(...) {AllocationFault::disarm();throw;}
    terminal=AllocationFault::attempts();AllocationFault::disarm();
  }
  require(terminal>0 && terminal<512,"bounded complete mod manifest");
  const auto outer=AllocationFault::live(),outerFD=descriptors();
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal) {
    {
      FileSystem files;files.mountReadOnly({input.base.path.string()});
      NativeUserStorage storage(input.paths,files);files.attachUserStorage(&storage);
      const auto live=AllocationFault::live(),fds=descriptors();bool failed=false;
      AllocationFault::arm(ordinal);
      try {files.mountReadOnlyMods(input.selected,input.directory);}
      catch(const std::bad_alloc&) {failed=true;}
      catch(...) {AllocationFault::disarm();throw;}
      AllocationFault::disarm();
      require(failed==(ordinal<terminal) && AllocationFault::triggered()==failed,"exact mod terminal");
      if(failed) {
        require(AllocationFault::live()==live && descriptors()==fds,"offside failure retires all backing");
        require(content(files,"Data/Value.ini")=="base","failure retains original index");
        files.mountReadOnlyMods(input.selected,input.directory);
      }
      require(content(files,"Data/Value.ini")=="last-directory","each corrected mod retry");
    }
    require(AllocationFault::live()==outer && descriptors()==outerFD,"whole namespace owner retires exactly");
  }
  std::cout<<"mod allocation ordinals [0,"<<terminal<<"); terminal "<<terminal<<'\n';
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"generated mod family required");initMemoryManager();initialized=true;
    // Retire discovery and warm original File pools before ownership baselines.
    {Inputs input;FileSystem files;files.mountReadOnly({input.base.path.string()});
      require(content(files,"Data/Value.ini")=="base","original pooled reader warmed");}
    const auto live=AllocationFault::live(),fds=descriptors();
    for(unsigned repeat=0;repeat<3;++repeat) {
      const std::string family=argv[1];
      if(family=="precedence") precedence();else if(family=="protection") protection();
      else if(family=="faults") faults();else throw std::runtime_error("unknown mod family");
      require(AllocationFault::live()==live && descriptors()==fds,"repeat namespace exact retirement");
    }
    shutdownMemoryManager();initialized=false;std::cout<<"PASS: actual native mod namespace (startup pending)\n";
    return 0;
  } catch(ErrorCode) {std::cerr<<"FAIL: generated mod admission\n";}
  catch(const std::exception& error) {std::cerr<<"FAIL: "<<error.what()<<'\n';}
  AllocationFault::disarm();if(initialized) shutdownMemoryManager();return 1;
}
