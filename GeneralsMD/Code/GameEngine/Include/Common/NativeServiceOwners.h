// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/Errors.h"
#include <array>
#include <cstddef>
#include <memory>

// Simulation-thread ownership, not a cross-thread publication mechanism.
// Slots outlive their parent. Entries own acquired objects, not current globals.
template<std::size_t Capacity> class NativeServiceOwners {
    struct Entry {
        void* slot=nullptr;
        void* owner=nullptr;
        void (*retire)(void*,void*) noexcept=nullptr;
    };
    std::array<Entry,Capacity> entries{};
    std::size_t highWater=0;
    bool draining=false;
public:
    NativeServiceOwners()=default;
    NativeServiceOwners(const NativeServiceOwners&)=delete;
    NativeServiceOwners& operator=(const NativeServiceOwners&)=delete;
    ~NativeServiceOwners() {clear();}

    template<class T,class FACTORY>
    T* create(T*& slot, FACTORY&& factory, bool required=true) {
        if (draining || slot) throw ERROR_BAD_ARG;
        // Do not construct an unjournaled object or replace an accepted owner.
        // Preserve acquisition order; only reclaim holes after tail retirement.
        if (highWater==Capacity) throw ERROR_BAD_ARG;
        std::unique_ptr<T> candidate(factory());
        if (!candidate) {
            if (required) throw ERROR_BAD_ARG;
            return nullptr;
        }
        Entry& entry=entries[highWater++];
        entry={&slot,candidate.get(),[](void* address,void* acquired) noexcept {
            auto& reference=*static_cast<T**>(address);
            // Cleanup may clear its own slot, and may call this live identity.
            reference=static_cast<T*>(acquired);
            delete static_cast<T*>(acquired);
            reference=nullptr;
        }};
        slot=candidate.release(); // No fallible step after publication.
        return slot;
    }
    template<class T> bool owns(T*& slot) const noexcept {
        for(std::size_t i=0;i<highWater;++i)
            if(entries[i].slot==&slot) return true;
        return false;
    }
    template<class T> void retire(T*& slot) noexcept {
        for(std::size_t i=highWater;i>0;--i) {
            if(entries[i-1].slot!=&slot) continue;
            const Entry entry=entries[i-1];
            entries[i-1]={}; // Recursive cleanup cannot release this unit twice.
            const bool wasDraining=draining;
            draining=true;
            entry.retire(entry.slot,entry.owner);
            draining=wasDraining;
            while(highWater && !entries[highWater-1].slot) --highWater;
            return;
        }
    }
    void clear() noexcept {
        if(draining) return;
        draining=true;
        while(highWater) {
            const Entry entry=entries[--highWater];
            entries[highWater]={};
            if(entry.slot) entry.retire(entry.slot,entry.owner);
        }
        draining=false;
    }
};
