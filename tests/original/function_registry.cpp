// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/NativeFunctionRegistry.h"
#include <iostream>
namespace {
using Index=NativeFunctionTableIndex;
using Entry=NativeFunctionEntry;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejects(F action) {
  bool rejected=false; try { action(); } catch (ErrorCode error) { rejected=error==ERROR_BAD_ARG; }
  require(rejected,"registry rejection");
}
unsigned calls=0; void* received=nullptr;
WindowMsgHandledType system(GameWindow*,UnsignedInt msg,WindowMsgData a,WindowMsgData b) {
  require(msg==17 && a==WindowMsgData(29),"actual source message arguments");
  received=reinterpret_cast<void*>(b); ++calls; return MSG_HANDLED;
}
void draw(GameWindow*,WinInstanceData*) { ++calls; }
void tooltip(GameWindow*,WinInstanceData*,UnsignedInt time) { require(time==97,"tooltip argument"); ++calls; }
void layout(WindowLayout*,void* data) { received=data; ++calls; }
void layoutOther(WindowLayout*,void*) { ++calls; }
// Compilation proves the real typed representation supports constant table
// initialization; no dynamic startup callback acquisitions are required.
constinit const Entry staticCallbacks[]{
  {NAMEKEY_INVALID,"System",system}, {NAMEKEY_INVALID,"Draw",draw},
  {NAMEKEY_INVALID,"Tooltip",tooltip}, {NAMEKEY_INVALID,"Layout",layout},
  {NAMEKEY_INVALID,nullptr,nullptr}};
void functional() {
  require(staticCallbacks[0].func.get<GameWinSystemFunc>()==system &&
      staticCallbacks[1].func.get<GameWinDrawFunc>()==draw &&
      staticCallbacks[2].func.get<GameWinTooltipFunc>()==tooltip &&
      staticCallbacks[3].func.get<WindowLayoutInitFunc>()==layout &&
      !staticCallbacks[4].func.get<GameWinSystemFunc>(),"constant tables preserve every typed pointer and sentinel");
  NameKeyGenerator names; names.init(); NativeFunctionRegistry registry;
  Entry s[]{{NAMEKEY_INVALID,"System",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry d[]{{NAMEKEY_INVALID,"Draw",draw},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry t[]{{NAMEKEY_INVALID,"Tooltip",tooltip},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry l[]{{NAMEKEY_INVALID,"Layout",layout},{NAMEKEY_INVALID,"Absent",nullptr},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable batch[]{{s,Index::TABLE_GAME_WIN_SYSTEM},{d,Index::TABLE_GAME_WIN_DRAW},
      {t,Index::TABLE_GAME_WIN_TOOLTIP},{l,Index::TABLE_WIN_LAYOUT_INIT}};
  registry.loadTables(batch,names); require(registry.validate(),"distinct source prototypes/identities");
  int data=71; calls=0; received=nullptr;
  auto msg=registry.find(s[0].key,Index::TABLE_ANY).get<GameWinInputFunc>();
  require(msg && msg(nullptr,17,29,reinterpret_cast<WindowMsgData>(&data))==MSG_HANDLED && received==&data,"full-width callback payload and shared System/Input protocol");
  registry.find(d[0].key,Index::TABLE_GAME_WIN_DRAW).get<GameWinDrawFunc>()(nullptr,nullptr);
  registry.find(t[0].key,Index::TABLE_GAME_WIN_TOOLTIP).get<GameWinTooltipFunc>()(nullptr,nullptr,97);
  registry.find(l[0].key,Index::TABLE_WIN_LAYOUT_INIT).get<WindowLayoutShutdownFunc>()(nullptr,&data);
  require(calls==4 && received==&data,"all four retained callback prototype families");
  require(!registry.find(d[0].key,Index::TABLE_ANY).get<GameWinSystemFunc>() &&
          !registry.find(l[1].key,Index::TABLE_ANY) && !registry.find(NAMEKEY_INVALID,Index::TABLE_ANY),"wrong-family/absent/invalid-key lookup cannot call another prototype");
  for (Int raw:{-2,-1,9,10,INT32_MAX,INT32_MIN}) {
    const auto index=static_cast<Index>(raw);
    require(registry.table(index).empty(),"defined raw table bound");
    if (raw!=-1) require(!registry.find(s[0].key,index),"invalid index cannot inspect backing");
  }
  Entry replacement[]{{NAMEKEY_INVALID,"Replacement",layoutOther},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable next{replacement,Index::TABLE_WIN_LAYOUT_INIT}; registry.loadTables(std::span(&next,1),names);
  require(registry.table(Index::TABLE_WIN_LAYOUT_INIT).data()==replacement &&
          registry.table(Index::TABLE_GAME_WIN_SYSTEM).data()==s,"subset replacement preserves parallel table links");
  require(!registry.find(l[0].key,Index::TABLE_WIN_LAYOUT_INIT),"retired table no longer reachable");
  Entry duplicate[]{{NAMEKEY_INVALID,"Duplicate",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable alias{duplicate,Index::TABLE_GAME_WIN_INPUT}; registry.loadTables(std::span(&alias,1),names);
  require(!registry.validate(),"source duplicate callback diagnostics retained within shared prototype");
  static_assert(!std::is_constructible_v<NativeWindowCallback,void*>);
  static_assert(sizeof(Index)==sizeof(Int) && alignof(Index)==alignof(Int));
}
void malformed() {
  NameKeyGenerator names; names.init(); NativeFunctionRegistry registry;
  Entry accepted[]{{NAMEKEY_INVALID,"Accepted",layout},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable prior{accepted,Index::TABLE_WIN_LAYOUT_INIT}; registry.loadTables(std::span(&prior,1),names);
  const auto key=accepted[0].key;
  Entry candidate[]{{NAMEKEY_INVALID,"Candidate",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  Entry wrong[]{{NAMEKEY_INVALID,"Wrong",draw},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable late[]{{candidate,Index::TABLE_GAME_WIN_SYSTEM},{wrong,Index::TABLE_GAME_WIN_TOOLTIP}};
  rejects([&]{registry.loadTables(late,names);});
  require(candidate[0].key==NAMEKEY_INVALID && wrong[0].key==NAMEKEY_INVALID &&
          registry.table(prior.index).data()==accepted && accepted[0].key==key,"whole late-invalid batch rejects before table/key mutation");
  require(names.nameToKey("Next")==key+1,"late-invalid batch does not intern names");
  const NativeFunctionTable repeated[]{prior,prior}; rejects([&]{registry.loadTables(repeated,names);});
  for (Int raw:{-2,-1,9,INT32_MAX}) {
    const NativeFunctionTable invalid{candidate,static_cast<Index>(raw)};
    rejects([&]{registry.loadTables(std::span(&invalid,1),names);});
  }
  const NativeFunctionTable truncated{std::span(candidate,1),Index::TABLE_GAME_WIN_SYSTEM};
  rejects([&]{registry.loadTables(std::span(&truncated,1),names);});
  Entry interior[]{{NAMEKEY_INVALID,nullptr,nullptr},{NAMEKEY_INVALID,"Unexpected",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable bad{interior,Index::TABLE_GAME_WIN_SYSTEM};
  rejects([&]{registry.loadTables(std::span(&bad,1),names);});
  candidate[1].key=key; const NativeFunctionTable sentinel{candidate,Index::TABLE_GAME_WIN_SYSTEM};
  rejects([&]{registry.loadTables(std::span(&sentinel,1),names);});
  candidate[1].key=NAMEKEY_INVALID; registry.loadTables(std::span(&sentinel,1),names);
  require(registry.find(candidate[0].key,sentinel.index).get<GameWinSystemFunc>()==system,"same-owner semantic retry");
  NameKeyGenerator bounded(2); bounded.init(); NativeFunctionRegistry limited;
  Entry old[]{{NAMEKEY_INVALID,"Accepted",layout},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable initial{old,Index::TABLE_WIN_LAYOUT_INIT}; limited.loadTables(std::span(&initial,1),bounded);
  Entry over[]{{NAMEKEY_INVALID,"First",system},{NAMEKEY_INVALID,"Second",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable exhausted{over,Index::TABLE_GAME_WIN_SYSTEM};
  bool full=false;
  try { limited.loadTables(std::span(&exhausted,1),bounded); } catch(ErrorCode e) { full=e==ERROR_OUT_OF_MEMORY; }
  require(full && over[0].key==NAMEKEY_INVALID && over[1].key==NAMEKEY_INVALID &&
          limited.table(initial.index).data()==old && limited.table(exhausted.index).empty(),
          "late namespace capacity failure cannot publish a partial table");
  Entry corrected[]{{NAMEKEY_INVALID,"First",system},{NAMEKEY_INVALID,nullptr,nullptr}};
  const NativeFunctionTable retry{corrected,Index::TABLE_GAME_WIN_SYSTEM}; limited.loadTables(std::span(&retry,1),bounded);
  require(corrected[0].key==static_cast<NameKeyType>(2),"same-owner retry reuses accepted monotonic name at exact capacity");
}
void faults(bool warm,bool longNames=false) {
  bool terminal=false;
  for (std::size_t ordinal=0;ordinal<32;++ordinal) {
    const auto baseline=AllocationFault::live();
    {
      NameKeyGenerator names; names.init(); NativeFunctionRegistry registry;
      Entry accepted[]{{NAMEKEY_INVALID,"Accepted",layout},{NAMEKEY_INVALID,nullptr,nullptr}};
      const NativeFunctionTable prior{accepted,Index::TABLE_WIN_LAYOUT_INIT}; registry.loadTables(std::span(&prior,1),names);
      Entry a[]{{NAMEKEY_INVALID,longNames?"FirstGeneratedCallbackWithOwnedNameBacking":"First",system},
                {NAMEKEY_INVALID,longNames?"SecondGeneratedCallbackWithOwnedNameBacking":"Second",system},
                {NAMEKEY_INVALID,nullptr,nullptr}};
      Entry b[]{{NAMEKEY_INVALID,longNames?"LastGeneratedCallbackWithOwnedNameBacking":"Last",layoutOther},
                {NAMEKEY_INVALID,nullptr,nullptr}};
      const NativeFunctionTable batch[]{{a,Index::TABLE_GAME_WIN_SYSTEM},{b,Index::TABLE_WIN_LAYOUT_INIT}};
      if (warm) for (auto name:{a[0].name,a[1].name,b[0].name}) names.nameToKey(name);
      const auto acceptedLive=AllocationFault::live(); bool failed=false;
      auto* pool=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");
      require(pool,"actual shared source name pool");
      const auto acceptedBlocks=pool->getUsedBlockCount();
      AllocationFault::arm(ordinal);
      try { registry.loadTables(batch,names); }
      catch(const std::bad_alloc&) { failed=true; }
      catch(...) { AllocationFault::disarm(); throw; }
      AllocationFault::disarm();
      if (!AllocationFault::triggered()) {
        require(!failed && ordinal==(warm?1u:(longNames?7u:4u)),"complete exact registration allocation terminal");
        terminal=true;
      } else {
        require(failed && registry.table(prior.index).data()==accepted &&
          registry.table(Index::TABLE_GAME_WIN_SYSTEM).empty() && a[0].key==NAMEKEY_INVALID &&
          a[1].key==NAMEKEY_INVALID && b[0].key==NAMEKEY_INVALID,"commit-ready prior tables survive every early/late allocation fault");
        if (warm) require(AllocationFault::live()==acceptedLive,"warm preparation exact residual");
        else {
          unsigned interned=0;
          for (Int key=2;key<=4;++key) if (!names.keyToName(static_cast<NameKeyType>(key)).isEmpty()) ++interned;
          require(AllocationFault::live()==acceptedLive+interned &&
                  pool->getUsedBlockCount()==acceptedBlocks+static_cast<Int>(interned),
                  "cold immediate residual is exactly accepted monotonic namespace ownership");
        }
        registry.loadTables(batch,names);
      }
      require(registry.find(b[0].key,prior.index).get<WindowLayoutInitFunc>()==layoutOther &&
          registry.table(Index::TABLE_GAME_WIN_SYSTEM).data()==a,"same-owner corrected registration retry");
    }
    require(AllocationFault::live()==baseline,"all retained interned-name and table preparation resources retire");
    if (terminal) break;
  }
  require(terminal,"bounded manifest complete");
}
}
int main(int argc,char** argv) {
  bool initialized=false;
  try {
    require(argc==2,"registry family"); initMemoryManager(); initialized=true;
    // Baseline after process-global pool initialization, not before its first use.
    { NameKeyGenerator warm; warm.init(); for (auto name:{"warm1","warm2","warm3","warm4"}) warm.nameToKey(name); }
    const std::string family(argv[1]);
    for (int repeat=0;repeat<3;++repeat) {
      const auto baseline=AllocationFault::live();
      if (family=="functional") functional(); else if (family=="malformed") malformed();
      else if (family=="fault-warm") faults(true);
      else if (family=="fault-cold") { faults(false); faults(false,true); }
      else throw std::runtime_error("unknown family");
      require(AllocationFault::live()==baseline,"same-process registry lifecycle");
      require(TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount()==0,
              "every namespace releases all shared source pool ownership");
    }
    shutdownMemoryManager(); initialized=false; std::cout << "PASS original callback registry (physical GUI pending)\n"; return 0;
  } catch(const std::exception& e) { std::cerr << "FAIL " << e.what() << '\n'; }
    catch(ErrorCode) { std::cerr << "FAIL source callback registration\n"; }
  AllocationFault::disarm(); if (initialized) shutdownMemoryManager(); return 1;
}
