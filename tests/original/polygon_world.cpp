// SPDX-License-Identifier: GPL-3.0-or-later
// Actual source trigger owners/parsers; generated transport only, not world acceptance.
#include "AllocationFault.h"
#include "Common/DataChunk.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/XferLoad.h"
#include "GameLogic/PolygonTrigger.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void require(bool value,const char* text) {if(!value)throw std::runtime_error(text);}
struct Retire {void operator()(PolygonTrigger* p) const noexcept {if(p)p->deleteInstance();}};
using Owner=std::unique_ptr<PolygonTrigger,Retire>;
struct Tree {
  std::filesystem::path path;
  Tree(){char pattern[]="/tmp/zh-polygons-XXXXXX";const auto* created=::mkdtemp(pattern);
    require(created,"generated empty input root");path=created;}
  ~Tree(){std::error_code error;std::filesystem::remove_all(path,error);}
};
struct Context {
  Tree tree;
  FileSystem files;
  FileSystem* previousFiles=TheFileSystem;
  PolygonTrigger::WorldState previous;
  Context(){files.mountReadOnly({tree.path.string()});TheFileSystem=&files;PolygonTrigger::exchangeWorldState(previous);}
  ~Context(){PolygonTrigger::deleteTriggers();PolygonTrigger::exchangeWorldState(previous);TheFileSystem=previousFiles;}
};
struct MemoryInput : ChunkInputStream {
  const std::string& bytes;UnsignedInt position=0;
  explicit MemoryInput(const std::string& input):bytes(input){}
  Int read(void* out,Int count) override {
    if(count<0 || (!out && count))throw ERROR_BAD_ARG;
    const auto size=std::min<std::size_t>(count,bytes.size()-position);
    if(size)std::memcpy(out,bytes.data()+position,size);
    position+=size;return size;
  }
  UnsignedInt tell()override{return position;}
  Bool absoluteSeek(UnsignedInt value)override{if(value>bytes.size())return FALSE;position=value;return TRUE;}
  Bool eof()override{return position==bytes.size();}
};
struct MemoryOutput : OutputStream {
  std::string bytes;
  Int write(const void* data,Int size)override{bytes.append(static_cast<const char*>(data),size);return size;}
};
// In-memory input transport retains the actual original XferLoad scalar adapters,
// snapshot dispatcher and failure poisoning, with no gameplay/terrain substitute.
struct MemoryLoad : XferLoad {
  explicit MemoryLoad(const std::string& input){m_bytes.assign(input.begin(),input.end());m_open=true;setOptions(XO_NO_POST_PROCESSING);}
  bool consumed()const{return m_position==m_bytes.size();}
};
void put(std::string& output,UnsignedInt value,unsigned width=4){for(unsigned n=0;n<width;++n)output+=char(value>>(8*n));}
void overwrite(std::string& output,std::size_t at,UnsignedInt value){for(unsigned n=0;n<4;++n)output.at(at+n)=char(value>>(8*n));}
void ascii(std::string& output,const char* text){put(output,std::strlen(text),2);output+=text;}
std::string toc(){std::string out="CkMp";put(out,1);put(out,15,1);out+="PolygonTriggers";put(out,1);return out;}
std::string wire(const std::string& payload,unsigned version=4){auto out=toc();put(out,1);put(out,version,2);put(out,payload.size());return out+payload;}
std::string payload(unsigned version=4,Int offset=0,Int nodes=3,Int points=4){
  std::string out;put(out,nodes);
  for(Int node=0;node<nodes;++node){
    ascii(out,node==0 ? "GeneratedFirst" : node==1 ? "GeneratedSecond" : "GeneratedDiscarded");
    if(version>=4)ascii(out,"GeneratedLayer");put(out,7+node);
    if(version>=2)put(out,node==0,1);
    if(version>=3){put(out,node==1,1);put(out,2);}
    const Int count=node==2 ? 1 : points;put(out,count);
    for(Int n=0;n<count;++n){put(out,offset+(n%4==1 || n%4==2 ? 100 : 0));
      put(out,n%4>=2 ? 100 : 0);put(out,5+n);}
  }
  return out;
}
void load(const std::string& bytes){MemoryInput stream(bytes);DataChunkInput input(&stream);
  input.registerParser("PolygonTriggers",AsciiString::TheEmptyString,PolygonTrigger::ParsePolygonTriggersDataChunk);
  if(!input.isValidFileType() || !input.parse() || !stream.eof())throw ERROR_CORRUPT_FILE_FORMAT;}
MemoryPool* pool(){return TheMemoryPoolFactory->findMemoryPool("PolygonTrigger");}
void check(Int offset=0,unsigned version=4){
  auto* first=PolygonTrigger::getFirstPolygonTrigger();require(first && first->getID()==7 &&
    first->getTriggerName()=="GeneratedFirst" && first->getPoint(0)->x==offset && first->getNumPoints()==4,
    "source first trigger/file order/points");
  auto* second=first->getNext();require(second && second->getID()==8 && second->getTriggerName()=="GeneratedSecond",
    "source second trigger identity");
  require(first->isWaterArea()==(version>=2) && second->isRiver()==(version>=3) &&
    second->getRiverStart()==(version>=3 ? 2 : 0) &&
    first->getLayerName()==(version>=4 ? "GeneratedLayer" : ""),"version field defaults");
  if(version==1){auto* water=second->getNext();require(water && !water->getNext() && water->getID()==9 &&
    water->isWaterArea() && water->getNumPoints()==4 && water->getPoint(0)->x==-300 && water->getPoint(0)->z==7 &&
    water->getPoint(1)->x==940 && water->getPoint(2)->y==780,"source v1 default water rectangle and ID policy");}
  else require(!second->getNext(),"short polygon consumed/discarded");
}
void functional(){Context context;
  GlobalData data;data.m_waterExtentX=640;data.m_waterExtentY=480;
  for(unsigned version=1;version<=4;++version){
    auto* previous=TheWritableGlobalData;TheWritableGlobalData=&data;
    try{load(wire(payload(version),version));}catch(...){TheWritableGlobalData=previous;throw;}
    TheWritableGlobalData=previous;check(0,version);
    Owner next(newInstance(PolygonTrigger)(2));require(next->getID()==(version==1 ? 11 : 10),"source cursor includes discarded record");
    MemoryOutput out;DataChunkOutput writer(&out);PolygonTrigger::WritePolygonTriggersDataChunk(writer);writer.finish();
    load(out.bytes);auto* first=PolygonTrigger::getFirstPolygonTrigger();
    require(first->getNumPoints()==4 && first->getPoint(2)->y==100 && first->isWaterArea()==(version>=2),
      "actual writer/reader round trip");
  }
  load(wire(payload()));auto* original=PolygonTrigger::getFirstPolygonTrigger();
  PolygonTrigger::WorldState detached;PolygonTrigger::exchangeWorldState(detached);
  require(!PolygonTrigger::getFirstPolygonTrigger(),"whole original publication detached");
  {Owner first(newInstance(PolygonTrigger)(2));require(first->getID()==1,"detached candidate starts independent cursor");}
  AllocationFault::arm(0);PolygonTrigger::exchangeWorldState(detached);
  const auto attempts=AllocationFault::attempts();AllocationFault::disarm();
  require(attempts==0 && original==PolygonTrigger::getFirstPolygonTrigger(),"borrowed publication restores exact identity without allocation");
  {Owner next(newInstance(PolygonTrigger)(2));require(next->getID()==10,"borrowed publication restores ID cursor too");}
  ICoord3D inside{50,50,999},outside{101,50,0},edge{0,50,0};
  require(original->pointInTrigger(inside) && !original->pointInTrigger(outside) && !original->pointInTrigger(edge),
    "source ray/boundary rules ignore Z");
  require(original->getWaterHandle() && original->getWaterHandle()->m_polygon==original &&
    !original->getNext()->getWaterHandle(),"real water handle owner");
  require(PolygonTrigger::getPolygonTriggerByID(8)==original->getNext() &&
    !PolygonTrigger::getPolygonTriggerByID(999),"actual source ID lookup");
  std::string empty;put(empty,0);load(wire(empty));
  require(!PolygonTrigger::getFirstPolygonTrigger(),"explicit empty publication retires accepted list");
  {Owner next(newInstance(PolygonTrigger)(2));require(next->getID()==1,"empty publication resets ID cursor");}
}
void negative(){Context context;const auto valid=wire(payload());load(valid);
  auto reject=[&](const std::string& bytes){auto* head=PolygonTrigger::getFirstPolygonTrigger();
    const auto* points=head->getPoint(0);const auto live=AllocationFault::live();const auto used=pool()->getUsedBlockCount();bool failed=false;
    try{load(bytes);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"trigger input rejection class");failed=true;}
    require(failed && PolygonTrigger::getFirstPolygonTrigger()==head && head->getPoint(0)==points &&
      AllocationFault::live()==live && pool()->getUsedBlockCount()==used,"malformed trigger preserves full accepted ownership");
    {Owner next(newInstance(PolygonTrigger)(2));require(next->getID()==10,"malformed candidate restores cursor");}
    load(valid);check();};
  for(std::size_t prefix=0;prefix<valid.size();++prefix)reject(valid.substr(0,prefix));
  // Earlier-version physical truncations include the generated v1-water path.
  {GlobalData data;auto* previous=TheWritableGlobalData;TheWritableGlobalData=&data;
    try{for(unsigned version=1;version<4;++version){const auto bytes=wire(payload(version),version);
      for(std::size_t prefix=0;prefix<bytes.size();++prefix)reject(bytes.substr(0,prefix));}}
    catch(...){TheWritableGlobalData=previous;throw;}TheWritableGlobalData=previous;}
  reject(wire(payload(),0));reject(wire(payload(),5));reject(wire(payload()+"x"));
  for(unsigned value:{0xffffffffu,0x7fffffffu}){auto data=payload();overwrite(data,0,value);reject(wire(data));}
  const std::size_t id=4+2+14+2+14,pointCount=id+4+1+1+4;
  {auto data=payload();overwrite(data,id,0x7fffffffu);reject(wire(data));}
  for(unsigned value:{0xffffffffu,0x7fffffffu}){auto data=payload();overwrite(data,pointCount,value);reject(wire(data));}
  // Physically short but internally consistent huge envelope, without huge fixture allocation.
  {auto data=payload(4,0,1);overwrite(data,pointCount,10000000);auto forged=wire(data);
    overwrite(forged,toc().size()+6,pointCount+4+120000000);reject(forged);}
  {auto data=payload(4,0,1);overwrite(data,id,0x7ffffffeu);load(wire(data));
    bool failed=false;try{Owner next(newInstance(PolygonTrigger)(2));}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}
    require(failed,"exhausted cursor rejects before acquisition/overflow");load(valid);}
  auto* prior=TheWritableGlobalData;TheWritableGlobalData=nullptr;bool failed=false;
  try{load(wire(payload(1),1));}catch(ErrorCode){failed=true;}catch(...){TheWritableGlobalData=prior;throw;}
  TheWritableGlobalData=prior;require(failed,"v1 requires actual source water configuration");check();
  {GlobalData data;TheWritableGlobalData=&data;
    try{for(Real value:{std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN(),
        std::numeric_limits<Real>::max(),-std::numeric_limits<Real>::max()}){
      data.m_waterExtentX=value;reject(wire(payload(1),1));
      data.m_waterExtentX=0;data.m_waterExtentY=value;reject(wire(payload(1),1));data.m_waterExtentY=0;}}
    catch(...){TheWritableGlobalData=prior;throw;}TheWritableGlobalData=prior;}
}
void edit(){Context context;Owner owner(newInstance(PolygonTrigger)(2));
  require(!owner->getPoint(0) && !owner->isValid(),"empty point lookup has no invalid pointer");
  owner->addPoint({0,0,5});owner->addPoint({100,0,6});const auto* accepted=owner->getPoint(0);
  const auto live=AllocationFault::live();AllocationFault::arm(0);bool failed=false;
  try{owner->insertPoint({100,100,7},1);}catch(const std::bad_alloc&){failed=true;}AllocationFault::disarm();
  require(failed && owner->getNumPoints()==2 && owner->getPoint(0)==accepted && AllocationFault::live()==live,
    "growth failure preserves capacity/backing/point count");
  owner->insertPoint({100,100,7},1);owner->insertPoint({0,100,8},3);
  require(owner->getNumPoints()==4 && owner->getPoint(1)->z==7 && owner->getPoint(2)->z==6,"growth retry and insertion order");
  owner->insertPoint({1,1,1},5);require(owner->getNumPoints()==4,"cannot skip insertion indices");
  owner->setPoint({9,8,7},1);owner->deletePoint(2);require(owner->getNumPoints()==3 && owner->getPoint(1)->x==9 &&
    owner->getPoint(2)->z==8 && owner->getPoint(-1)==owner->getPoint(0) && owner->getPoint(99)==owner->getPoint(2),"source edit/clamped query contract");
  owner->setPoint({0,0,0},3);require(owner->getNumPoints()==4,"setting first unused point appends");
  Owner wide(newInstance(PolygonTrigger)(4));const auto lo=std::numeric_limits<Int>::min(),hi=std::numeric_limits<Int>::max();
  for(const auto& point:{ICoord3D{lo,lo,0},ICoord3D{hi,lo,0},ICoord3D{hi,hi,0},ICoord3D{lo,hi,0}})wide->addPoint(point);
  ICoord3D middle{0,0,0};require(wide->pointInTrigger(middle) && std::isfinite(wide->getRadius()),"hostile full-width coordinates avoid signed geometry overflow");
}
std::string snapshotBytes(unsigned count=4){std::string out;put(out,1,1);put(out,count);
  for(unsigned n=0;n<count;++n){put(out,20+n);put(out,30+n);put(out,40+n);}
  put(out,20);put(out,30);put(out,23);put(out,33);put(out,std::bit_cast<UnsignedInt>(3.5f));put(out,0,1);return out;}
void snapshots(){Context context;load(wire(payload()));auto* owner=PolygonTrigger::getFirstPolygonTrigger();const auto valid=snapshotBytes();
  auto reject=[&](const std::string& bytes){const auto before=*owner->getPoint(0);const auto* backing=owner->getPoint(0);
    bool failed=false;try{MemoryLoad input(bytes);input.xferSnapshot(owner);}catch(XferStatus){failed=true;}catch(ErrorCode){failed=true;}
    require(failed && owner->getNumPoints()==4 && owner->getPoint(0)==backing && owner->getPoint(0)->x==before.x,
      "snapshot failure preserves accepted point ownership/value/count");};
  for(std::size_t prefix=0;prefix<valid.size();++prefix)reject(valid.substr(0,prefix));
  for(unsigned value:{0xffffffffu,0x7fffffffu}){auto malformed=valid;overwrite(malformed,1,value);reject(malformed);}
  {MemoryLoad input(valid);const auto* backing=owner->getPoint(0);const auto live=AllocationFault::live();
    AllocationFault::arm(0);bool failed=false;try{input.xferSnapshot(owner);}catch(const std::bad_alloc&){failed=true;}
    AllocationFault::disarm();require(failed && owner->getPoint(0)==backing && owner->getPoint(0)->x==0 &&
      AllocationFault::live()==live,"snapshot allocation guard preserves backing");}
  MemoryLoad input(valid);input.xferSnapshot(owner);require(input.consumed() && owner->getPoint(0)->x==20 &&
    owner->getPoint(3)->z==43 && owner->getRadius()==3.5f,"actual snapshot dispatcher/scalar order and corrected retry");
  for(unsigned count:{9u,3u,0u,4u}){MemoryLoad replacement(snapshotBytes(count));replacement.xferSnapshot(owner);
    require(replacement.consumed() && owner->getNumPoints()==static_cast<Int>(count) &&
      (!count ? !owner->getPoint(0) : owner->getPoint(count-1)->z==40+static_cast<Int>(count)-1),
      "serialized point count grows/shrinks/empties independently of map backing capacity");}
  // Include the final larger backing acquisition and prove every retry/terminal.
  const auto changed=snapshotBytes(9);std::size_t census;
  load(wire(payload()));owner=PolygonTrigger::getFirstPolygonTrigger();
  {MemoryLoad discovery(changed);AllocationFault::arm(SIZE_MAX);
    try{discovery.xferSnapshot(owner);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0,"snapshot growth acquisition census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){load(wire(payload()));owner=PolygonTrigger::getFirstPolygonTrigger();
    MemoryLoad replacement(changed);const auto live=AllocationFault::live();const auto* backing=owner->getPoint(0);bool failed=false;
    AllocationFault::arm(ordinal);try{replacement.xferSnapshot(owner);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
      AllocationFault::attempts()==(failed ? ordinal+1 : census),"every snapshot prefix and exact terminal");
    if(failed){require(owner->getNumPoints()==4 && owner->getPoint(0)==backing && owner->getPoint(0)->x==0 &&
      AllocationFault::live()==live,"failed snapshot preserves accepted owner and retires partial records");
      MemoryLoad retry(changed);retry.xferSnapshot(owner);}
    require(owner->getNumPoints()==9 && owner->getPoint(8)->z==48 && AllocationFault::live()==live,
      "each corrected snapshot retry retires old backing and publishes all records");
  }
  std::cout<<"trigger snapshot prefixes [0,"<<census<<"); terminal "<<census<<'\n';
}
void faults(){Context context;
  GlobalData data;data.m_waterExtentX=640;data.m_waterExtentY=480;
  for(unsigned version=1;version<=4;++version){
    auto* previous=TheWritableGlobalData;TheWritableGlobalData=&data;
    try{for(Int count:{4,96}){const auto valid=wire(payload(version,0,3,count),version);
      const auto candidate=wire(payload(version,200,3,count),version);load(valid);std::size_t census;
      AllocationFault::arm(SIZE_MAX);try{load(candidate);}catch(...){AllocationFault::disarm();throw;}
      census=AllocationFault::attempts();AllocationFault::disarm();load(valid);
      require(census>0 && census<1000,"bounded complete trigger allocation census");
      for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto live=AllocationFault::live();const auto used=pool()->getUsedBlockCount();
        auto* accepted=PolygonTrigger::getFirstPolygonTrigger();const auto* points=accepted->getPoint(0);bool failed=false;
        AllocationFault::arm(ordinal);try{load(candidate);}catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
        require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
          AllocationFault::attempts()==(failed ? ordinal+1 : census),"all trigger acquisition prefixes and exact terminal");
        require(AllocationFault::live()==live && pool()->getUsedBlockCount()==used,"candidate/previous owner retires exactly");
        if(failed){require(PolygonTrigger::getFirstPolygonTrigger()==accepted && accepted->getPoint(0)==points &&
          accepted->getPoint(0)->x==0,"failure preserves accepted trigger identity and backing");
          {Owner probe(newInstance(PolygonTrigger)(2));require(probe->getID()==(version==1 ? 11 : 10),"each failed candidate restores ID allocation");}
          load(candidate);}
        require(PolygonTrigger::getFirstPolygonTrigger()->getPoint(0)->x==200 && AllocationFault::live()==live,
          "every distinct corrected candidate retry publishes/retire");load(valid);
      }
      std::cout<<"trigger version "<<version<<" points "<<count<<" prefixes [0,"<<census<<"); terminal "<<census<<'\n';
    }}catch(...){TheWritableGlobalData=previous;throw;}TheWritableGlobalData=previous;
  }
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"trigger test family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool("PolygonTrigger",sizeof(PolygonTrigger),32,0);
    {Context warm;GlobalData data;load(wire(payload()));}
    {const std::string family=argv[1];for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      if(family=="functional")functional();else if(family=="negative")negative();else if(family=="edit")edit();
      else if(family=="snapshots")snapshots();else if(family=="faults")faults();else require(false,"unknown family");
      require(!PolygonTrigger::getFirstPolygonTrigger() && pool()->getUsedBlockCount()==0 && AllocationFault::live()==live,
        "whole trigger world retires exactly across repeated lifetimes");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole pool metadata retires");
    std::cout<<"PASS original trigger owner; complete world/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
  catch(ErrorCode error){std::cerr<<"FAIL source trigger admission "<<static_cast<int>(error)<<'\n';}
  catch(...){std::cerr<<"FAIL source trigger admission\n";}
  AllocationFault::disarm();PolygonTrigger::deleteTriggers();if(initialized)shutdownMemoryManager();return 1;
}
