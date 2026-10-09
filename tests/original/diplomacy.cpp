// SPDX-License-Identifier: GPL-3.0-or-later
// Actual briefing/disconnect lifetimes. Observation callbacks are not a GUI.
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/INI.h"
#include "Common/UnicodeString.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameClient/NativeDiplomacyBriefing.h"
#include "GameClient/DisconnectMenu.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
static_assert(std::is_same_v<decltype(TheLAN),LANAPIInterface*>);
static_assert(std::is_abstract_v<LANAPIInterface> && std::has_virtual_destructor_v<LANAPIInterface>);
static_assert(std::is_same_v<decltype(&LANAPIInterface::LookupPlayer),LANPlayer*(LANAPIInterface::*)(UnsignedInt)>);
static_assert(std::is_same_v<decltype(&LANAPIInterface::GetLocalIP),UnsignedInt(LANAPIInterface::*)()>);
namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
  require(rejected,"invalid briefing transition rejects");}
struct View {
  NativeDiplomacyBriefing* owner=nullptr;
  BriefingList displayed;
  Bool partialFailure=FALSE,reentrant=FALSE,lastReset=FALSE;
  unsigned calls=0;
  static void apply(const BriefingList& entries,Bool reset,void* context){
    auto& view=*static_cast<View*>(context);++view.calls;view.lastReset=reset;
    if(view.reentrant){
      rejects([&]{view.owner->update("reentrant",FALSE);});
      rejects([&]{view.owner->attach(apply,&view);});
      require(!view.owner->detach(),"reentrant detach cannot retire active borrowed callback");
    }
    if(view.partialFailure){view.displayed.clear();view.displayed.push_back("partial");throw ERROR_BAD_ARG;}
    if(!reset){
      require(entries.size()==view.displayed.size()+1,"ordinary update requests exactly one append");
      require(std::equal(view.displayed.begin(),view.displayed.end(),entries.begin()),"append preserves original prefix");
    }
    BriefingList candidate=entries;view.displayed.swap(candidate);
  }
};
void functional(){
  NativeDiplomacyBriefing owner;auto* identity=owner.entries();
  owner.update("First",FALSE);owner.update("First",FALSE);owner.update("first",FALSE);owner.update("",FALSE);
  require(identity==owner.entries() && identity->size()==2 && identity->front()=="First" && identity->back()=="first",
      "stable list identity, original case-sensitive dedup/order and empty no-op");
  View view{&owner};owner.attach(View::apply,&view);require(view.lastReset && view.displayed==*identity,"initial full replay");
  auto calls=view.calls;owner.attach(View::apply,&view);require(view.calls==calls,"idempotent accepted binding");
  owner.update("Third",FALSE);require(!view.lastReset && view.displayed==*identity,"ordinary append does not reset selection/presentation");
  calls=view.calls;owner.update("Third",FALSE);owner.update("",FALSE);require(view.calls==calls,"duplicate/empty does not notify clean view");
  owner.update("Replacement",TRUE);require(view.lastReset && view.displayed.size()==1 && view.displayed.front()=="Replacement",
      "clear-plus-add ordered publication");
  owner.update("",TRUE);require(identity->empty() && view.displayed.empty() && identity==owner.entries(),"clear retains stable container address");
  require(owner.detach() && owner.detach() && !owner.attached(),"idempotent withdrawal without callback");
  calls=view.calls;owner.update("Detached",FALSE);require(view.calls==calls && view.displayed.empty(),"retired view is never called");
  owner.attach(View::apply,&view);require(view.displayed==*identity && view.lastReset,"same-owner rebind fully replays retained model");
  require(owner.detach(),"withdraw before observer retirement");
}
void negative(){
  NativeDiplomacyBriefing owner;owner.update("Accepted",FALSE);View view{&owner},other{&owner};
  rejects([&]{owner.attach(nullptr,&view);});owner.attach(View::apply,&view);
  rejects([&]{owner.attach(View::apply,&other);});require(owner.attached() && other.calls==0,"conflict preserves admitted view");
  view.partialFailure=TRUE;rejects([&]{owner.update("Rejected",FALSE);});
  require(owner.entries()->size()==1 && owner.entries()->front()=="Accepted" && view.displayed.front()=="partial",
      "partial callback failure never commits source model");
  view.partialFailure=FALSE;owner.update("Rejected",FALSE);
  require(view.lastReset && view.displayed==*owner.entries() && owner.entries()->size()==2,"retry resynchronizes rather than duplicating append");
  view.partialFailure=TRUE;rejects([&]{owner.update("Replacement",TRUE);});view.partialFailure=FALSE;
  owner.update("Accepted",FALSE);require(view.lastReset && view.displayed==*owner.entries() && owner.entries()->size()==2,
      "duplicate retry resynchronizes accepted model after failed clear");
  view.reentrant=TRUE;owner.update("Reentrancy",FALSE);view.reentrant=FALSE;require(owner.detach(),"reentrant observation leaves binding usable");
  view.partialFailure=TRUE;rejects([&]{owner.attach(View::apply,&view);});require(!owner.attached(),"failed first synchronization does not publish capture");
  view.partialFailure=FALSE;owner.attach(View::apply,&view);require(view.displayed==*owner.entries(),"corrected initial bind retry");owner.detach();
  const auto calls=view.calls;
  {NativeDiplomacyBriefing temporary;temporary.update("Lifetime",FALSE);temporary.attach(View::apply,&view);}
  require(view.calls==calls+1,"owner destruction never calls borrowed view");
}
void globals(){
  auto& owner=originalDiplomacyBriefing();require(owner.detach(),"withdraw original source presentation");
  UpdateDiplomacyBriefingText("",TRUE);auto* identity=GetBriefingTextList();
  UpdateDiplomacyBriefingText("OriginalA",FALSE);UpdateDiplomacyBriefingText("OriginalB",FALSE);
  UpdateDiplomacyBriefingText("OriginalA",FALSE);
  require(identity==GetBriefingTextList() && identity->size()==2 && identity->back()=="OriginalB",
      "actual public original entry points share stable retained owner");
  View view{&owner};owner.attach(View::apply,&view);owner.detach();
  require(identity->size()==2,"view/reset withdrawal does not clear briefing source state");
  UpdateDiplomacyBriefingText("NewWorld",TRUE);require(identity->size()==1 && identity->front()=="NewWorld","actual source clear-plus-new-world replacement");
  UpdateDiplomacyBriefingText("",TRUE);require(identity->empty() && !owner.attached(),"explicit source clearing retires process-held entries");
}
struct Context {
  NativeDiplomacyBriefing owner;View view{&owner};
  AsciiString incoming{"Generated long incoming briefing label"};
  Context(){owner.update("Generated long accepted briefing label A",FALSE);owner.update("Generated long accepted briefing label B",FALSE);}
  void prepare(const std::string& family){if(family!="fault-attach")owner.attach(View::apply,&view);}
  void run(const std::string& family){if(family=="fault-attach")owner.attach(View::apply,&view);
    else owner.update(incoming,family=="fault-clear");}
};
void faults(const std::string& family){
  std::size_t census=0;
  {Context discovery;discovery.prepare(family);AllocationFault::arm(SIZE_MAX);
    try{discovery.run(family);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<64,"bounded original briefing manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto outside=AllocationFault::live();
    {Context context;context.prepare(family);auto* identity=context.owner.entries();
      const auto prior=*identity;const auto display=context.view.displayed;const auto live=AllocationFault::live();
      const Bool bound=context.owner.attached();bool failed=false;AllocationFault::arm(ordinal);
      try{context.run(family);}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
      AllocationFault::disarm();require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
          (failed || AllocationFault::attempts()==census),"every failure/retry pair and exact terminal");
      if(failed){require(AllocationFault::live()==live && *identity==prior && context.owner.entries()==identity &&
          context.view.displayed==display && context.owner.attached()==bound,"candidate/list/view/publication rollback");context.run(family);}
      require(context.view.displayed==*identity && context.owner.entries()==identity && context.owner.attached(),"same-owner corrected retry");
      if(family=="fault-clear")require(identity->size()==1 && identity->front()==context.incoming,"clear candidate survives complete retry");
      else if(family=="fault-append")require(identity->size()==3 && identity->back()==context.incoming,"append candidate survives complete retry");
      context.owner.detach();
    }require(AllocationFault::live()==outside,"whole owner/view retirement exactly");
  }std::cout<<family<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
struct Disconnect:DisconnectMenu {DisconnectManager* borrowed() const{return m_disconnectManager;}};
void disconnect(){
  Disconnect owner;require(!owner.isScreenVisible() && !owner.borrowed(),"original disconnect pre-init state is defined");
  owner.attachDisconnectManager(nullptr);require(!owner.borrowed() && !owner.isScreenVisible(),"borrowed manager capture does not imply a visible screen");
  DisconnectMenu* base=&owner;require(dynamic_cast<Disconnect*>(base)==&owner,"actual original destructor provides RTTI without GUI stubs");
  auto* prior=TheDisconnectMenu;TheDisconnectMenu=&owner;TheDisconnectMenu=prior;
}
}
int main(int argc,char** argv){bool initialized=false;
  try{require(argc==2,"diplomacy family required");initMemoryManager();initialized=true;const std::string family=argv[1];
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      if(family=="functional")functional();else if(family=="negative")negative();else if(family=="globals")globals();
      else if(family=="disconnect")disconnect();else if(family.starts_with("fault-"))faults(family);
      else throw std::runtime_error("unknown diplomacy family");
      require(AllocationFault::live()==live && !originalDiplomacyBriefing().attached() && GetBriefingTextList()->empty(),
          "repeated complete owner/public briefing retirement");}
    shutdownMemoryManager();initialized=false;std::cout<<"PASS original briefing/disconnect; nonnull GUI/animation pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL original diplomacy admission\n";}
  AllocationFault::disarm();originalDiplomacyBriefing().detach();UpdateDiplomacyBriefingText("",TRUE);
  if(initialized)shutdownMemoryManager();return 1;
}
