// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original FunctionLexicon with generated metadata, not GUI acceptance.
#include "AllocationFault.h"
#include "Common/FunctionLexicon.h"
#include "Common/GameMemory.h"
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
using Entry=NativeFunctionEntry;
using Index=NativeFunctionTableIndex;
void require(bool value,const char* message){if(!value) throw std::runtime_error(message);}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode e){rejected=e==ERROR_BAD_ARG;}
  require(rejected,"actual lexicon invalid input rejects");}
unsigned calls=0;
WindowMsgHandledType message(GameWindow*,UnsignedInt a,WindowMsgData b,WindowMsgData c){
  require(a==17 && b==29 && c==97,"full-width original callback arguments");++calls;return MSG_HANDLED;}
void draw(GameWindow*,WinInstanceData*){++calls;}
void deviceDraw(GameWindow*,WinInstanceData*){calls+=10;}
void tooltip(GameWindow*,WinInstanceData*,UnsignedInt value){require(value==97,"tooltip argument");++calls;}
void layout(WindowLayout*,void*){++calls;}
void deviceLayout(WindowLayout*,void*){calls+=10;}
void priorLayout(WindowLayout*,void*){calls+=100;}
class Exposed:public FunctionLexicon {
public:
  using FunctionLexicon::FunctionLexicon;
  void device(std::span<Entry> d,std::span<Entry> l){initWithDeviceTables(d,l);}
  void add(std::span<Entry> entries,Index index){loadTable(entries,index);}
};
struct Context {
  NameKeyGenerator names;
  NameKeyGenerator* priorNames=TheNameKeyGenerator;
  FunctionLexicon* priorLexicon=TheFunctionLexicon;
  Entry system[2]{{NAMEKEY_INVALID,"GeneratedLongSystemCallbackName",message},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry d[2]{{NAMEKEY_INVALID,"GeneratedLongDrawCallbackName",draw},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry tip[2]{{NAMEKEY_INVALID,"GeneratedLongTooltipCallbackName",tooltip},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry l[2]{{NAMEKEY_INVALID,"GeneratedLongLayoutCallbackName",layout},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry dd[2]{{NAMEKEY_INVALID,"GeneratedLongDrawCallbackName",deviceDraw},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry dl[2]{{NAMEKEY_INVALID,"GeneratedLongLayoutCallbackName",deviceLayout},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry prior[2]{{NAMEKEY_INVALID,"Prior",priorLayout},{NAMEKEY_INVALID,nullptr,nullptr}};
  NativeFunctionTable sources[4]{{system,Index::TABLE_GAME_WIN_SYSTEM},{d,Index::TABLE_GAME_WIN_DRAW},
      {tip,Index::TABLE_GAME_WIN_TOOLTIP},{l,Index::TABLE_WIN_LAYOUT_INIT}};
  Exposed owner{sources};
  Context(){names.init();TheNameKeyGenerator=&names;TheFunctionLexicon=&owner;}
  ~Context(){TheFunctionLexicon=priorLexicon;TheNameKeyGenerator=priorNames;}
  void prepare(bool warm){owner.add(prior,Index::TABLE_WIN_LAYOUT_INIT);
    if(warm) for(auto* table:{system,d,tip,l}) names.nameToKey(table[0].name);}
  void run(bool device){if(device)owner.device(dd,dl);else owner.init();}
  void check(){require(owner.gameWinSystemFunc(system[0].key)==message &&
    owner.gameWinDrawFunc(d[0].key,Index::TABLE_GAME_WIN_DRAW)==draw &&
    owner.gameWinTooltipFunc(tip[0].key)==tooltip &&
    owner.winLayoutInitFunc(l[0].key,Index::TABLE_WIN_LAYOUT_INIT)==layout,"actual typed table lookups");}
};
void functional(){
  Context context;
  // Descriptor backing may retire/change after capture; entry backing remains borrowed.
  context.sources[0]={};context.owner.init();context.check();require(context.owner.validate(),"unique source callback identities");
  calls=0;context.owner.gameWinSystemFunc(context.system[0].key)(nullptr,17,29,97);
  context.owner.gameWinTooltipFunc(context.tip[0].key)(nullptr,nullptr,97);
  require(calls==2,"actual source dispatch executes captured metadata");
  context.owner.device(context.dd,context.dl);
  require(context.owner.gameWinDrawFunc(context.d[0].key)==deviceDraw &&
      context.owner.winLayoutInitFunc(context.l[0].key)==deviceLayout,"device-first table selection by shared source name key");
  calls=0;context.owner.gameWinDrawFunc(context.d[0].key)(nullptr,nullptr);
  context.owner.winLayoutInitFunc(context.l[0].key)(nullptr,nullptr);require(calls==20,"real device-family dispatch");
  context.owner.reset();context.check();require(context.owner.getTable(Index::TABLE_GAME_WIN_DEVICEDRAW)==context.dd,
      "source subset reset preserves unrelated admitted table links");
  require(!context.owner.gameWinDrawFunc(NAMEKEY_INVALID) &&
      !context.owner.gameWinSystemFunc(context.d[0].key,Index::TABLE_GAME_WIN_DRAW),"absent/wrong-prototype callback cannot dispatch");
  Exposed headless(std::span<const NativeFunctionTable>{});headless.init();headless.reset();headless.update();
  require(headless.validate() && !headless.getTable(Index::TABLE_GAME_WIN_SYSTEM) &&
      !headless.gameWinDrawFunc(context.d[0].key),"explicit empty metadata makes no GUI initialization claim");
}
void negative(){
  Context context;context.prepare(false);const auto key=context.prior[0].key;
  TheNameKeyGenerator=nullptr;rejects([&]{context.owner.reset();});TheNameKeyGenerator=&context.names;
  require(context.owner.winLayoutInitFunc(key)==priorLayout,"missing service preserves admitted owner");
  rejects([&]{context.owner.device(context.dd,{});});
  context.d[1].name="invalid sentinel";rejects([&]{context.owner.reset();});context.d[1].name=nullptr;
  context.d[0].func=layout;rejects([&]{context.owner.reset();});context.d[0].func=draw;
  context.system[0].name="";rejects([&]{context.owner.reset();});context.system[0].name="GeneratedLongSystemCallbackName";
  require(context.owner.winLayoutInitFunc(key)==priorLayout && context.system[0].key==NAMEKEY_INVALID &&
      context.d[0].key==NAMEKEY_INVALID && context.l[0].key==NAMEKEY_INVALID,
      "late candidate validation preserves accepted tables and all unpublished keys");
  std::array<NativeFunctionTable,10> excessive{};rejects([&]{Exposed bad(excessive);});
  for(Int raw:{-2,-1,9,INT32_MIN,INT32_MAX}){
    NativeFunctionTable invalid{context.system,static_cast<Index>(raw)};Exposed bad(std::span(&invalid,1));
    rejects([&]{bad.init();});}
  const NativeFunctionTable duplicate[]{context.sources[0],context.sources[0]};Exposed bad(duplicate);rejects([&]{bad.init();});
  std::array<NativeFunctionTable,8> full{};Exposed bounded(full);rejects([&]{bounded.device(context.dd,context.dl);});
  context.owner.reset();context.check();
}
void faults(bool warm,bool device){
  std::size_t census=0;
  {Context discovery;discovery.prepare(warm);AllocationFault::arm(SIZE_MAX);
    try{discovery.run(device);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<128,"bounded actual lexicon manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){
    const auto outside=AllocationFault::live();
    {Context context;context.prepare(warm);const auto accepted=AllocationFault::live();
      auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");require(pool,"source name pool");
      const auto blocks=pool->getUsedBlockCount();const auto key=context.prior[0].key;
      bool failed=false;AllocationFault::arm(ordinal);
      try{context.run(device);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      AllocationFault::disarm();
      require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
          (failed || AllocationFault::attempts()==census),"each allocation failure/retry pair and exact terminal");
      if(failed){
        require(context.owner.winLayoutInitFunc(key)==priorLayout && context.l[0].key==NAMEKEY_INVALID &&
            context.system[0].key==NAMEKEY_INVALID && !context.owner.getTable(Index::TABLE_GAME_WIN_SYSTEM) &&
            TheFunctionLexicon==&context.owner,"all entry/table/global publication survives failure");
        unsigned interned=0;
        if(!warm) for(Int value=key+1;value<key+5;++value)
          if(!context.names.keyToName(static_cast<NameKeyType>(value)).isEmpty())++interned;
        require(AllocationFault::live()==accepted+interned && pool->getUsedBlockCount()==blocks+Int(interned),
            "cold residual is only accepted monotonic names; warm failure retires exactly");
        context.run(device);
      }
      context.check();if(device) require(context.owner.gameWinDrawFunc(context.d[0].key)==deviceDraw,"same-owner full device retry");
    }
    require(AllocationFault::live()==outside,"complete original lexicon/name ownership retirement");
  }
  std::cout<<"actual lexicon warm "<<warm<<" device "<<device<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){bool initialized=false;
  try{require(argc==2,"lexicon family required");initMemoryManager();initialized=true;
    {Context warm;warm.owner.init();}
    const std::string family=argv[1];
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      if(family=="functional")functional();else if(family=="negative")negative();
      else if(family=="fault-warm")faults(true,false);else if(family=="fault-cold")faults(false,false);
      else if(family=="fault-device")faults(false,true);else throw std::runtime_error("unknown lexicon family");
      require(AllocationFault::live()==live && TheFunctionLexicon==nullptr && TheNameKeyGenerator==nullptr,
          "repeated source owners withdraw borrowed publication and retire backing");}
    shutdownMemoryManager();initialized=false;std::cout<<"PASS actual lexicon owner; GUI/provider execution pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL actual lexicon admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
