// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeTransferWire.h"
#include <cstring>
#include <exception>
#include <cstdio>
#include <algorithm>
#include <bit>
#include <ctime>

// Original-game replay ownership over captured protected user storage. No
// supplied asset descriptors, ambient CWD or raw writable pathname escapes.
class NativeReplaySession final {
    std::unique_ptr<NativeAtomicOutput> m_output;
    std::vector<unsigned char> m_input;
    std::size_t m_cursor=0;
    bool m_recording=false,m_poisoned=false,m_committed=false;
    NativeReplaySession()=default;
    static std::size_t byteCount(std::size_t size,std::size_t count) {
        if (size && count>std::size_t(std::numeric_limits<Int>::max())/size) throw XFER_INVALID_PARAMETERS;
        return size*count;
    }
    void ready() const { if (m_poisoned || m_committed) throw XFER_INVALID_PARAMETERS; }
public:
    static constexpr std::size_t MaximumBytes=std::numeric_limits<Int>::max();
    class Guard {
        NativeReplaySession& owner; int exceptions=std::uncaught_exceptions();
    public:
        explicit Guard(NativeReplaySession& value) noexcept : owner(value) {}
        ~Guard() { if (std::uncaught_exceptions()>exceptions) owner.poison(); }
    };
    // Parent operations may withdraw/recreate a session. Refer to its owning
    // link rather than retaining a pointer that reset/EOF could destroy.
    class OwnerGuard {
        std::unique_ptr<NativeReplaySession>& owner; int exceptions=std::uncaught_exceptions();
    public:
        explicit OwnerGuard(std::unique_ptr<NativeReplaySession>& value) noexcept : owner(value) {}
        ~OwnerGuard() { if (std::uncaught_exceptions()>exceptions && owner) owner->poison(); }
    };
    static std::unique_ptr<NativeReplaySession> record(const NativeUserStorage& storage,const AsciiString& filename) {
        auto candidate=std::unique_ptr<NativeReplaySession>(new NativeReplaySession);
        candidate->m_output=storage.beginWrite(NativeUserArea::Data,nativeTransferUserPath(storage,filename));
        candidate->m_recording=true; return candidate;
    }
    static std::unique_ptr<NativeReplaySession> playback(const NativeUserStorage& storage,const AsciiString& filename) {
        auto bytes=storage.readFile(NativeUserArea::Data,nativeTransferUserPath(storage,filename),MaximumBytes);
        if (!bytes) return nullptr;
        auto candidate=std::unique_ptr<NativeReplaySession>(new NativeReplaySession);
        candidate->m_input=std::move(*bytes); return candidate;
    }
    ~NativeReplaySession()=default; // Unpublished output aborts, never commits.
    NativeReplaySession(const NativeReplaySession&)=delete;
    NativeReplaySession& operator=(const NativeReplaySession&)=delete;
    void poison() noexcept { m_poisoned=true; }
    bool poisoned() const noexcept { return m_poisoned; }
    std::uint64_t position() const {
        ready(); return m_recording ? m_output->position() : m_cursor;
    }
    std::size_t read(void* bytes,std::size_t size,std::size_t count) {
        Guard guard(*this); ready();
        const auto requested=byteCount(size,count);
        if (m_recording || (requested && !bytes)) throw XFER_INVALID_PARAMETERS;
        if (!size || !count) return 0;
        const auto obtained=std::min(requested,m_input.size()-m_cursor);
        if (obtained) std::memcpy(bytes,m_input.data()+m_cursor,obtained);
        m_cursor+=obtained; return obtained/size;
    }
    std::size_t write(const void* bytes,std::size_t size,std::size_t count) {
        Guard guard(*this); ready();
        const auto requested=byteCount(size,count);
        if (!m_recording || (requested && !bytes) || requested>MaximumBytes-m_output->position()) throw XFER_INVALID_PARAMETERS;
        if (!size || !count) return 0;
        if (m_output->write(bytes,static_cast<Int>(requested))!=static_cast<Int>(requested)) throw XFER_WRITE_ERROR;
        return count;
    }
    Int seek(std::int64_t offset,Int origin=SEEK_SET) {
        Guard guard(*this); ready();
        if (origin!=SEEK_SET || offset<0 || std::uint64_t(offset)>MaximumBytes) throw XFER_INVALID_PARAMETERS;
        if (m_recording) m_output->seek(std::uint64_t(offset));
        else {
            if (std::uint64_t(offset)>m_input.size()) throw XFER_INVALID_PARAMETERS;
            m_cursor=static_cast<std::size_t>(offset);
        }
        return 0;
    }
    Int flush() { Guard guard(*this); ready(); if (!m_recording) throw XFER_INVALID_PARAMETERS; return 0; }
    void readExact(void* bytes,std::size_t count) {
        Guard guard(*this); if (read(bytes,1,count)!=count) throw XFER_READ_ERROR;
    }
    UnsignedInt readWord() {
        Guard guard(*this); std::array<unsigned char,4> bytes{}; readExact(bytes.data(),bytes.size());
        return UnsignedInt(bytes[0]) | (UnsignedInt(bytes[1])<<8) | (UnsignedInt(bytes[2])<<16) | (UnsignedInt(bytes[3])<<24);
    }
    void writeWord(UnsignedInt value) {
        Guard guard(*this); std::array<unsigned char,4> bytes{};
        for (unsigned i=0;i<4;++i) bytes[i]=static_cast<unsigned char>(value>>(i*8));
        write(bytes.data(),1,bytes.size());
    }
    void writeEpoch(std::time_t value) {
        Guard guard(*this);
        if (value<std::numeric_limits<Int>::min() || value>std::numeric_limits<Int>::max()) throw XFER_INVALID_PARAMETERS;
        writeWord(std::bit_cast<UnsignedInt>(static_cast<Int>(value)));
    }
    std::time_t readEpoch() { Guard guard(*this); return std::bit_cast<Int>(readWord()); }
    Bool readBoolean() {
        Guard guard(*this); unsigned char value=0; readExact(&value,1);
        if (value>1) throw XFER_INVALID_PARAMETERS;
        return value!=0;
    }
    void writeUnicode(const UnicodeString& value) {
        Guard guard(*this); ready();
        const auto bytes=nativeTransferUTF16(value,1023);
        write(bytes.data(),1,bytes.size());
        const std::array<unsigned char,2> terminal{}; write(terminal.data(),1,terminal.size());
    }
    NativeCommitResult commit() {
        Guard guard(*this); ready();
        if (!m_recording) throw XFER_INVALID_PARAMETERS;
        const auto result=m_output->commit(); m_committed=true; return result;
    }
};
