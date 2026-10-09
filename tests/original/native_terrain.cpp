// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "NativeTerrainDeviceFixture.h"
#include "GameLogic/NativeTerrainHeightMap.h"
#include "GameClient/NativeTerrainVisual.h"
#include "GameLogic/GameLogic.h"
#include "Common/TerrainTypes.h"
#include "Common/Terrain.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
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
void rays() {
  Map owner;load(owner,wire(payload()));
  Coord3D contact{-99,-98,-97};
  // Independent source plane oracle for every cell and both diagonals. Unlike
  // groundHeight, this query interpolates display vertices even at map edges.
  for(Int y=0;y<6;++y)for(Int x=0;x<6;++x)
    for(const auto& fraction : {Coord2D{.25f,.75f},Coord2D{.75f,.25f},Coord2D{.5f,.5f}}){
      const Real z0=owner.rawHeight(x,y),z1=owner.rawHeight(x+1,y);
      const Real z2=owner.rawHeight(x+1,y+1),z3=owner.rawHeight(x,y+1);
      const Real expected=(fraction.y>fraction.x ? z0+(z2-z3)*fraction.x+(z3-z0)*fraction.y
                                                : z0+(z1-z0)*fraction.x+(z2-z1)*fraction.y)*MAP_HEIGHT_SCALE;
      const Real wx=(x-1+fraction.x)*MAP_XY_FACTOR,wy=(y-1+fraction.y)*MAP_XY_FACTOR;
      require(owner.intersectSegment({wx,wy,200},{wx,wy,-20},&contact),"display mesh both triangle intersections");
      near(contact.x,wx,"mesh X contact");near(contact.y,wy,"mesh Y contact");near(contact.z,expected,"independent source display plane");
      require(owner.intersectSegment({wx,wy,-20},{wx,wy,200},&contact),"two-sided terrain segment");
      near(contact.z,expected,"reverse terrain contact");
    }
  require(owner.intersectSegment({10,10,200},{10,10,-20},nullptr),"optional contact output");
  for(const auto& xy : {Coord2D{-100,-100},Coord2D{100,100},Coord2D{-100,100},Coord2D{100,-100}}){
    const Int x=xy.x<0 ? 0 : 6,y=xy.y<0 ? 0 : 6;
    require(owner.intersectSegment({xy.x,xy.y,200},{xy.x,xy.y,-20},&contact),"clamped overhang mesh intersection");
    near(contact.z,owner.rawHeight(x,y)*MAP_HEIGHT_SCALE,"source clamped overhang vertex heights");
  }
  // Two crossings. Reversed direction must choose the other nearest contact,
  // not the first triangle in ascending grid order.
  auto ridge=payload(4,7,7,1,0);
  for(Int y=0;y<7;++y)ridge[ridge.size()-49+y*7+3]=char(160);
  load(owner,wire(ridge));
  require(owner.intersectSegment({0,10,50},{40,10,50},&contact),"ridge multiple-hit forward");
  near(contact.x,15,"forward nearest terrain hit");near(contact.z,50,"forward ridge height");
  require(owner.intersectSegment({40,10,50},{0,10,50},&contact),"ridge multiple-hit reverse");
  near(contact.x,25,"reverse nearest terrain hit independent of grid order");
  Coord3D alias{40,10,50};const Coord3D destination{0,10,50};
  require(owner.intersectSegment(alias,destination,&alias),"contact aliases input safely");near(alias.x,25,"aliased contact");
  load(owner,wire(payload(4,7,7,1,80)));
  require(owner.intersectSegment({10,10,50},{10,10,0},&contact),"start on surface accepted");near(contact.z,50,"surface start contact");
  contact={-99,-98,-97};
  require(!owner.intersectSegment({10,10,100},{10,10,50},&contact),"source exact end excluded");
  require(contact.x==-99 && contact.y==-98 && contact.z==-97,"miss leaves contact unchanged");
  require(!owner.intersectSegment({0,0,50},{40,0,50},&contact),"coplanar segment is not a source hit");
  require(!owner.intersectSegment({10,10,50},{10,10,50},&contact),"zero-length surface segment misses");
  require(!owner.intersectSegment({0,0,100},{40,0,100},&contact),"parallel elevated ray misses");
  require(!owner.intersectSegment({-1000,-1000,100},{-1000,-1000,0},&contact),"outside finite source overhang misses");
  Map empty;require(!empty.intersectSegment({0,0,100},{0,0,-10},&contact),"unloaded query misses");
  Map single;load(single,wire(payload(2,1,1,0,9),2));
  require(single.intersectSegment({0,0,100},{0,0,-10},&contact),"one-sample clamped mesh");near(contact.z,5.625f,"one-sample display contact");
}
void rayNegative() {
  Map owner;load(owner,wire(payload()));
  const auto* backing=owner.bytes().data();const auto live=AllocationFault::live();
  for(const Real value : {std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),
                         -std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::max(),
                         -std::numeric_limits<Real>::max()})
    for(unsigned axis=0;axis<3;++axis)for(unsigned endpoint=0;endpoint<2;++endpoint){
      // Finite full-range Z is supported using double intermediates. XY must
      // obey the same bounded grid conversion as the original native queries.
      if(axis==2 && std::isfinite(value))continue;
      Coord3D a{10,10,200},b{10,10,-20},contact{-99,-98,-97};
      auto& point=endpoint ? b : a;
      if(axis==0)point.x=value;else if(axis==1)point.y=value;else point.z=value;
      bool rejected=false;try{owner.intersectSegment(a,b,&contact);}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG;}
      require(rejected && contact.x==-99 && contact.y==-98 && contact.z==-97 &&
        owner.bytes().data()==backing && AllocationFault::live()==live,"hostile segment rejects without mutation or ownership drift");
      require(owner.intersectSegment({10,10,200},{10,10,-20},&contact),"corrected segment retry");
    }
  // Exact no-acquisition terminal, including virtual entry point in provider
  // family below. Do not invent a fault prefix for allocation-free queries.
  AllocationFault::arm(0);bool hit=false;
  try{hit=owner.intersectSegment({10,10,200},{10,10,-20});}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto triggered=AllocationFault::triggered();AllocationFault::disarm();
  require(hit && attempts==0 && !triggered,"complete terrain query allocation terminal0");
  const Real huge=std::numeric_limits<Real>::max();Coord3D contact;
  require(owner.intersectSegment({10,10,huge},{10,10,-huge},&contact),"finite full-range Z crossing without intermediate overflow");
  near(contact.z,12.5f,"huge Z cancellation cannot erase display altitude");
  require(owner.intersectSegment({10,10,-huge},{10,10,huge},&contact),"finite full-range reversed Z crossing");
  near(contact.z,12.5f,"reversed huge Z plane contact");
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
std::string blendPayload(unsigned version=8,Int width=10,Int height=6,bool marked=true,bool splitClasses=false) {
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
  put(result,splitClasses ? 3 : 1);put(result,0);put(result,splitClasses ? 2 : 16);put(result,4);put(result,0);
  ascii(result,"GeneratedTerrain");
  if(splitClasses){put(result,0);put(result,1);put(result,1);put(result,0);ascii(result,"OverlappingTerrain");
    put(result,2);put(result,14);put(result,4);put(result,0);ascii(result,"OtherTerrain");}
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
std::string visualToc(){auto data=topologyToc();overwrite(data,4,3);
  put(data,14,1);data+="GlobalLighting";put(data,3);return data;}
Real authoredLight(unsigned time,unsigned light,unsigned kind,unsigned field){
  return Real(time*100+light*10+kind*1000+field)*0.25f;
}
std::string lightingPayload(unsigned version=3,bool shadow=true){std::string data;put(data,TIME_OF_DAY_NIGHT);
  auto light=[&](unsigned t,unsigned l,unsigned kind){for(unsigned f=0;f<9;++f)
    put(data,std::bit_cast<UnsignedInt>(authoredLight(t,l,kind,f)));};
  for(unsigned t=TIME_OF_DAY_FIRST;t<TIME_OF_DAY_COUNT;++t){light(t,0,0);light(t,0,1);
    if(version>=2)for(unsigned l=1;l<3;++l)light(t,l,1);
    if(version>=3)for(unsigned l=1;l<3;++l)light(t,l,0);}
  if(shadow)put(data,0x89abcdefu);return data;
}
std::string visualWire(unsigned lightVersion=3,bool shadow=true,bool lightFirst=false,unsigned blendVersion=8){
  const auto lights=chunk(lightingPayload(lightVersion,shadow),lightVersion,3);
  const auto heights=chunk(payload(blendVersion==1 ? 1 : 4,10,6,blendVersion==1 ? 0 : 1,80),blendVersion==1 ? 1 : 4);
  const auto blends=chunk(blendPayload(blendVersion),blendVersion,2);
  return visualToc()+(lightFirst ? lights+heights+blends : heights+blends+lights);
}
void loadVisual(Map& owner,GlobalData& data,const std::string& wire){MemoryInput input(wire);
  owner.loadVisualTerrain(input,TRUE,data);require(input.eof() && owner.topologyReady(),"complete visual-owned terrain input");}
bool sameLight(const GlobalData::TerrainLighting& a,const GlobalData::TerrainLighting& b){
  return a.ambient.red==b.ambient.red && a.ambient.green==b.ambient.green && a.ambient.blue==b.ambient.blue &&
    a.diffuse.red==b.diffuse.red && a.diffuse.green==b.diffuse.green && a.diffuse.blue==b.diffuse.blue &&
    a.lightPos.x==b.lightPos.x && a.lightPos.y==b.lightPos.y && a.lightPos.z==b.lightPos.z;
}
struct LightCapture{TimeOfDay time;GlobalData::TerrainLighting terrain[TIME_OF_DAY_COUNT][MAX_GLOBAL_LIGHTS],objects[TIME_OF_DAY_COUNT][MAX_GLOBAL_LIGHTS];
  explicit LightCapture(const GlobalData& data):time(data.m_timeOfDay){for(unsigned t=0;t<TIME_OF_DAY_COUNT;++t)for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){
    terrain[t][l]=data.m_terrainLighting[t][l];objects[t][l]=data.m_terrainObjectsLighting[t][l];}}
  bool matches(const GlobalData& data)const{if(time!=data.m_timeOfDay)return false;
    for(unsigned t=0;t<TIME_OF_DAY_COUNT;++t)for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l)
      if(!sameLight(terrain[t][l],data.m_terrainLighting[t][l]) || !sameLight(objects[t][l],data.m_terrainObjectsLighting[t][l]))return false;
    return true;}
};
void visualFunctional(){GlobalData data;Map owner;const auto unchanged=data.m_terrainLighting[0][0];
  for(unsigned version=1;version<=3;++version)for(unsigned blend=1;blend<=8;++blend){
    loadVisual(owner,data,visualWire(version,true,blend%2,blend));
    require(owner.shadowColor()==0x89abcdefu && data.m_timeOfDay==TIME_OF_DAY_NIGHT,"source time and raw unsigned shadow color");
    require(sameLight(unchanged,data.m_terrainLighting[0][0]),"source invalid-time row remains borrowed configuration");
    for(unsigned t=TIME_OF_DAY_FIRST;t<TIME_OF_DAY_COUNT;++t)for(unsigned l=0;l<MAX_GLOBAL_LIGHTS;++l){
      const auto& terrain=data.m_terrainLighting[t][l];const auto& object=data.m_terrainObjectsLighting[t][l];
      if(l==0 || version>=3){near(terrain.ambient.red,authoredLight(t,l,0,0),"source terrain light selection");near(terrain.lightPos.z,authoredLight(t,l,0,8),"source terrain direction");}
      else require(terrain.ambient.red==0 && terrain.diffuse.blue==0 && terrain.lightPos.z==-1,"legacy absent terrain lights initialized downward");
      if(l==0 || version>=2){near(object.diffuse.green,authoredLight(t,l,1,4),"source object light selection");near(object.lightPos.z,authoredLight(t,l,1,8),"source object direction");}
      else require(object.ambient.blue==0 && object.diffuse.green==0 && object.lightPos.z==-1,"legacy absent object lights initialized downward");
    }
    require(owner.terrainName(-100,-100)=="GeneratedTerrain" && owner.terrainName(10000,10000)=="GeneratedTerrain","source floor/border/clipped packed tile class query");
  }
  const LightCapture before(data);loadVisual(owner,data,topologyWire());
  require(before.matches(data) && owner.shadowColor()==0x89abcdefu,"omitted global lighting retains accepted lights and manager shadow color");
  loadVisual(owner,data,visualWire(1,false));require(owner.shadowColor()==0x89abcdefu,"old optional shadow omission retains previous color");
  Map fresh;loadVisual(fresh,data,visualWire(1,false));require(!fresh.shadowColor(),"no invented shadow color before authored value");
  loadVisual(owner,data,visualWire());const auto flags=owner.topology().cliffFlags;
  const ICoord2D position{0,0};require(owner.constructionHeight(position)==80,"source raw-height query adds border");
  require(!owner.lowerRawHeight(position,90) && !owner.lowerRawHeight(position,80),"construction never raises or rewrites equal height");
  require(owner.lowerRawHeight(position,32) && owner.constructionHeight(position)==32,"source lower-only byte mutation");
  near(owner.groundHeight(0,0),20,"deformation reaches logical ground query");
  require(owner.topology().cliffFlags==flags,"authored cliffs are not silently regenerated by construction");
  loadVisual(owner,data,visualToc()+chunk(payload(4,10,6,1,80))+chunk(blendPayload(8,10,6,true,true),8,2));
  require(owner.terrainName(-10,-10)=="GeneratedTerrain" && owner.terrainName(-0.01f,-10)=="GeneratedTerrain" &&
    owner.terrainName(60,-10)=="GeneratedTerrain" && owner.terrainName(70,-10)=="OtherTerrain",
    "packed divide by4, floor/clipping, border and first overlapping class selection");
  auto visualOnlyToc=visualToc();overwrite(visualOnlyToc,4,4);put(visualOnlyToc,11,1);visualOnlyToc+="ObjectsList";put(visualOnlyToc,4);
  loadVisual(owner,data,visualOnlyToc+chunk("not a logical object list",1,4)+chunk(payload(4,10,6,1,80))+chunk(blendPayload(),8,2));
  require(owner.topologyReady(),"full visual reader skips logical-owner payloads instead of reparsing them");
}
void visualNegative(){GlobalData data;Map owner;const auto valid=visualWire(3,true,true);loadVisual(owner,data,valid);
  auto reject=[&](const std::string& bytes){const LightCapture lights(data);const auto shadow=owner.shadowColor();
    const auto* backing=owner.bytes().data();const auto* tiles=owner.topology().tiles.data();const auto live=AllocationFault::live();bool failed=false;
    try{loadVisual(owner,data,bytes);}catch(ErrorCode error){require(error==ERROR_CORRUPT_FILE_FORMAT,"visual map error classification");failed=true;}
    require(failed && lights.matches(data) && owner.shadowColor()==shadow && owner.bytes().data()==backing && owner.topology().tiles.data()==tiles && AllocationFault::live()==live,"late failure preserves terrain and all configuration fields");
    loadVisual(owner,data,valid);};
  for(std::size_t n=0;n<valid.size();++n)reject(valid.substr(0,n));
  const auto height=chunk(payload(4,10,6,1,80)),blend=chunk(blendPayload(),8,2),light=lightingPayload();
  for(unsigned version:{0u,4u})reject(visualToc()+chunk(light,version,3)+height+blend);
  reject(visualToc()+chunk(light,3,3)+height+blend+chunk(light,3,3));
  reject(visualToc()+chunk(light+"x",3,3)+height+blend);
  for(UnsignedInt time:{0xffffffffu,UnsignedInt(TIME_OF_DAY_COUNT)}){auto bad=light;overwrite(bad,0,time);reject(visualToc()+chunk(bad,3,3)+height+blend);}
  for(std::size_t at:{std::size_t(4),light.size()-8}){auto bad=light;overwrite(bad,at,0x7fc00000u);reject(visualToc()+chunk(bad,3,3)+height+blend);}
  auto corrupt=blendPayload();corrupt+='x';reject(visualToc()+chunk(light,3,3)+height+chunk(corrupt,8,2));
  const auto before=owner.bytes();const auto* backing=owner.bytes().data();
  auto badMutation=[&](auto operation){bool failed=false;try{operation();}catch(ErrorCode error){failed=error==ERROR_BAD_ARG;}
    require(failed && owner.bytes()==before && owner.bytes().data()==backing,"invalid deformation/query preserves accepted backing");};
  for(Int value:{-1,256})badMutation([&]{owner.lowerRawHeight({0,0},value);});
  for(ICoord2D position:{ICoord2D{std::numeric_limits<Int>::min(),0},ICoord2D{std::numeric_limits<Int>::max(),0},ICoord2D{0,6}})
    badMutation([&]{owner.lowerRawHeight(position,1);});
  badMutation([&]{owner.terrainName(std::numeric_limits<Real>::quiet_NaN(),0);});
  Map heightOnly;load(heightOnly,wire(payload()));badMutation([&]{heightOnly.terrainName(0,0);});
}
void visualFaults(){GlobalData data;Map owner;const auto valid=visualWire(1,true,true),replacement=visualWire(3,false,true);
  loadVisual(owner,data,valid);AllocationFault::arm(SIZE_MAX);try{loadVisual(owner,data,replacement);}catch(...){AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();loadVisual(owner,data,valid);
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const LightCapture lights(data);const auto* bytes=owner.bytes().data();const auto* tiles=owner.topology().tiles.data();const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{loadVisual(owner,data,replacement);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<census),"complete visual-owned terrain manifest and exact terminal");
    if(failed)require(lights.matches(data) && owner.bytes().data()==bytes && owner.topology().tiles.data()==tiles && AllocationFault::live()==live,"every partial terrain/lighting owner rolls back");
    loadVisual(owner,data,replacement);require(data.m_terrainLighting[TIME_OF_DAY_MORNING][1].lightPos.z==authoredLight(1,1,0,8),"corrected same-owner retry commits source extra lighting");
    loadVisual(owner,data,valid);require(lights.matches(data) && AllocationFault::live()==live,"accepted lighting/terrain restore after every retry");
  }std::cout<<"visual terrain prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
}

// Test-only device, intentionally no presentation acceptance. Production's CPU
// adapter stays abstract until real output and the complete snapshot are bound.
struct ProviderContext {
  GlobalData data;
  TerrainTypeCollection types;
  GlobalData* oldData=TheWritableGlobalData;
  TerrainTypeCollection* oldTypes=TheTerrainTypes;
  ProviderContext(){TheWritableGlobalData=&data;TheTerrainTypes=&types;}
  ~ProviderContext(){TheWritableGlobalData=oldData;TheTerrainTypes=oldTypes;}
};
void providerLoad(DeviceFixture& owner,const std::string& bytes,const AsciiString& name){
  MemoryInput input(bytes);owner.loadStream(input,name,TRUE);require(input.eof(),"complete native provider terrain admission");
}
void providerFunctional(){ProviderContext context;DeviceFixture owner;TerrainVisual& api=owner;
  const ICoord2D position{0,0};require(api.getRawMapHeight(nullptr)==0 && api.getTerrainTile(0,0)==nullptr,"absent source provider defaults");
  api.setRawMapHeight(nullptr,1);api.init();require(!owner.waterGrid().enabled(),"actual default profile inert through base dispatch");
  providerLoad(owner,visualToc()+chunk(payload(4,10,6,1,80))+chunk(blendPayload(8,10,6,true,true),8,2),"GeneratedAccepted.map");
  TerrainType* type=context.types.newTerrain("GeneratedTerrain");type->friend_setRestrictConstruction(TRUE);
  require(api.getTerrainTile(-10,-10)==type && api.getTerrainTile(-10,-10)->getRestrictConstruction(),"construction receives actual authored restricted TerrainType");
  require(api.getTerrainTile(70,-10)==nullptr,"unknown source terrain class remains absent");
  const auto dirty=owner.dirty;api.setRawMapHeight(&position,90);require(owner.dirty==dirty,"no dirty upload on nonlowering construction");
  api.setRawMapHeight(&position,32);require(api.getRawMapHeight(&position)==32 && owner.dirty==dirty+1,"original virtual deformation and dirty notification");
  near(owner.heightMap().groundHeight(0,0),20,"provider deformation reaches retained simulation grid");
  api.oversizeTerrain(100);require(owner.heightMap().width()==10,"original oversize method remains inert");
  api.setWaterGridResolution(nullptr,3,3,10);api.setWaterGridHeightClamps(nullptr,0,20);
  api.setWaterTransform(nullptr,0,100,200,5);api.setWaterAttenuationFactors(nullptr,1,0,0,0);api.enableWaterGrid(TRUE);
  api.changeWaterHeight(100,200,6);Real height=0;require(api.getWaterGridHeight(100,200,&height),"actual virtual water query");near(height,11,"actual virtual source water mutation");
  Real x=0,y=0,spacing=0;api.getWaterGridResolution(nullptr,&x,&y,&spacing);require(x==3 && y==3 && spacing==10,"virtual resolution outputs");
  api.getWaterGridResolution(nullptr,nullptr,&y,nullptr);api.setWaterTransform(static_cast<const Matrix3D*>(nullptr));api.getWaterTransform(nullptr,nullptr);
  Matrix3D transform(1);api.getWaterTransform(nullptr,&transform);near(transform.Get_Translation().Z,5,"virtual matrix output");
  api.addWaterVelocity(100,200,2,10);
  {GameLogic logic;auto* prior=TheGameLogic;TheGameLogic=&logic;
    struct Restore{GameLogic* prior;~Restore(){TheGameLogic=prior;}} restore{prior};api.update();
    near(owner.waterGrid().vertex(0,0).height,6,"source frame zero through actual GameLogic publication");}
  DeviceFixture candidate;providerLoad(candidate,visualWire(1,false),"GeneratedCandidate.map");
  auto* original=owner.heightMap().bytes().data();auto* next=candidate.heightMap().bytes().data();
  owner.exchangeCpuState(candidate);require(owner.heightMap().bytes().data()==next && candidate.heightMap().bytes().data()==original && candidate.waterGrid().enabled(),"nonallocating complete provider CPU publication exchange");
  owner.exchangeCpuState(candidate);const auto* backing=owner.heightMap().bytes().data();api.reset();
  require(owner.filename().isEmpty() && owner.heightMap().bytes().data()==backing && owner.waterGrid().enabled() && owner.waterGrid().vertex(0,0).height==0,"source reset keeps prior terrain/enabled water backing and clears waves/name");
}
void providerRays(){ProviderContext context;DeviceFixture owner;TerrainVisual& api=owner;
  Coord3D from{10,10,200},to{10,10,-20},contact{-99,-98,-97};
  require(!api.intersectTerrain(&from,&to,&contact),"unloaded virtual terrain ray misses");
  require(!api.intersectTerrain(nullptr,&to,&contact) && !api.intersectTerrain(&from,nullptr,&contact),"nullable ray endpoints miss");
  const auto flat=[&](Int value){return visualToc()+chunk(payload(4,10,6,1,value))+chunk(blendPayload(8,10,6,true,true),8,2);};
  providerLoad(owner,flat(80),"GeneratedDisplay.map");
  require(api.intersectTerrain(&from,&to,&contact),"actual virtual terrain ray admission");near(contact.z,50,"visual source height contact");
  api.setWaterGridResolution(nullptr,3,3,10);api.setWaterGridHeightClamps(nullptr,0,100);
  api.setWaterAttenuationFactors(nullptr,1,0,0,0);api.enableWaterGrid(TRUE);api.changeWaterHeight(10,10,80);
  require(api.intersectTerrain(&from,&to,&contact),"water backing does not replace mesh picking");near(contact.z,50,"terrain query separate from water");
  const ICoord2D lowered{1,1};const auto dirty=owner.dirty;api.setRawMapHeight(&lowered,0);
  require(owner.dirty==dirty+1 && api.intersectTerrain(&from,&to,&contact),"construction lowering immediately updates ray backing");near(contact.z,0,"lowered display vertex contact");
  api.reset();require(api.intersectTerrain(&from,&to,&contact),"source reset retains accepted mesh");near(contact.z,0,"reset retained lowered vertex");
  providerLoad(owner,flat(160),"GeneratedReplacement.map");
  require(api.intersectTerrain(&from,&to,&contact),"replacement query uses new backing");near(contact.z,100,"replacement contact height");
  NativeTerrainVisual::TerrainState exchange;
  owner.exchangeTerrainState(exchange);
  require(!api.intersectTerrain(&from,&to,&contact),"withdrawn CPU mesh cannot fabricate contact");
  owner.exchangeTerrainState(exchange);
  AllocationFault::arm(0);bool hit=false;
  try{hit=api.intersectTerrain(&from,&to,&contact);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto triggered=AllocationFault::triggered();AllocationFault::disarm();
  require(hit && attempts==0 && !triggered,"actual virtual query complete allocation terminal0");near(contact.z,100,"restored backing contact");
  const Coord3D accepted=contact;from.x=std::numeric_limits<Real>::quiet_NaN();
  bool rejected=false;try{api.intersectTerrain(&from,&to,&contact);}catch(ErrorCode error){rejected=error==ERROR_BAD_ARG;}
  require(rejected && contact.x==accepted.x && contact.y==accepted.y && contact.z==accepted.z,"virtual rejection preserves accepted result");
  from={10,10,200};require(api.intersectTerrain(&from,&to,&contact),"virtual corrected retry");
}
void providerNegative(){ProviderContext context;DeviceFixture owner;TerrainVisual& api=owner;
  // Lighting is optional; put it first so every tested physical prefix ends
  // before mandatory terrain admission, rather than reject valid omission.
  const auto valid=visualWire(3,true,true);providerLoad(owner,valid,"GeneratedAccepted.map");
  auto rejected=[&](auto operation){const auto* backing=owner.heightMap().bytes().data();const auto dirty=owner.dirty;const LightCapture lighting(context.data);
    bool failed=false;try{operation();}catch(ErrorCode){failed=true;}
    require(failed && owner.heightMap().bytes().data()==backing && owner.filename()=="GeneratedAccepted.map" && owner.dirty==dirty && lighting.matches(context.data),"provider failure retains accepted map/filename/lighting/dirty publication");};
  for(std::size_t n=0;n<valid.size();++n){rejected([&]{providerLoad(owner,valid.substr(0,n),"GeneratedRejected.map");});
    providerLoad(owner,valid,"GeneratedAccepted.map");}
  rejected([&]{providerLoad(owner,valid,AsciiString());});rejected([&]{api.setRawMapHeight(nullptr,1);});
  const ICoord2D position{0,0};rejected([&]{api.setRawMapHeight(&position,256);});
  {auto* prior=TheWritableGlobalData;TheWritableGlobalData=nullptr;
    struct Restore{GlobalData* prior;~Restore(){TheWritableGlobalData=prior;}}restore{prior};
    rejected([&]{api.init();});rejected([&]{providerLoad(owner,valid,"GeneratedRejected.map");});}
  {auto* prior=TheTerrainTypes;TheTerrainTypes=nullptr;
    struct Restore{TerrainTypeCollection* prior;~Restore(){TheTerrainTypes=prior;}}restore{prior};rejected([&]{api.getTerrainTile(0,0);});}
  {auto* prior=TheGameLogic;TheGameLogic=nullptr;
    struct Restore{GameLogic* prior;~Restore(){TheGameLogic=prior;}}restore{prior};rejected([&]{api.update();});}
  require(!api.load(AsciiString()) && !api.load("GeneratedMissing.map"),"empty/missing filename fails without accepted mutation");
  api.setWaterGridResolution(nullptr,3,3,10);api.setWaterGridHeightClamps(nullptr,0,20);
  api.setWaterAttenuationFactors(nullptr,1,0,0,0);api.enableWaterGrid(TRUE);api.changeWaterHeight(0,0,5);
  const auto water=owner.waterGrid().backing();const auto* waterPointer=owner.waterGrid().backing().data();
  context.data.m_vertexWaterGridSize[0]=std::numeric_limits<Real>::quiet_NaN();
  rejected([&]{api.init();});
  require(owner.waterGrid().backing()==water && owner.waterGrid().backing().data()==waterPointer && owner.waterGrid().enabled(),"late profile failure preserves accepted virtual water owner");
  const auto dirty=owner.dirty;providerLoad(owner,valid,"GeneratedRetry.map");require(owner.filename()=="GeneratedRetry.map" && owner.dirty==dirty+1,"corrected provider replacement retry");
}
void providerFaults(){ProviderContext context;DeviceFixture owner;
  const auto valid=visualWire(1,true,true),replacement=visualWire(3,false,true);
  const AsciiString acceptedName("GeneratedAccepted.map"),candidateName("GeneratedCandidate.map");
  providerLoad(owner,valid,acceptedName);AllocationFault::arm(SIZE_MAX);
  try{providerLoad(owner,replacement,candidateName);}catch(...){AllocationFault::disarm();throw;}
  const auto census=AllocationFault::attempts();AllocationFault::disarm();providerLoad(owner,valid,acceptedName);
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){const LightCapture lights(context.data);
    const auto* bytes=owner.heightMap().bytes().data();const auto live=AllocationFault::live();const auto dirty=owner.dirty;bool failed=false;
    AllocationFault::arm(ordinal);try{providerLoad(owner,replacement,candidateName);}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<census),"complete virtual provider fault manifest and exact terminal");
    if(failed)require(lights.matches(context.data) && owner.heightMap().bytes().data()==bytes && owner.filename()==acceptedName && owner.dirty==dirty && AllocationFault::live()==live,"every provider admission prefix restores accepted publication");
    providerLoad(owner,replacement,candidateName);require(owner.filename()==candidateName,"every provider corrected retry");
    providerLoad(owner,valid,acceptedName);require(lights.matches(context.data) && AllocationFault::live()==live,"accepted provider restore after every retry");
  }std::cout<<"native provider prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
  TerrainVisual& api=owner;
  api.setWaterGridResolution(nullptr,3,3,10);
  auto waterFault=[&](auto operation){const auto* backing=owner.waterGrid().backing().data();const auto live=AllocationFault::live();
    AllocationFault::arm(0);bool failed=false;try{operation();}catch(const std::bad_alloc&){failed=true;}
    catch(...){AllocationFault::disarm();throw;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(failed && hit && owner.waterGrid().backing().data()==backing && AllocationFault::live()==live,"virtual water allocation prefix retires and restores");
    AllocationFault::arm(1);try{operation();}catch(...){AllocationFault::disarm();throw;}
    const auto terminal=AllocationFault::attempts();const auto terminalHit=AllocationFault::triggered();AllocationFault::disarm();
    require(terminal==1 && !terminalHit,"virtual water corrected retry and exact terminal1");};
  waterFault([&]{api.enableWaterGrid(TRUE);});
  auto* original=owner.waterGrid().backing().data();
  waterFault([&]{api.setWaterGridResolution(nullptr,3,4,10);});
  require(owner.waterGrid().cellsY()==4 && owner.waterGrid().backing().data()!=original,"actual virtual Y-only resize replacement");
}
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try {
    require(argc==2,"terrain family required");initMemoryManager();initialized=true;
    {FileSystem files;auto* previous=TheFileSystem;TheFileSystem=&files;
    struct RestoreFiles{FileSystem* previous;~RestoreFiles(){TheFileSystem=previous;}} restoreFiles{previous};
    {GlobalData warmData;Map warm;load(warm,wire(payload()));loadTopology(warm,topologyWire());
      TerrainTypeCollection warmTypes;warmTypes.newTerrain("GeneratedTerrain");}
    {const std::string family=argv[1];
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
        if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();
        else if(family=="rays")rays();else if(family=="rays-negative")rayNegative();else if(family=="provider-rays")providerRays();
        else if(family=="topology")topologyFunctional();else if(family=="topology-negative")topologyNegative();
        else if(family=="topology-faults")topologyFaults();
        else if(family=="visual")visualFunctional();else if(family=="visual-negative")visualNegative();
        else if(family=="visual-faults")visualFaults();
        else if(family=="provider")providerFunctional();else if(family=="provider-negative")providerNegative();
        else if(family=="provider-faults")providerFaults();
        else throw std::runtime_error("unknown terrain family");
        require(AllocationFault::live()==live,"whole height owner retirement");}}}
    shutdownMemoryManager();initialized=false;
    require(AllocationFault::live()==initial,"whole source memory metadata retirement");
    std::cout<<"PASS source height/topology/queries; full world/ghost/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
  catch(ErrorCode){std::cerr<<"FAIL source terrain admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
