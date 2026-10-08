// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/UnicodeString.h"
#include "Common/Xfer.h"
#include "Common/NativeUserStorage.h"
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>
#include <array>

inline std::string nativeTransferUserPath(const NativeUserStorage& storage,
                                           const AsciiString& identifier) {
    std::string path(identifier.str(),static_cast<std::size_t>(identifier.getLength()));
    if (!path.empty() && path.front()=='/') {
        auto relative=storage.relativeDataPath(path);
        if (!relative || relative->empty()) throw XFER_INVALID_PARAMETERS;
        return *relative;
    }
    for (char& byte:path) if (byte=='\\') byte='/';
    return path; // The captured storage owner validates all remaining components.
}

// Complete offside UTF16LE wire payload, never native wchar_t backing.
inline std::vector<unsigned char> nativeTransferUTF16(const UnicodeString& text,
                                                     std::size_t maxUnits) {
    std::size_t units=0;
    for (Int i=0;i<text.getLength();++i) {
        const auto scalar=static_cast<std::uint32_t>(text.getCharAt(i));
        if (!scalar || scalar>0x10ffffu || (scalar>=0xd800u && scalar<=0xdfffu))
            throw XFER_STRING_ERROR;
        const std::size_t count=scalar>=0x10000u ? 2 : 1;
        if (units>maxUnits || count>maxUnits-units) throw XFER_STRING_ERROR;
        units+=count;
    }
    if (units>static_cast<std::size_t>(std::numeric_limits<Int>::max())/2)
        throw XFER_STRING_ERROR;
    std::vector<unsigned char> result(units*2);
    std::size_t index=0;
    const auto append=[&](std::uint32_t unit) {
        result[index++]=static_cast<unsigned char>(unit);
        result[index++]=static_cast<unsigned char>(unit>>8);
    };
    for (Int i=0;i<text.getLength();++i) {
        auto scalar=static_cast<std::uint32_t>(text.getCharAt(i));
        if (scalar<0x10000u) append(scalar);
        else {
            scalar-=0x10000u;
            append(0xd800u+(scalar>>10)); append(0xdc00u+(scalar&1023u));
        }
    }
    return result;
}
inline UnicodeString nativeTransferDecodeUTF16(std::span<const unsigned char> bytes) {
    if (bytes.size()%2) throw XFER_STRING_ERROR;
    std::wstring result;
    const auto word=[&](std::size_t i) {
        return std::uint32_t(bytes[i]) | (std::uint32_t(bytes[i+1])<<8);
    };
    for (std::size_t i=0;i<bytes.size();i+=2) {
        std::uint32_t scalar=word(i);
        if (!scalar) throw XFER_STRING_ERROR;
        if (scalar>=0xd800u && scalar<=0xdbffu) {
            if (bytes.size()-i<4) throw XFER_STRING_ERROR;
            const auto low=word(i+2);
            if (low<0xdc00u || low>0xdfffu) throw XFER_STRING_ERROR;
            scalar=0x10000u+((scalar-0xd800u)<<10)+(low-0xdc00u); i+=2;
        } else if (scalar>=0xdc00u && scalar<=0xdfffu) throw XFER_STRING_ERROR;
        result.push_back(static_cast<WideChar>(scalar));
    }
    return UnicodeString(result.c_str());
}

// Original replay readers reserve 1024 slots including the terminator. Every
// unit is read in byte mode; the stream must supply the complete requested span.
template<class READ> UnicodeString nativeReplayReadUnicode(READ read) {
    std::array<unsigned char,2046> payload{};
    for (std::size_t units=0;units<=1023;++units) {
        std::array<unsigned char,2> word{};
        if (!read(word.data(),2)) throw XFER_READ_ERROR;
        if (!word[0] && !word[1])
            return nativeTransferDecodeUTF16(std::span<const unsigned char>(payload).first(units*2));
        if (units==1023) throw XFER_STRING_ERROR;
        payload[units*2]=word[0]; payload[units*2+1]=word[1];
    }
    throw XFER_STRING_ERROR;
}
template<class READ> AsciiString nativeReplayReadAscii(READ read) {
    std::array<Char,1024> payload{};
    for (std::size_t index=0;index<payload.size();++index) {
        if (!read(&payload[index],1)) throw XFER_READ_ERROR;
        if (!payload[index]) return AsciiString(payload.data());
    }
    throw XFER_STRING_ERROR;
}
