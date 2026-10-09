// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/MapObject.h"
#include "Common/WellKnownKeys.h"
#include "GameLogic/NativeWaypointGraph.h"
#include "GameLogic/NativeTerrainHeightMap.h"
#include "GameLogic/TerrainLogic.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
namespace {
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Input:ChunkInputStream {const std::string& bytes;UnsignedInt position=0;
  explicit Input(const std::string& value):bytes(value){}
  Int read(void* target,Int count)override{if(count<0 || (!target && count))throw ERROR_BAD_ARG;
    auto n=std::min<std::size_t>(count,bytes.size()-position);if(n)std::memcpy(target,bytes.data()+position,n);position+=n;return n;}
  UnsignedInt tell()override{return position;}Bool absoluteSeek(UnsignedInt value)override{if(value>bytes.size())return FALSE;position=value;return TRUE;}
  Bool eof()override{return position==bytes.size();}};
void word(std::string& bytes,UnsignedInt value,unsigned width=4){for(unsigned n=0;n<width;++n)bytes+=char(value>>(8*n));}
std::string toc(){std::string result="CkMp";word(result,2);
  for(const auto [name,id]:{std::pair{"WaypointsList",1u},std::pair{"IgnoredVisualChunk",2u}}){word(result,std::strlen(name),1);result+=name;word(result,id);}return result;}
std::string chunk(const std::string& payload,UnsignedInt id=1){std::string result;word(result,id);word(result,1,2);word(result,payload.size());return result+payload;}
std::string links(){std::string data;std::vector<std::pair<UnsignedInt,UnsignedInt>> edges{{1,2},{1,2},{2,0x80000000u},{0x80000000u,1},{1,1},{999,2}};
  for(unsigned n=4;n<=12;++n)edges.emplace_back(1,n);
  word(data,edges.size());for(auto [from,to]:edges){word(data,from);word(data,to);}return data;}
std::string wire(){return toc()+chunk(links());}
struct Objects {MapObject* head=nullptr;MapObject* tail=nullptr;
  ~Objects(){while(head){auto* node=head;head=head->getNext();node->setNextMap(nullptr);node->deleteInstance();}}
  void add(UnsignedInt id,const char* name,bool bi=false,bool waypoint=true,Real x=10){Dict dict;
    dict.setInt(TheKey_waypointID,std::bit_cast<Int>(id));dict.setAsciiString(TheKey_waypointName,name);
    dict.setAsciiString(TheKey_waypointPathLabel1,"Generated primary path label which is deliberately longer than short string storage");
    dict.setAsciiString(TheKey_waypointPathLabel2,"Generated secondary path");dict.setAsciiString(TheKey_waypointPathLabel3,"Generated tertiary path");
    dict.setBool(TheKey_waypointPathBiDirectional,bi);
    auto* node=newInstance(MapObject)(Coord3D{x,20,99},AsciiString("GeneratedWaypoint"),0,0,&dict,nullptr);
    if(waypoint)node->setIsWaypoint();if(tail)tail->setNextMap(node);else head=node;tail=node;
  }
  void populate(){add(1,"First duplicate",true);add(2,"Second");add(1,"Later duplicate");add(0x80000000u,"High bit");
    add(77,"Not a waypoint",false,false);for(unsigned n=4;n<=12;++n)add(n,"Ordinary waypoint");}
};
Waypoint* find(NativeWaypointGraph& graph,UnsignedInt id){Waypoint* selected=nullptr;
  for(auto* node=graph.first();node;node=node->getNext())if(node->getID()==id)selected=node;return selected;}
bool linked(Waypoint* from,Waypoint* to){for(Int n=0;n<from->getNumLinks();++n)if(from->getLink(n)==to)return true;return false;}
void load(NativeWaypointGraph& graph,Objects& objects,const std::string& bytes,const NativeTerrainHeightMap* height=nullptr){Input input(bytes);graph.load(input,objects.head,height);}
NativeTerrainHeightMap terrain(UnsignedByte height){std::string payload;word(payload,7);word(payload,7);word(payload,1);word(payload,1);word(payload,5);word(payload,5);word(payload,49);payload.append(49,char(height));
  std::string data="CkMp";word(data,1);word(data,13,1);data+="HeightMapData";word(data,1);
  std::string envelope;word(envelope,1);word(envelope,4,2);word(envelope,payload.size());data+=envelope+payload;
  NativeTerrainHeightMap result;Input input(data);result.loadHeightData(input,NativeTerrainHeightMap::Purpose::HeightChunkBacking);return result;}
void check(NativeWaypointGraph& graph){require(graph.first()->getID()==12,"source prepend reverses file object order");unsigned count=0;
  for(auto* node=graph.first();node;node=node->getNext()){++count;require(node->getLocation()->z==0 && node->getLink(-1)==nullptr && node->getLink(8)==nullptr,"absent initial grid and safe exact link boundary");}
  require(count==13 && find(graph,77)==nullptr,"only source-classified waypoint objects become original pooled nodes");
  auto* first=find(graph,1);auto* second=find(graph,2);auto* high=find(graph,0x80000000u);
  require(first->getName()=="First duplicate" && first->getPathLabel2()=="Generated secondary path" && first->getPathLabel3()=="Generated tertiary path","first-file duplicate and owned source string labels");
  require(first->getNumLinks()==8 && linked(first,second) && linked(second,first) && linked(second,high) && !linked(high,second),"duplicates suppressed, source-node-only reverse and unsigned high ID");
  require(!linked(first,find(graph,12)) && linked(find(graph,12),first),"reverse edge still admitted when forward graph is full");
  unsigned duplicateCount=0;for(auto* node=graph.first();node;node=node->getNext())if(node->getID()==1 && node!=first){++duplicateCount;require(node->getNumLinks()==0,"later-file duplicate never selected for source links");}
  require(duplicateCount==1,"both duplicate source nodes retained");
}
void functional(){Objects objects;objects.populate();NativeWaypointGraph graph;load(graph,objects,wire());check(graph);
  auto old=terrain(80),next=terrain(32);load(graph,objects,wire(),&old);
  for(auto* node=graph.first();node;node=node->getNext())require(node->getLocation()->z==50,"initial admission uses prior retained grid");
  graph.resnap(next);for(auto* node=graph.first();node;node=node->getNext())require(node->getLocation()->z==20,"final resnap uses newly loaded terrain, not prior metadata");
  Waypoint* external=nullptr;const auto* head=graph.first();graph.exchange(external);require(!graph.first() && external==head,"nonallocating actual TerrainLogic head publication");
  graph.exchange(external);require(graph.first()==head && !external,"borrowed head publication restores exactly");
  load(graph,objects,toc()+chunk("ignored",2));for(auto* node=graph.first();node;node=node->getNext())require(node->getNumLinks()==0,"omitted waypoint links retain source nodes with no invented edges");
}
void negative(){Objects objects;objects.populate();NativeWaypointGraph graph;const auto valid=wire();load(graph,objects,valid);
  auto reject=[&](const std::string& bytes){const auto* head=graph.first();const auto live=AllocationFault::live();bool failed=false;
    try{load(graph,objects,bytes);}catch(ErrorCode error){failed=error==ERROR_CORRUPT_FILE_FORMAT;}
    require(failed && graph.first()==head && AllocationFault::live()==live,"late link rejection preserves actual graph identity and retires candidates");check(graph);load(graph,objects,valid);check(graph);};
  for(std::size_t n=0;n<valid.size();++n)reject(valid.substr(0,n));
  std::string invalid;word(invalid,0xffffffffu);reject(toc()+chunk(invalid));invalid.clear();word(invalid,0x7fffffffu);reject(toc()+chunk(invalid));
  reject(toc()+chunk(links()+"x"));reject(valid+chunk(links()));
  // Source requires a payload after the TOC. A different complete chunk permits
  // omission, whereas a bare TOC isn't an opened source file.
  const auto omitted=toc()+chunk("ignored",2);
  load(graph,objects,omitted);for(auto* node=graph.first();node;node=node->getNext())require(!node->getNumLinks(),"complete optional omission is valid");
  Objects huge;huge.add(2,"hostile",false,true,std::numeric_limits<Real>::max());huge.add(1,"ordinary");
  load(graph,huge,omitted);const auto* head=graph.first();auto grid=terrain(80);bool failed=false;
  try{graph.resnap(grid);}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}
  require(failed && graph.first()==head && head->getLocation()->z==0 && head->getNext()->getLocation()->z==0,"resnap arithmetic rejects before any accepted graph mutation");
}
void faults(){Objects objects;objects.populate();NativeWaypointGraph graph;const auto valid=wire(),replacement=toc()+chunk("ignored",2);
  load(graph,objects,valid);load(graph,objects,replacement);load(graph,objects,valid);
  AllocationFault::arm(SIZE_MAX);try{load(graph,objects,replacement);}catch(...){AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();load(graph,objects,valid);
  auto* pool=TheMemoryPoolFactory->findMemoryPool("Waypoint");require(pool,"actual Waypoint pool exists");
  for(std::size_t n=0;n<=census;++n){const auto* head=graph.first();const auto live=AllocationFault::live();const auto used=pool->getUsedBlockCount();bool failed=false;
    AllocationFault::arm(n);try{load(graph,objects,replacement);}catch(const std::bad_alloc&){failed=true;}catch(ErrorCode error){require(error==ERROR_OUT_OF_MEMORY,"source pooled constructor failure class");failed=true;}
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();require(hit==failed && failed==(n<census),"every graph allocation failure and exact terminal");
    if(failed){require(graph.first()==head && AllocationFault::live()==live && pool->getUsedBlockCount()==used,"partial node/string/parser resources retire exactly");check(graph);}
    load(graph,objects,replacement);load(graph,objects,valid);check(graph);
    require(AllocationFault::live()==live && pool->getUsedBlockCount()==used,"every corrected retry restores heap and pooled ownership");
  }std::cout<<"waypoint prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}
void poolFailure(){Objects objects;objects.populate();NativeWaypointGraph graph;load(graph,objects,wire());
  auto* pool=TheMemoryPoolFactory->findMemoryPool("Waypoint");std::vector<void*> held;held.reserve(pool->getFreeBlockCount());
  struct Retire{MemoryPool* pool;std::vector<void*>& held;~Retire(){for(auto* block:held)pool->freeBlock(block);}} retire{pool,held};
  while(pool->getFreeBlockCount())held.push_back(pool->allocateBlock("generated waypoint pool exhaustion"));
  const auto* head=graph.first();const auto used=pool->getUsedBlockCount();const auto live=AllocationFault::live();bool failed=false;
  try{load(graph,objects,wire());}catch(ErrorCode error){failed=error==ERROR_OUT_OF_MEMORY;}
  require(failed && graph.first()==head && pool->getUsedBlockCount()==used && AllocationFault::live()==live,"actual pool exhaustion preserves accepted graph");check(graph);
  for(auto* block:held)pool->freeBlock(block);held.clear();load(graph,objects,wire());check(graph);
  std::cout<<"waypoint pool failure and corrected retry\n";
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"waypoint family");initMemoryManager();initialized=true;
    if(std::string_view(argv[1])=="pool-failures")TheMemoryPoolFactory->createMemoryPool("Waypoint",sizeof(Waypoint),64,0);
    {NameKeyGenerator names;names.init();auto* previous=TheNameKeyGenerator;TheNameKeyGenerator=&names;
    struct Restore{NameKeyGenerator* previous;~Restore(){TheNameKeyGenerator=previous;}} restore{previous};
    {Objects objects;objects.populate();NativeWaypointGraph warm;load(warm,objects,wire());load(warm,objects,wire());}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto used=TheMemoryPoolFactory->findMemoryPool("Waypoint")->getUsedBlockCount();
      const std::string_view family=argv[1];if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();else if(family=="pool-failures")poolFailure();else require(false,"unknown family");
      require(AllocationFault::live()==live && TheMemoryPoolFactory->findMemoryPool("Waypoint")->getUsedBlockCount()==used,"three whole original graph owners retire exactly");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole original manager retirement");std::cout<<"PASS actual waypoint graph; terrain/root integration pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL waypoint "<<Int(error)<<'\n';}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;}
