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
std::string chunk(const std::string& data, unsigned version = 4) {
  std::string result; put(result, 1); put(result, version, 2); put(result, data.size());
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
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try {
    require(argc==2,"terrain family required");initMemoryManager();initialized=true;
    {Map warm;load(warm,wire(payload()));}
    {const std::string family=argv[1];
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
        if(family=="functional")functional();else if(family=="negative")negative();else if(family=="faults")faults();
        else throw std::runtime_error("unknown terrain family");
        require(AllocationFault::live()==live,"whole height owner retirement");}}
    shutdownMemoryManager();initialized=false;
    require(AllocationFault::live()==initial,"whole source memory metadata retirement");
    std::cout<<"PASS source height data/queries; full world/cliff/ghost/startup pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
  catch(ErrorCode){std::cerr<<"FAIL source terrain admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
