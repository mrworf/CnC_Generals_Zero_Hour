// SPDX-License-Identifier: GPL-3.0-or-later
// Actual ownership journal used by engine/client/logic, not gameplay providers.
#include "AllocationFault.h"
#include "Common/NativeServiceOwners.h"
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool value,const char* message) {if(!value) throw std::runtime_error(message);}
template<class F> void rejects(F f) {
    bool rejected=false;try {f();} catch(ErrorCode) {rejected=true;}
    require(rejected,"service admission rejects");
}
struct Log {unsigned events[32]{};unsigned count=0;bool valid=true;};
struct Child {
    Child** slot;Log& log;unsigned id;
    std::unique_ptr<unsigned[]> backing;
    NativeServiceOwners<3>* parent=nullptr;
    Child(Child*& publication,Log& events,unsigned identity)
        :slot(&publication),log(events),id(identity),backing(new unsigned[16]{}) {}
    ~Child() {
        if(*slot!=this || log.count==32) log.valid=false;
        else log.events[log.count++]=id;
        if(parent) parent->clear(); // Parent reentry must not drain live peers.
        *slot=nullptr;
    }
};
void lifecycle() {
    Log log;Child* a=nullptr;Child* b=nullptr;Child* c=nullptr;
    const auto baseline=AllocationFault::live();
    {
        NativeServiceOwners<3> owner;
        owner.create(a,[&]{return new Child(a,log,1);});
        owner.create(b,[&]{return new Child(b,log,2);});
        owner.create(c,[&]{return new Child(c,log,3);});
        require(owner.owns(a)&&owner.owns(b)&&owner.owns(c),"exact acquired slots");
        b->parent=&owner;
        owner.retire(b);
        require(!b&&a&&c&&log.count==1&&log.events[0]==2&&log.valid,
                "explicit source order and recursive retirement preserve live peers");
        owner.retire(b);
        owner.retire(c);
        owner.create(c,[&]{return new Child(c,log,4);});
        owner.clear();owner.clear();
        require(!a&&!b&&!c&&log.count==4&&log.events[2]==4&&log.events[3]==1,
                "tail capacity reuse and reverse idempotent drain");
        owner.create(a,[&]{return new Child(a,log,5);});
    }
    require(!a&&log.valid&&log.events[4]==5&&AllocationFault::live()==baseline,
            "member destruction retires every acquired unit");
}
void negative() {
    Log log;Child* slot=nullptr;Child* other=nullptr;unsigned factories=0;
    NativeServiceOwners<1> owner;
    owner.create(slot,[&]{return new Child(slot,log,1);});
    const auto baseline=AllocationFault::live();
    rejects([&]{owner.create(slot,[&]{++factories;return new Child(slot,log,2);});});
    rejects([&]{owner.create(other,[&]{++factories;return new Child(other,log,3);});});
    require(!factories&&slot&&!other&&AllocationFault::live()==baseline,
            "occupied slot and exact journal bound reject before factory/effects");
    owner.clear();
    require(!owner.create(slot,[]{return static_cast<Child*>(nullptr);},false)&&!owner.owns(slot),
            "optional null creates no ownership unit");
    rejects([&]{owner.create(slot,[]{return static_cast<Child*>(nullptr);});});
    // Failed enclosing construction never runs that class's destructor.
    struct Parent {
        NativeServiceOwners<1> owners;
        Parent(Child*& slot,Log& log) {
            owners.create(slot,[&]{return new Child(slot,log,4);});
            throw ERROR_BAD_INI;
        }
    };
    const auto empty=AllocationFault::live();
    rejects([&]{Parent failed(slot,log);});
    require(!slot&&log.valid&&AllocationFault::live()==empty,
            "member acquisition rollback despite failed enclosing constructor");
    owner.create(slot,[&]{return new Child(slot,log,5);});
    owner.clear();require(!slot&&log.valid,"same owner retry after all admissions");
}
void faults() {
    constexpr unsigned terminal=2; // Child object and its owned array; journal allocates nothing.
    for(unsigned ordinal=0;ordinal<=terminal;++ordinal) {
        Log log;Child* prior=nullptr;Child* slot=nullptr;
        NativeServiceOwners<3> owner;
        owner.create(prior,[&]{return new Child(prior,log,1);});
        const auto baseline=AllocationFault::live();bool rejected=false;
        AllocationFault::arm(ordinal);
        try {owner.create(slot,[&]{return new Child(slot,log,2);});}
        catch(const std::bad_alloc&) {rejected=true;}
        catch(...) {AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        if(ordinal<terminal) {
            require(rejected&&AllocationFault::triggered()&&!slot&&prior&&
                    AllocationFault::live()==baseline&&!log.count,
                    "every factory construction fault preserves exact accepted owner");
            owner.create(slot,[&]{return new Child(slot,log,2);});
        } else require(!rejected&&!AllocationFault::triggered()&&slot,
                       "exact no-allocation journal publication terminal");
        owner.clear();require(!slot&&!prior&&log.valid,"fault/retry exact cleanup");
    }
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"service owner family");const std::string family(argv[1]);
        for(unsigned repeat=0;repeat<3;++repeat) {
            const auto baseline=AllocationFault::live();
            if(family=="lifecycle") lifecycle();else if(family=="negative") negative();
            else if(family=="faults") faults();else throw std::runtime_error("unknown family");
            require(AllocationFault::live()==baseline,"complete same-process retirement");
        }
        std::cout<<"PASS original parent service ownership (full startup pending)\n";return 0;
    } catch(const std::exception& error) {std::cerr<<"FAIL "<<error.what()<<'\n';}
    catch(ErrorCode) {std::cerr<<"FAIL source error\n";}
    AllocationFault::disarm();return 1;
}
