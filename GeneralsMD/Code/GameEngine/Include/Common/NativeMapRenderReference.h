// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
class RenderObjClass;
struct NativeMapRenderOwnership {
    void (*retain)(RenderObjClass*) noexcept=nullptr;
    void (*release)(RenderObjClass*) noexcept=nullptr;
};
// One captured acquired ownership unit per attachment, independent of numerical
// pointer identity and of the lifetime of a renderer service registry.
class NativeMapRenderReference final {
    RenderObjClass* m_resource=nullptr;
    NativeMapRenderOwnership m_owner{};
public:
    NativeMapRenderReference() noexcept=default;
    ~NativeMapRenderReference() noexcept { if(m_resource) m_owner.release(m_resource); }
    NativeMapRenderReference(const NativeMapRenderReference&)=delete;
    NativeMapRenderReference& operator=(const NativeMapRenderReference&)=delete;
    RenderObjClass* get() const noexcept { return m_resource; }
    void reset(RenderObjClass* resource=nullptr,NativeMapRenderOwnership owner={}) {
        if(resource && (!owner.retain || !owner.release)) throw ERROR_BAD_ARG;
        if(resource) owner.retain(resource);
        auto* retired=m_resource; const auto retiredOwner=m_owner;
        m_resource=resource; m_owner=resource ? owner : NativeMapRenderOwnership{};
        if(retired) retiredOwner.release(retired);
    }
};
