// SPDX-License-Identifier: GPL-3.0-or-later
// Fixture-only complete standard allocation boundary. No production interposition.
#include "AllocationFault.h"
#include <atomic>
#include <cstdlib>
#include <new>
namespace {
std::atomic<std::size_t> outstanding{0};
thread_local bool enabled=false,hit=false;
thread_local std::size_t remaining=0;
thread_local std::size_t attempted=0;
void* allocate(std::size_t bytes,std::size_t alignment=0) {
    if(enabled){++attempted;if(remaining--==0){enabled=false;hit=true;throw std::bad_alloc();}}
    void* result=nullptr;
    if(alignment){if(::posix_memalign(&result,alignment,bytes?bytes:1)!=0)result=nullptr;}
    else result=std::malloc(bytes?bytes:1);
    if(!result)throw std::bad_alloc();
    ++outstanding;return result;
}
void release(void* value) noexcept {if(value){--outstanding;std::free(value);}}
}
namespace AllocationFault {
void arm(std::size_t ordinal) noexcept {remaining=ordinal;attempted=0;hit=false;enabled=true;}
void disarm() noexcept {enabled=false;}
std::size_t live() noexcept {return outstanding.load();}
bool triggered() noexcept {return hit;}
std::size_t attempts() noexcept {return attempted;}
}
void* operator new(std::size_t size){return allocate(size);}
void* operator new[](std::size_t size){return allocate(size);}
void* operator new(std::size_t size,std::align_val_t alignment){return allocate(size,std::size_t(alignment));}
void* operator new[](std::size_t size,std::align_val_t alignment){return allocate(size,std::size_t(alignment));}
void* operator new(std::size_t size,const std::nothrow_t&) noexcept {try{return allocate(size);}catch(...){return nullptr;}}
void* operator new[](std::size_t size,const std::nothrow_t&) noexcept {try{return allocate(size);}catch(...){return nullptr;}}
void* operator new(std::size_t size,std::align_val_t alignment,const std::nothrow_t&) noexcept {try{return allocate(size,std::size_t(alignment));}catch(...){return nullptr;}}
void* operator new[](std::size_t size,std::align_val_t alignment,const std::nothrow_t&) noexcept {try{return allocate(size,std::size_t(alignment));}catch(...){return nullptr;}}
void operator delete(void* p) noexcept {release(p);}
void operator delete[](void* p) noexcept {release(p);}
void operator delete(void* p,std::size_t) noexcept {release(p);}
void operator delete[](void* p,std::size_t) noexcept {release(p);}
void operator delete(void* p,std::align_val_t) noexcept {release(p);}
void operator delete[](void* p,std::align_val_t) noexcept {release(p);}
void operator delete(void* p,std::size_t,std::align_val_t) noexcept {release(p);}
void operator delete[](void* p,std::size_t,std::align_val_t) noexcept {release(p);}
void operator delete(void* p,const std::nothrow_t&) noexcept {release(p);}
void operator delete[](void* p,const std::nothrow_t&) noexcept {release(p);}
void operator delete(void* p,std::align_val_t,const std::nothrow_t&) noexcept {release(p);}
void operator delete[](void* p,std::align_val_t,const std::nothrow_t&) noexcept {release(p);}
