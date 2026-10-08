// SPDX-License-Identifier: GPL-3.0-or-later
#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include "Common/GameMemory.h"
#include "Common/CriticalSection.h"
#include "Common/RandomValue.h"
#include "Common/crc.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/LogicRandomValue.h"
#include "Common/AudioRandomValue.h"
#include <array>
#include <cfenv>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

void* allocateFromW3DMemPool(void*,int);
void freeFromW3DMemPool(void*,void*);

namespace {
void require(bool condition,const char* message) {if(!condition)throw std::runtime_error(message);}
template<class F> void rejects(F action,const char* message) {
    bool failed=false;try{action();}catch(ErrorCode){failed=true;}catch(const std::exception&){failed=true;}
    require(failed,message);
}
void values() {
    static_assert(sizeof(Int)==4 && sizeof(UnsignedInt)==4 && sizeof(Int64)==8 && sizeof(Real)==4);
    for(float value:{-12.75f,-1.f,-.125f,0.f,.125f,1.f,12.75f}) {
        require(FAST_REAL_TRUNC(value)==std::trunc(value),"defined original truncation");
        require(FAST_REAL_FLOOR(value)==std::floor(value),"defined original floor");
        require(FAST_REAL_CEIL(value)==std::ceil(value),"defined original ceil");
    }
    const int prior=std::fegetround();std::fesetround(FE_TOWARDZERO);
    require(fast_float2long_round(-12.75f)==-12,"source chopping conversion");
    require(fast_float2long_round(-2147483648.f)==INT32_MIN,"signed width minimum");
    for(float value:{2147483648.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()})
        rejects([&]{(void)fast_float2long_round(value);},"invalid conversion before cast");
    std::fesetround(prior);
    const std::array<UnsignedByte,6> bytes{0xff,0,0x80,0x7f,1,0xaa};
    CRC all,parts;all.computeCRC(bytes.data(),Int(bytes.size()));
    parts.computeCRC(bytes.data(),2);parts.computeCRC(bytes.data()+2,4);
    require(all.get()==parts.get() && all.get()==9864,"original byte CRC and chunking");
    const auto before=all.get();all.computeCRC(nullptr,8);all.computeCRC(bytes.data(),-1);
    require(before==all.get(),"CRC empty input unchanged");
    Coord3D vector{3,4,0};vector.normalize();require(std::abs(vector.length()-1)<1e-6,"original vector normalization");
    require(Sin(0)==0 && Cos(0)==1 && std::abs(ACos(0)-PI/2)<1e-6,"retained active original trig path");
}
void randomStreams() {
    constexpr std::array<Int,12> expected{823,209,704,299,632,971,682,948,750,591,960,331};
    InitRandom(0x12345678);
    for(Int value:expected)require(GameLogicRandomValue(0,1000)==value,"original seeded sequence");
    InitRandom(0x12345678);
    for(Int value:expected) {
        (void)GameClientRandomValue(-100,100);(void)GameAudioRandomValueReal(0,1);
        require(GameLogicRandomValue(0,1000)==value,"client/audio never consume logic seed");
    }
    using Integer=Int(*)(int,int,const char*,int);
    for(Integer function:{GetGameLogicRandomValue,GetGameClientRandomValue,GetGameAudioRandomValue}) {
        InitRandom(7);const auto before=GetGameLogicRandomSeedCRC();
        require(function(INT32_MIN,INT32_MAX,__FILE__,__LINE__)==INT32_MAX,"legacy full-range zero-delta convention");
        require(GetGameLogicRandomSeedCRC()==before,"full-range convention does not consume seed");
        rejects([&]{function(1,-1,__FILE__,__LINE__);},"reversed range rejected");
        require(GetGameLogicRandomSeedCRC()==before,"rejected range leaves logic seed");
        for(int i=0;i<100;++i) {
            const auto value=function(INT32_MIN,INT32_MAX-1,__FILE__,__LINE__);
            require(value>=INT32_MIN && value<=INT32_MAX-1,"wide signed range defined");
        }
    }
    using Floating=Real(*)(Real,Real,const char*,int);
    for(Floating function:{GetGameLogicRandomValueReal,GetGameClientRandomValueReal,GetGameAudioRandomValueReal}) {
        const auto before=GetGameLogicRandomSeedCRC();
        rejects([&]{function(0,std::numeric_limits<float>::infinity(),__FILE__,__LINE__);},"nonfinite random range");
        require(GetGameLogicRandomSeedCRC()==before,"invalid real range does not consume seed");
    }
}
void strings() {
    AsciiString source("portable"),copy(source);copy.concat(copy.str());
    require(source=="portable" && copy=="portableportable","source/alias copy-on-write");
    copy.set(copy.str()+8);require(copy=="portable","interior pointer alias");
    copy=copy;copy.format(AsciiString("%s-%d"),source.str(),42);require(copy=="portable-42","native varargs pattern forwarding");
    auto* bytes=copy.getBufferForRead(64);
    for(int i=0;i<65;++i)require(bytes[i]==0,"initialized ASCII backing including tail");
    copy="unchanged";rejects([&]{copy.getBufferForRead(INT32_MAX);},"ASCII capacity rejection");
    require(copy=="unchanged","ASCII rejected request unchanged");
    rejects([&]{copy.format("%02048d",1);},"format truncation rejection");
    require(copy=="unchanged","rejected formatting unchanged");
    rejects([&]{copy.format(static_cast<const char*>(nullptr));},"null ASCII format");
    require(copy=="unchanged","null ASCII format preserves owner");
    rejects([&]{copy.nextToken(nullptr);},"null ASCII token owner");
    AsciiString tokens("  alpha beta"),token;require(tokens.nextToken(&token)&&token=="alpha","original token splitting");
    UnicodeString wide(L"native Ω"),other(wide);other.concat(other.str());
    require(wide.compare(L"native Ω")==0 && other.getLength()==2*wide.getLength(),"wide copy and aliases");
    other.set(other.str()+wide.getLength());require(other==wide,"wide interior alias");
    auto* chars=other.getBufferForRead(64);
    for(int i=0;i<65;++i)require(chars[i]==0,"initialized native wide backing");
    other=wide;rejects([&]{other.getBufferForRead(INT32_MAX);},"wide capacity rejection");
    require(other==wide,"wide rejection keeps accepted owner");
    rejects([&]{other.format(static_cast<const wchar_t*>(nullptr));},"null wide format");
    rejects([&]{other.format(L"%02048d",1);},"wide format truncation");
    require(other==wide,"wide format rejection preserves owner");
    rejects([&]{other.nextToken(nullptr,UnicodeString(L" "));},"null Unicode token owner");
    // Exceed the old 16-bit reference counter with real ownership units.
    std::vector<AsciiString> asciiCopies(70000,source);
    std::vector<UnicodeString> wideCopies(70000,wide);
    asciiCopies.back()="changed";wideCopies.back()=L"changed";
    require(source=="portable" && wide.compare(L"native Ω")==0,"large reference census preserves backing");
    std::array<std::thread,4> workers;
    for(auto& worker:workers)worker=std::thread([&]{for(int i=0;i<1000;++i){AsciiString a(source);UnicodeString u(wide);a.concat("!");u.concat(L"!");}});
    for(auto& worker:workers)worker.join();
}
class Probe : public MemoryPoolObject {
    MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(Probe,"GeneratedProbe",4,4)
public:
    static bool fail;
    Probe(){if(fail)throw std::runtime_error("generated constructor fault");}
};
Probe::~Probe()=default;bool Probe::fail=false;
class alignas(64) AlignedProbe : public MemoryPoolObject {
    MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(AlignedProbe,"GeneratedAlignedProbe",4,4)
};
AlignedProbe::~AlignedProbe()=default;
void pools() {
    MemoryPoolFactory factory;factory.init();
    auto* pool=factory.createMemoryPool("generated slots",33,4,4);
    std::array<void*,9> owners{};
    for(auto& owner:owners) {
        owner=pool->allocateBlock("generated");
        require(reinterpret_cast<std::uintptr_t>(owner)%alignof(std::max_align_t)==0,"native pool alignment");
        const auto* data=static_cast<const unsigned char*>(owner);
        for(Int i=0;i<pool->getAllocationSize();++i)require(data[i]==0,"zeroed declared pool backing");
    }
    require(pool->getUsedBlockCount()==9 && pool->getTotalBlockCount()==12,"pool growth census");
    int foreign=0;rejects([&]{pool->freeBlock(&foreign);},"foreign pool pointer admission");
    rejects([&]{pool->freeBlock(static_cast<char*>(owners[0])+1);},"interior pointer admission");
    rejects([&]{factory.destroyMemoryPool(pool);},"live pool retirement rejected");
    require(pool->getUsedBlockCount()==9,"rejections have no ownership effects");
    for(void* owner:owners)pool->freeBlock(owner);
    rejects([&]{pool->freeBlock(owners[0]);},"duplicate release rejected");
    require(pool->getUsedBlockCount()==0,"all acquisition units retired");
    factory.destroyMemoryPool(pool);
    const PoolInitRec parms[]={{"shared-small",32,4,4},{"shared-large",64,4,4}};
    auto* first=factory.createDynamicMemoryAllocator(2,parms);
    auto* second=factory.createDynamicMemoryAllocator(2,parms);
    void* shared=first->allocateBytes(20,"first owner");
    rejects([&]{second->freeBytes(shared);},"shared pool does not imply DMA ownership");
    require(first->getNthDmaMemoryPool(0)->getUsedBlockCount()==1,"wrong owner leaves reference");
    rejects([&]{factory.destroyMemoryPool(first->getNthDmaMemoryPool(1));},"unused shared dependency still borrowed");
    first->freeBytes(shared);first->freeBytes(nullptr);
    void* raw=second->allocateBytes(4096,"raw owner");
    require(reinterpret_cast<std::uintptr_t>(raw)%alignof(std::max_align_t)==0,"raw native alignment");
    rejects([&]{first->freeBytes(raw);},"raw DMA owner admission");second->freeBytes(raw);
    rejects([&]{first->allocateBytes(-1,"invalid");},"negative DMA length");
    rejects([&]{first->allocateBytes(0,"invalid");},"zero DMA length");
    auto* correctedOwner=first->allocateBytes(1,"corrected owner");first->freeBytes(correctedOwner);
    rejects([&]{first->getNthDmaMemoryPool(2);},"DMA table bound");
    const PoolInitRec late[]={{"candidate-first",16,4,4},{"candidate-overflow",32,INT32_MAX,4}};
    rejects([&]{factory.createDynamicMemoryAllocator(2,late);},"partial offside graph rejection");
    require(!factory.findMemoryPool("candidate-first")&&!factory.findMemoryPool("candidate-overflow"),"candidate registries withdraw");
    const PoolInitRec corrected[]={{"candidate-first",16,4,4},{"candidate-overflow",32,4,4}};
    auto* retry=factory.createDynamicMemoryAllocator(2,corrected);factory.destroyDynamicMemoryAllocator(retry);
    factory.destroyDynamicMemoryAllocator(second);factory.destroyDynamicMemoryAllocator(first);
    auto* concurrent=factory.createMemoryPool("threaded slots",32,4,4);
    std::array<std::thread,4> workers;
    for(auto& worker:workers)worker=std::thread([&]{for(int i=0;i<1000;++i){auto* p=concurrent->allocateBlock("thread");concurrent->freeBlock(p);}});
    for(auto& worker:workers)worker.join();require(concurrent->getUsedBlockCount()==0,"threaded pool units settle");
}
void repeat() {
    AsciiString processString("process-owned");UnicodeString processWide(L"process-owned");
    for(int cycle=0;cycle<3;++cycle) {
        initMemoryManager();require(isMemoryManagerOfficiallyInited(),"original manager startup");
        rejects([]{(void)newInstance(AlignedProbe);},"over-aligned explicit class admission");
        require(!TheMemoryPoolFactory->findMemoryPool("GeneratedAlignedProbe"),"rejected class does not publish pool");
        void* malformed=reinterpret_cast<void*>(std::uintptr_t(1));
        rejects([&]{(void)allocateFromW3DMemPool(malformed,1);},"W3D borrowed identity before dereference");
        rejects([&]{freeFromW3DMemPool(malformed,nullptr);},"W3D free borrowed identity before dereference");
        Probe::fail=true;rejects([]{(void)newInstance(Probe);},"class constructor fault");
        auto* pool=TheMemoryPoolFactory->findMemoryPool("GeneratedProbe");
        require(pool && pool->getUsedBlockCount()==0,"failed class constructor returns acquired slot");
        Probe::fail=false;{MemoryPoolObjectHolder owner(newInstance(Probe));}
        require(pool->getUsedBlockCount()==0,"corrected class retry and holder retirement");
        {MemoryPoolObjectHolder empty;}
        static_assert(!std::is_copy_constructible_v<MemoryPoolObjectHolder>);
        {MemoryPoolObjectHolder owner(newInstance(Probe));
         rejects([&]{owner.hold(nullptr);},"occupied holder rejects overwrite");
         require(pool->getUsedBlockCount()==1,"holder rejection preserves acquisition");}
        require(pool->getUsedBlockCount()==0,"holder rejection settles on destruction");
        void* bytes=TheDynamicMemoryAllocator->allocateBytes(7,"original explicit owner");
        TheDynamicMemoryAllocator->freeBytes(bytes);
        auto normal=std::make_unique<int>(17);auto nothrow=std::unique_ptr<int>(new(std::nothrow) int(19));
        auto array=std::make_unique<int[]>(8);array[7]=29;
        struct alignas(64) Aligned {int value=23;};auto aligned=std::make_unique<Aligned>();
        shutdownMemoryManager();require(!TheMemoryPoolFactory&&!TheDynamicMemoryAllocator,"manager links retire");
        require(*normal==17 && *nothrow==19 && aligned->value==23,"standard allocations outlive game pools");
        require(array[7]==29,"standard array outlives game pools");
        require(processString=="process-owned" && processWide.compare(L"process-owned")==0,"strings outlive pool shutdown");
    }
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"original core family required");const std::string family=argv[1];
        if(family=="values")values();else if(family=="random")randomStreams();else if(family=="strings")strings();
        else if(family=="pools")pools();else if(family=="repeat")repeat();else require(false,"unknown core family");
        std::cout<<"PASS: original core "<<family<<" (not GameLogic acceptance)\n";return 0;
    }catch(ErrorCode error){std::cerr<<"FAIL: original core error "<<std::uint32_t(error)<<'\n';}
    catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';}
    return 1;
}
