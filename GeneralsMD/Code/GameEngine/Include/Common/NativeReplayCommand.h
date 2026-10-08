// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/MessageStream.h"
#include "Common/GameCommon.h"
#include "Common/Xfer.h"
#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <limits>

// Original game-owned command protocol. No parent singleton or native enum,
// bool, wchar_t, aggregate padding or host byte-order is a wire representation.
struct NativeReplayMessageRetire {
    void operator()(GameMessage* value) const noexcept { if (value) value->deleteInstance(); }
};
using NativeReplayMessage=std::unique_ptr<GameMessage,NativeReplayMessageRetire>;
static_assert(sizeof(Int)==4 && sizeof(Real)==4 && sizeof(UnsignedInt)==4);
inline GameMessage::Type nativeReplayCommandType(Int raw) {
    if (raw<=GameMessage::MSG_INVALID || raw>=GameMessage::MSG_COUNT) throw ERROR_BAD_ARG;
    return static_cast<GameMessage::Type>(raw);
}
inline void nativeReplayPlayerIndex(Int player) {
    if (player<-1 || player>=MAX_PLAYER_COUNT) throw ERROR_BAD_ARG;
}
struct NativeReplayCommandBytes {
    // Frame/type/player, run count, maximum run table, largest argument payload.
    std::array<unsigned char,13+255*2+255*16> bytes{};
    std::size_t count=0;
    void byte(unsigned char value) {
        if (count==bytes.size()) throw ERROR_BAD_ARG;
        bytes[count++]=value;
    }
    void word(std::uint32_t value) { for (int i=0;i<4;++i) byte(static_cast<unsigned char>(value>>(i*8))); }
    void integer(Int value) { word(std::bit_cast<std::uint32_t>(value)); }
    void real(Real value) { word(std::bit_cast<std::uint32_t>(value)); }
    std::span<const unsigned char> payload() const noexcept { return {bytes.data(),count}; }
};
inline void nativeReplayEncodeArgument(NativeReplayCommandBytes& output,
        GameMessageArgumentDataType type,const GameMessageArgumentType& arg) {
    switch (type) {
    case ARGUMENTDATATYPE_INTEGER: output.integer(arg.integer); break;
    case ARGUMENTDATATYPE_REAL: output.real(arg.real); break;
    case ARGUMENTDATATYPE_BOOLEAN: output.byte(arg.boolean ? 1 : 0); break;
    case ARGUMENTDATATYPE_OBJECTID: output.word(static_cast<UnsignedInt>(arg.objectID)); break;
    case ARGUMENTDATATYPE_DRAWABLEID: output.word(static_cast<UnsignedInt>(arg.drawableID)); break;
    case ARGUMENTDATATYPE_TEAMID: output.word(arg.teamID); break;
    case ARGUMENTDATATYPE_LOCATION: output.real(arg.location.x); output.real(arg.location.y); output.real(arg.location.z); break;
    case ARGUMENTDATATYPE_PIXEL: output.integer(arg.pixel.x); output.integer(arg.pixel.y); break;
    case ARGUMENTDATATYPE_PIXELREGION:
        output.integer(arg.pixelRegion.lo.x); output.integer(arg.pixelRegion.lo.y);
        output.integer(arg.pixelRegion.hi.x); output.integer(arg.pixelRegion.hi.y); break;
    case ARGUMENTDATATYPE_TIMESTAMP: output.word(arg.timestamp); break;
    case ARGUMENTDATATYPE_WIDECHAR: {
        const auto unit=static_cast<std::uint32_t>(arg.wChar);
        if (unit>0xffffu) throw ERROR_BAD_ARG;
        output.byte(static_cast<unsigned char>(unit)); output.byte(static_cast<unsigned char>(unit>>8)); break;
    }
    default: throw ERROR_BAD_ARG;
    }
}
inline NativeReplayCommandBytes nativeReplayEncodeCommand(GameMessage& message,UnsignedInt frame) {
    nativeReplayCommandType(static_cast<Int>(message.getType()));
    nativeReplayPlayerIndex(message.getPlayerIndex());
    const Int count=message.getArgumentCount();
    if (count<0 || count>255) throw ERROR_BAD_ARG;
    NativeReplayCommandBytes result;
    result.word(frame); result.integer(static_cast<Int>(message.getType())); result.integer(message.getPlayerIndex());
    const auto runCountOffset=result.count; result.byte(0);
    unsigned char runs=0;
    for (Int i=0;i<count;) {
        const auto type=message.getArgumentDataType(i);
        if (type<ARGUMENTDATATYPE_INTEGER || type>=ARGUMENTDATATYPE_UNKNOWN) throw ERROR_BAD_ARG;
        Int end=i+1;
        while (end<count && message.getArgumentDataType(end)==type) ++end;
        result.byte(static_cast<unsigned char>(type)); result.byte(static_cast<unsigned char>(end-i));
        ++runs; i=end;
    }
    result.bytes[runCountOffset]=runs;
    for (Int i=0;i<count;++i) nativeReplayEncodeArgument(result,message.getArgumentDataType(i),*message.getArgument(i));
    return result; // Allocation-free complete candidate, before any output mutation.
}
template<class READ> struct NativeReplayCommandReader {
    READ& read;
    void exact(void* bytes,std::size_t count) { if (read(bytes,count)!=count) throw XFER_READ_ERROR; }
    unsigned char byte() { unsigned char result=0; exact(&result,1); return result; }
    std::uint32_t word() {
        std::array<unsigned char,4> bytes{}; exact(bytes.data(),bytes.size());
        return std::uint32_t(bytes[0]) | (std::uint32_t(bytes[1])<<8)
            | (std::uint32_t(bytes[2])<<16) | (std::uint32_t(bytes[3])<<24);
    }
    Int integer() { return std::bit_cast<Int>(word()); }
    Real real() { return std::bit_cast<Real>(word()); }
};
template<class WRITE> void nativeReplayWriteCommand(GameMessage& message,UnsignedInt frame,WRITE write) {
    const auto candidate=nativeReplayEncodeCommand(message,frame);
    if (write(candidate.bytes.data(),candidate.count)!=candidate.count) throw XFER_WRITE_ERROR;
}
template<class READ> void nativeReplayReadArgument(READ& read,GameMessage& message,Int rawType) {
    NativeReplayCommandReader<READ> input{read};
    switch (rawType) {
    case ARGUMENTDATATYPE_INTEGER: message.appendIntegerArgument(input.integer()); break;
    case ARGUMENTDATATYPE_REAL: message.appendRealArgument(input.real()); break;
    case ARGUMENTDATATYPE_BOOLEAN: {
        const auto value=input.byte(); if (value>1) throw ERROR_BAD_ARG;
        message.appendBooleanArgument(value!=0); break;
    }
    case ARGUMENTDATATYPE_OBJECTID: message.appendObjectIDArgument(static_cast<ObjectID>(input.word())); break;
    case ARGUMENTDATATYPE_DRAWABLEID: message.appendDrawableIDArgument(static_cast<DrawableID>(input.word())); break;
    case ARGUMENTDATATYPE_TEAMID: message.appendTeamIDArgument(input.word()); break;
    case ARGUMENTDATATYPE_LOCATION: {
        Coord3D value{}; value.x=input.real(); value.y=input.real(); value.z=input.real();
        message.appendLocationArgument(value); break;
    }
    case ARGUMENTDATATYPE_PIXEL: {
        ICoord2D value{}; value.x=input.integer(); value.y=input.integer(); message.appendPixelArgument(value); break;
    }
    case ARGUMENTDATATYPE_PIXELREGION: {
        IRegion2D value{}; value.lo.x=input.integer(); value.lo.y=input.integer();
        value.hi.x=input.integer(); value.hi.y=input.integer(); message.appendPixelRegionArgument(value); break;
    }
    case ARGUMENTDATATYPE_TIMESTAMP: message.appendTimestampArgument(input.word()); break;
    case ARGUMENTDATATYPE_WIDECHAR: {
        const auto low=input.byte(); const auto high=input.byte();
        const WideChar unit=static_cast<WideChar>(unsigned(low)|(unsigned(high)<<8));
        message.appendWideCharArgument(unit); break;
    }
    default: throw ERROR_BAD_ARG;
    }
}
// The caller has already consumed the frame; candidate remains unpublished.
template<class READ> NativeReplayMessage nativeReplayReadCommand(READ read) {
    NativeReplayCommandReader<READ> input{read};
    const auto type=nativeReplayCommandType(input.integer());
    const Int player=input.integer();
    nativeReplayPlayerIndex(player);
    const unsigned runs=input.byte();
    std::array<unsigned char,255> types{},counts{};
    unsigned arguments=0;
    for (unsigned i=0;i<runs;++i) {
        types[i]=input.byte(); counts[i]=input.byte();
        if (types[i]>=ARGUMENTDATATYPE_UNKNOWN || !counts[i] || counts[i]>255-arguments) throw ERROR_BAD_ARG;
        arguments+=counts[i];
    }
    NativeReplayMessage candidate(newInstance(GameMessage)(type,player));
    for (unsigned i=0;i<runs;++i)
        for (unsigned j=0;j<counts[i];++j) nativeReplayReadArgument(read,*candidate,types[i]);
    return candidate;
}
template<class READ> std::optional<UnsignedInt> nativeReplayReadFrame(READ read) {
    unsigned char first=0;
    const auto obtained=read(&first,1);
    if (!obtained) return std::nullopt; // Caller must distinguish I/O error from clean EOF.
    if (obtained!=1) throw XFER_READ_ERROR;
    std::array<unsigned char,3> rest{};
    if (read(rest.data(),rest.size())!=rest.size()) throw XFER_READ_ERROR;
    return UnsignedInt(first) | (UnsignedInt(rest[0])<<8) | (UnsignedInt(rest[1])<<16) | (UnsignedInt(rest[2])<<24);
}
template<class READ> void nativeReplayPublishCommand(READ read,GameMessageList* destination,Bool analysis) {
    if (!analysis && !destination) throw ERROR_BAD_ARG;
    auto candidate=nativeReplayReadCommand(read);
    const auto type=candidate->getType();
    if (analysis || type==GameMessage::MSG_BEGIN_NETWORK_MESSAGES || type==GameMessage::MSG_CLEAR_GAME_DATA)
        return;
    destination->appendMessage(candidate.get());
    candidate.release();
}
