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

// FILE: QuotedPrintable.cpp /////////////////////////////////////////////////////////
// Author: Matt Campbell, February 2002
// Description: Quoted-printable encode/decode
////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/QuotedPrintable.h"
#include <cstdint>
#include <string>
#include <string_view>

namespace {
constexpr char hex[]="0123456789ABCDEF";
bool literal(unsigned byte) noexcept {
    return (byte>='0' && byte<='9') || (byte>='A' && byte<='Z') ||
           (byte>='a' && byte<='z');
}
unsigned digit(char value) {
    if(value>='0' && value<='9') return unsigned(value-'0');
    if(value>='a' && value<='f') return unsigned(value-'a')+10;
    if(value>='A' && value<='F') return unsigned(value-'A')+10;
    throw ERROR_BAD_ARG;
}
struct Bytes {
    std::string_view input;
    std::size_t position=0;
    bool empty() const noexcept {return position==input.size();}
    unsigned next() {
        if(empty()) throw ERROR_BAD_ARG;
        const auto value=static_cast<unsigned char>(input[position++]);
        if(value!='_') return value;
        if(input.size()-position<2) throw ERROR_BAD_ARG;
        const unsigned high=digit(input[position++]);
        return (high<<4)|digit(input[position++]);
    }
    unsigned word() {
        const unsigned low=next();
        // Source odd-tail policy pads the last low byte, not a stale tail.
        return low | (empty()?0u:next()<<8);
    }
};
std::string_view view(const AsciiString& value) {
    return {value.str(),static_cast<std::size_t>(value.getLength())};
}
void countByte(std::size_t& count,unsigned value) {
    const std::size_t cost=literal(value)?1u:3u;
    if(count>std::size_t(AsciiString::MAX_LEN-1)-cost) throw ERROR_OUT_OF_MEMORY;
    count+=cost;
}
void appendByte(std::string& output,unsigned value) {
    if(literal(value)) output.push_back(static_cast<char>(value));
    else {output.push_back('_');output.push_back(hex[value>>4]);output.push_back(hex[value&15]);}
}
template<class EMIT> void unicodeBytes(const UnicodeString& input,EMIT emit) {
    for(Int index=0;index<input.getLength();++index) {
        std::uint32_t scalar=static_cast<std::uint32_t>(input.getCharAt(index));
        if(!scalar || scalar>0x10ffffu || (scalar>=0xd800u && scalar<=0xdfffu))
            throw ERROR_BAD_ARG;
        const auto unit=[&](std::uint32_t value) {emit(value&255u);emit(value>>8);};
        if(scalar<0x10000u) unit(scalar);
        else {scalar-=0x10000u;unit(0xd800u+(scalar>>10));unit(0xdc00u+(scalar&1023u));}
    }
}
template<class EMIT> void unicodeScalars(Bytes bytes,EMIT emit) {
    while(!bytes.empty()) {
        std::uint32_t scalar=bytes.word();
        if(!scalar) throw ERROR_BAD_ARG;
        if(scalar>=0xd800u && scalar<=0xdbffu) {
            if(bytes.empty()) throw ERROR_BAD_ARG;
            const unsigned low=bytes.word();
            if(low<0xdc00u || low>0xdfffu) throw ERROR_BAD_ARG;
            scalar=0x10000u+((scalar-0xd800u)<<10)+(low-0xdc00u);
        } else if(scalar>=0xdc00u && scalar<=0xdfffu) throw ERROR_BAD_ARG;
        emit(static_cast<WideChar>(scalar));
    }
}
}
AsciiString AsciiStringToQuotedPrintable(AsciiString original) {
    std::size_t size=0;
    for(unsigned char byte:view(original)) countByte(size,byte);
    std::string output;output.reserve(size);
    for(unsigned char byte:view(original)) appendByte(output,byte);
    return AsciiString(output.c_str());
}
AsciiString UnicodeStringToQuotedPrintable(UnicodeString original) {
    std::size_t size=0;
    unicodeBytes(original,[&](unsigned byte){countByte(size,byte);});
    std::string output;output.reserve(size);
    unicodeBytes(original,[&](unsigned byte){appendByte(output,byte);});
    return AsciiString(output.c_str());
}
AsciiString QuotedPrintableToAsciiString(AsciiString original) {
    Bytes prepared{view(original)};std::size_t size=0;
    while(!prepared.empty()) {
        if(!prepared.next()) throw ERROR_BAD_ARG;
        if(++size>=std::size_t(AsciiString::MAX_LEN)) throw ERROR_OUT_OF_MEMORY;
    }
    std::string output;output.reserve(size);
    Bytes bytes{view(original)};
    while(!bytes.empty()) output.push_back(static_cast<char>(bytes.next()));
    return AsciiString(output.c_str());
}
UnicodeString QuotedPrintableToUnicodeString(AsciiString original) {
    std::size_t size=0;
    unicodeScalars(Bytes{view(original)},[&](WideChar) {
        if(++size>=std::size_t(UnicodeString::MAX_LEN)) throw ERROR_OUT_OF_MEMORY;
    });
    std::wstring output;output.reserve(size);
    unicodeScalars(Bytes{view(original)},[&](WideChar scalar){output.push_back(scalar);});
    return UnicodeString(output.c_str());
}
