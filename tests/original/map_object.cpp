// SPDX-License-Identifier: GPL-3.0-or-later
// Actual source map objects and pool ownership; not whole-map/game acceptance.
#include "AllocationFault.h"
#include "Common/MapObject.h"
#include "Common/NameKeyGenerator.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/NativeUserStorage.h"
#include <array>
#include <filesystem>
#include <cstdio>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
namespace {
void require(bool value,const char* label) {if(!value) throw std::runtime_error(label);}
struct Retire {void operator()(MapObject* value) const noexcept {if(value)value->deleteInstance();}};
using Owner=std::unique_ptr<MapObject,Retire>;
Owner object(const char* name="Folder/Generated",const Dict* props=nullptr) {
    return Owner(newInstance(MapObject)(Coord3D{1,2,3},AsciiString(name),0.5f,FLAG_ROAD_POINT1,props,nullptr));
}
struct Context {
    NameKeyGenerator names;
    NameKeyGenerator* previousNames;
    MapObject* previousHead;
    Dict previousWorld;
    Context() {
        names.init(); previousNames=TheNameKeyGenerator; previousHead=MapObject::TheMapObjectListPtr;
        TheNameKeyGenerator=&names; MapObject::TheMapObjectListPtr=nullptr;
        MapObject::TheWorldDict.swap(previousWorld);
    }
    ~Context() {
        if(MapObject::TheMapObjectListPtr) MapObject::TheMapObjectListPtr->deleteInstance();
        MapObject::TheMapObjectListPtr=previousHead;
        MapObject::TheWorldDict.clear(); MapObject::TheWorldDict.swap(previousWorld);
        TheNameKeyGenerator=previousNames;
    }
};
MemoryPool* pool() {return TheMemoryPoolFactory->findMemoryPool("MapObject");}
MemoryPool* keyPool() {return TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool");}
void values() {
    Context context; auto source=object();
    require(source->getLocation()->x==1 && source->getLocation()->z==3 && source->getAngle()==0.5f &&
        source->getFlags()==FLAG_ROAD_POINT1 && source->getColor()==0xff00,"source record values");
    const auto* properties=source->getProperties();
    require(properties->getPairCount()==7 && properties->getInt(TheKey_objectInitialHealth)==100 &&
        properties->getBool(TheKey_objectEnabled) && properties->getBool(TheKey_objectPowered) &&
        properties->getBool(TheKey_objectRecruitableAI) && !properties->getBool(TheKey_objectIndestructible) &&
        !properties->getBool(TheKey_objectUnsellable) && !properties->getBool(TheKey_objectTargetable),"source defaults");
    Bool exists=true; properties->getBool(TheKey_objectSelectable,&exists); require(!exists,"source omitted selectable");
    source->setColor(77); source->setSelected(true); source->setIsWaypoint();
    source->setWaypointID(8); source->setWaypointName("GeneratedWaypoint");
    Owner clone(source->duplicate());
    require(clone->getName()==source->getName() && clone->getColor()==77 && clone->isSelected() && clone->isWaypoint() &&
        clone->getWaypointID()==source->getWaypointID() && clone->getWaypointName()=="GeneratedWaypoint" &&
        !clone->getRenderObj() && !clone->getShadowObj(),"source duplicate logical payload without presentation copy");
    clone->setWaypointName("Changed"); require(source->getWaypointName()=="GeneratedWaypoint","shared dictionary copy detaches");
    bool rejected=false;try{source->setThingTemplate(nullptr);}catch(ErrorCode){rejected=true;}
    require(rejected && !source->getThingTemplate() && source->getName()=="Folder/Generated","invalid template preserves accepted record");
    auto tail=object("Second"); source->setNextMap(tail.release());
    require(pool()->getUsedBlockCount()==3,"source linked units acquired"); source.reset();
    require(pool()->getUsedBlockCount()==1,"source linked cleanup is iterative and complete");
}
// Generated opaque identity used only to observe ownership operations. There is
// no RenderObjClass definition or fake renderer/gameplay provider in this test.
struct Lease {unsigned references=1,acquired=0,released=0;bool underflow=false;};
RenderObjClass* identity(Lease& value) {return static_cast<RenderObjClass*>(static_cast<void*>(&value));}
Lease& lease(RenderObjClass* value) {return *static_cast<Lease*>(static_cast<void*>(value));}
void retain(RenderObjClass* value) noexcept {auto& owner=lease(value);++owner.references;++owner.acquired;}
void release(RenderObjClass* value) noexcept {auto& owner=lease(value);if(!owner.references)owner.underflow=true;else --owner.references;++owner.released;}
void references() {
    Context context; Lease first,second; const NativeMapRenderOwnership binding{retain,release};
    {
        auto source=object();
        for(Int i=0;i<BRIDGE_MAX_TOWERS;++i) require(!source->getBridgeRenderObject(BridgeTowerType(i)),"initialized tower slot");
        source->setRenderObj(identity(first),binding);
        for(Int i=0;i<BRIDGE_MAX_TOWERS;++i)source->setBridgeRenderObject(BridgeTowerType(i),identity(first),binding);
        require(first.references==6 && first.acquired==5,"same pointer owns five separate acquired references");
        AllocationFault::arm(0);
        source->setRenderObj(identity(first),binding);
        source->setBridgeRenderObject(BRIDGE_TOWER_FROM_LEFT,identity(second),binding);
        const auto attempted=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
        require(!hit && !attempted && first.references==5 && second.references==2,"replacement acquires before retiring exactly one unit without allocation");
        bool rejected=false;try{source->setRenderObj(identity(second));}catch(ErrorCode){rejected=true;}
        require(rejected && source->getRenderObj()==identity(first) && first.references==5 && second.references==2,"missing ownership rejects before mutation");
        source->setRenderObj(nullptr); require(first.references==4,"null withdrawal uses captured owner");
        for(auto raw:{UnsignedInt(BRIDGE_MAX_TOWERS),std::numeric_limits<UnsignedInt>::max()})
            require(!source->getBridgeRenderObject(BridgeTowerType(raw)),"defined tower bounds");
        source->setBridgeRenderObject(BridgeTowerType(BRIDGE_MAX_TOWERS),identity(second),binding);
        require(second.references==2,"out-of-range source setter omitted");
    }
    require(first.references==1 && second.references==1 && first.acquired==first.released &&
        second.acquired==second.released && !first.underflow && !second.underflow,"every acquired unit retires, including equal identities");
}
void constructorFaults() {
    std::size_t census=0;
    {Context context;AllocationFault::arm(std::numeric_limits<std::size_t>::max());auto candidate=object();
     census=AllocationFault::attempts();AllocationFault::disarm();}
    require(census>0 && !pool()->getUsedBlockCount() && !keyPool()->getUsedBlockCount(),"discovery retires before baselines");
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
        Context context;const auto live=AllocationFault::live();bool failed=false;Owner candidate;
        AllocationFault::arm(ordinal);
        try{candidate=object();}catch(const std::bad_alloc&){failed=true;}
        const auto attempted=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
        if(ordinal<census) {
            require(failed && hit && attempted==ordinal+1 && !pool()->getUsedBlockCount() && !keyPool()->getUsedBlockCount() &&
                AllocationFault::live()==live && context.names.keyToName(NameKeyType(1)).isEmpty(),"cold construction rejects complete acquired prefix and namespace");
            candidate=object();
        } else require(!failed && !hit && attempted==census,"exact constructor terminal");
        require(candidate->getProperties()->getInt(TheKey_objectInitialHealth)==100,"corrected constructor retry");
    }
    std::printf("map-constructor allocations=%zu every failure/retry and terminal retained\n",census);
}
std::array<MapObject*,3> scene() {
    std::array<Owner,3> owners{object("Folder/FirstLongGeneratedMapName"),object("Folder/SecondLongGeneratedMapName"),object("Waypoint")};
    owners[2]->setIsWaypoint();owners[2]->setWaypointName("SourceWaypoint");
    for(auto& owner:owners) owner->getProperties()->setAsciiString(TheKey_uniqueID,"PriorAcceptedIdentity");
    std::array<MapObject*,3> result{owners[0].get(),owners[1].get(),owners[2].get()};
    owners[0]->setNextMap(owners[1].release());owners[0]->getNext()->setNextMap(owners[2].release());
    MapObject::TheMapObjectListPtr=owners[0].release();return result;
}
void verifyNames(const std::array<MapObject*,3>& objects) {
    require(objects[0]->getProperties()->getAsciiString(TheKey_uniqueID)=="FirstLongGeneratedMapName 2" &&
        objects[1]->getProperties()->getAsciiString(TheKey_uniqueID)=="SecondLongGeneratedMapName 1" &&
        objects[2]->getProperties()->getAsciiString(TheKey_uniqueID)=="SourceWaypoint","source reverse stack and waypoint ordinal order");
}
void names(bool faults) {
    Context context;auto objects=scene();
    if(!faults) {
        MapObject::fastAssignAllUniqueIDs();verifyNames(objects);
        objects[0]->getProperties()->setAsciiString(TheKey_uniqueID,"First 2147483647");
        const auto* prior=objects[1]->getProperties()->getAsciiString(TheKey_uniqueID).str();
        bool rejected=false;try{objects[1]->verifyValidUniqueID();}catch(ErrorCode){rejected=true;}
        require(rejected && objects[1]->getProperties()->getAsciiString(TheKey_uniqueID).str()==prior,"source ID increment bounds preserve prior backing");
        return;
    }
    std::size_t census=0;
    {
        std::array<Dict,3> before{*objects[0]->getProperties(),*objects[1]->getProperties(),*objects[2]->getProperties()};
        AllocationFault::arm(std::numeric_limits<std::size_t>::max());MapObject::fastAssignAllUniqueIDs();
        census=AllocationFault::attempts();AllocationFault::disarm();verifyNames(objects);
        for(std::size_t i=0;i<3;++i)objects[i]->getProperties()->swap(before[i]);
    }
    require(census>0,"complete ID discovery");
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
        std::array<Dict,3> before{*objects[0]->getProperties(),*objects[1]->getProperties(),*objects[2]->getProperties()};
        std::array<const char*,3> backing{};
        for(std::size_t i=0;i<3;++i)backing[i]=objects[i]->getProperties()->getAsciiString(TheKey_uniqueID).str();
        const auto live=AllocationFault::live();const auto used=pool()->getUsedBlockCount();bool failed=false;
        AllocationFault::arm(ordinal);
        try{MapObject::fastAssignAllUniqueIDs();}catch(const std::bad_alloc&){failed=true;}
        const auto attempted=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
        if(ordinal<census) {
            require(failed && hit && attempted==ordinal+1 && AllocationFault::live()==live && pool()->getUsedBlockCount()==used,"ID preparation immediate retirement");
            for(std::size_t i=0;i<3;++i)require(objects[i]->getProperties()->getAsciiString(TheKey_uniqueID).str()==backing[i],"all accepted ID backing retained before retry");
            MapObject::fastAssignAllUniqueIDs();
        } else require(!failed && !hit && attempted==census,"exact ID terminal");
        verifyNames(objects);
        for(std::size_t i=0;i<3;++i)objects[i]->getProperties()->swap(before[i]);
    }
    std::printf("map-ID allocations=%zu every failure/retry and terminal retained\n",census);
}
void poolFailure() {
    Context context;auto source=object();std::array<void*,16> held{};std::size_t count=0;
    while(pool()->getFreeBlockCount())held[count++]=pool()->allocateBlock("generated owned capacity");
    const auto used=pool()->getUsedBlockCount();const auto live=AllocationFault::live();bool failed=false;
    try{Owner clone(source->duplicate());}catch(ErrorCode error){failed=error==ERROR_OUT_OF_MEMORY;}
    require(failed && used==pool()->getUsedBlockCount() && AllocationFault::live()==live &&
        source->getName()=="Folder/Generated","pooled duplicate exhaustion preserves source and ownership");
    for(std::size_t i=0;i<count;++i)pool()->freeBlock(held[i]);
    Owner retry(source->duplicate());require(retry->getName()==source->getName(),"same-owner pooled duplicate retry");
}
struct TemplateOwner : ThingTemplate {~TemplateOwner() override=default;};
void templateBinding() {
    Context context;
    char pattern[]="/tmp/zh-map-template-XXXXXX";
    const auto* directory=::mkdtemp(pattern);
    require(directory,"generated template data parents");
    const std::filesystem::path root(directory);
    struct RetireRoot {
        const std::filesystem::path& root;
        ~RetireRoot() noexcept {std::error_code ignored;std::filesystem::remove_all(root,ignored);}
    } retireRoot{root};
    std::filesystem::create_directory(root/"assets");
    FileSystem files;files.mountReadOnly({(root/"assets").string()});
    NativeUserStorage storage(NativeUserPaths::resolve(root.string(),(root/"data").string(),(root/"cache").string()),files);
    struct RestoreDataParents {
        FileSystem* files;
        NativeUserStorage* storage;
        ~RestoreDataParents() noexcept {TheFileSystem=files;TheNativeUserStorage=storage;}
    } restoreParents{TheFileSystem,TheNativeUserStorage};
    TheFileSystem=&files;TheNativeUserStorage=&storage;
    GlobalData globals;
    globals.m_defaultOcclusionDelay=37;
    auto* previous=TheWritableGlobalData;
    TheWritableGlobalData=&globals;
    struct RestoreGlobal {
        GlobalData* previous;
        ~RestoreGlobal() noexcept {TheWritableGlobalData=previous;}
    } restore{previous};
    TemplateOwner original,middle,final;
    original.friend_setTemplateName("Generated/BaseTemplate");
    middle.friend_setTemplateName("Generated/MiddleTemplate");
    final.friend_setTemplateName("Generated/FinalTemplate");
    original.setNextOverride(&middle);middle.setNextOverride(&final);
    struct DetachBorrowedOverrides {
        ThingTemplate& original;ThingTemplate& middle;
        ~DetachBorrowedOverrides() noexcept {original.setNextOverride(nullptr);middle.setNextOverride(nullptr);}
    } detach{original,middle};
    auto source=object();
    AllocationFault::arm(0);
    source->setThingTemplate(&original);
    const auto attempted=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(!attempted && !hit && source->getThingTemplate()==&final &&
        source->getName()=="Generated/BaseTemplate","actual template binding and final override without allocation");
    source->verifyValidUniqueID();
    require(source->getProperties()->getAsciiString(TheKey_uniqueID)=="FinalTemplate 0",
        "source ID uses final template name rather than base record name");
    Owner clone(source->duplicate());
    require(clone->getThingTemplate()==&final && clone->getName()=="Generated/BaseTemplate",
        "duplicate retains actual borrowed base identity and override resolution");
    middle.setNextOverride(nullptr);
    require(source->getThingTemplate()==&middle && clone->getThingTemplate()==&middle,
        "live original override chain remains authoritative");
    bool rejected=false;try{source->setThingTemplate(nullptr);}catch(ErrorCode){rejected=true;}
    require(rejected && source->getThingTemplate()==&middle && source->getName()=="Generated/BaseTemplate",
        "null template rejects without losing actual binding");
}
}
int main(int argc,char** argv) {
    bool initialized=false;
    try {
        require(argc==2,"family");const std::string_view family=argv[1];initMemoryManager();initialized=true;
        TheMemoryPoolFactory->createMemoryPool("MapObject",sizeof(MapObject),16,0);
        TheMemoryPoolFactory->createMemoryPool("NameKeyBucketPool",sizeof(Bucket),128,0);
        {Context context;auto warm=object();warm->setWaypointName("Warm");warm->setWaypointID(1);}
        if(family=="template-binding")templateBinding(); // retire discovery before repeated-owner baselines
        for(int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if(family=="values")values();else if(family=="references")references();else if(family=="constructor-faults")constructorFaults();
            else if(family=="names")names(false);else if(family=="name-faults")names(true);else if(family=="pool-failure")poolFailure();
            else if(family=="template-binding")templateBinding();else require(false,"unknown family");
            require(AllocationFault::live()==live && !pool()->getUsedBlockCount() && !keyPool()->getUsedBlockCount(),"whole source owner retirement");
        }
        shutdownMemoryManager();initialized=false;return 0;
    }catch(const std::exception& error){AllocationFault::disarm();std::fprintf(stderr,"%s\n",error.what());}
     catch(...){AllocationFault::disarm();std::fprintf(stderr,"unexpected source error\n");}
    if(initialized)shutdownMemoryManager();return 1;
}
