// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
#include "Common/Errors.h"
#include <charconv>
#include <string>
#include <string_view>

struct NativeCinematicFont {
    std::string name;
    Int pointSize;
    Bool bold;
};
NativeCinematicFont parseNativeCinematicFont(std::string_view label);
class NativeUserStorage;
std::string nativeMapCompanionPath(std::string_view mapName, std::string_view companion,
    const NativeUserStorage* storage=nullptr);
std::string_view nativePathLeaf(std::string_view path) noexcept;
Int nativeScriptFrames(Int seconds);
std::string nativeEncodePlayerName(std::wstring_view text);
std::wstring nativeDecodePlayerName(std::string_view text);
std::string_view nativePlayerNamePrefix(std::string_view text,std::size_t maximumBytes);
template<class INTEGER>
INTEGER nativeSourceInteger(std::string_view text,int base=10) {
    constexpr std::string_view whitespace=" \t\r\n\v\f";
    const auto first=text.find_first_not_of(whitespace);
    if (first==text.npos) throw ERROR_BAD_ARG;
    text=text.substr(first,text.find_last_not_of(whitespace)-first+1);
    if (text.front()=='+') {
        text.remove_prefix(1);
        if (!text.empty() && (text.front()=='+' || text.front()=='-')) throw ERROR_BAD_ARG;
    }
    if (base==16 && (text.starts_with("0x") || text.starts_with("0X"))) text.remove_prefix(2);
    if (text.empty()) throw ERROR_BAD_ARG;
    INTEGER value{};
    const auto result=std::from_chars(text.data(),text.data()+text.size(),value,base);
    if (result.ec!=std::errc{} || result.ptr!=text.data()+text.size()) throw ERROR_BAD_ARG;
    return value;
}
