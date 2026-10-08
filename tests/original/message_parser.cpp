// SPDX-License-Identifier: GPL-3.0-or-later
// Real original message/pool/run owners; not whole Recorder or simulation acceptance.
#include "AllocationFault.h"
#include "Common/MessageStream.h"
#include "Common/NativeSourceStrings.h"
#include "Common/NativeReplayCommand.h"
#include "Common/Xfer.h"
#include "GameNetwork/GameMessageParser.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
namespace {
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class ACTION> void rejects(ACTION action) {
    bool rejected=false;
    try { action(); } catch(ErrorCode) { rejected=true; } catch(XferStatus) { rejected=true; }
    require(rejected,"source boundary rejects");
}
template<class T> struct Retire { void operator()(T* value) const noexcept { if (value) value->deleteInstance(); } };
template<class T> using Owner=std::unique_ptr<T,Retire<T>>;
struct Pools {
    std::array<MemoryPool*,4> all;
    explicit Pools(Int runs=260) : all{
        TheMemoryPoolFactory->createMemoryPool("GameMessage",sizeof(GameMessage),4,0),
        TheMemoryPoolFactory->createMemoryPool("GameMessageArgument",sizeof(GameMessageArgument),260,0),
        TheMemoryPoolFactory->createMemoryPool("GameMessageParser",sizeof(GameMessageParser),4,0),
        TheMemoryPoolFactory->createMemoryPool("GameMessageParserArgumentType",sizeof(GameMessageParserArgumentType),runs,0)} {
        Owner<GameMessageParser> parser(newInstance(GameMessageParser));
        Owner<GameMessageParserArgumentType> node(newInstance(GameMessageParserArgumentType)(ARGUMENTDATATYPE_INTEGER,1));
    }
    ~Pools() { for (auto* pool:all) TheMemoryPoolFactory->destroyMemoryPool(pool); }
    void empty() { for (auto* pool:all) require(pool->getUsedBlockCount()==0,"all actual pooled units retired"); }
};
// Same inherited owner and field types as the prior native declarations.
// Raw admission changes method signatures, not pooled representation.
struct PriorParserLayout : MemoryPoolObject {
    GameMessageParserArgumentType *first,*last;
    Int count;
};
struct PriorNodeLayout : MemoryPoolObject {
    GameMessageParserArgumentType* next;
    GameMessageArgumentDataType type;
    Int count;
};
static_assert(sizeof(GameMessageParser)==sizeof(PriorParserLayout));
static_assert(alignof(GameMessageParser)==alignof(PriorParserLayout));
static_assert(sizeof(GameMessageParserArgumentType)==sizeof(PriorNodeLayout));
static_assert(alignof(GameMessageParserArgumentType)==alignof(PriorNodeLayout));
static_assert(sizeof(GameMessageArgumentDataType)==sizeof(Int));
static_assert(alignof(GameMessageArgumentDataType)==alignof(Int));
struct ParserLayoutProbe : GameMessageParser {
    void verify() {
        const auto* begin=reinterpret_cast<const unsigned char*>(this);
        require(std::size_t(reinterpret_cast<const unsigned char*>(&m_first)-begin)==offsetof(PriorParserLayout,first)
            && std::size_t(reinterpret_cast<const unsigned char*>(&m_last)-begin)==offsetof(PriorParserLayout,last)
            && std::size_t(reinterpret_cast<const unsigned char*>(&m_argTypeCount)-begin)==offsetof(PriorParserLayout,count),"prior native parser member offsets preserved");
    }
};
struct NodeLayoutProbe : GameMessageParserArgumentType {
    NodeLayoutProbe() : GameMessageParserArgumentType(ARGUMENTDATATYPE_INTEGER,1) {}
    void verify() {
        const auto* begin=reinterpret_cast<const unsigned char*>(this);
        require(std::size_t(reinterpret_cast<const unsigned char*>(&m_next)-begin)==offsetof(PriorNodeLayout,next)
            && std::size_t(reinterpret_cast<const unsigned char*>(&m_type)-begin)==offsetof(PriorNodeLayout,type)
            && std::size_t(reinterpret_cast<const unsigned char*>(&m_argCount)-begin)==offsetof(PriorNodeLayout,count),"prior native run-node member offsets preserved");
    }
};
void values() {
    Pools pools;
    ParserLayoutProbe{}.verify(); NodeLayoutProbe{}.verify();
    {
        Owner<GameMessage> message(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        require(!message->next() && !message->prev() && !message->getOwningList() && message->getArgumentCount()==0,"complete message links initialized");
        message->appendIntegerArgument(7); message->appendIntegerArgument(8);
        message->appendRealArgument(1.25f); message->appendBooleanArgument(true);
        require(message->getArgumentDataType(-1)==ARGUMENTDATATYPE_UNKNOWN,"negative lookup does not select first field");
        const auto* argument=message->getArgument(3);
        const auto* backing=reinterpret_cast<const unsigned char*>(argument);
        for (std::size_t i=sizeof(Bool);i<sizeof(*argument);++i) require(backing[i]==0,"inactive union backing initialized");
        Owner<GameMessageParser> parser(newInstance(GameMessageParser)(message.get()));
        require(parser->getNumTypes()==3,"actual source adjacent-type run grouping");
        auto* first=parser->getFirstArgumentType();
        require(first->getType()==ARGUMENTDATATYPE_INTEGER && first->getArgCount()==2
            && first->getNext()->getType()==ARGUMENTDATATYPE_REAL && first->getNext()->getNext()->getType()==ARGUMENTDATATYPE_BOOLEAN,"exact recorded run sequence");
        for (Int invalid : {-1,Int(ARGUMENTDATATYPE_UNKNOWN),std::numeric_limits<Int>::max()})
            rejects([&]{parser->addArgType(invalid,1);});
        for (Int invalid : {-1,0,256}) rejects([&]{parser->addArgType(ARGUMENTDATATYPE_INTEGER,invalid);});
        require(parser->getNumTypes()==3 && parser->getFirstArgumentType()==first,"all late-invalid admissions preserve accepted ledger");
        GameMessageList firstList,otherList;
        firstList.appendMessage(message.get()); rejects([&]{otherList.appendMessage(message.get());});
        rejects([&]{otherList.removeMessage(message.get());}); rejects([&]{otherList.insertMessage(nullptr,message.get());});
        require(firstList.getFirstMessage()==message.get() && !otherList.getFirstMessage(),"foreign list rejection without publication");
        firstList.removeMessage(message.get()); require(!message->next() && !message->prev() && !message->getOwningList(),"withdrawal clears both links");
        otherList.appendMessage(message.get()); otherList.removeMessage(message.get());
        Owner<GameMessage> peer(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        firstList.appendMessage(message.get()); firstList.appendMessage(peer.get());
        firstList.removeMessage(message.get());
        require(message->next()==peer.get() && firstList.getFirstMessage()==peer.get(),"withdrawal retains source traversal links");
        otherList.transferMessage(peer.get());
        require(!firstList.getFirstMessage() && otherList.getFirstMessage()==peer.get() && peer->getOwningList()==&otherList,"explicit source-owner transfer preserves single publication");
        otherList.removeMessage(peer.get());
    }
    {
        Owner<GameMessage> maximum(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        for (Int i=0;i<255;++i) if (i%2) maximum->appendRealArgument(float(i)); else maximum->appendIntegerArgument(i);
        const auto used=pools.all[1]->getUsedBlockCount();
        rejects([&]{maximum->appendIntegerArgument(256);});
        require(maximum->getArgumentCount()==255 && pools.all[1]->getUsedBlockCount()==used,"Byte max+1 rejected before pool acquisition");
        Owner<GameMessageParser> runs(newInstance(GameMessageParser)(maximum.get()));
        require(runs->getNumTypes()==255,"exact maximum recorded type runs");
        rejects([&]{runs->addArgType(ARGUMENTDATATYPE_INTEGER,1);}); require(runs->getNumTypes()==255,"terminal run budget unchanged");
    }
    {
        Owner<GameMessageParser> parser(newInstance(GameMessageParser)); parser->addArgType(ARGUMENTDATATYPE_INTEGER,255);
        rejects([&]{parser->addArgType(ARGUMENTDATATYPE_REAL,1);}); require(parser->getNumTypes()==1,"total argument budget independent of run count");
        const auto used=pools.all[3]->getUsedBlockCount();
        rejects([&]{Owner<GameMessageParserArgumentType> invalid(newInstance(GameMessageParserArgumentType)(-1,1));});
        require(pools.all[3]->getUsedBlockCount()==used,"failed direct node constructor returns acquired ownership unit");
    }
    pools.empty();
}
void faults() {
    Pools pools(3);
    {
        Owner<GameMessage> message(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        message->appendIntegerArgument(1); message->appendRealArgument(2); message->appendBooleanArgument(true);
        for (Int ordinal=0;ordinal<=3;++ordinal) {
            std::array<void*,3> held{};
            for (Int i=0;i<3-ordinal;++i) held[i]=pools.all[3]->allocateBlock("generated parser capacity");
            const auto used=pools.all[3]->getUsedBlockCount(); const auto live=AllocationFault::live(); bool failed=false;
            try { Owner<GameMessageParser> candidate(newInstance(GameMessageParser)(message.get())); require(candidate->getNumTypes()==3,"terminal complete source constructor"); }
            catch(ErrorCode error) { require(error==ERROR_OUT_OF_MEMORY,"exact physical pool rejection"); failed=true; }
            require(failed==(ordinal<3) && pools.all[3]->getUsedBlockCount()==used && pools.all[2]->getUsedBlockCount()==0
                && AllocationFault::live()==live,"every constructor boundary retires partial graph and construction owner");
            for (auto* unit:held) if (unit) pools.all[3]->freeBlock(unit);
            { Owner<GameMessageParser> retry(newInstance(GameMessageParser)(message.get())); require(retry->getNumTypes()==3,"same-pool same-source constructor retry"); }
        }
        Owner<GameMessageParser> parser(newInstance(GameMessageParser)); parser->addArgType(ARGUMENTDATATYPE_INTEGER,1);
        auto* accepted=parser->getFirstArgumentType();
        std::array<void*,2> held{pools.all[3]->allocateBlock("generated accepted parser"),pools.all[3]->allocateBlock("generated accepted parser")};
        rejects([&]{parser->addArgType(ARGUMENTDATATYPE_REAL,1);});
        require(parser->getNumTypes()==1 && parser->getFirstArgumentType()==accepted && !accepted->getNext(),"failed add retains accepted owner graph");
        for (auto* unit:held) pools.all[3]->freeBlock(unit);
        parser->addArgType(ARGUMENTDATATYPE_REAL,1); require(parser->getNumTypes()==2,"same-owner add retry");
    }
    pools.empty();
}
void text() {
    const std::wstring source=L"A\u03a9\U0001f642";
    const std::string bytes="A\xce\xa9\xf0\x9f\x99\x82";
    require(nativeEncodePlayerName(source)==bytes && nativeDecodePlayerName(bytes)==source,"source UTF8 multilingual supplementary bytes");
    require(nativeDecodePlayerName("A\r\nB")==L"A  B","original single-line CR/LF conversion");
    for (std::size_t budget=0;budget<=bytes.size()+1;++budget) {
        const auto prefix=nativePlayerNamePrefix(bytes,budget);
        require(prefix.size()<=budget && (prefix.size()==0 || prefix.size()==1 || prefix.size()==3 || prefix.size()==7),"complete scalar byte-budget prefixes");
        nativeDecodePlayerName(prefix);
    }
    for (const std::string invalid : {std::string("\xc0\x80",2),std::string("\x80",1),std::string("\xf0\x9f",2),std::string("\xed\xa0\x80",3),std::string("\xf4\x90\x80\x80",4),std::string("A\0B",3)}) {
        rejects([&]{nativeDecodePlayerName(invalid);}); rejects([&]{nativePlayerNamePrefix(invalid,0);});
    }
    for (wchar_t invalid : {wchar_t(0),wchar_t(0xd800),wchar_t(0x110000)}) rejects([&]{nativeEncodePlayerName(std::wstring_view(&invalid,1));});
    require(nativeSourceInteger<Int>(" -2147483648 ")==std::numeric_limits<Int>::min()
        && nativeSourceInteger<UnsignedInt>("+4294967295")==std::numeric_limits<UnsignedInt>::max()
        && nativeSourceInteger<UnsignedShort>("65535")==65535 && nativeSourceInteger<UnsignedByte>("0xff",16)==255,"defined source-width integer maxima/minima");
    for (std::string_view invalid : {"", "2147483648","-2147483649","1tail","+-1","++1","999999999999999999999999"}) rejects([&]{nativeSourceInteger<Int>(invalid);});
    rejects([]{nativeSourceInteger<UnsignedShort>("65536");}); rejects([]{nativeSourceInteger<UnsignedInt>("-1");});
}
// Generated protocol bytes only; never reads supplied game content.
NativeReplayCommandBytes commandBytes() {
    Owner<GameMessage> source(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,-1));
    source->appendIntegerArgument(-2); source->appendRealArgument(std::bit_cast<Real>(std::uint32_t(0x7fc01234)));
    source->appendBooleanArgument(true); source->appendObjectIDArgument(static_cast<ObjectID>(0xffffffffu));
    source->appendDrawableIDArgument(static_cast<DrawableID>(0x01020304)); source->appendTeamIDArgument(0x10203040);
    source->appendLocationArgument(Coord3D{1,-2,0.5f}); source->appendPixelArgument(ICoord2D{-1,2});
    source->appendPixelRegionArgument(IRegion2D{{-3,4},{5,-6}}); source->appendTimestampArgument(0xffffffffu);
    source->appendWideCharArgument(WideChar(0xd800)); // A unit, not a scalar string.
    return nativeReplayEncodeCommand(*source,0x12345678);
}
struct CommandInput {
    std::span<const unsigned char> bytes; std::size_t position=0;
    std::size_t operator()(void* output,std::size_t count) {
        const auto obtained=std::min(count,bytes.size()-position);
        if (obtained) std::memcpy(output,bytes.data()+position,obtained);
        position+=obtained; return obtained;
    }
};
NativeReplayMessage decodeCommand(std::span<const unsigned char> bytes) {
    CommandInput input{bytes};
    const auto frame=nativeReplayReadFrame([&](void* output,std::size_t count){return input(output,count);});
    if (!frame) throw XFER_READ_ERROR;
    require(*frame==0x12345678,"source little-endian frame");
    auto candidate=nativeReplayReadCommand([&](void* output,std::size_t count){return input(output,count);});
    require(input.position==bytes.size(),"exact complete command consume");
    return candidate;
}
void command() {
    Pools pools;
    const auto wire=commandBytes();
    constexpr std::array<unsigned char,98> expected{
        0x78,0x56,0x34,0x12,1,0,0,0,255,255,255,255,11,
        0,1,1,1,2,1,3,1,4,1,5,1,6,1,7,1,8,1,9,1,10,1,
        254,255,255,255,0x34,0x12,0xc0,0x7f,1,255,255,255,255,
        4,3,2,1,0x40,0x30,0x20,0x10,
        0,0,128,63,0,0,0,192,0,0,0,63,
        255,255,255,255,2,0,0,0,
        253,255,255,255,4,0,0,0,5,0,0,0,250,255,255,255,
        255,255,255,255,0,0xd8};
    require(wire.count==expected.size() && std::equal(expected.begin(),expected.end(),wire.bytes.begin()),"source widths/order exact authored bytes");
    {
        auto message=decodeCommand(wire.payload());
        const auto encoded=nativeReplayEncodeCommand(*message,0x12345678);
        require(encoded.count==wire.count && std::equal(wire.payload().begin(),wire.payload().end(),encoded.bytes.begin()),"all argument families bit-exact roundtrip including NaN and surrogate unit");
        require(message->getPlayerIndex()==-1 && !message->getOwningList(),"recorded context without local-player lookup or publication");
    }
    for (std::size_t end=0;end<wire.count;++end) {
        rejects([&]{decodeCommand(wire.payload().first(end));}); pools.empty();
        auto retry=decodeCommand(wire.payload()); retry.reset(); pools.empty();
    }
    {
        GameMessageList destination;
        Owner<GameMessage> accepted(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        destination.appendMessage(accepted.get());
        const auto live=AllocationFault::live();
        for (std::size_t end=0;end<wire.count-4;++end) {
            CommandInput input{wire.payload().subspan(4).first(end)};
            rejects([&]{nativeReplayPublishCommand([&](void* output,std::size_t count){return input(output,count);},&destination,false);});
            require(destination.getFirstMessage()==accepted.get() && !accepted->next()
                && pools.all[0]->getUsedBlockCount()==1 && pools.all[1]->getUsedBlockCount()==0
                && AllocationFault::live()==live,"truncated candidate leaves accepted destination and exact owners unchanged");
        }
        for (Int type : {Int(GameMessage::MSG_FRAME_TICK),Int(GameMessage::MSG_BEGIN_NETWORK_MESSAGES),Int(GameMessage::MSG_CLEAR_GAME_DATA)})
            for (Bool analysis : {false,true}) {
                auto changed=wire;
                for (int i=0;i<4;++i) changed.bytes[4+i]=static_cast<unsigned char>(UnsignedInt(type)>>(i*8));
                CommandInput input{changed.payload().subspan(4)};
                nativeReplayPublishCommand([&](void* output,std::size_t count){return input(output,count);},&destination,analysis);
                if (!analysis && type==GameMessage::MSG_FRAME_TICK) {
                    require(accepted->next() && accepted->next()->getArgumentCount()==11,"complete candidate published once");
                    accepted->next()->deleteInstance();
                }
                require(!accepted->next() && pools.all[0]->getUsedBlockCount()==1 && pools.all[1]->getUsedBlockCount()==0,"analysis/suppression overlaps retire exactly once");
            }
        CommandInput missing{wire.payload().subspan(4)};
        rejects([&]{nativeReplayPublishCommand([&](void* output,std::size_t count){return missing(output,count);},nullptr,false);});
        require(!missing.position,"missing publication owner rejected before input consumption");
        destination.removeMessage(accepted.get());
    }
    CommandInput empty{{}};
    require(!nativeReplayReadFrame([&](void* output,std::size_t count){return empty(output,count);}),"clean inter-command EOF");
    for (const std::size_t offset : {std::size_t(4),std::size_t(13),std::size_t(34),std::size_t(43)}) {
        auto invalid=wire;
        if (offset==4) { for (int i=0;i<4;++i) invalid.bytes[4+i]=255; }
        else if (offset==13) invalid.bytes[offset]=ARGUMENTDATATYPE_UNKNOWN;
        else if (offset==34) invalid.bytes[offset]=255; // Late combined count >255.
        else invalid.bytes[offset]=2; // Boolean payload, never materialized as Bool.
        rejects([&]{decodeCommand(invalid.payload());}); pools.empty();
    }
    for (Int player : {-2,Int(MAX_PLAYER_COUNT),std::numeric_limits<Int>::max()}) {
        auto invalid=wire;
        for (unsigned i=0;i<4;++i) invalid.bytes[8+i]=static_cast<unsigned char>(std::bit_cast<UnsignedInt>(player)>>(i*8));
        rejects([&]{decodeCommand(invalid.payload());}); pools.empty();
        Owner<GameMessage> source(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,player));
        std::size_t calls=0;
        rejects([&]{nativeReplayWriteCommand(*source,0,[&](const void*,std::size_t count){++calls;return count;});});
        require(!calls,"runtime player bound rejects before first output mutation");
    }
    {
        auto source=decodeCommand(wire.payload());
        for (std::size_t obtained=0;obtained<wire.count;++obtained)
            rejects([&]{nativeReplayWriteCommand(*source,0x12345678,[&](const void*,std::size_t count){require(count==wire.count,"one complete write request");return obtained;});});
        std::size_t calls=0;
        const auto live=AllocationFault::live();
        AllocationFault::arm(0);
        nativeReplayWriteCommand(*source,0x12345678,[&](const void* bytes,std::size_t count){++calls;require(count==wire.count && !std::memcmp(bytes,expected.data(),count),"complete output retry");return count;});
        const auto attempts=AllocationFault::attempts(); AllocationFault::disarm();
        require(attempts==0 && AllocationFault::live()==live,"bounded whole output preparation is actually allocation-free");
        require(calls==1,"single prepared publication request");
        source->appendWideCharArgument(WideChar(0x10000));
        rejects([&]{nativeReplayWriteCommand(*source,0,[&](const void*,std::size_t count){++calls;return count;});});
        require(calls==1,"invalid late field rejects before first write");
    }
    {
        Owner<GameMessage> maximum(newInstance(GameMessage)(GameMessage::MSG_FRAME_TICK,0));
        for (Int i=0;i<255;++i) if (i%2) maximum->appendLocationArgument(Coord3D{1,2,3}); else maximum->appendPixelRegionArgument(IRegion2D{{1,2},{3,4}});
        const auto maximumWire=nativeReplayEncodeCommand(*maximum,0x12345678);
        maximum.reset(); auto restored=decodeCommand(maximumWire.payload());
        require(restored->getArgumentCount()==255,"maximum argument and run ledgers");
    }
    pools.empty();
}
void commandFaults() {
    Pools pools;
    const auto wire=commandBytes();
    std::array<void*,260> held{};
    for (unsigned ordinal=0;ordinal<=11;++ordinal) {
        const unsigned occupied=260-ordinal;
        for (unsigned i=0;i<occupied;++i) held[i]=pools.all[1]->allocateBlock("generated command fault");
        const auto baseline=AllocationFault::live();
        bool completed=false;
        try { auto candidate=decodeCommand(wire.payload()); completed=true; }
        catch(ErrorCode) {} // Explicit bounded source pool exhaustion.
        require(completed==(ordinal==11),"all eleven acquired argument boundaries and exact terminal");
        require(pools.all[0]->getUsedBlockCount()==0 && pools.all[1]->getUsedBlockCount()==occupied
            && AllocationFault::live()==baseline,"partial command owner and arguments retired immediately");
        for (unsigned i=0;i<occupied;++i) pools.all[1]->freeBlock(held[i]);
        auto retry=decodeCommand(wire.payload()); retry.reset(); pools.empty();
    }
    std::array<void*,4> messages{};
    for (auto& unit:messages) unit=pools.all[0]->allocateBlock("generated message owner fault");
    rejects([&]{decodeCommand(wire.payload());});
    require(pools.all[1]->getUsedBlockCount()==0,"message-owner acquisition failure before arguments");
    for (auto* unit:messages) pools.all[0]->freeBlock(unit);
    auto retry=decodeCommand(wire.payload()); retry.reset(); pools.empty();
    std::puts("command pooled manifest: owner=1 arguments=11 terminal/retry exact");
}
}
int main(int argc,char** argv) {
    bool initialized=false;
    try {
        require(argc==2,"generated parser family");
        for (int repeat=0;repeat<3;++repeat) {
            initMemoryManager(); initialized=true;
            if (!std::strcmp(argv[1],"values")) values(); else if (!std::strcmp(argv[1],"faults")) faults();
            else if (!std::strcmp(argv[1],"text")) text();
            else if (!std::strcmp(argv[1],"command")) command();
            else if (!std::strcmp(argv[1],"command_faults")) commandFaults();
            else throw std::runtime_error("unknown parser family");
            shutdownMemoryManager(); initialized=false;
        }
        std::puts("PASS real message/parser/text support; whole replay pending"); return 0;
    } catch(const std::exception& error) { std::fprintf(stderr,"FAIL %s\n",error.what()); }
    catch(...) { std::fputs("FAIL source parser boundary\n",stderr); }
    AllocationFault::disarm(); if (initialized) shutdownMemoryManager(); return 1;
}
