// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameLogic/NativeTerrainHeightMap.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
using Map = NativeTerrainHeightMap;
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
struct MemoryInput : ChunkInputStream {
  const std::string& bytes;
  UnsignedInt position = 0;
  explicit MemoryInput(const std::string& input) : bytes(input) {}
  Int read(void* output, Int count) override {
    if (count < 0 || (!output && count)) throw ERROR_BAD_ARG;
    const auto size = std::min<std::size_t>(count, bytes.size() - position);
    if (size) std::memcpy(output, bytes.data() + position, size);
    position += static_cast<UnsignedInt>(size);
    return static_cast<Int>(size);
  }
  UnsignedInt tell() override { return position; }
  Bool absoluteSeek(UnsignedInt value) override {
    if (value > bytes.size()) return FALSE;
    position = value; return TRUE;
  }
  Bool eof() override { return position == bytes.size(); }
};
void put(std::string& data, UnsignedInt value, unsigned width = 4) {
  for (unsigned n = 0; n < width; ++n) data.push_back(char(value >> (8*n)));
}
void overwrite(std::string& data, std::size_t at, UnsignedInt value) {
  for (unsigned n = 0; n < 4; ++n) data.at(at+n) = char(value >> (8*n));
}
std::string payload(unsigned version = 4, Int width = 7, Int height = 7,
                    Int border = 1, int constant = -1) {
  std::string data;
  put(data, width); put(data, height);
  if (version >= 3) put(data, border);
  if (version >= 4) { put(data, 2); put(data, width-2*border); put(data, height-2*border);
    put(data, 0); put(data, 0); } // Authored dormant boundary, not an error.
  put(data, width*height);
  for (Int y = 0; y < height; ++y)
    for (Int x = 0; x < width; ++x)
      data.push_back(char(constant >= 0 ? constant : x*x+3*y*y+x*y));
  return data;
}
std::string toc() {
  std::string data = "CkMp"; put(data, 1); put(data, 13, 1);
  data += "HeightMapData"; put(data, 1); return data;
}
std::string chunk(const std::string& data, unsigned version = 4, UnsignedInt id = 1) {
  std::string result; put(result, id); put(result, version, 2); put(result, data.size());
  return result + data;
}
std::string wire(const std::string& data, unsigned version = 4) { return toc() + chunk(data, version); }
void load(Map& owner, const std::string& data, Map::Purpose purpose = Map::Purpose::HeightChunkBacking) {
  MemoryInput input(data); owner.loadHeightData(input, purpose);
  require(input.eof(), "complete height-only stream consumption");
}
void near(Real value, Real expected, const char* message) {
  require(std::abs(value - expected) < 0.000002f, message);
}
void accepted(const Map& owner) {
  require(owner.width() == 7 && owner.height() == 7 && owner.border() == 1 &&
          owner.bytes().size() == 49 && owner.rawHeight(2,2) == 20 &&
          owner.boundaries().size() == 2 && owner.boundaries()[0].x == 5 &&
          owner.boundaries()[1].x == 0 && owner.boundaries()[1].y == 0,
          "accepted authored backing/boundaries retained");
}
void functional() {
  Map owner; load(owner, wire(payload())); accepted(owner);
  Coord3D normal;
  near(owner.groundHeight(12.5f,17.5f,&normal), 21.71875f, "source upper triangle, not bilinear");
  const Real length = static_cast<Real>(std::sqrt(500.0*500+1040.0*1040+1024.0*1024));
  near(normal.x, -500/length, "source unusual smoothed X stencil");
  near(normal.y, -1040/length, "source smoothed Y stencil");
  near(normal.z, 1024/length, "source portable cross normalization");
  near(owner.groundHeight(17.5f,12.5f,&normal), 18.59375f, "source lower triangle");
  const Real lowerLength = static_cast<Real>(std::sqrt(500.0*500+1200.0*1200+1024.0*1024));
  near(normal.y, -1200/lowerLength, "parallel lower smoothed normal");
  near(owner.groundHeight(15,15), 20.3125f, "source diagonal tie");
  near(owner.groundHeight(-15,20,&normal), 16.875f, "negative coordinates floor then clip");
  require(normal.x == 0 && normal.y == 0 && normal.z == 1, "source edge normal up");
  Map display; load(display, wire(payload(4,7,7,1,200)));
  near(owner.groundHeight(-15,20,nullptr,&display), 125, "edge uses separate display grid");
  near(owner.groundHeight(12.5f,17.5f,nullptr,&display), 21.71875f, "interior remains logical, not display");
  display.groundHeight(12.5f,17.5f,&normal);
  require(normal.x==0 && normal.y==0 && !std::signbit(normal.x) && !std::signbit(normal.y) && normal.z==1,
          "flat source cross product preserves positive zero components");
  near(owner.groundHeight(1e8f,1e8f), owner.rawHeight(6,6)*0.625f, "finite offmap clips independently");
  require(owner.clearLineOfSight({10,10,27.625f},{20,10,27.625f},112.5f), "LOS exact 0.5 tolerance");
  require(!owner.clearLineOfSight({10,10,27.624f},{20,10,27.624f},112.5f), "LOS below tolerance blocked");
  require(owner.clearLineOfSight({10,10,30},{20,10,30},112.5f) &&
          !owner.clearLineOfSight({20,10,30},{10,10,30},112.5f), "source endpoint-exclusive directional LOS");
  require(owner.clearLineOfSight({10,10,-10},{19,19,-10},112.5f), "same-cell source true without unused division by zero");
  require(owner.clearLineOfSight({-100,-100,-10},{20,20,-10},112.5f), "source starts outside map terminates true");
  require(!owner.clearLineOfSight({10,10,50},{40,20,50},112.5f) &&
          owner.clearLineOfSight({10,10,58},{40,20,58},112.5f), "X-dominant source cells have maximum 58.125");
  require(!owner.clearLineOfSight({10,10,60},{20,40,60},112.5f) &&
          owner.clearLineOfSight({10,10,69},{20,40,69},112.5f), "Y-dominant source cells have maximum 69.375");
  require(!owner.clearLineOfSight({10,10,77},{40,40,77},112.5f) &&
          owner.clearLineOfSight({10,10,78},{40,40,78},112.5f), "diagonal source cells have maximum 78.125");
  require(!owner.clearLineOfSight({40,20,66},{10,10,66},112.5f) &&
          owner.clearLineOfSight({40,20,67},{10,10,67},112.5f), "descending source cells have maximum 67.5");
  for (const auto& target : {Coord3D{40,20,150},Coord3D{20,40,150},Coord3D{40,40,150},
                            Coord3D{0,20,150},Coord3D{20,0,150},Coord3D{0,0,150}})
    require(owner.clearLineOfSight({10,10,140},target,112.5f), "all source Bresenham orientations clear above terrain");
  Map empty; near(empty.groundHeight(0,0,&normal),0,"unloaded grid height");
  require(!empty.clearLineOfSight({0,0,0},{10,10,0},0), "unloaded LOS false, not fake success");
  for (unsigned version = 1; version <= 4; ++version) {
    const auto data = wire(payload(version),version);
    Map runtime, metadata;
    load(runtime,data); load(metadata,data,Map::Purpose::LogicalMetadata);
    require(runtime.width()==7 && metadata.width()==(version==1 ? 4 : 7), "source distinct version1 reader dimensions");
    require(runtime.border()==(version>=3 ? 1 : 0) && runtime.bytes().size()==49,
            "source version border and retained byte tail");
    if(version==1) {
      require(runtime.bytes()[5]==20 && metadata.rawHeight(1,1)==20 && runtime.rawHeight(5,0)==20 &&
              metadata.boundaries()[0].x==7,"source version1 prefix resampling retains original boundary contract");
    } else require(runtime.rawHeight(2,2)==20,"modern raw height unchanged");
  }
  Map single; load(single,wire(payload(2,1,1,0,9),2));
  near(single.groundHeight(-10,10,&normal),5.625f,"one-sample map safe clipping");
}
void negative() {
  Map owner; const auto valid = wire(payload()); load(owner, valid);
  auto reject = [&](const std::string& data) {
    const auto live = AllocationFault::live(); bool failed = false;
    try { load(owner,data); } catch(ErrorCode error) { require(error==ERROR_CORRUPT_FILE_FORMAT,"malformed height error"); failed=true; }
    require(failed && AllocationFault::live()==live,"malformed candidate retires exactly");
    accepted(owner); load(owner, valid); accepted(owner);
  };
  for(std::size_t length=0;length<valid.size();++length) reject(valid.substr(0,length));
  reject(toc()); reject(toc()+chunk(payload())+chunk(payload()));
  reject(wire(payload(),0)); reject(wire(payload(),5));
  for (const auto [offset,value] : {std::pair<std::size_t,UnsignedInt>{0,0}, {0,0xffffffffu},
       {0,0x7fffffffu}, {4,0}, {4,0xffffffffu}, {8,0xffffffffu}, {8,4},
       {12,0xffffffffu}, {12,0x7fffffffu}, {32,48}, {32,0}, {32,0xffffffffu}}) {
    auto data=payload(); overwrite(data,offset,value); reject(wire(data));
  }
  reject(wire(payload()+"x"));
  // Physically short inputs with internally consistent enormous declared sizes.
  // No corresponding large fixture buffer is constructed.
  {auto data=payload();overwrite(data,0,32768);overwrite(data,4,32768);
    overwrite(data,32,1073741824u);auto forged=wire(data);
    overwrite(forged,toc().size()+6,36u+1073741824u);reject(forged);}
  {auto data=payload();overwrite(data,12,1048576u);auto forged=wire(data);
    overwrite(forged,toc().size()+6,16u+8u*1048576u+4u+49u);reject(forged);}
  for (const Real value : {std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),
                          -std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::max(),
                          -std::numeric_limits<Real>::max()}) {
    bool ground=false, sight=false;
    try{owner.groundHeight(value,0);}catch(ErrorCode error){ground=error==ERROR_BAD_ARG;}
    try{owner.clearLineOfSight({0,0,1},{value,0,1},160);}catch(ErrorCode error){sight=error==ERROR_BAD_ARG;}
    require(ground && sight,"hostile coordinate conversion rejects without UB");
  }
  bool raw=false;try{owner.rawHeight(-1,1);}catch(ErrorCode error){raw=error==ERROR_BAD_ARG;}
  require(raw,"negative raw XY cannot alias valid linear index");
  raw=false;try{owner.rawHeight(7,0);}catch(ErrorCode error){raw=error==ERROR_BAD_ARG;}
  require(raw,"oversized raw XY cannot alias next row");
}
void faults() {
  for(const auto& valid : {wire(payload()),wire(payload(4,96,96))}) {
  Map owner;load(owner,valid);
  const auto width=owner.width(),height=owner.height(),border=owner.border();
  const auto checksum=owner.rawHeight(width-1,height-1);
  const auto candidate=wire(payload(4,width,height,border,200));
  auto check=[&]{require(owner.width()==width && owner.height()==height && owner.border()==border &&
    owner.bytes().size()==std::size_t(width)*height && owner.rawHeight(width-1,height-1)==checksum,
    "accepted small/multi-block source backing retained");};
  std::size_t census;
  {AllocationFault::arm(SIZE_MAX);try{load(owner,candidate);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  load(owner,valid);
  require(census>0 && census<128,"complete bounded native height acquisition census");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    const auto live=AllocationFault::live();bool failed=false;
    const auto* acceptedBytes=owner.bytes().data();
    const auto* acceptedBoundaries=owner.boundaries().data();
    AllocationFault::arm(ordinal);
    try{load(owner,candidate);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}
    AllocationFault::disarm();
    require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
            (failed || AllocationFault::attempts()==census),"every height acquisition prefix and exact terminal");
    require(AllocationFault::live()==live,"partial/full height transaction retires exactly");
    if(failed){check();require(owner.bytes().data()==acceptedBytes && owner.boundaries().data()==acceptedBoundaries,
        "failed replacement preserves actual accepted backing identity");load(owner,candidate);}
    require(owner.rawHeight(width-1,height-1)==200,"corrected/terminal replacement publishes distinct candidate");
    require(AllocationFault::live()==live,"corrected same-owner height retry retires");
    load(owner,valid);check();require(AllocationFault::live()==live,"accepted original restores for next failure pair");
  }
  std::cout<<"height prefixes [0,"<<census<<"); terminal "<<census<<'\n';
  }
}
std::string topologyToc() {
  std::string result="CkMp";put(result,2);
  for(const auto [name,id] : {std::pair{"HeightMapData",1u}, {"BlendTileData",2u}}) {
    put(result,std::strlen(name),1);result+=name;put(result,id);
  }
  return result;
}
void ascii(std::string& data,const char* name) {put(data,std::strlen(name),2);data+=name;}
std::string blendPayload(unsigned version=8,Int width=10,Int height=6,bool marked=true) {
  const Int count=width*height;std::string result;put(result,count);
  for(Int index=0;index<count;++index)put(result,index%64,2);
  for(Int index=0;index<count;++index)put(result,index==0 ? 0xffffu : index==1 ? 32767u : 1u,2);
  if(version>=6)for(Int index=0;index<count;++index)put(result,index==0 ? 0xffffu : 1u,2);
  if(version>=5)for(Int index=0;index<count;++index)put(result,index==0 ? 0xffffu : 1u,2);
  if(version>=7) {
    const Int stride=version==7 ? (width+1)/8 : (width+7)/8;
    for(Int y=0;y<height;++y)for(Int x=0;x<stride;++x)
      put(result,marked && y==1 ? (x==0 ? 2u : x==1 ? 1u : 0u) : 0u,1);
  }
  put(result,16);put(result,2);if(version>=5)put(result,2);
  put(result,1);put(result,0);put(result,16);put(result,4);put(result,0);
  ascii(result,"GeneratedTerrain");
  if(version>=4) {put(result,1);put(result,1);put(result,0);put(result,1);put(result,1);ascii(result,"GeneratedEdge");}
  put(result,4);
  for(unsigned byte : {0xfeu,1u,2u,3u,0x83u})put(result,byte,1);
  if(version>=3)put(result,7,1);
  if(version>=4)put(result,0);
  put(result,0x7ada0000u);
  if(version>=5) {put(result,4);for(unsigned n=0;n<8;++n)put(result,std::bit_cast<UnsignedInt>(Real(n)*0.25f));
    put(result,1,1);put(result,1,1);}
  return result;
}
std::string topologyWire(unsigned version=8,Int width=10,Int height=6,bool marked=true) {
  const unsigned heightVersion=version==1 ? 1u : 4u;
  const Int border=version==1 || width<3 || height<3 ? 0 : 1;
  return topologyToc()+chunk(payload(heightVersion,width,height,border,0),heightVersion)+
    chunk(blendPayload(version,width,height,marked),version,2);
}
void loadTopology(Map& owner,const std::string& bytes,Bool threeWay=TRUE) {
  MemoryInput input(bytes);owner.loadTerrainTopology(input,threeWay);
  require(input.eof() && owner.topologyReady(),"complete topology input/candidate publication");
}
void topologyFunctional() {
  for(unsigned version=1;version<=8;++version) {
    Map owner;loadTopology(owner,topologyWire(version));const auto& data=owner.topology();
    require(data.version==version && data.tiles.size()==60 && data.blendEntries.size()==(version==1 ? 1u : 2u) &&
      data.textureClasses.size()==1 && data.textureClasses[0].name=="GeneratedTerrain" &&
      data.textureClasses[0].numTiles==16 && data.textureClasses[0].width==4,"all version texture/index metadata retained");
    require(data.blends[0]==0 && data.blends[1]==0 && data.extraBlends[0]==0 && data.cliffs[0]==0,
      "source bad-index repair and absent-version sentinel policy");
    if(version==1) {
      require(owner.width()==5 && owner.height()==3 && owner.border()==0 && owner.bytes().size()==60 &&
        data.tiles[6]==22 && data.blends[6]==0 && data.extraBlends[6]==0 && data.cliffs[6]==0 &&
        data.cliffStride==2,"coupled legacy blend completion retains original backing/stride and resamples tile prefix");
    } else {
      require(owner.width()==10 && owner.height()==6 && data.blends[2]==1,"modern dimensions and valid source indices");
      const auto& entry=data.blendEntries[1];
      require(entry.tileIndex==4 && entry.flags[0]==0xfe && entry.flags[4]==0x83 &&
        entry.flags[5]==(version>=3 ? 7 : 0) && entry.customEdgeClass==(version>=4 ? 0 : -1),
        "packed blend flags and versioned edge metadata preserved");
      require(data.edgeClasses.size()==(version>=4 ? 1u : 0u),"versioned edge table");
      require(data.extraBlends[2]==(version>=6 ? 1 : 0) && data.cliffs[2]==(version>=5 ? 1 : 0),
        "missing old arrays are no extra/cliff mapping, not uninitialized reads");
      if(version>=5)require(data.cliffEntries.size()==2 && data.cliffEntries[1].tileIndex==4 &&
        data.cliffEntries[1].uv[7]==1.75f && data.cliffEntries[1].flags[1]==1,"complete cliff UV/flag records");
      require(owner.isCliffCell(0,0)==(version>=7),"authored cliffs override flat height-derived slope");
      require(owner.isCliffCell(70,0)==(version>=8),"version7 undersized row versus version8 full row");
      require(owner.isCliffCell(-1,0)==(version>=7) && !owner.isCliffCell(-10,0),
        "source cliff query truncates negative world coordinates instead of floor");
    }
  }
  Map filtered;loadTopology(filtered,topologyWire(),FALSE);
  require(filtered.topology().extraBlends[2]==0 && filtered.topology().blendEntries[1].flags[4]==0x81,
    "source disabled three-way filter clears only forced flip bit and extra indices");
  for(const unsigned spike : {15u,16u}) {
    auto heights=payload(4,10,6,1,0);heights[36+22]=char(spike);
    const auto input=topologyToc()+chunk(heights)+chunk(blendPayload(2),2,2);
    Map owner;loadTopology(owner,input);
    require(owner.isCliffCell(10,10)==(spike==16),"source old slope limit strictly greater than9.8");
  }
  Map heightOnly;load(heightOnly,wire(payload()));bool unavailable=false;
  try{heightOnly.isCliffCell(0,0);}catch(ErrorCode error){unavailable=error==ERROR_BAD_ARG;}
  require(unavailable && !heightOnly.topologyReady(),"height-only data is not accepted cliff/topology backing");
  Map degenerate;loadTopology(degenerate,topologyWire(8,1,1));
  require(!degenerate.isCliffCell(0,0),"one-point source grid has no cells");
}
void topologyNegative() {
  const auto valid=topologyWire();Map owner;loadTopology(owner,valid);
  auto check=[&]{require(owner.width()==10 && owner.topology().version==8 && owner.isCliffCell(0,0) &&
    owner.topology().textureClasses[0].name=="GeneratedTerrain","accepted topology remains after rejection");};
  auto reject=[&](const std::string& input) {
    const auto live=AllocationFault::live();const auto* acceptedBytes=owner.bytes().data();
    const auto* acceptedTiles=owner.topology().tiles.data();bool failed=false;
    try{loadTopology(owner,input);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"topology malformed error");failed=true;}
    require(failed && AllocationFault::live()==live && owner.bytes().data()==acceptedBytes &&
      owner.topology().tiles.data()==acceptedTiles,"late/topology rejection preserves actual accepted owners");
    check();loadTopology(owner,valid);check();
  };
  for(std::size_t prefix=0;prefix<valid.size();++prefix)reject(valid.substr(0,prefix));
  const auto heightChunk=chunk(payload(4,10,6,1,0));const auto blendChunk=chunk(blendPayload(),8,2);
  reject(topologyToc()+heightChunk);reject(topologyToc()+blendChunk);
  reject(topologyToc()+blendChunk+heightChunk);
  reject(topologyToc()+heightChunk+heightChunk+blendChunk);
  reject(topologyToc()+heightChunk+blendChunk+blendChunk);
  reject(topologyToc()+heightChunk+chunk(blendPayload(),0,2));
  reject(topologyToc()+heightChunk+chunk(blendPayload(),9,2));
  for(const auto [offset,value] : {std::pair<std::size_t,UnsignedInt>{0,59u},{0,0xffffffffu},
      {496,0u},{500,0u},{500,0x7fffffffu},{504,0u},{504,0x7fffffffu},{508,0xffffffffu}}) {
    auto data=blendPayload();overwrite(data,offset,value);reject(topologyToc()+heightChunk+chunk(data,8,2));
  }
  {auto data=blendPayload();const auto marker=data.size()-38-4;overwrite(data,marker,0);
    reject(topologyToc()+heightChunk+chunk(data,8,2));}
  {auto data=blendPayload();overwrite(data,data.size()-34,0x7fc00000u);
    reject(topologyToc()+heightChunk+chunk(data,8,2));}
  reject(topologyToc()+heightChunk+chunk(blendPayload()+"x",8,2));
  for(const Real value : {std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),
                         -std::numeric_limits<Real>::max(),std::numeric_limits<Real>::max()}) {
    bool failed=false;try{owner.isCliffCell(value,0);}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}
    require(failed,"cliff conversion rejects hostile coordinates without UB");
  }
}
void topologyFaults() {
  for(const Int size : {10,96}) {
    const auto valid=topologyWire(8,size,size);
    const auto corrected=topologyWire(8,size,size,false);
    Map owner;loadTopology(owner,valid);std::size_t census;
    {AllocationFault::arm(SIZE_MAX);try{loadTopology(owner,corrected);}catch(...){AllocationFault::disarm();throw;}
      census=AllocationFault::attempts();AllocationFault::disarm();}loadTopology(owner,valid);
    require(census>0 && census<512,"complete bounded topology ownership census");
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
      const auto live=AllocationFault::live();const auto* acceptedBytes=owner.bytes().data();
      const auto* acceptedTiles=owner.topology().tiles.data();const auto* acceptedFlags=owner.topology().cliffFlags.data();
      bool failed=false;AllocationFault::arm(ordinal);
      try{loadTopology(owner,corrected);}catch(const std::bad_alloc&){failed=true;}
      catch(...){AllocationFault::disarm();throw;}AllocationFault::disarm();
      require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
        (failed || AllocationFault::attempts()==census),"every topology failure ordinal and exact terminal");
      require(AllocationFault::live()==live,"all partial topology backing retires exactly");
      if(failed) {require(owner.bytes().data()==acceptedBytes && owner.topology().tiles.data()==acceptedTiles &&
          owner.topology().cliffFlags.data()==acceptedFlags && owner.isCliffCell(0,0),"accepted whole topology identity/semantics retained");
        loadTopology(owner,corrected);}
      require(!owner.isCliffCell(0,0) && AllocationFault::live()==live,"distinct corrected authored topology publishes and retires");
      loadTopology(owner,valid);require(owner.isCliffCell(0,0) && AllocationFault::live()==live,"restore accepted source candidate for next failure pair");
    }
    std::cout<<"topology prefixes [0,"<<census<<"); terminal "<<census<<'\n';
  }
}
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try {
    require(argc==2,"terrain family required");initMemoryManager();initialized=true;
    {Map warm;load(warm,wire(payload()));loadTopology(warm,topologyWire());}
    {const std::string family=argv[1];
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
        if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();
        else if(family=="topology")topologyFunctional();else if(family=="topology-negative")topologyNegative();
        else if(family=="topology-faults")topologyFaults();
        else throw std::runtime_error("unknown terrain family");
        require(AllocationFault::live()==live,"whole height owner retirement");}}
    shutdownMemoryManager();initialized=false;
    require(AllocationFault::live()==initial,"whole source memory metadata retirement");
    std::cout<<"PASS source height/topology/queries; full world/ghost/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
  catch(ErrorCode){std::cerr<<"FAIL source terrain admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
