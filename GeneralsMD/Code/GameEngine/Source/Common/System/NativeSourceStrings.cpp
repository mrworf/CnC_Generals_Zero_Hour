// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeSourceStrings.h"
#include "Common/Errors.h"
#include "Common/GameCommon.h"
#include "Common/NativeUserStorage.h"
#include <charconv>
#include <cstdint>
#include <limits>

NativeCinematicFont parseNativeCinematicFont(std::string_view label)
{
    if (label.find('\0') != std::string_view::npos) throw ERROR_BAD_ARG;
    constexpr std::string_view marker = " - Size:";
    constexpr std::string_view boldSuffix = " [Bold]";
    const auto separator = label.rfind(marker);
    if (separator == std::string_view::npos || separator == 0) throw ERROR_BAD_ARG;
    auto size = label.substr(separator + marker.size());
    const bool bold = size.ends_with(boldSuffix);
    if (bold) size.remove_suffix(boldSuffix.size());
    if (size.starts_with('+')) size.remove_prefix(1);
    Int pointSize = 0;
    const auto result = std::from_chars(size.data(), size.data()+size.size(), pointSize);
    if (result.ec != std::errc{} || result.ptr != size.data()+size.size() || pointSize <= 0)
        throw ERROR_BAD_ARG;
    return {std::string(label.substr(0, separator)), pointSize, bold};
}

std::string_view nativePathLeaf(std::string_view path) noexcept {
    const auto slash=path.find_last_of("/\\");
    return path.substr(slash==path.npos?0:slash+1);
}
std::string nativeMapCompanionPath(std::string_view mapName, std::string_view companion,
    const NativeUserStorage* storage)
{
    if(!mapName.empty() && (mapName.front()=='/' || mapName.front()=='\\') && storage) {
        const auto physical=storage->mapIdentityPath(mapName);
        const auto relative=storage->relativeDataPath(physical);
        if(!relative) throw ERROR_BAD_ARG;
        const auto path=nativeMapCompanionPath(*relative,companion);
        return storage->mapIdentityPath(storage->paths().data+"/"+path);
    }
    if (mapName.size() < 4 || mapName.front() == '/' || mapName.front() == '\\' ||
        mapName.find(':') != std::string_view::npos || mapName.find('\0') != std::string_view::npos ||
        companion.empty() || companion == "." || companion == ".." ||
        companion.find_first_of("/\\:\0",0,4) != std::string_view::npos)
        throw ERROR_BAD_ARG;
    // Preserve the source's four-byte-extension/backward separator rule. This
    // is lexical preparation; the rooted file owner still admits each read.
    auto end = mapName.size()-4;
    while (end > 0 && mapName[end] != '/' && mapName[end] != '\\') --end;
    std::string candidate(mapName.substr(0,end));
    if (!candidate.empty()) candidate.push_back('\\');
    candidate.append(companion);
    return candidate;
}

Int nativeScriptFrames(Int seconds)
{
    const auto frames = static_cast<std::int64_t>(seconds)*LOGICFRAMES_PER_SECOND;
    if (frames < std::numeric_limits<Int>::min() || frames > std::numeric_limits<Int>::max())
        throw ERROR_BAD_ARG;
    return static_cast<Int>(frames);
}

namespace {
std::uint32_t playerNameScalar(std::string_view text,std::size_t& position) {
    const auto first=static_cast<unsigned char>(text[position++]);
    if (first>0 && first<0x80) return first;
    unsigned trailing=0; std::uint32_t scalar=0,minimum=0;
    if (first>=0xc2 && first<=0xdf) { trailing=1; scalar=first&0x1f; minimum=0x80; }
    else if (first>=0xe0 && first<=0xef) { trailing=2; scalar=first&0xf; minimum=0x800; }
    else if (first>=0xf0 && first<=0xf4) { trailing=3; scalar=first&7; minimum=0x10000; }
    else throw ERROR_BAD_ARG;
    if (trailing>text.size()-position) throw ERROR_BAD_ARG;
    for (unsigned i=0;i<trailing;++i) {
        const auto byte=static_cast<unsigned char>(text[position++]);
        if ((byte&0xc0)!=0x80) throw ERROR_BAD_ARG;
        scalar=(scalar<<6)|(byte&0x3f);
    }
    if (scalar<minimum || scalar>0x10ffff || (scalar>=0xd800 && scalar<=0xdfff)) throw ERROR_BAD_ARG;
    return scalar;
}
}
std::string nativeEncodePlayerName(std::wstring_view text) {
    std::string output;
    const auto append=[&](std::uint32_t byte){output.push_back(static_cast<char>(byte));};
    for (const auto wide:text) {
        const auto scalar=static_cast<std::uint32_t>(wide);
        if (!scalar || scalar>0x10ffff || (scalar>=0xd800 && scalar<=0xdfff)) throw ERROR_BAD_ARG;
        if (scalar<0x80) append(scalar);
        else if (scalar<0x800) { append(0xc0|(scalar>>6)); append(0x80|(scalar&0x3f)); }
        else if (scalar<0x10000) { append(0xe0|(scalar>>12)); append(0x80|((scalar>>6)&0x3f)); append(0x80|(scalar&0x3f)); }
        else { append(0xf0|(scalar>>18)); append(0x80|((scalar>>12)&0x3f)); append(0x80|((scalar>>6)&0x3f)); append(0x80|(scalar&0x3f)); }
    }
    return output;
}
std::wstring nativeDecodePlayerName(std::string_view text) {
    std::wstring output;
    for (std::size_t position=0;position<text.size();) {
        const auto scalar=playerNameScalar(text,position);
        output.push_back(static_cast<wchar_t>(scalar=='\r' || scalar=='\n' ? ' ' : scalar));
    }
    return output;
}
std::string_view nativePlayerNamePrefix(std::string_view text,std::size_t maximumBytes) {
    std::size_t last=0;
    for (std::size_t position=0;position<text.size();) {
        playerNameScalar(text,position); // Admit the whole source, including omitted suffixes.
        if (position<=maximumBytes) last=position;
    }
    return text.substr(0,last);
}
