/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Memory.cpp 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Memory.cpp
//
// Created:   Steven Johnson, August 2001
//
// Desc:      Memory manager
//
// ----------------------------------------------------------------------------

#include "Common/GameMemory.h"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

// Explicit Zero Hour pools, independent of standard/library C++ allocation.
MemoryPoolFactory* TheMemoryPoolFactory=nullptr;
DynamicMemoryAllocator* TheDynamicMemoryAllocator=nullptr;
std::recursive_mutex& originalPoolMutex() {static std::recursive_mutex mutex;return mutex;}
namespace {
bool initialized=false;
constexpr std::size_t alignment=alignof(std::max_align_t);
Int alignedSize(Int size) {
    if(size<=0 || size>std::numeric_limits<Int>::max()-Int(alignment-1))throw ERROR_BAD_ARG;
    return Int((std::size_t(size)+alignment-1)&~(alignment-1));
}
}
class MemoryPoolBlob {
public:
    MemoryPoolBlob* next=nullptr;
    MemoryPoolBlob* previous=nullptr;
    void* backing=nullptr;
    std::size_t stride=0;
    std::vector<unsigned char> live;
    std::vector<Int> freeSlots;
    MemoryPoolBlob(Int size,Int count) : stride(std::size_t(alignedSize(size))) {
        if(count<=0 || std::size_t(count)>std::size_t(std::numeric_limits<Int>::max())/stride)
            throw ERROR_BAD_ARG;
        live.resize(std::size_t(count),0);
        freeSlots.reserve(std::size_t(count));
        for(Int i=count;i>0;--i)freeSlots.push_back(i-1);
        backing=std::malloc(stride*std::size_t(count));
        if(!backing)throw ERROR_OUT_OF_MEMORY;
    }
    ~MemoryPoolBlob(){std::free(backing);}
    bool contains(void* pointer) const {
        const auto address=reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin=reinterpret_cast<std::uintptr_t>(backing);
        return address>=begin && address-begin<stride*live.size() && (address-begin)%stride==0;
    }
    void* acquire() {
        const Int slot=freeSlots.back();freeSlots.pop_back();
        live[std::size_t(slot)]=1;
        return static_cast<char*>(backing)+std::size_t(slot)*stride;
    }
    void release(void* pointer) {
        const auto index=(reinterpret_cast<std::uintptr_t>(pointer)-reinterpret_cast<std::uintptr_t>(backing))/stride;
        if(!live[index])throw ERROR_BAD_ARG;
        live[index]=0;freeSlots.push_back(Int(index)); // reserved, nonallocating
    }
    bool empty() const {return freeSlots.size()==live.size();}
};
class MemoryPoolSingleBlock {
public:
    void* data;
    Int size;
    MemoryPoolSingleBlock* next;
    MemoryPoolSingleBlock(Int bytes) : data(std::malloc(std::size_t(bytes))),size(bytes),next(nullptr) {
        if(!data)throw ERROR_OUT_OF_MEMORY;
    }
    ~MemoryPoolSingleBlock(){std::free(data);}
};

MemoryPool::MemoryPool() :
    m_factory(nullptr),m_nextPoolInFactory(nullptr),m_poolName(""),
    m_allocationSize(0),m_initialAllocationCount(0),m_overflowAllocationCount(0),
    m_usedBlocksInPool(0),m_totalBlocksInPool(0),m_peakUsedBlocksInPool(0),
    m_firstBlob(nullptr),m_lastBlob(nullptr),m_firstBlobWithFreeBlocks(nullptr) {}
void MemoryPool::init(MemoryPoolFactory* factory,const char* name,Int size,Int initial,Int overflow) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!factory || !name || !*name || initial<=0 || overflow<0 || m_firstBlob)throw ERROR_BAD_ARG;
    m_ownedPoolName=name;
    m_factory=factory;m_poolName=m_ownedPoolName.c_str();
    m_allocationSize=alignedSize(size);m_initialAllocationCount=initial;m_overflowAllocationCount=overflow;
    createBlob(initial);
}
MemoryPool::~MemoryPool(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());reset();}
MemoryPoolBlob* MemoryPool::createBlob(Int count) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(count<=0 || count>std::numeric_limits<Int>::max()-m_totalBlocksInPool)throw ERROR_BAD_ARG;
    auto candidate=std::make_unique<MemoryPoolBlob>(m_allocationSize,count);
    candidate->previous=m_lastBlob;
    if(m_lastBlob)m_lastBlob->next=candidate.get();else m_firstBlob=candidate.get();
    m_lastBlob=candidate.get();m_firstBlobWithFreeBlocks=candidate.get();
    m_totalBlocksInPool+=count;
    return candidate.release();
}
Int MemoryPool::freeBlob(MemoryPoolBlob* blob) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!blob || !blob->empty())throw ERROR_BAD_ARG;
    const Int count=Int(blob->live.size());
    if(blob->previous)blob->previous->next=blob->next;else m_firstBlob=blob->next;
    if(blob->next)blob->next->previous=blob->previous;else m_lastBlob=blob->previous;
    if(m_firstBlobWithFreeBlocks==blob)m_firstBlobWithFreeBlocks=nullptr;
    m_totalBlocksInPool-=count;delete blob;return count;
}
void* MemoryPool::allocateBlockDoNotZeroImplementation(DECLARE_LITERALSTRING_ARG1) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    MemoryPoolBlob* blob=m_firstBlobWithFreeBlocks;
    if(!blob || blob->freeSlots.empty()) {
        for(blob=m_firstBlob;blob && blob->freeSlots.empty();blob=blob->next){}
        if(!blob) {
            if(m_overflowAllocationCount==0)throw ERROR_OUT_OF_MEMORY;
            blob=createBlob(m_overflowAllocationCount);
        }
        m_firstBlobWithFreeBlocks=blob;
    }
    void* result=blob->acquire();++m_usedBlocksInPool;
    m_peakUsedBlocksInPool=std::max(m_peakUsedBlocksInPool,m_usedBlocksInPool);
    return result;
}
void* MemoryPool::allocateBlockImplementation(DECLARE_LITERALSTRING_ARG1) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    void* result=allocateBlockDoNotZeroImplementation(PASS_LITERALSTRING_ARG1);
    std::memset(result,0,std::size_t(m_allocationSize));return result;
}
void MemoryPool::freeBlock(void* pointer) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!pointer)return;
    for(auto* blob=m_firstBlob;blob;blob=blob->next)if(blob->contains(pointer)) {
        blob->release(pointer);--m_usedBlocksInPool;m_firstBlobWithFreeBlocks=blob;return;
    }
    throw ERROR_BAD_ARG;
}
Int MemoryPool::countBlobsInPool(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());Int count=0;for(auto* b=m_firstBlob;b;b=b->next)++count;return count;}
Int MemoryPool::releaseEmpties() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    Int count=0;for(auto* blob=m_firstBlob;blob;) {
        auto* next=blob->next;if(blob->empty())count+=freeBlob(blob);blob=next;
    }return count;
}
void MemoryPool::reset() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    while(m_firstBlob){auto* next=m_firstBlob->next;delete m_firstBlob;m_firstBlob=next;}
    m_lastBlob=nullptr;m_firstBlobWithFreeBlocks=nullptr;
    m_usedBlocksInPool=0;m_totalBlocksInPool=0;
}
void MemoryPool::addToList(MemoryPool** head){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());m_nextPoolInFactory=*head;*head=this;}
void MemoryPool::removeFromList(MemoryPool** head) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    for(auto** slot=head;*slot;slot=&(*slot)->m_nextPoolInFactory)
        if(*slot==this){*slot=m_nextPoolInFactory;m_nextPoolInFactory=nullptr;return;}
    throw ERROR_BAD_ARG;
}

DynamicMemoryAllocator::DynamicMemoryAllocator() :
    m_factory(nullptr),m_nextDmaInFactory(nullptr),m_numPools(0),m_usedBlocksInDma(0),m_pools{},m_rawBlocks(nullptr) {}
void DynamicMemoryAllocator::init(MemoryPoolFactory* factory,Int count,const PoolInitRec* parms) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!factory || count<0 || count>MAX_DYNAMICMEMORYALLOCATOR_SUBPOOLS || (count && !parms) || m_factory)
        throw ERROR_BAD_ARG;
    // Complete admission before any shared pool mutation.
    Int previous=0;
    for(Int i=0;i<count;++i) {
        if(!parms[i].poolName || !*parms[i].poolName || parms[i].allocationSize<=previous ||
           parms[i].initialAllocationCount<=0 || parms[i].overflowAllocationCount<0)throw ERROR_BAD_ARG;
        previous=parms[i].allocationSize;
        alignedSize(previous);
    }
    m_factory=factory;
    for(Int i=0;i<count;++i){m_pools[i]=factory->createMemoryPool(&parms[i]);++m_numPools;}
}
DynamicMemoryAllocator::~DynamicMemoryAllocator(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());reset();}
MemoryPool* DynamicMemoryAllocator::findPoolForSize(Int bytes) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    for(Int i=0;i<m_numPools;++i)if(bytes<=m_pools[i]->getAllocationSize())return m_pools[i];
    return nullptr;
}
void* DynamicMemoryAllocator::allocateBytesDoNotZeroImplementation(Int bytes DECLARE_LITERALSTRING_ARG2) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(bytes<=0 || m_usedBlocksInDma==std::numeric_limits<Int>::max())throw ERROR_BAD_ARG;
    MemoryPool* pool=findPoolForSize(bytes);
    std::unique_ptr<MemoryPoolSingleBlock> raw;
    void* result;
    if(pool)result=pool->allocateBlockDoNotZeroImplementation(PASS_LITERALSTRING_ARG1);
    else {raw=std::make_unique<MemoryPoolSingleBlock>(bytes);result=raw->data;}
    try {
        if(!m_liveAllocations.insert(result).second)throw ERROR_BUG;
    } catch(...) {if(pool)pool->freeBlock(result);throw;}
    if(raw){raw->next=m_rawBlocks;m_rawBlocks=raw.release();}
    ++m_usedBlocksInDma;return result;
}
void* DynamicMemoryAllocator::allocateBytesImplementation(Int bytes DECLARE_LITERALSTRING_ARG2) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    void* result=allocateBytesDoNotZeroImplementation(bytes PASS_LITERALSTRING_ARG2);
    std::memset(result,0,std::size_t(getActualAllocationSize(bytes)));return result;
}
void DynamicMemoryAllocator::freeBytes(void* pointer) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!pointer)return;
    const auto position=m_liveAllocations.find(pointer);
    if(position==m_liveAllocations.end())throw ERROR_BAD_ARG;
    for(auto** slot=&m_rawBlocks;*slot;slot=&(*slot)->next)if((*slot)->data==pointer) {
        auto* raw=*slot;*slot=raw->next;delete raw;
        m_liveAllocations.erase(position);--m_usedBlocksInDma;return;
    }
    for(Int i=0;i<m_numPools;++i) {
        // Pool admission compares addresses before dereferencing any backing.
        try {m_pools[i]->freeBlock(pointer);}
        catch(ErrorCode code){if(code==ERROR_BAD_ARG)continue;throw;}
        m_liveAllocations.erase(position);--m_usedBlocksInDma;return;
    }
    throw ERROR_BUG;
}
Int DynamicMemoryAllocator::getActualAllocationSize(Int bytes) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(bytes<=0)throw ERROR_BAD_ARG;
    auto* pool=findPoolForSize(bytes);return pool?pool->getAllocationSize():bytes;
}
void DynamicMemoryAllocator::reset(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());while(!m_liveAllocations.empty())freeBytes(*m_liveAllocations.begin());}
void DynamicMemoryAllocator::addToList(DynamicMemoryAllocator** head){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());m_nextDmaInFactory=*head;*head=this;}
void DynamicMemoryAllocator::removeFromList(DynamicMemoryAllocator** head) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    for(auto** slot=head;*slot;slot=&(*slot)->m_nextDmaInFactory)
        if(*slot==this){*slot=m_nextDmaInFactory;m_nextDmaInFactory=nullptr;return;}
    throw ERROR_BAD_ARG;
}
MemoryPoolFactory::MemoryPoolFactory() : m_firstPoolInFactory(nullptr),m_firstDmaInFactory(nullptr) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());}
void MemoryPoolFactory::init(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());}
MemoryPoolFactory::~MemoryPoolFactory() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    while(m_firstDmaInFactory)destroyDynamicMemoryAllocator(m_firstDmaInFactory);
    while(m_firstPoolInFactory) {
        auto* pool=m_firstPoolInFactory;m_firstPoolInFactory=pool->getNextPoolInList();delete pool;
    }
}
MemoryPool* MemoryPoolFactory::createMemoryPool(const PoolInitRec* parms) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!parms)throw ERROR_BAD_ARG;
    return createMemoryPool(parms->poolName,parms->allocationSize,parms->initialAllocationCount,parms->overflowAllocationCount);
}
MemoryPool* MemoryPoolFactory::createMemoryPool(const char* name,Int size,Int initial,Int overflow) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!name || !*name || size<=0 || initial<-1 || overflow<-1)throw ERROR_BAD_ARG;
    if(auto* pool=findMemoryPool(name)) {
        if(alignedSize(size)!=pool->getAllocationSize())throw ERROR_BAD_ARG;
        return pool;
    }
    userMemoryAdjustPoolSize(name,initial,overflow);
    auto pool=std::make_unique<MemoryPool>();pool->init(this,name,size,initial,overflow);
    pool->addToList(&m_firstPoolInFactory);return pool.release();
}
MemoryPool* MemoryPoolFactory::findMemoryPool(const char* name) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!name)throw ERROR_BAD_ARG;
    for(auto* p=m_firstPoolInFactory;p;p=p->getNextPoolInList())if(std::strcmp(name,p->getPoolName())==0)return p;
    return nullptr;
}
Bool MemoryPoolFactory::ownsMemoryPool(const MemoryPool* owner) const {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    for(auto* p=m_firstPoolInFactory;p;p=p->getNextPoolInList())if(p==owner)return true;
    return false;
}
void MemoryPoolFactory::destroyMemoryPool(MemoryPool* pool) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!pool)return;
    bool found=false;for(auto* p=m_firstPoolInFactory;p;p=p->getNextPoolInList())if(p==pool){found=true;break;}
    if(!found || pool->getUsedBlockCount()!=0)throw ERROR_BAD_ARG;
    for(auto* dma=m_firstDmaInFactory;dma;dma=dma->getNextDmaInList())
        for(Int i=0;i<dma->getDmaMemoryPoolCount();++i)if(dma->getNthDmaMemoryPool(i)==pool)throw ERROR_BAD_ARG;
    pool->removeFromList(&m_firstPoolInFactory);delete pool;
}
DynamicMemoryAllocator* MemoryPoolFactory::createDynamicMemoryAllocator(Int count,const PoolInitRec* parms) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    auto dma=std::make_unique<DynamicMemoryAllocator>();
    auto* oldHead=m_firstPoolInFactory;
    try {dma->init(this,count,parms);}
    catch(...) {
        dma.reset();
        while(m_firstPoolInFactory!=oldHead) {
            auto* pool=m_firstPoolInFactory;m_firstPoolInFactory=pool->getNextPoolInList();delete pool;
        }
        throw;
    }
    dma->addToList(&m_firstDmaInFactory);return dma.release();
}
void MemoryPoolFactory::destroyDynamicMemoryAllocator(DynamicMemoryAllocator* dma) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!dma)return;
    bool found=false;for(auto* p=m_firstDmaInFactory;p;p=p->getNextDmaInList())if(p==dma){found=true;break;}
    if(!found)throw ERROR_BAD_ARG;
    dma->removeFromList(&m_firstDmaInFactory);delete dma;
}
void MemoryPoolFactory::reset() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    for(auto* dma=m_firstDmaInFactory;dma;dma=dma->getNextDmaInList())dma->reset();
    for(auto* pool=m_firstPoolInFactory;pool;pool=pool->getNextPoolInList())pool->reset();
}
void MemoryPoolFactory::memoryPoolUsageReport(const char*,FILE* output) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!output)return;
    std::size_t used=0,capacity=0;
    for(auto* p=m_firstPoolInFactory;p;p=p->getNextPoolInList()){used+=std::size_t(p->getUsedBlockCount());capacity+=std::size_t(p->getTotalBlockCount());}
    std::fprintf(output,"game pool slots live=%zu capacity=%zu\n",used,capacity);
}
void initMemoryManager() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(initialized)throw ERROR_BAD_ARG;
    Int count=0;const PoolInitRec* parms=nullptr;userMemoryManagerGetDmaParms(&count,&parms);
    auto factory=std::make_unique<MemoryPoolFactory>();factory->init();
    auto* dma=factory->createDynamicMemoryAllocator(count,parms);
    userMemoryManagerInitPools();
    TheMemoryPoolFactory=factory.release();TheDynamicMemoryAllocator=dma;initialized=true;
}
Bool isMemoryManagerOfficiallyInited(){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());return initialized;}
void shutdownMemoryManager() {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!initialized)return;
    delete TheMemoryPoolFactory;TheMemoryPoolFactory=nullptr;TheDynamicMemoryAllocator=nullptr;initialized=false;
    userMemoryManagerInitPools(); // Retire startup profile backing as well.
}
void* STLSpecialAlloc::allocate(std::size_t bytes){return ::operator new(bytes);}
void STLSpecialAlloc::deallocate(void* pointer,std::size_t){::operator delete(pointer);}
void* createW3DMemPool(const char* name,int size) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!TheMemoryPoolFactory)throw ERROR_BAD_ARG;
    return TheMemoryPoolFactory->createMemoryPool(name,size,0,0);
}
void* allocateFromW3DMemPool(void* pool,int size) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    auto* owner=static_cast<MemoryPool*>(pool);
    if(!TheMemoryPoolFactory || !TheMemoryPoolFactory->ownsMemoryPool(owner) || size<=0 || size>owner->getAllocationSize())throw ERROR_BAD_ARG;
    return owner->allocateBlockDoNotZeroImplementation(PASS_LITERALSTRING_ARG1);
}
void* allocateFromW3DMemPool(void* pool,int size,const char*,int){
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());return allocateFromW3DMemPool(pool,size);}
void freeFromW3DMemPool(void* pool,void* pointer) {
    std::lock_guard<std::recursive_mutex> lock(originalPoolMutex());
    if(!TheMemoryPoolFactory || !TheMemoryPoolFactory->ownsMemoryPool(static_cast<MemoryPool*>(pool)))throw ERROR_BAD_ARG;
    static_cast<MemoryPool*>(pool)->freeBlock(pointer);
}
