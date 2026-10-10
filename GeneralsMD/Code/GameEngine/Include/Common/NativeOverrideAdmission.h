// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/INI.h"
#include "Common/Override.h"
#include <memory>

struct NativePooledRetire {
  void operator()(MemoryPoolObject* owner) const noexcept { if(owner)owner->deleteInstance(); }
};

// Payload assignment must never lend the source's owning next link to a
// candidate, including while unwinding a later member-copy failure.
template<class T>void nativeCopyOverridePayload(T& destination,const T& source) {
  struct Links {
    T& owner;Overridable* next;Bool status;
    ~Links() noexcept {owner.friend_restoreOverrideOwnership(next,status);}
  } links{destination,destination.friend_getNextOverride(),destination.friend_isOverride()};
  destination=source;
}

template<class T,class Factory,class Parse,class Notify>
void nativeAdmitOverride(OVERRIDE<T>& slot,Bool overrideLoad,Factory create,Parse parse,Notify notify) {
  T* base=const_cast<T*>(slot.getNonOverloadedPointer());
  if(base && !overrideLoad)throw INI_INVALID_DATA;
  std::unique_ptr<T,NativePooledRetire> candidate(create());
  if(!candidate)throw ERROR_BAD_ARG;
  if(base){nativeCopyOverridePayload(*candidate,*base);candidate->markAsOverride();}
  parse(*candidate); // No owning publication before the complete payload parses.
  Overridable* tail=base?base->friend_getFinalOverride():nullptr;
  if(tail)tail->setNextOverride(candidate.get());else slot=candidate.get();
  try{notify(base,*candidate);}catch(...){
    if(tail)tail->setNextOverride(nullptr);else slot=base;
    throw;
  }
  candidate.release(); // The source chain acquired this one unit.
}
