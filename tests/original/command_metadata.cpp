// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "GameClient/ControlBar.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <stdexcept>

namespace {
void require(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
struct Tree {
  std::filesystem::path path;
  Tree(){char name[]="/tmp/zh-command-metadata-XXXXXX";auto* root=mkdtemp(name);require(root,"generated root");path=root;}
  ~Tree(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
  void put(const char* name,const char* contents){auto file=path/name;std::filesystem::create_directories(file.parent_path());
    std::ofstream output(file);output<<contents;output.close();require(bool(output),"generated command metadata");}
};
struct Context {
  Tree tree;FileSystem files;
  FileSystem* previous=TheFileSystem;
  explicit Context(bool invalid=false){
    tree.put("Data/INI/Default/CommandButton.ini","CommandButton BaseStop\n Command = STOP\nEnd\n");
    tree.put("Data/INI/CommandButton.ini","CommandButton BaseMove\n Command = ATTACK_MOVE\nEnd\n");
    tree.put("Data/INI/CommandSet.ini",invalid?"CommandSet InvalidSet\n 1 = UndefinedCommand\nEnd\n":
      "CommandSet BaseSet\n 1 = BaseStop\n 2 = BaseMove\nEnd\n");
    tree.put("override.ini","CommandButton BaseStop\n Command = ATTACK_MOVE\nEnd\n"
      "CommandButton Temporary\n Command = STOP\nEnd\nCommandSet TemporarySet\n 1 = Temporary\nEnd\n"
      "CommandSet BaseSet\n 1 = Temporary\nEnd\n");
    tree.put("bad.ini","CommandSet InvalidSet\n 1 = UndefinedCommand\nEnd\n");
    files.mountReadOnly({tree.path.string()});TheFileSystem=&files;
  }
  ~Context(){TheFileSystem=previous;}
};
struct Owner {
  struct Bar:ControlBar {
    using ControlBar::newCommandButton;
    using ControlBar::newCommandSet;
    using ControlBar::newCommandButtonOverride;
    using ControlBar::findNonConstCommandButton;
  } bar;
  Owner(){TheControlBar=&bar;}
};
// Declare the restoration lease before the owner so source teardown sees itself.
struct Restore {ControlBar* previous=TheControlBar;~Restore(){TheControlBar=previous;}};
struct Usage {
  Int buttons=TheMemoryPoolFactory->findMemoryPool("CommandButton")->getUsedBlockCount();
  Int sets=TheMemoryPoolFactory->findMemoryPool("CommandSet")->getUsedBlockCount();
  bool matches()const{return buttons==TheMemoryPoolFactory->findMemoryPool("CommandButton")->getUsedBlockCount() &&
    sets==TheMemoryPoolFactory->findMemoryPool("CommandSet")->getUsedBlockCount();}
};
void check(ControlBar& bar){
  auto* stop=bar.findCommandButton("BaseStop");auto* move=bar.findCommandButton("BaseMove");auto* set=bar.findCommandSet("BaseSet");
  require(stop && move && set && set->getCommandButton(0)==stop && set->getCommandButton(1)==move,
    "actual command files bind original CommandSet slots to source definitions");
}
void functional(){Restore restore;Owner owner;auto& bar=owner.bar;bar.initCommandMetadata();check(bar);
  bool rejected=false;try{bar.initCommandMetadata();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
  require(rejected,"duplicate logical initialization preserves accepted definitions");check(bar);
  auto* base=bar.findNonConstCommandButton("BaseStop");INI ini;ini.load("override.ini",INI_LOAD_CREATE_OVERRIDES,nullptr);
  auto* changed=bar.findCommandButton("BaseStop");require(changed!=base && bar.findCommandSet("TemporarySet"),"actual parser admits override chain and override-only roots");
  auto* firstSet=bar.findCommandSet("BaseSet");ini.load("override.ini",INI_LOAD_CREATE_OVERRIDES,nullptr);
  require(bar.findCommandSet("BaseSet")!=firstSet && firstSet->getFinalOverride()==bar.findCommandSet("BaseSet"),
    "repeated actual CommandSet parsing appends to the terminal override without orphaning earlier owners");
  changed=bar.findCommandButton("BaseStop");
  // Insert survivors at the head, forcing temporary definitions into the interior.
  auto* survivor=bar.newCommandButton("Survivor");auto* survivorSet=bar.newCommandSet("SurvivorSet");
  rejected=false;try{bar.newCommandButtonOverride(base);}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
  require(rejected && bar.findCommandButton("BaseStop")==changed,"nonterminal override admission preserves its existing chain");
  bar.resetCommandMetadata();bar.resetCommandMetadata();check(bar);
  require(bar.findCommandButton("BaseStop")==base && !bar.findCommandButton("Temporary") && !bar.findCommandSet("TemporarySet") &&
    bar.findCommandButton("Survivor")==survivor && bar.findCommandSet("SurvivorSet")==survivorSet,
    "logical reset relinks interior override-only roots and retains every base survivor");
}
void faults(){
  auto operation=[] {Restore restore;Owner owner;owner.bar.initCommandMetadata();check(owner.bar);};
  operation();operation();AllocationFault::arm(SIZE_MAX);operation();const auto terminal=AllocationFault::attempts();AllocationFault::disarm();
  require(terminal<256,"bounded complete metadata acquisition manifest");
  for(std::size_t ordinal=0;ordinal<=terminal;++ordinal){const auto live=AllocationFault::live();Usage used;
    AllocationFault::arm(ordinal);bool failed=false;try{operation();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}
    const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && hit==failed && (failed || attempts==terminal),"every complete acquisition prefix and exact terminal");
    require(used.matches() && AllocationFault::live()==live,"failed whole metadata owner retires definitions and allocations exactly");
    operation();require(used.matches() && AllocationFault::live()==live,"every failed acquisition has complete corrected retry");
  }
  std::cout<<"metadata allocation prefixes [0,"<<terminal<<"); exact terminal "<<terminal<<"; every retry\n";
}
void pools(){
  for(const char* name:{"CommandButton","CommandSet"}){
    auto* pool=TheMemoryPoolFactory->findMemoryPool(name);void* held[16]{};unsigned count=0;
    struct Free {MemoryPool* pool;void** held;unsigned& count;~Free(){while(count)pool->freeBlock(held[--count]);}}free{pool,held,count};
    while(pool->getFreeBlockCount())held[count++]=pool->allocateBlock("generated command metadata exhaustion");
    Restore restore;Owner owner;Usage used;const auto live=AllocationFault::live();bool failed=false;
    try{owner.bar.initCommandMetadata();}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
    require(failed && used.matches() && AllocationFault::live()==live && !owner.bar.findCommandButton("BaseStop") && !owner.bar.findCommandSet("BaseSet"),
      "typed exhaustion retires complete partial metadata initialization");
    while(count)pool->freeBlock(held[--count]);owner.bar.initCommandMetadata();check(owner.bar);
  }
}
void negative(){
  Context context(true);Restore restore;Owner owner;Usage used;const auto live=AllocationFault::live();bool failed=false;
  try{owner.bar.initCommandMetadata();}catch(const INIException&){failed=true;}catch(ErrorCode){failed=true;}
  require(failed && used.matches() && AllocationFault::live()==live && !owner.bar.findCommandButton("BaseStop") &&
    !owner.bar.findCommandSet("InvalidSet"),"invalid actual command reference retires every preceding parsed definition");
  context.tree.put("Data/INI/CommandSet.ini","CommandSet BaseSet\n 1 = BaseStop\n 2 = BaseMove\nEnd\n");
  context.files.mountReadOnly({context.tree.path.string()});owner.bar.initCommandMetadata();check(owner.bar);
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"command metadata family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool("CommandButton",sizeof(CommandButton),16,0);
    TheMemoryPoolFactory->createMemoryPool("CommandSet",sizeof(CommandSet),16,0);
    {Context context;functional();functional();const auto live=AllocationFault::live();Usage used;
      for(unsigned repeat=0;repeat<3;++repeat){const std::string_view family=argv[1];
        if(family=="functional")functional();else if(family=="faults")faults();else if(family=="pools")pools();else if(family=="negative")negative();else require(false,"unknown family");
        require(!TheControlBar && used.matches() && AllocationFault::live()==live,"repeated source metadata owners retire exactly");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole pool factory retirement");
    std::cout<<"PASS original command metadata; native root remains pending\n";return 0;
  }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';}catch(ErrorCode code){std::cerr<<"FAIL metadata "<<Int(code)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
