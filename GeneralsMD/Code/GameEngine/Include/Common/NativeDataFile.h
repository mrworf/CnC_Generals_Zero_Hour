// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/file.h"
#include <memory>
#include <string>

// Shared read-only descriptor owner. File views keep this alive after unmount.
struct NativeDataBacking {
    int descriptor=-1;
    std::uint64_t length=0;
    explicit NativeDataBacking(const std::string& path);
    // Caller owns this already-admitted descriptor until construction returns.
    // Failure leaves it with the caller; successful construction adopts it.
    struct AdmittedDescriptor {};
    NativeDataBacking(int descriptor, AdmittedDescriptor);
    ~NativeDataBacking();
    NativeDataBacking(const NativeDataBacking&)=delete;
    NativeDataBacking& operator=(const NativeDataBacking&)=delete;
    Int readAt(void* data,Int bytes,std::uint64_t offset) const;
};
class NativeDataFile final : public File {
    MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(NativeDataFile,"NativeDataFile",32,32)
    std::shared_ptr<NativeDataBacking> m_backing;
    std::uint64_t m_offset=0;
    Int m_length=0,m_cursor=0;
public:
    NativeDataFile(std::shared_ptr<NativeDataBacking> backing,std::uint64_t offset,
                   Int length,const char* logicalName,Int access);
    Bool open(const Char*,Int=0) override {return FALSE;}
    void close() override;
    Int read(void*,Int) override;
    Int write(const void*,Int) override {return -1;}
    Int seek(Int,seekMode=CURRENT) override;
    Int size() override {return m_open?m_length:0;}
    void nextLine(Char* =nullptr,Int=0) override;
    Bool scanInt(Int&) override;
    Bool scanReal(Real&) override;
    Bool scanString(AsciiString&) override;
    char* readEntireAndClose() override;
    File* convertToRAMFile() override;
};
