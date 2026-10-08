// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
#include "Common/Errors.h"
#include <span>
#include <string>
#include <string_view>
inline std::wstring decodeOriginalUTF16(std::span<const UnsignedShort> units) {
    static_assert(sizeof(WideChar)==4,"Native Linux character storage is not a file unit");
    std::wstring result;result.reserve(units.size());
    for(std::size_t ordinal=0;ordinal<units.size();++ordinal) {
        UnsignedInt value=units[ordinal];
        if(value>=0xd800 && value<=0xdbff) {
            if(++ordinal>=units.size() || units[ordinal]<0xdc00 || units[ordinal]>0xdfff)throw ERROR_BAD_ARG;
            value=0x10000+((value-0xd800)<<10)+(units[ordinal]-0xdc00);
        }else if(value>=0xdc00 && value<=0xdfff)throw ERROR_BAD_ARG;
        if(value==0)throw ERROR_BAD_ARG;
        result+=WideChar(value);
    }
    return result;
}
inline std::wstring normalizeOriginalText(std::wstring_view text) {
    std::wstring output;bool skip=true;WideChar last=0;
    for(WideChar value:text) {
        if(value==L' '&&(last==L' '||skip))continue;
        if(value==L'\n'||value==L'\t') {
            if(last==L' ')output.pop_back();
            output+=value;last=value;skip=true;continue;
        }
        output+=value;last=value;skip=false;
    }
    if(last==L' ')output.pop_back();
    return output;
}
