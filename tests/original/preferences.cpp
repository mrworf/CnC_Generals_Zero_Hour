// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/AsciiString.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/NativeUserStorage.h"
#include "Common/UserPreferences.h"
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejects(F action) {
  bool rejected=false; try { action(); } catch (const NativeStorageError&) { rejected=true; }
  require(rejected, "preference input rejected");
}
struct Tree {
  std::filesystem::path path;
  Tree() { char name[]="/tmp/zh-preferences-XXXXXX"; auto* p=::mkdtemp(name); require(p,"generated tree"); path=p; }
  ~Tree() { std::error_code ignored; std::filesystem::remove_all(path,ignored); }
};
unsigned descriptors() {
  auto* d=::opendir("/proc/self/fd"); require(d,"fd census"); unsigned n=0;
  while (::readdir(d)) ++n; ::closedir(d); return n;
}
struct FaultIO : NativeStorageIO {
  bool failWrite=false, failPublish=false;
  Int writeFile(int fd,const void* bytes,Int count) override {
    if (failWrite) throw NativeStorageError();
    return NativeStorageIO::writeFile(fd,bytes,count);
  }
  void preparePublish() override { if (failPublish) throw NativeStorageError(); }
};
struct Context {
  Tree assets,user; FileSystem files; FaultIO io;
  NativeUserStorage storage;
  Context() : storage(NativeUserPaths::resolve(user.path.string(),
       (user.path/"data").string(),(user.path/"cache").string()),files,&io) {
    files.mountReadOnly({assets.path.string()});
  }
  void put(const char* leaf,const std::string& text) {
    auto output=storage.beginWrite(NativeUserArea::Data,leaf);
    output->write(text.data(),static_cast<Int>(text.size())); output->commit();
  }
  std::string get(const char* leaf) {
    auto bytes=storage.readFile(NativeUserArea::Data,leaf,INT32_MAX); require(bytes.has_value(),"generated file present");
    return std::string(bytes->begin(),bytes->end());
  }
  std::filesystem::path file(const char* leaf) { return user.path/"data/cnc-generals-zero-hour"/leaf; }
  bool noTemporary() {
    for (const auto& entry:std::filesystem::recursive_directory_iterator(user.path))
      if (entry.path().filename().string().starts_with(".zh-write-")) return false;
    return true;
  }
};
void parsing() {
  Context c; UserPreferences prefs(&c.storage);
  require(!prefs.load("Options.ini") && prefs.empty(),"first-boot missing directory is accepted");
  prefs.setInt("Retained",7); require(prefs.write(),"first-boot writable leaf retained");
  c.put("Options.ini"," Blank \n = skipped\nEmpty = \n Name = original\r\nName = latest\n"
         "Flag = YES\nNumber = +42suffix\nReal = 1.25suffix\nEquals = a=b\n");
  require(prefs.load("Options.ini") && prefs.getInt("Retained",0)==7,"load overlays prior map");
  require(prefs.getAsciiString("Name","")=="latest" && prefs.getBool("Flag",false) &&
          prefs.getInt("Number",0)==42 && prefs.getReal("Real",0)==1.25f &&
          prefs.getAsciiString("Equals","")=="a=b" && prefs.count("Empty")==0,
          "original trim, duplicate-last, decimal-prefix and first-equals behavior");
  for (const char* value:{"2147483648","-2147483649","999999999999999999999999"}) {
    prefs.setAsciiString("Number",value); require(prefs.getInt("Number",91)==91,"integer overflow fallback defined");
  }
  prefs.setAsciiString("Number","junk"); require(prefs.getInt("Number",91)==0,"legacy malformed numeric zero");
  for (const char* value:{"nan","inf","1e999"}) {
    prefs.setAsciiString("Real",value); require(prefs.getReal("Real",91)==91,"finite preference boundary");
  }
  rejects([&]{prefs.setReal("New",std::numeric_limits<Real>::infinity());});
  require(!prefs.count("New"),"invalid real cannot insert key");
  c.put("Bad.ini",std::string("Name = bad\n\0tail",17));
  const auto n=prefs.size(); rejects([&]{prefs.load("Bad.ini");});
  c.put("Bad.ini","Name = bad\n"+std::string(AsciiString::MAX_LEN+1,'x'));
  rejects([&]{prefs.load("Bad.ini");});
  require(prefs.size()==n && prefs.getAsciiString("Name","")=="latest","late bad row preserves complete prior map");
  for (const char* name:{"","/absolute","../bad","a/b","a\\b","C:bad",".",".."})
    rejects([&]{prefs.load(name);});
  require(prefs.write() && c.get("Options.ini").find("Name = latest\n")!=std::string::npos,
          "failed loads preserve previous output target");
  auto moved=AsciiString("owned"); const auto* backing=moved.str();
  AsciiString transferred(std::move(moved)); require(transferred.str()==backing && moved.isEmpty(),"move transfers exact COW ownership");
  transferred=std::move(transferred); require(transferred.str()==backing,"self move preserves owner");
  AsciiString alias(transferred),other("other"); transferred.swap(other);
  require(alias.str()==backing && other.str()==backing && transferred=="other","swap retains aliases without refcount admission");
}
void persistence() {
  Context c; UserPreferences prefs(&c.storage); require(!prefs.load("Options.ini"),"empty owner prepared");
  prefs.setAsciiString("Name","accepted"); prefs.write();
  const auto accepted=c.get("Options.ini"); const auto fds=descriptors();
  prefs.setAsciiString("Name","candidate");
  for (int phase=0;phase<2;++phase) {
    c.io.failWrite=phase==0; c.io.failPublish=phase==1;
    rejects([&]{prefs.write();}); c.io.failWrite=c.io.failPublish=false;
    require(c.get("Options.ini")==accepted && descriptors()==fds && c.noTemporary(),"write/publish faults preserve complete accepted file");
  }
  prefs.write(); require(c.get("Options.ini")=="Name = candidate\n","same-owner write retry");
  UserPreferences reload(&c.storage); require(reload.load("Options.ini") && reload.getAsciiString("Name","")=="candidate","actual owner persistence roundtrip");
  for (auto pair:{std::pair{"Name\nBad","value"},std::pair{"Name=Bad","value"},std::pair{"Name","bad\rvalue"}}) {
    UserPreferences invalid(&c.storage); invalid.load("Options.ini"); invalid.setAsciiString(pair.first,pair.second);
    rejects([&]{invalid.write();}); require(c.get("Options.ini")=="Name = candidate\n","serialization rejection before file mutation");
  }
  require(!c.storage.readFile(NativeUserArea::Data,"missing",100),"missing leaf");
  reload.setAsciiString("Unset",""); reload.write();
  require(c.get("Options.ini").find("Unset = \n")!=std::string::npos,
          "original unset fields serialize and remain ignored on reload");
  rejects([&]{c.storage.readFile(NativeUserArea::Data,"Options.ini",1);});
  std::filesystem::create_symlink(c.file("Options.ini"),c.file("Alias.ini"));
  rejects([&]{reload.load("Alias.ini");});
  require(::mkfifo(c.file("Pipe.ini").c_str(),0600)==0,"generated fifo");
  rejects([&]{reload.load("Pipe.ini");}); // nonblocking admission, never hang
  NativeUserStorage forbidden(NativeUserPaths::resolve(c.user.path.string(),c.assets.path.string(),c.assets.path.string()),c.files);
  rejects([&]{forbidden.readFile(NativeUserArea::Data,"Options.ini",100);});
  require(std::filesystem::is_empty(c.assets.path),"read/write cannot alter supplied asset root");
}
void faults(bool writing) {
  Context c; c.put("Previous.ini","Name = accepted\n"); c.put("Target.ini","Name = candidate\nOther = added\n");
  const AsciiString target("Target.ini"); std::size_t terminal=0;
  for (std::size_t ordinal=0;ordinal<256;++ordinal) {
    UserPreferences prefs(&c.storage); prefs.load("Previous.ini");
    if (writing) prefs.setAsciiString("Name","candidate");
    const auto baseline=AllocationFault::live(); const auto fds=descriptors(); bool failed=false;
    AllocationFault::arm(ordinal);
    try { if (writing) prefs.write(); else prefs.load(target); }
    catch(const std::bad_alloc&) { failed=true; }
    catch(...) { AllocationFault::disarm(); throw; }
    AllocationFault::disarm();
    require(descriptors()==fds && c.noTemporary(),"every injected ordinal retires descriptors/temporary files");
    if (!AllocationFault::triggered()) { require(!failed,"untriggered terminal accepts"); terminal=ordinal; break; }
    require(failed && AllocationFault::live()==baseline,"injected allocation rejected with exact owner residuals");
    if (writing) {
      require(c.get("Previous.ini")=="Name = accepted\n","failed serialization preserves file");
      prefs.write(); require(c.get("Previous.ini")=="Name = candidate\n","corrected write retry");
      c.put("Previous.ini","Name = accepted\n");
    } else {
      require(prefs.size()==1 && prefs.getAsciiString("Name","")=="accepted","late parse fault preserves previous map");
      prefs.write(); require(c.get("Previous.ini")=="Name = accepted\n","late parse fault preserves target identity");
      require(prefs.load(target) && prefs.getAsciiString("Name","")=="candidate" && prefs.size()==2,"same-owner load retry");
    }
  }
  require(terminal==(writing?9u:19u),"complete disjoint allocation manifest and exact terminal");
  std::cout << (writing?"write":"load") << " allocation ordinals [0," << terminal << "); terminal " << terminal << '\n';
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"preference family"); initMemoryManager(); initialized=true;
    const std::string family(argv[1]);
    for (int repeat=0;repeat<3;++repeat) {
      const auto baseline=AllocationFault::live();
      if (family=="parsing") parsing(); else if (family=="persistence") persistence();
      else if (family=="fault-load") faults(false); else if (family=="fault-write") faults(true);
      else throw std::runtime_error("unknown family");
      require(AllocationFault::live()==baseline,"same-process owner teardown");
    }
    shutdownMemoryManager(); initialized=false; std::cout << "PASS generated original preferences (GameLogic pending)\n"; return 0;
  } catch(const std::exception& e) { std::cerr << "FAIL " << e.what() << '\n'; }
    catch(ErrorCode) { std::cerr << "FAIL source error\n"; }
  AllocationFault::disarm(); if (initialized) shutdownMemoryManager(); return 1;
}
