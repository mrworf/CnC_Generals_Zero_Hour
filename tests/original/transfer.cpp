// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/NativeTransferWire.h"
#include "Common/NativeTransferServices.h"
#include "Common/NativeReplaySession.h"
#include "Common/XferSave.h"
#include "Common/XferLoad.h"
#include "Common/XferCRC.h"
#include "Common/XferDeepCRC.h"
#include "Common/Snapshot.h"
#include "Common/BitFlagsIO.h"
#include "Common/KindOf.h"
#include "WWMath/matrix3d.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cerrno>
namespace {
void require(bool condition,const char* message) { if (!condition) throw std::runtime_error(message); }
template<class ACTION> void rejects(ACTION action) {
    bool rejected=false;
    try { action(); } catch (XferStatus) { rejected=true; }
    catch (const std::exception&) { rejected=true; }
    require(rejected,"generated transfer rejection");
}
struct Tree {
    std::filesystem::path path;
    Tree() {
        char pattern[]="/tmp/zh-transfer-XXXXXX";
        const auto* created=::mkdtemp(pattern);
        require(created,"generated transfer tree"); path=created;
    }
    ~Tree() { std::error_code error; std::filesystem::remove_all(path,error); }
};
struct Context {
    Tree assets,user;
    FileSystem files;
    NativeUserStorage storage;
    NativeUserStorage* prior;
    explicit Context(NativeStorageIO* io=nullptr) : storage({(user.path/"data").string(),(user.path/"cache").string()},files,io),prior(TheNativeUserStorage) {
        files.mountReadOnly({assets.path.string()});
        storage.validateRootOwnership(); TheNativeUserStorage=&storage;
    }
    ~Context() { TheNativeUserStorage=prior; }
    void put(const char* name,std::span<const unsigned char> bytes) {
        auto candidate=storage.beginWrite(NativeUserArea::Data,name);
        candidate->write(bytes.data(),static_cast<Int>(bytes.size())); candidate->commit();
    }
    std::vector<unsigned char> get(const char* name) {
        auto result=storage.readFile(NativeUserArea::Data,name,1048576);
        require(result.has_value(),"generated transfer output exists"); return std::move(*result);
    }
    void clean() {
        for (const auto& entry:std::filesystem::recursive_directory_iterator(user.path))
            require(!entry.path().filename().string().starts_with(".zh-write-"),"no unpublished temporary after teardown");
        require(std::filesystem::is_empty(assets.path),"generated asset root unchanged");
    }
};
class ThrowingSnapshot final : public Snapshot {
    void crc(Xfer* xfer) override { Int value=17; xfer->xferInt(&value); throw XFER_INVALID_PARAMETERS; }
    void xfer(Xfer* xfer) override { Int value=17; xfer->xferInt(&value); throw XFER_INVALID_PARAMETERS; }
    void loadPostProcess() override {}
};
class ValueSnapshot final : public Snapshot {
public:
    Int value=41; unsigned calls=0;
    void crc(Xfer* xfer) override { ++calls; xfer->xferInt(&value); }
    void xfer(Xfer* xfer) override { ++calls; xfer->xferInt(&value); }
    void loadPostProcess() override {}
};
void wire() {
    const UnicodeString text(L"A\u03a9\U0001f642");
    const std::vector<unsigned char> expected{0x41,0,0xa9,3,0x3d,0xd8,0x42,0xde};
    require(nativeTransferUTF16(text,4)==expected,"exact UTF16LE scalar/supplementary payload");
    require(std::wcscmp(nativeTransferDecodeUTF16(expected).str(),text.str())==0,"actual UnicodeString roundtrip");
    rejects([&]{nativeTransferUTF16(text,3);});
    for (const auto& malformed:std::vector<std::vector<unsigned char>>{{1},{0,0},{0,0xd8},{0,0xdc},{0,0xd8,1,0}})
        rejects([&]{nativeTransferDecodeUTF16(malformed);});
    std::vector<unsigned char> terminated=expected; terminated.insert(terminated.end(),{0,0});
    std::size_t cursor=0;
    auto decoded=nativeReplayReadUnicode([&](void* output,Int count) {
        if (std::size_t(count)>terminated.size()-cursor) return false;
        std::memcpy(output,terminated.data()+cursor,count); cursor+=count; return true;
    });
    require(std::wcscmp(decoded.str(),text.str())==0 && cursor==terminated.size(),"bounded byte-mode replay decoding");
    std::vector<unsigned char> maximum(2046,0); for (std::size_t i=0;i<maximum.size();i+=2) maximum[i]='x';
    maximum.insert(maximum.end(),{0,0}); cursor=0;
    require(nativeReplayReadUnicode([&](void* out,Int count){std::memcpy(out,maximum.data()+cursor,count);cursor+=count;return true;}).getLength()==1023,"exact replay terminal capacity");
    maximum[2046]='x'; cursor=0;
    rejects([&]{nativeReplayReadUnicode([&](void* out,Int count){std::memcpy(out,maximum.data()+cursor,count);cursor+=count;return true;});});
    rejects([&]{nativeReplayReadAscii([](void* out,Int){*static_cast<char*>(out)='x';return true;});});
}
void functional() {
    Context context;
    XferSave writer;
    writer.open(AsciiString("Save/generated.sav"));
    require(writer.beginBlock()==XFER_OK,"actual save begin");
    Int value=0x12345678; Bool flag=true;
    AsciiString ascii("generated"); UnicodeString unicode(L"\u03a9\U0001f642");
    writer.xferInt(&value); writer.xferBool(&flag);
    writer.xferAsciiString(&ascii); writer.xferUnicodeString(&unicode);
    writer.endBlock(); writer.close();
    auto bytes=context.get("Save/generated.sav");
    require(bytes.size()==26 && bytes[0]==22 && bytes[4]==0x78 && bytes[8]==1 && bytes[9]==9 && bytes[19]==3,"source block/scalar/string widths");
    XferLoad reader; reader.setOptions(XO_NO_POST_PROCESSING);
    reader.open(AsciiString("Save/generated.sav")); require(reader.beginBlock()==22,"actual load block bounds");
    Int actual=0; Bool actualFlag=false; AsciiString actualAscii; UnicodeString actualUnicode;
    reader.xferInt(&actual); reader.xferBool(&actualFlag);
    reader.xferAsciiString(&actualAscii); reader.xferUnicodeString(&actualUnicode); reader.endBlock(); reader.close();
    require(actual==value && actualFlag && actualAscii==ascii && actualUnicode==unicode,"actual transport roundtrip");
    XferDeepCRC deep; deep.open(AsciiString("Save/deep.bin")); deep.xferAsciiString(&ascii); deep.xferUnicodeString(&unicode); deep.close();
    const auto diagnostic=context.get("Save/deep.bin");
    require(diagnostic.size()==18 && diagnostic[0]==9 && diagnostic[1]==0 && diagnostic[11]==3,"deep CRC distinct ASCII header and UTF16 payload");
    XferCRC crc; crc.open(AsciiString("generated CRC"));
    std::array<unsigned char,12> unaligned{9,1,2,3,4,5,6,7,8,9,10,11};
    crc.xferUser(unaligned.data()+1,11);
    const auto digest=crc.getCRC(); require(digest==0x29241f0eu,"source-derived word/leftover CRC oracle");
    crc.open(AsciiString("retry CRC"));
    std::array<unsigned char,11> aligned{}; std::memcpy(aligned.data(),unaligned.data()+1,11);
    crc.xferUser(aligned.data(),11); require(crc.getCRC()==digest,"unaligned CRC matches physical source bytes");
    std::cout<<"generated transfer CRC "<<std::hex<<digest<<std::dec<<'\n';
    context.clean();
}
void malformed() {
    Context context;
    const std::array<unsigned char,3> shortWord{1,2,3}; context.put("Save/bad.bin",shortWord);
    XferLoad reader; reader.open(AsciiString("Save/bad.bin")); Int value=71;
    rejects([&]{reader.xferInt(&value);}); require(value==71,"truncated read leaves scalar untouched");
    rejects([&]{reader.skip(0);}); reader.abort();
    const std::array<unsigned char,4> beyond{5,0,0,0}; context.put("Save/bad.bin",beyond);
    reader.open(AsciiString("Save/bad.bin")); rejects([&]{reader.beginBlock();}); reader.abort();
    const std::array<unsigned char,1> invalidBool{2}; context.put("Save/bad.bin",invalidBool);
    reader.open(AsciiString("Save/bad.bin")); Bool flag=true;
    rejects([&]{reader.xferBool(&flag);}); require(flag,"invalid bool rejected before publication"); reader.abort();
    reader.open(AsciiString("Save/bad.bin")); XferVersion version=1;
    rejects([&]{reader.xferVersion(&version,1);}); require(version==1,"invalid version output preserved"); reader.abort();
    const std::array<unsigned char,3> invalidUnicode{1,0,0xd8}; context.put("Save/bad.bin",invalidUnicode);
    reader.open(AsciiString("Save/bad.bin")); UnicodeString text(L"retained");
    rejects([&]{reader.xferUnicodeString(&text);}); require(std::wcscmp(text.str(),L"retained")==0,"invalid string preserves accepted backing"); reader.abort();
    context.clean();
}
void poisoning() {
    Context context;
    const std::array<unsigned char,4> previous{'k','e','e','p'}; context.put("Save/owned.bin",previous);
    XferSave save;
    save.open(AsciiString("Save/owned.bin")); Int value=41; save.xferInt(&value);
    XferVersion invalid=2; rejects([&]{save.xferVersion(&invalid,1);});
    rejects([&]{save.close();}); save.abort(); require(context.get("Save/owned.bin")==std::vector<unsigned char>(previous.begin(),previous.end()),"commit-ready typed rejection poisons actual save");
    ThrowingSnapshot callback;
    save.open(AsciiString("Save/owned.bin")); save.xferInt(&value);
    rejects([&]{save.xferSnapshot(&callback);}); rejects([&]{save.close();}); save.abort();
    XferDeepCRC deep; deep.open(AsciiString("Save/owned.bin")); deep.xferInt(&value);
    rejects([&]{deep.xferSnapshot(&callback);}); rejects([&]{deep.close();}); deep.abort();
    require(context.get("Save/owned.bin")==std::vector<unsigned char>(previous.begin(),previous.end()),"callback families cannot publish rejected data");
    save.open(AsciiString("Save/owned.bin")); save.xferInt(&value); save.close();
    require(context.get("Save/owned.bin")[0]==41,"same-owner corrected save retry"); context.clean();
}
void faults() {
    Context context;
    const UnicodeString text(L"generated \u03a9\U0001f642");
    const std::array<unsigned char,4> previous{'k','e','e','p'};
    context.put("Save/fault.bin",previous);
    std::size_t census=0;
    {
        XferSave save; save.open(AsciiString("Save/fault.bin"));
        AllocationFault::arm(std::numeric_limits<std::size_t>::max());
        save.beginBlock(); save.xferUnicodeString(const_cast<UnicodeString*>(&text)); save.endBlock();
        census=AllocationFault::attempts(); AllocationFault::disarm(); save.abort();
    }
    require(census==2,"complete block journal plus UTF16 payload allocation census");
    for (std::size_t ordinal=0;ordinal<=census;++ordinal) {
        XferSave save; save.open(AsciiString("Save/fault.bin"));
        bool failed=false; AllocationFault::arm(ordinal);
        try { save.beginBlock(); save.xferUnicodeString(const_cast<UnicodeString*>(&text)); save.endBlock(); }
        catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto triggered=AllocationFault::triggered();
        const auto attempts=AllocationFault::attempts(); AllocationFault::disarm();
        if (ordinal<census) { require(failed && triggered,"each allocation ordinal rejects"); rejects([&]{save.close();}); save.abort(); }
        else { require(!failed && !triggered && attempts==census,"exact allocation terminal"); save.abort(); }
        require(context.get("Save/fault.bin")==std::vector<unsigned char>(previous.begin(),previous.end()),"all failed/unpublished operations preserve accepted file");
        save.open(AsciiString("Save/retry.bin")); save.xferUnicodeString(const_cast<UnicodeString*>(&text)); save.close();
        context.clean();
    }
}
template<class CONTAINER, class TRANSFER>
void collectionOwner(Context& context, TRANSFER transfer) {
    using Value=typename CONTAINER::value_type;
    CONTAINER source;
    for (UnsignedInt i=1;i<=3;++i) source.push_back(static_cast<Value>(i));
    XferSave save; save.open(AsciiString("Save/collection.bin"));
    transfer(save,source); save.close();
    const auto accepted=context.get("Save/collection.bin");
    require(accepted.size()==3+3*sizeof(Value) && accepted[0]==1 && accepted[1]==3 && accepted[2]==0,"unchanged collection version/count/element widths");
    XferLoad load; CONTAINER output;
    load.open(AsciiString("Save/collection.bin")); transfer(load,output); load.close();
    require(output==source,"actual collection roundtrip");
    context.put("Save/truncated.bin",std::span(accepted).first(accepted.size()-1));
    output.clear(); load.open(AsciiString("Save/truncated.bin"));
    rejects([&]{transfer(load,output);}); require(output.empty(),"late truncated collection cannot publish its prefix");
    rejects([&]{load.skip(0);}); load.abort();
    load.open(AsciiString("Save/collection.bin")); output=source;
    rejects([&]{transfer(load,output);}); require(output==source,"original nonempty-destination rejection preserves values"); load.abort();
    CONTAINER{}.swap(output); load.open(AsciiString("Save/collection.bin"));
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    transfer(load,output); const auto census=AllocationFault::attempts(); AllocationFault::disarm(); load.close();
    require(census==3,"complete three-element candidate allocation manifest");
    for (std::size_t ordinal=0;ordinal<=census;++ordinal) {
        CONTAINER{}.swap(output); load.open(AsciiString("Save/collection.bin"));
        AllocationFault::arm(ordinal); bool failed=false;
        try { transfer(load,output); } catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto attempts=AllocationFault::attempts(); const auto triggered=AllocationFault::triggered(); AllocationFault::disarm();
        if (ordinal<census) {
            require(failed && triggered && output.empty(),"each failed candidate ordinal leaves destination empty");
            rejects([&]{load.skip(0);});
        } else require(!failed && !triggered && attempts==census && output==source,"complete allocation terminal");
        load.abort(); output.clear(); load.open(AsciiString("Save/collection.bin"));
        transfer(load,output); load.close(); require(output==source,"same-owner collection retry");
    }
    CONTAINER maximum(65535,static_cast<Value>(1));
    save.open(AsciiString("Save/maximum.bin")); transfer(save,maximum); save.close();
    require(context.get("Save/maximum.bin").size()==3+65535*sizeof(Value),"exact UInt16 collection maximum");
    maximum.push_back(static_cast<Value>(2));
    save.open(AsciiString("Save/collection.bin")); Int ready=17; save.xferInt(&ready);
    rejects([&]{transfer(save,maximum);}); rejects([&]{save.close();}); save.abort();
    require(context.get("Save/collection.bin")==accepted,"max+1 rejection poisons commit-ready candidate");
}
void collections() {
    Context context;
    collectionOwner<std::vector<ObjectID>>(context,[](Xfer& x,auto& c){x.xferSTLObjectIDVector(&c);});
    collectionOwner<std::list<ObjectID>>(context,[](Xfer& x,auto& c){x.xferSTLObjectIDList(&c);});
    collectionOwner<std::list<Int>>(context,[](Xfer& x,auto& c){x.xferSTLIntList(&c);});
    context.clean();
}
void masks() {
    Context context;
    KindOfMaskType source; source.set(0); source.set(source.size()-1);
    XferSave save; save.open(AsciiString("Save/mask.bin")); source.xfer(&save); save.close();
    XferLoad load; KindOfMaskType output; output.set(1);
    load.open(AsciiString("Save/mask.bin")); output.xfer(&load); load.close();
    require(output==source,"actual named mask replaces accepted bits completely");
    save.open(AsciiString("Save/badmask.bin"));
    XferVersion version=1; Int count=2; AsciiString first(KindOfMaskType::getNameFromSingleBit(0)); AsciiString unknown("GENERATED_INVALID_FLAG");
    save.xferVersion(&version,1); save.xferInt(&count); save.xferAsciiString(&first); save.xferAsciiString(&unknown); save.close();
    output.clear(); output.set(1); const auto prior=output;
    load.open(AsciiString("Save/badmask.bin")); rejects([&]{output.xfer(&load);});
    require(output==prior,"late invalid mask name preserves accepted bits"); rejects([&]{load.skip(0);}); load.abort();
    for (Int invalid : {-1,source.size()+1}) {
        save.open(AsciiString("Save/badmask.bin")); save.xferVersion(&version,1); save.xferInt(&invalid); save.close();
        load.open(AsciiString("Save/badmask.bin")); rejects([&]{output.xfer(&load);});
        require(output==prior,"invalid signed mask count preserves accepted bits"); load.abort();
    }
    std::vector<UnsignedInt> words((source.size()+31)/32,0);
    words.front()|=1; words[(source.size()-1)/32]|=UnsignedInt{1}<<((source.size()-1)%32);
    XferCRC crc,oracle; crc.open(AsciiString("actual mask")); source.xfer(&crc);
    oracle.open(AsciiString("canonical words")); oracle.xferVersion(&version,1);
    oracle.xferUser(words.data(),static_cast<Int>(words.size()*4));
    require(crc.getCRC()==oracle.getCRC(),"complete canonical initialized 32-bit mask payload");
    source.set(source.size()-1,0); XferCRC changed; changed.open(AsciiString("changed high bit")); source.xfer(&changed);
    require(changed.getCRC()!=crc.getCRC(),"high mask bits participate in CRC");
    load.open(AsciiString("Save/mask.bin")); output.xfer(&load); load.close();
    require(output.test(output.size()-1),"same-owner mask corrected retry"); context.clean();
}
template<class VALUE, class TRANSFER>
void aggregateOwner(Context& context, VALUE source, VALUE retained, TRANSFER transfer) {
    XferSave save; save.open(AsciiString("Save/aggregate.bin")); transfer(save,source); save.close();
    const auto bytes=context.get("Save/aggregate.bin");
    VALUE output=retained; XferLoad load;
    load.open(AsciiString("Save/aggregate.bin")); transfer(load,output); load.close();
    require(std::memcmp(&output,&source,sizeof(VALUE))==0,"complete aggregate roundtrip");
    for (std::size_t length=0;length<bytes.size();++length) {
        context.put("Save/shortaggregate.bin",std::span(bytes).first(length)); output=retained;
        load.open(AsciiString("Save/shortaggregate.bin")); rejects([&]{transfer(load,output);});
        require(std::memcmp(&output,&retained,sizeof(VALUE))==0,"every truncated aggregate boundary preserves accepted fields");
        rejects([&]{load.skip(0);}); load.abort();
    }
    load.open(AsciiString("Save/aggregate.bin")); transfer(load,output); load.close();
    require(std::memcmp(&output,&source,sizeof(VALUE))==0,"same-owner aggregate retry");
}
void aggregates() {
    Context context;
    aggregateOwner(context,Coord3D{1,2,3},Coord3D{4,5,6},[](Xfer& x,auto& v){x.xferCoord3D(&v);});
    aggregateOwner(context,ICoord3D{1,2,3},ICoord3D{4,5,6},[](Xfer& x,auto& v){x.xferICoord3D(&v);});
    aggregateOwner(context,Coord2D{1,2},Coord2D{4,5},[](Xfer& x,auto& v){x.xferCoord2D(&v);});
    aggregateOwner(context,ICoord2D{1,2},ICoord2D{4,5},[](Xfer& x,auto& v){x.xferICoord2D(&v);});
    aggregateOwner(context,RealRange{1,2},RealRange{4,5},[](Xfer& x,auto& v){x.xferRealRange(&v);});
    aggregateOwner(context,Region3D{{1,2,3},{4,5,6}},Region3D{{7,8,9},{10,11,12}},[](Xfer& x,auto& v){x.xferRegion3D(&v);});
    aggregateOwner(context,IRegion3D{{1,2,3},{4,5,6}},IRegion3D{{7,8,9},{10,11,12}},[](Xfer& x,auto& v){x.xferIRegion3D(&v);});
    aggregateOwner(context,Region2D{{1,2},{4,5}},Region2D{{7,8},{10,11}},[](Xfer& x,auto& v){x.xferRegion2D(&v);});
    aggregateOwner(context,IRegion2D{{1,2},{4,5}},IRegion2D{{7,8},{10,11}},[](Xfer& x,auto& v){x.xferIRegion2D(&v);});
    aggregateOwner(context,RGBColor{1,2,3},RGBColor{4,5,6},[](Xfer& x,auto& v){x.xferRGBColor(&v);});
    aggregateOwner(context,RGBAColorReal{1,2,3,4},RGBAColorReal{5,6,7,8},[](Xfer& x,auto& v){x.xferRGBAColorReal(&v);});
    aggregateOwner(context,RGBAColorInt{1,2,3,4},RGBAColorInt{5,6,7,8},[](Xfer& x,auto& v){x.xferRGBAColorInt(&v);});
    Matrix3D source(true),retained(true); source[0].W=3; retained[1].W=7;
    aggregateOwner(context,source,retained,[](Xfer& x,auto& v){x.xferMatrix3D(&v);});
    context.clean();
}
struct FaultIO final : NativeStorageIO {
    enum class Phase { None, Open, Write, Seek, FileSync, Publish, DirectorySync };
    Phase phase=Phase::None; bool throwing=false; unsigned syncCount=0;
    bool fault(Phase at) {
        if (phase!=at) return false;
        if (throwing) throw NativeStorageError();
        errno=EIO; return true;
    }
    int openFile(int directory,const char* name,int flags,unsigned mode) override {
        if (fault(Phase::Open)) return -1;
        return NativeStorageIO::openFile(directory,name,flags,mode);
    }
    Int writeFile(int fd,const void* data,Int count) override {
        if (fault(Phase::Write)) return -1;
        return NativeStorageIO::writeFile(fd,data,count);
    }
    std::int64_t seekFile(int fd,std::uint64_t offset) override {
        if (fault(Phase::Seek)) return -1;
        return NativeStorageIO::seekFile(fd,offset);
    }
    int sync(int fd) override {
        if (fault(++syncCount==1 ? Phase::FileSync : Phase::DirectorySync)) return -1;
        return NativeStorageIO::sync(fd);
    }
    void preparePublish() override { if (fault(Phase::Publish)) throw NativeStorageError(); }
};
std::size_t descriptors() {
    std::size_t count=0;
    for (const auto& entry:std::filesystem::directory_iterator("/proc/self/fd")) { (void)entry; ++count; }
    return count;
}
template<class WRITER>
void outputIOOwner(Context& context,FaultIO& io,bool block) {
    const std::vector<unsigned char> prior{'k','e','e','p'};
    for (bool throwing : {false,true}) for (auto phase : {FaultIO::Phase::Open,FaultIO::Phase::Write,FaultIO::Phase::Seek,
        FaultIO::Phase::FileSync,FaultIO::Phase::Publish,FaultIO::Phase::DirectorySync}) {
        if (!block && phase==FaultIO::Phase::Seek) continue;
        io.phase=FaultIO::Phase::None; io.syncCount=0; context.put("Save/io.bin",prior);
        const auto live=AllocationFault::live(),fds=descriptors();
        {
            WRITER writer; bool rejected=false; io.throwing=throwing;
            try {
                io.phase=phase==FaultIO::Phase::Open ? phase : FaultIO::Phase::None;
                writer.open(AsciiString("Save/io.bin"));
                if (block) writer.beginBlock();
                Int value=17; writer.xferInt(&value); // otherwise commit-ready candidate
                io.phase=phase; io.syncCount=0;
                if (phase==FaultIO::Phase::Write) writer.xferInt(&value);
                if (block) writer.endBlock();
                writer.close();
                require(phase==FaultIO::Phase::DirectorySync && writer.getCommitResult()==NativeCommitResult::PublishedDurabilityUnknown,
                    "post-rename directory failure reports accepted data with unknown durability");
            } catch (XferStatus) { rejected=true; }
            catch (const NativeStorageError&) { rejected=true; }
            require(rejected==(phase!=FaultIO::Phase::DirectorySync),"exact transport I/O outcome contract");
            if (rejected && phase!=FaultIO::Phase::Open) rejects([&]{writer.close();});
            writer.abort(); io.phase=FaultIO::Phase::None; io.syncCount=0;
            const auto actual=context.get("Save/io.bin");
            require(phase==FaultIO::Phase::DirectorySync ? actual!=prior : actual==prior,"truthful accepted file at every publication phase");
            writer.open(AsciiString("Save/io.bin")); Int retry=23; writer.xferInt(&retry); writer.close(); writer.abort();
            require(context.get("Save/io.bin")==std::vector<unsigned char>({23,0,0,0}),"same-owner I/O retry");
        }
        require(AllocationFault::live()==live && descriptors()==fds,"exact I/O owner resource residuals"); context.clean();
    }
}
void ioFaults() {
    FaultIO io; Context context(&io);
    outputIOOwner<XferSave>(context,io,true); outputIOOwner<XferDeepCRC>(context,io,false);
}
void replaySession() {
    Context context;
    const std::vector<unsigned char> prior{'k','e','e','p'};
    const AsciiString filename("Replays/generated.rep");
    context.put(filename.str(),prior);
    const UnicodeString unicode(L"A\u03a9\U0001f642");
    {
        auto writer=NativeReplaySession::record(context.storage,filename);
        writer->write("GENREP",1,6); writer->writeEpoch(std::numeric_limits<Int>::min());
        writer->writeEpoch(std::numeric_limits<Int>::max()); writer->writeWord(0x12345678);
        writer->writeUnicode(unicode);
        require(writer->position()==28 && context.get(filename.str())==prior,"unpublished source-sized header and UTF16 candidate");
        writer->seek(14); writer->writeWord(0x87654321); writer->seek(28); writer->flush();
        require(writer->commit()==NativeCommitResult::Durable,"explicit recording commit");
        rejects([&]{writer->writeWord(0);});
    }
    const std::vector<unsigned char> expected{'G','E','N','R','E','P',0,0,0,128,255,255,255,127,
        0x21,0x43,0x65,0x87,'A',0,0xa9,3,0x3d,0xd8,0x42,0xde,0,0};
    require(context.get(filename.str())==expected,"fixed signed epochs/backpatch/byte-mode Unicode source oracle");
    {
        auto reader=NativeReplaySession::playback(context.storage,filename);
        std::array<char,6> magic{}; reader->readExact(magic.data(),magic.size());
        require(std::string_view(magic.data(),6)=="GENREP" && reader->readEpoch()==std::numeric_limits<Int>::min()
            && reader->readEpoch()==std::numeric_limits<Int>::max() && reader->readWord()==0x87654321,"protected immutable playback header");
        const auto text=nativeReplayReadUnicode([&](void* bytes,Int count){return reader->read(bytes,1,count)==std::size_t(count);});
        require(text==unicode,"paired protected Unicode reader/writer");
        unsigned char byte=0; require(!reader->read(&byte,1,1),"clean input EOF");
        context.put(filename.str(),prior); reader->seek(0); reader->readExact(magic.data(),magic.size());
        require(std::string_view(magic.data(),6)=="GENREP","accepted playback independent of later file replacement");
    }
    for (unsigned operation=0;operation<6;++operation) {
        auto writer=NativeReplaySession::record(context.storage,filename); writer->writeWord(17);
        rejects([&]{
            if (operation==0) writer->seek(5);
            else if (operation==1) writer->write(nullptr,1,1);
            else if (operation==2) writer->write(&operation,std::numeric_limits<std::size_t>::max(),2);
            else if (operation==3) writer->writeEpoch(std::time_t(std::numeric_limits<Int>::max())+1);
            else if (operation==4) { unsigned char byte; writer->read(&byte,1,1); }
            else { NativeReplaySession::OwnerGuard guard(writer); throw XFER_INVALID_PARAMETERS; }
        });
        require(writer->poisoned(),"commit-ready session rejected/parent operation poisons");
        rejects([&]{writer->commit();}); writer.reset();
        require(context.get(filename.str())==prior,"poisoned/destructed candidate preserves previous replay");
    }
    {
        auto writer=NativeReplaySession::record(context.storage,filename); writer->writeWord(41);
        writer.reset(); require(context.get(filename.str())==prior,"destructor abort without fallible close");
        writer=NativeReplaySession::record(context.storage,filename); writer->writeWord(23); writer->commit();
    }
    context.put("Replays/bad.rep",std::array<unsigned char,1>{2});
    auto bad=NativeReplaySession::playback(context.storage,AsciiString("Replays/bad.rep"));
    rejects([&]{bad->readBoolean();}); require(bad->poisoned(),"raw invalid Bool admitted before native boolean materialization"); bad.reset();
    for (unsigned length=0;length<4;++length) {
        const std::array<unsigned char,4> word{}; context.put("Replays/bad.rep",std::span(word).first(length));
        bad=NativeReplaySession::playback(context.storage,AsciiString("Replays/bad.rep"));
        rejects([&]{bad->readEpoch();}); require(bad->poisoned(),"every short fixed epoch read poisons before publication"); bad.reset();
    }
    require(!NativeReplaySession::playback(context.storage,AsciiString("Replays/missing.rep")),"optional missing replay");
    rejects([&]{NativeReplaySession::record(context.storage,AsciiString("../escape.rep"));}); context.clean();
}
void replaySessionFaults() {
    FaultIO io; Context context(&io);
    const AsciiString filename("Replays/fault.rep"); const std::vector<unsigned char> prior{'k','e','e','p'};
    const UnicodeString text(L"generated \u03a9\U0001f642");
    context.put(filename.str(),prior);
    const auto run=[&]{
        auto candidate=NativeReplaySession::record(context.storage,filename);
        candidate->writeWord(0); candidate->writeUnicode(text); const auto end=candidate->position();
        candidate->seek(0); candidate->writeWord(17); candidate->seek(end); candidate->commit();
    };
    AllocationFault::arm(std::numeric_limits<std::size_t>::max()); run();
    const auto census=AllocationFault::attempts(); AllocationFault::disarm();
    require(census==14,"current complete protected recording allocation manifest");
    for (std::size_t ordinal=0;ordinal<=census;++ordinal) {
        context.put(filename.str(),prior); const auto live=AllocationFault::live(),fds=descriptors();
        bool failed=false; AllocationFault::arm(ordinal);
        try { run(); } catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto attempts=AllocationFault::attempts(); const auto triggered=AllocationFault::triggered(); AllocationFault::disarm();
        require(failed==(ordinal<census) && triggered==(ordinal<census)
            && (ordinal<census || attempts==census),"every allocation boundary and exact terminal");
        require(AllocationFault::live()==live && descriptors()==fds,"complete session allocation rollback/terminal residuals");
        require((context.get(filename.str())==prior)==failed,"prepublication allocations preserve accepted recording");
        run(); context.clean();
    }
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    auto playback=NativeReplaySession::playback(context.storage,filename);
    const auto inputCensus=AllocationFault::attempts(); AllocationFault::disarm(); playback.reset();
    require(inputCensus>0 && inputCensus<100,"complete protected immutable playback acquisition census");
    for (std::size_t ordinal=0;ordinal<=inputCensus;++ordinal) {
        const auto live=AllocationFault::live(),fds=descriptors(); bool failed=false;
        AllocationFault::arm(ordinal);
        try { playback=NativeReplaySession::playback(context.storage,filename); }
        catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto attempts=AllocationFault::attempts(); const auto triggered=AllocationFault::triggered(); AllocationFault::disarm();
        require(failed==(ordinal<inputCensus) && triggered==(ordinal<inputCensus)
            && (ordinal<inputCensus || attempts==inputCensus),"complete immutable input allocation ordinals and terminal");
        playback.reset();
        require(AllocationFault::live()==live && descriptors()==fds,"immutable input constructor failure/terminal retires all acquired ownership");
        playback=NativeReplaySession::playback(context.storage,filename);
        require(playback->readWord()==17,"same-storage/same-owning-link playback retry"); playback.reset();
    }
    for (bool throwing : {false,true}) for (auto phase : {FaultIO::Phase::Open,FaultIO::Phase::Write,FaultIO::Phase::Seek,
            FaultIO::Phase::FileSync,FaultIO::Phase::Publish,FaultIO::Phase::DirectorySync}) {
        io.phase=FaultIO::Phase::None; io.syncCount=0; context.put(filename.str(),prior);
        const auto live=AllocationFault::live(),fds=descriptors();
        std::unique_ptr<NativeReplaySession> candidate; bool failed=false; io.throwing=throwing;
        try {
            io.phase=phase==FaultIO::Phase::Open ? phase : FaultIO::Phase::None;
            candidate=NativeReplaySession::record(context.storage,filename); candidate->writeWord(17);
            io.phase=phase; io.syncCount=0;
            if (phase==FaultIO::Phase::Write) candidate->writeWord(23);
            if (phase==FaultIO::Phase::Seek) candidate->seek(0);
            const auto result=candidate->commit();
            require(phase==FaultIO::Phase::DirectorySync && result==NativeCommitResult::PublishedDurabilityUnknown,"truthful postpublication durability status");
        } catch(XferStatus) { failed=true; } catch(const NativeStorageError&) { failed=true; }
        require(failed==(phase!=FaultIO::Phase::DirectorySync),"exact session I/O outcome");
        if (failed && candidate) { require(candidate->poisoned(),"I/O rejection poisons session"); rejects([&]{candidate->commit();}); }
        candidate.reset(); io.phase=FaultIO::Phase::None; io.syncCount=0;
        require(AllocationFault::live()==live && descriptors()==fds,"exact descriptors/backing after all I/O faults");
        require((context.get(filename.str())==prior)==failed,"accepted recording preserved at every reversible I/O boundary");
        run(); context.clean();
    }
    std::cout<<"protected replay recording allocation manifest "<<census<<" playback "<<inputCensus<<'\n';
}
void services() {
    // Generated service-contract evidence only; this is not a GameState fixture.
    Context context;
    struct Owner { bool failEncode=false,failDecode=false,failNotify=false; unsigned notifications=0; } owner,other;
    struct Withdrawal { const void* owner; ~Withdrawal(){nativeWithdrawTransferServices(owner);} } withdrawal{&owner};
    NativeTransferServices binding{&owner,
        [](void* raw,const AsciiString& value) { if (static_cast<Owner*>(raw)->failEncode) throw XFER_UNKNOWN_STRING; return value; },
        [](void* raw,const AsciiString& value) { if (static_cast<Owner*>(raw)->failDecode) throw XFER_UNKNOWN_STRING; return value; },
        [](void* raw,Snapshot*) { auto& o=*static_cast<Owner*>(raw); ++o.notifications; if (o.failNotify) throw XFER_INVALID_PARAMETERS; },
        [](void*,ScienceType) { return AsciiString("GENERATED_SCIENCE"); },
        [](void*,const AsciiString& name) { if (name!=AsciiString("GENERATED_SCIENCE")) throw XFER_UNKNOWN_STRING; return static_cast<ScienceType>(1); },
        [](void*,const UpgradeMaskType&) { return std::vector<AsciiString>{AsciiString("GENERATED_UPGRADE")}; },
        [](void*,const AsciiString& name) { if (name!=AsciiString("GENERATED_UPGRADE")) throw XFER_UNKNOWN_STRING; UpgradeMaskType mask; mask.set(0); return mask; }};
    require(!nativeTransferServices().owner,"generated fixture begins without parent service");
    const std::vector<unsigned char> previous{'k','e','e','p'}; context.put("Save/service.bin",previous);
    XferSave save; AsciiString name("generated-map");
    save.open(AsciiString("Save/service.bin")); Int ready=17; save.xferInt(&ready);
    rejects([&]{save.xferMapName(&name);}); rejects([&]{save.close();}); save.abort();
    require(context.get("Save/service.bin")==previous,"missing map provider poisons commit-ready output");
    context.put("Save/value.bin",std::array<unsigned char,4>{41,0,0,0});
    XferLoad load; ValueSnapshot snapshot;
    load.open(AsciiString("Save/value.bin")); rejects([&]{load.xferSnapshot(&snapshot);});
    require(snapshot.calls==0,"missing notification provider admitted before snapshot callback"); load.abort();
    AllocationFault::arm(0); nativeBindTransferServices(binding);
    require(AllocationFault::attempts()==0,"allocation-free complete service registration"); AllocationFault::disarm();
    auto incomplete=binding; incomplete.decodeMap=nullptr;
    rejects([&]{nativeBindTransferServices(incomplete);});
    auto conflicting=binding; conflicting.owner=&other;
    rejects([&]{nativeBindTransferServices(conflicting);});
    nativeWithdrawTransferServices(&other);
    require(nativeTransferServices().owner==&owner && nativeTransferServices().decodeMap==binding.decodeMap,"invalid registration/foreign withdrawal preserve accepted binding");
    owner.failEncode=true; save.open(AsciiString("Save/service.bin")); save.xferInt(&ready);
    rejects([&]{save.xferMapName(&name);}); rejects([&]{save.close();}); save.abort(); owner.failEncode=false;
    save.open(AsciiString("Save/map.bin")); save.xferMapName(&name); save.close();
    AsciiString retained("retained"); owner.failDecode=true;
    load.open(AsciiString("Save/map.bin")); rejects([&]{load.xferMapName(&retained);});
    require(retained==AsciiString("retained"),"decode callback failure preserves map output"); rejects([&]{load.skip(0);}); load.abort(); owner.failDecode=false;
    load.open(AsciiString("Save/map.bin")); load.xferMapName(&retained); load.close(); require(retained==name,"same-owner map codec retry");
    owner.failNotify=true; load.open(AsciiString("Save/value.bin")); rejects([&]{load.xferSnapshot(&snapshot);});
    require(snapshot.calls==1 && owner.notifications==1,"fault occurs inside notification after snapshot callback");
    rejects([&]{load.skip(0);}); load.abort(); owner.failNotify=false;
    load.open(AsciiString("Save/value.bin")); load.xferSnapshot(&snapshot); load.close();
    require(owner.notifications==2 && snapshot.calls==2,"same-owner notification retry");
    ScienceVec sciences{static_cast<ScienceType>(1),static_cast<ScienceType>(1)};
    save.open(AsciiString("Save/sciences.bin")); save.xferScienceVec(&sciences); save.close();
    ScienceVec scienceOutput{static_cast<ScienceType>(2)};
    load.open(AsciiString("Save/sciences.bin")); load.xferScienceVec(&scienceOutput); load.close();
    require(scienceOutput==sciences,"science adapter replaces nonempty destination only on complete success");
    save.open(AsciiString("Save/badnames.bin")); XferVersion version=1; UnsignedShort count=2;
    AsciiString scienceName("GENERATED_SCIENCE"),upgradeName("GENERATED_UPGRADE"),unknownName("GENERATED_UNKNOWN");
    save.xferVersion(&version,1); save.xferUnsignedShort(&count); save.xferAsciiString(&scienceName); save.xferAsciiString(&unknownName); save.close();
    const ScienceVec sciencePrior{static_cast<ScienceType>(2)}; scienceOutput=sciencePrior;
    load.open(AsciiString("Save/badnames.bin")); rejects([&]{load.xferScienceVec(&scienceOutput);});
    require(scienceOutput==sciencePrior,"late science codec failure preserves creation-time sciences"); rejects([&]{load.skip(0);}); load.abort();
    std::size_t scienceCensus=0;
    load.open(AsciiString("Save/sciences.bin"));
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    load.xferScienceVec(&scienceOutput); scienceCensus=AllocationFault::attempts(); AllocationFault::disarm(); load.close();
    require(scienceCensus>0 && scienceCensus<32,"bounded complete generated science candidate manifest");
    for (std::size_t ordinal=0;ordinal<=scienceCensus;++ordinal) {
        scienceOutput=sciencePrior; load.open(AsciiString("Save/sciences.bin"));
        bool failed=false; AllocationFault::arm(ordinal);
        try { load.xferScienceVec(&scienceOutput); } catch(const std::bad_alloc&) { failed=true; }
        catch(...) { AllocationFault::disarm(); throw; }
        const auto attempts=AllocationFault::attempts(); const auto triggered=AllocationFault::triggered(); AllocationFault::disarm();
        if (ordinal<scienceCensus) {
            require(failed && triggered && scienceOutput==sciencePrior,"every science candidate fault preserves accepted sciences"); rejects([&]{load.skip(0);});
        } else require(!failed && !triggered && attempts==scienceCensus && scienceOutput==sciences,"exact science manifest terminal");
        load.abort(); load.open(AsciiString("Save/sciences.bin")); load.xferScienceVec(&scienceOutput); load.close();
        require(scienceOutput==sciences,"same-owner science allocation retry");
    }
    ScienceVec oversized(65536,static_cast<ScienceType>(1));
    save.open(AsciiString("Save/service.bin")); save.xferInt(&ready);
    rejects([&]{save.xferScienceVec(&oversized);}); rejects([&]{save.close();}); save.abort();
    require(context.get("Save/service.bin")==previous,"science UInt16 overflow poisons commit-ready output");
    UpgradeMaskType upgrade; upgrade.set(1); const auto upgradePrior=upgrade;
    save.open(AsciiString("Save/badnames.bin")); save.xferVersion(&version,1); save.xferUnsignedShort(&count);
    save.xferAsciiString(&upgradeName); save.xferAsciiString(&unknownName); save.close();
    load.open(AsciiString("Save/badnames.bin")); rejects([&]{load.xferUpgradeMask(&upgrade);});
    require(upgrade==upgradePrior,"late upgrade codec rejection preserves accepted mask"); rejects([&]{load.skip(0);}); load.abort();
    save.open(AsciiString("Save/upgrade.bin")); save.xferUpgradeMask(&upgrade); save.close();
    load.open(AsciiString("Save/upgrade.bin")); load.xferUpgradeMask(&upgrade); load.close();
    require(upgrade.count()==1 && upgrade.test(0),"same-owner named upgrade replacement retry");
    std::cout<<"generated science allocation manifest "<<scienceCensus<<'\n';
    nativeWithdrawTransferServices(&owner); require(!nativeTransferServices().owner,"exact owner withdrawal"); context.clean();
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"transfer family"); const std::string family(argv[1]);
        std::cout<<"generated transfer ownership families\n";
        for (int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if (family=="wire") wire(); else if (family=="functional") functional();
            else if (family=="malformed") malformed(); else if (family=="poisoning") poisoning();
            else if (family=="faults") faults(); else if (family=="collections") collections();
            else if (family=="masks") masks(); else if (family=="aggregates") aggregates();
            else if (family=="io-faults") ioFaults();
            else if (family=="services") services();
            else if (family=="replay-session") replaySession();
            else if (family=="replay-session-faults") replaySessionFaults();
            else throw std::runtime_error("unknown transfer family");
            require(AllocationFault::live()==live,"complete same-process transfer lifetimes");
        }
        std::cout<<"PASS actual transfer owner support (whole state/replay runtime pending)\n"; return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL "<<error.what()<<'\n'; }
    catch(XferStatus) { std::cerr<<"FAIL source transfer status\n"; }
    AllocationFault::disarm(); return 1;
}
