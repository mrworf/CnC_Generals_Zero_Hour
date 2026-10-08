// SPDX-License-Identifier: GPL-3.0-or-later
// Generated consumers of the actual subsystem ownership transaction.
// Not replacement gameplay/INI providers or complete GameEngine acceptance.
#include "AllocationFault.h"
#include "Common/NativeSubsystemInit.h"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class F> void rejects(F action) {
    bool rejected=false;
    try { action(); } catch(ErrorCode) { rejected=true; }
    require(rejected,"subsystem semantic admission rejects");
}
struct Observation {
    std::array<unsigned,64> events{};
    std::size_t count=0;
    bool cleanupValid=true;
    void record(unsigned event) noexcept {
        if (count==events.size()) {cleanupValid=false;return;}
        events[count++]=event;
    }
};
struct Registry {
    SubsystemInterfaceList list;
    SubsystemInterfaceList* prior=TheSubsystemList;
    Registry() {TheSubsystemList=&list;}
    ~Registry() {list.shutdownAll();TheSubsystemList=prior;}
};
struct Probe final : SubsystemInterface {
    Observation& observations;
    Probe** slot;
    unsigned identity;
    bool failInit;
    bool recursiveShutdown=false;
    std::unique_ptr<unsigned[]> backing;
    unsigned value=0;
    Probe(Observation& events, Probe** publication, unsigned id, bool fail=false)
        : observations(events),slot(publication),identity(id),failInit(fail) {}
    ~Probe() override {
        // Same source dependency as Object/Drawable callbacks: parent remains
        // the live published identity throughout its legitimate cleanup.
        if (*slot!=this) observations.cleanupValid=false;
        if (recursiveShutdown) {
            const auto remaining=TheSubsystemList->ownedCount();
            TheSubsystemList->shutdownAll();
            if (TheSubsystemList->ownedCount()!=remaining) observations.cleanupValid=false;
        }
        observations.record(100+identity);
        *slot=nullptr; // Some real owners already clear their own singleton.
    }
    void init() override {
        require(*slot==this,"init sees construction publication");
        backing=std::make_unique<unsigned[]>(32);
        if (failInit) throw ERROR_BAD_ARG;
    }
    void reset() override {observations.record(300+identity);}
    void update() override {}
    void postProcessLoad() override {observations.record(200+identity);}
};
void add(Registry& registry, Observation& events, Probe*& slot, unsigned identity,
         bool failInit=false, bool failDecode=false) {
    initOwnedSubsystem(slot,"generated owner",new Probe(events,&slot,identity,failInit),
        [&](Probe* owner) {
            std::string decoded(40,'d'); // Fallible generated definition work.
            if (failDecode) throw ERROR_BAD_INI;
            owner->value=static_cast<unsigned>(decoded.size());
        });
}
void lifecycle() {
    Observation events;
    Probe* slot=nullptr;
    {
        Registry registry;
        add(registry,events,slot,1);
        Probe* first=slot;
        add(registry,events,slot,2);
        slot->recursiveShutdown=true;
        require(first!=slot && first->value==40 && slot->value==40 &&
                registry.list.ownedCount()==2,"each registry acquisition owns one unit");
        registry.list.postProcessLoadAll();
        registry.list.resetAll();
        const std::array<unsigned,4> expected{201,202,302,301};
        require(events.count==expected.size(),"source callback cardinality");
        for(std::size_t i=0;i<expected.size();++i)
            require(events.events[i]==expected[i],"forward postprocess and reverse reset order");
        registry.list.shutdownAll();
        require(!slot && registry.list.ownedCount()==0 && events.count==6 &&
                events.events[4]==102 && events.events[5]==101 && events.cleanupValid,
                "reverse withdrawal restores prior live owner through destructor callbacks");
        registry.list.shutdownAll();
        require(events.count==6,"drained shutdown is idempotent");
        add(registry,events,slot,3);
        require(slot && registry.list.ownedCount()==1,"same registry accepts corrected lifecycle");
        // Scope teardown must drain the final owner without an explicit call.
    }
    require(!slot && events.cleanupValid && events.count==7 && events.events[6]==103,
            "list destructor owns exact retirement");
}
void negative() {
    Observation events;
    Probe* slot=nullptr;
    Registry registry;
    add(registry,events,slot,1);
    Probe* prior=slot;
    for(bool failInit:{true,false}) {
        const auto live=AllocationFault::live();
        rejects([&]{add(registry,events,slot,2,failInit,!failInit);});
        require(slot==prior && prior->value==40 && registry.list.ownedCount()==1 &&
                AllocationFault::live()==live && events.cleanupValid,
                "init/decode rejection retires candidate then restores accepted publication");
    }
    unsigned dispatches=0;
    rejects([&]{registry.list.initializeSubsystem(prior,"duplicate",[&]{++dispatches;});});
    require(dispatches==0 && slot==prior && prior->getName()==AsciiString("generated owner") &&
            registry.list.ownedCount()==1,"duplicate admission precedes name/init/dispatch effects");
    rejects([&]{registry.list.initializeSubsystem(nullptr,"missing",[&]{++dispatches;});});
    SubsystemPublication malformed{&slot,nullptr,nullptr};
    rejects([&]{registry.list.initializeSubsystem(prior,"invalid publication",[&]{++dispatches;},malformed);});
    SubsystemPublication ignored{nullptr,prior,nullptr};
    rejects([&]{registry.list.initializeSubsystem(prior,"ignored invalid publication",[&]{++dispatches;},ignored);});
    require(!dispatches && slot==prior,"complete publication admission before effects");
    add(registry,events,slot,3);
    require(slot!=prior && registry.list.ownedCount()==2,"same-owner retry after semantic rejection");
    registry.list.shutdownAll();
    require(!slot && events.cleanupValid,"negative teardown preserves cleanup identity");
}
void faults() {
    constexpr std::size_t expected=5; // object, name, init backing, decode, registry growth
    for(std::size_t ordinal=0;ordinal<=expected;++ordinal) {
        Observation events;
        Probe* slot=nullptr;
        Registry registry;
        add(registry,events,slot,1);
        Probe* prior=slot;
        const auto live=AllocationFault::live();
        bool rejected=false;
        AllocationFault::arm(ordinal);
        try {add(registry,events,slot,2);}
        catch(const std::bad_alloc&) {rejected=true;}
        catch(...) {AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        if (ordinal==expected) {
            require(!rejected && !AllocationFault::triggered() && slot!=prior &&
                    registry.list.ownedCount()==2,"exact terminal accepts complete registration");
        } else {
            require(rejected && AllocationFault::triggered() && slot==prior &&
                    prior->value==40 && registry.list.ownedCount()==1 &&
                    AllocationFault::live()==live && events.cleanupValid,
                    "each construction/preparation/publication fault restores exact accepted state");
            add(registry,events,slot,2);
            require(slot!=prior && slot->value==40 && registry.list.ownedCount()==2,
                    "each fault pairs with same-registry corrected retry");
        }
        registry.list.shutdownAll();
        require(!slot && events.cleanupValid,"fault and terminal cleanup callbacks retain valid identities");
    }
    std::cout<<"registration allocations [0,"<<expected<<"); terminal "<<expected<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"subsystem family");
        const std::string family(argv[1]);
        for(unsigned repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if(family=="lifecycle") lifecycle();
            else if(family=="negative") negative();
            else if(family=="faults") faults();
            else throw std::runtime_error("unknown subsystem family");
            require(!TheSubsystemList && AllocationFault::live()==live,
                    "complete same-process registry retirement");
        }
        std::cout<<"PASS original subsystem transaction (GameEngine pending)\n";
        return 0;
    } catch(const std::exception& error) {std::cerr<<"FAIL "<<error.what()<<'\n';}
      catch(ErrorCode) {std::cerr<<"FAIL source error\n";}
    AllocationFault::disarm();return 1;
}
