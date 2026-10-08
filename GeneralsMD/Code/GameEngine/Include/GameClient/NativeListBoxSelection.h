// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/Gadget.h"
#include "GameClient/WindowMessageData.h"
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>

// Offside backing acquisition borrows retained cell owners until publication.
// Failed construction retires both arrays without touching accepted rows.
struct NativeListBoxBacking {
    std::unique_ptr<ListEntryRow[]> rows;
    std::unique_ptr<Int[]> selections;
    NativeListBoxBacking(const ListboxData& list, Int length) {
        if (length<0 || length>std::numeric_limits<Short>::max()
            || list.listLength<0 || (list.listLength>0 && !list.listData))
            throw std::invalid_argument("invalid list backing capacity");
        rows=std::make_unique<ListEntryRow[]>(static_cast<std::size_t>(length));
        if (list.multiSelect) {
            selections=std::make_unique<Int[]>(static_cast<std::size_t>(length)+1);
            for (Int i=0;i<=length;++i) selections[i]=-1;
        }
        const Int keep=length<list.listLength ? length : list.listLength;
        if (keep>0) std::memcpy(rows.get(),list.listData,keep*sizeof(ListEntryRow));
    }
};

// Selection backing is capacity + one terminating slot; indices name live rows.
inline bool nativeListBoxSelectionOwner(const ListboxData& list) noexcept {
    return list.listLength >= 0 && list.endPos >= 0 && list.endPos <= list.listLength
        && (list.endPos == 0 || list.listData != nullptr)
        && (!list.multiSelect || list.selections != nullptr);
}
inline void nativeListBoxClearSelection(ListboxData& list) noexcept {
    if (list.multiSelect) {
        for (Int i=0; i<=list.listLength; ++i) list.selections[i]=-1;
    } else list.selectPos=-1;
}
inline bool nativeListBoxSetSelection(ListboxData& list, const Int* input,
                                      WindowMsgData rawCount) noexcept {
    if (!nativeListBoxSelectionOwner(list) || !input || rawCount==0
        || rawCount>static_cast<WindowMsgData>(std::numeric_limits<Int>::max())
        || (!list.multiSelect && rawCount!=1)) return false;
    // The source's first negative/out-of-capacity selection means clear.
    if (input[0]<0 || input[0]>=list.listLength) {
        nativeListBoxClearSelection(list); return true;
    }
    if (rawCount>static_cast<WindowMsgData>(list.endPos)) return false;
    const Int count=static_cast<Int>(rawCount);
    for (Int i=0; i<count; ++i)
        if (input[i]<0 || input[i]>=list.endPos || !list.listData[input[i]].cell)
            return false;
    if (list.multiSelect) {
        std::memmove(list.selections,input,count*sizeof(Int));
        list.selections[count]=-1;
    } else list.selectPos=input[0];
    return true;
}
inline void nativeListBoxRemoveSelection(ListboxData& list, Int index) noexcept {
    if (!nativeListBoxSelectionOwner(list) || !list.multiSelect
        || index<0 || index>=list.listLength) return;
    std::memmove(list.selections+index,list.selections+index+1,
                 (list.listLength-index)*sizeof(Int));
    list.selections[list.listLength]=-1;
}
inline bool nativeListBoxToggleSelection(ListboxData& list, Int row) noexcept {
    if (!nativeListBoxSelectionOwner(list) || !list.multiSelect || row<0
        || row>=list.endPos || !list.listData[row].cell) return false;
    Int i=0;
    for (; i<list.listLength && list.selections[i]>=0; ++i)
        if (list.selections[i]==row) {
            nativeListBoxRemoveSelection(list,i); return true;
        }
    if (i==list.listLength) return false;
    list.selections[i]=row; list.selections[i+1]=-1;
    return true;
}
// Tag 0 is an Int output; tag 1 is an Int* borrowed output. Never infer the
// output's storage width from window state alone.
inline bool nativeListBoxGetSelection(const ListboxData& list, WindowMsgData tag,
                                      void* output) noexcept {
    if (!nativeListBoxSelectionOwner(list) || !output) return false;
    if (tag==0 && !list.multiSelect) {
        *static_cast<Int*>(output)=list.selectPos; return true;
    }
    if (tag==1 && list.multiSelect) {
        *static_cast<Int**>(output)=list.selections; return true;
    }
    return false;
}
inline void nativeListBoxScrollSelection(ListboxData& list, Int removed) noexcept {
    if (list.multiSelect) {
        Int write=0;
        for (Int read=0; read<list.listLength && list.selections[read]>=0; ++read)
            if (list.selections[read]>=removed)
                list.selections[write++]=list.selections[read]-removed;
        list.selections[write]=-1;
    } else list.selectPos=list.selectPos>=removed ? list.selectPos-removed : -1;
}
inline bool nativeListBoxInsertRow(ListboxData& list, Int row) noexcept {
    if (!nativeListBoxSelectionOwner(list) || row<0 || row>list.endPos
        || list.endPos>=list.listLength) return false;
    std::memmove(list.listData+row+1,list.listData+row,
        (list.endPos-row)*sizeof(ListEntryRow));
    list.listData[row]={};
    ++list.endPos; list.insertPos=list.endPos;
    if (list.multiSelect) {
        for (Int i=0;i<list.listLength && list.selections[i]>=0;++i)
            if (list.selections[i]>=row) ++list.selections[i];
    } else if (list.selectPos>=row) ++list.selectPos;
    return true;
}
