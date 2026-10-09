// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameClient/NativeWaterGrid.h"
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/FileSystem.h"
#include "Common/NativeUserStorage.h"
#include "Common/XferSave.h"
#include "Common/XferLoad.h"
#include "Common/XferCRC.h"
#include <bit>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using Grid=NativeWaterGrid;
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
void near(Real value,Real expected,const char* text){require(std::fabs(value-expected)<0.00001f,text);}
template<class F>void reject(F operation){bool failed=false;try{operation();}catch(ErrorCode){failed=true;}
  catch(XferStatus){failed=true;}catch(SaveCode){failed=true;}require(failed,"invalid source water input rejected");}
void setup(Grid& grid,bool enable=true){grid.setResolution(3,3,10);grid.setClamps(0,20);
  grid.setTransform(0,100,200,5);grid.setAttenuation(1,0,0,0);if(enable)grid.enable(true);}
struct MemoryLoad:XferLoad{explicit MemoryLoad(const std::string& input){m_bytes.assign(input.begin(),input.end());
  m_open=true;setOptions(XO_NO_POST_PROCESSING);}bool consumed()const{return m_position==m_bytes.size();}};
void word(std::string& data,UnsignedInt value,unsigned bytes=4){for(unsigned n=0;n<bytes;++n)data+=char(value>>(8*n));}
void overwrite(std::string& data,std::size_t at,UnsignedInt value){for(unsigned n=0;n<4;++n)data.at(at+n)=char(value>>(8*n));}
std::string wire(Real height=7,Real velocity=2,UnsignedByte status=1,UnsignedByte preferred=9){std::string data;
  word(data,1,1);word(data,3);word(data,3);
  for(unsigned n=0;n<36;++n){word(data,std::bit_cast<UnsignedInt>(height));word(data,std::bit_cast<UnsignedInt>(velocity));word(data,status,1);word(data,preferred,1);}
  return data;}
void load(Grid& grid,const std::string& data){MemoryLoad input(data);input.xferSnapshot(&grid);require(input.consumed(),"complete source water snapshot");}
void functional(){Grid grid;setup(grid);require(grid.backing().size()==36,"source cells plus one visible vertex and invisible border");
  Real height=-1;require(grid.height(100,200,&height),"active water grid origin");near(height,5,"query adds transform Z");
  require(!grid.height(130,200,&height) && !grid.height(99,200,&height),"source query excludes last cell strip and outside point");
  grid.changeHeight(100,200,6);near(grid.vertex(0,0).height,6,"source height mutation");
  require(grid.height(109.9f,209.9f,&height),"fractional grid query");near(height,11,"source truncation rather than interpolation");
  grid.setAttenuation(1,1,0,10);grid.changeHeight(110,210,4);
  near(grid.vertex(1,1).height,4,"attenuation at center");near(grid.vertex(0,1).height,2,"distance attenuation axial neighbor");
  near(grid.vertex(2,2).height,4/(1+std::sqrt(2.0f)),"source influence box includes diagonal outside nominal circle");
  grid.changeHeight(110,210,100);near(grid.vertex(1,1).height,20,"upper clamp");grid.changeHeight(110,210,-100);near(grid.vertex(1,1).height,0,"lower clamp");
  grid.setTransform(Real(PI/2),100,200,5);Real gx,gy;
  require(grid.worldToGrid(95,210,gx,gy),"rotated interior world coordinates");near(gx,1,"source rotated X dot product");near(gy,0.5f,"source rotated Y dot product");
  Matrix3D matrix(1);matrix.Set_Translation(Vector3(700,800,17));grid.setTransform(matrix);
  require(grid.height(95,210,&height),"matrix overload keeps source query axes/origin");near(height,grid.vertex(1,0).height+17,"matrix-only transform changes query Z");
  grid.setResolution(3,4,10);require(grid.cellsY()==4 && grid.backing().size()==42 && grid.vertex(1,1).height==0,"Y-only resolution repair retires old grid");
  grid.enable(false);auto backing=grid.backing().data();require(!grid.height(95,210,&height),"disabled query");grid.enable(true);
  require(grid.backing().data()==backing,"enable reuses initialized backing");
  GlobalData data;Grid defaults;defaults.init(data);require(!defaults.enabled() && defaults.cellsX()==0 && defaults.spacing()==0,"actual zero profile stays inert");
  reject([&]{defaults.enable(true);});data.m_vertexWaterXGridCells[0]=3;data.m_vertexWaterYGridCells[0]=3;
  data.m_vertexWaterGridSize[0]=10;data.m_vertexWaterAttenuationA[0]=1;data.m_vertexWaterHeightClampHi[0]=20;
  defaults.init(data);defaults.enable(true);require(defaults.backing().size()==36,"actual populated GlobalData profile initializes backing");
}
void motion(){Grid grid;setup(grid);grid.addVelocity(100,200,2,10.9f);
  require(grid.vertex(0,0).preferredHeight==10 && grid.inMotion() && grid.riverDisabled(),"source byte preferred-height conversion/motion/river flags");
  grid.update(0,-0.1f);near(grid.vertex(0,0).height,0,"initial source frame zero does not advance");
  grid.update(1,-0.1f);near(grid.vertex(0,0).velocity,2.16f,"source damping then gravity times three");near(grid.vertex(0,0).height,2.16f,"source Euler step");
  grid.update(1,-0.1f);near(grid.vertex(0,0).height,2.16f,"repeated client frame does not advance logic");
  grid.update(2,-0.1f);near(grid.vertex(0,0).velocity,2.3088f,"next source force step");
  grid.reset();require(!grid.inMotion() && grid.vertex(0,0).velocity==0,"reset retires wave state without allocation");
  grid.addVelocity(100,200,0,-0.5f);require(grid.vertex(0,0).preferredHeight==0,"representable negative fraction truncates to source byte zero");
  grid.addVelocity(100,200,0,0);grid.update(3,0);require(!grid.inMotion() && grid.vertex(0,0).status==0,"source strict rest thresholds");
  grid.addVelocity(100,200,1,0);grid.update(4,0);near(grid.vertex(0,0).height,0,"source below-one damping snaps to preferred");
  Grid independent;setup(independent);independent.addVelocity(100,200,2,10);independent.update(1,-0.1f);
  near(independent.vertex(0,0).height,2.16f,"separate grid owns its own frame cursor");
  grid.enable(false);const auto before=grid.backing();grid.addVelocity(100,200,2,10);grid.update(5,-0.1f);
  require(grid.backing()==before,"disabled motion does not advance");
}
void negative(){Grid grid;setup(grid);grid.changeHeight(100,200,5);const auto before=grid.backing();const auto* identity=grid.backing().data();
  auto unchanged=[&]{require(grid.backing()==before && grid.backing().data()==identity && grid.cellsX()==3 && grid.cellsY()==3 && grid.spacing()==10,"invalid input preserves accepted grid");};
  for(Real value:{-1.0f,std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::max()}){
    reject([&]{grid.setResolution(value,3,10);});unchanged();reject([&]{grid.setResolution(3,value,10);});unchanged();}
  reject([&]{grid.setResolution(3,3,0);});unchanged();reject([&]{grid.setClamps(5,4);});unchanged();
  reject([&]{grid.setTransform(std::numeric_limits<Real>::infinity(),0,0,0);});unchanged();
  for(Real preferred:{-1.0f,256.0f,std::numeric_limits<Real>::quiet_NaN()}){reject([&]{grid.addVelocity(100,200,1,preferred);});unchanged();}
  reject([&]{grid.height(100,200,nullptr);});unchanged();reject([&]{grid.vertex(4,0);});unchanged();
  grid.setAttenuation(0,1,0,10);reject([&]{grid.changeHeight(110,210,4);});unchanged(); // Late zero denominator after earlier admitted neighbors.
  grid.setAttenuation(1,0,0,0);grid.changeHeight(100,200,1);near(grid.vertex(0,0).height,6,"corrected same-grid arithmetic retry");
  reject([&]{grid.setAttenuation(1,0,0,std::numeric_limits<Real>::infinity());});
}
struct Tree{std::filesystem::path path;Tree(){char pattern[]="/tmp/zh-native-water-XXXXXX";auto* value=::mkdtemp(pattern);require(value,"generated storage");path=value;}
  ~Tree(){std::error_code error;std::filesystem::remove_all(path,error);}};
void snapshots(){Grid grid;setup(grid);const auto valid=wire();load(grid,valid);
  require(grid.vertex(0,0).height==7 && grid.vertex(0,0).preferredHeight==9 && grid.inMotion(),"source snapshot scalar load and resumed motion");
  grid.update(1,-0.1f);near(grid.vertex(0,0).height,9.16f,"loaded wave continues on next frame");
  auto rejectLoad=[&](const std::string& data){const auto before=grid.backing();auto* pointer=grid.backing().data();const auto live=AllocationFault::live();
    reject([&]{load(grid,data);});require(grid.backing()==before && grid.backing().data()==pointer && AllocationFault::live()==live,"failed snapshot preserves all water state/backing");load(grid,valid);};
  for(std::size_t n=0;n<valid.size();++n)rejectLoad(valid.substr(0,n));
  auto bad=valid;bad[0]=2;rejectLoad(bad);bad=valid;overwrite(bad,1,4);rejectLoad(bad);bad=valid;overwrite(bad,5,4);rejectLoad(bad);
  for(std::size_t offset:{std::size_t(9),std::size_t(13),valid.size()-10,valid.size()-6}){bad=valid;overwrite(bad,offset,0x7fc00000u);rejectLoad(bad);}
  bad=valid;bad[17]=2;rejectLoad(bad);bad=valid;bad[valid.size()-2]=2;rejectLoad(bad);
  Tree assets,user;FileSystem files;files.mountReadOnly({assets.path.string()});
  NativeUserStorage storage({(user.path/"data").string(),(user.path/"cache").string()},files);storage.validateRootOwnership();
  auto* previous=TheNativeUserStorage;TheNativeUserStorage=&storage;
  struct Restore{NativeUserStorage* prior;~Restore(){TheNativeUserStorage=prior;}} restore{previous};
  XferSave save;save.open("Save/generated-water.sav");save.xferSnapshot(&grid);save.close();
  auto output=storage.readFile(NativeUserArea::Data,"Save/generated-water.sav",1024);
  require(output && std::string(output->begin(),output->end())==valid && valid.size()==369,"actual original save exact field widths, no padding/native-layout serialization");
  XferCRC crc;crc.open("generated water CRC");auto initial=crc.getCRC();crc.xferSnapshot(&grid);require(crc.getCRC()==initial,"source light CRC remains empty");
  require(std::filesystem::is_empty(assets.path),"asset fixture remains read-only");
}
void faults(){for(unsigned kind=0;kind<3;++kind){Grid grid;setup(grid,kind!=0);const auto replacement=wire();
    auto operation=[&]{if(kind==0)grid.enable(true);else if(kind==1)grid.setResolution(7,9,10);else load(grid,replacement);};
    auto restore=[&]{Grid baseline;setup(baseline,kind!=0);grid.swap(baseline);};
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());try{operation();}catch(...){AllocationFault::disarm();throw;}
    const auto census=AllocationFault::attempts();require(!AllocationFault::triggered(),"complete source water demand");AllocationFault::disarm();restore();
    for(std::size_t ordinal=0;ordinal<=census;++ordinal){const auto before=grid.backing();const auto* pointer=grid.backing().data();const auto live=AllocationFault::live();bool failed=false;
      AllocationFault::arm(ordinal);try{operation();}catch(const std::bad_alloc&){failed=true;}const auto hit=AllocationFault::triggered();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal<census),"every water ownership prefix and exact terminal");
      if(failed)require(grid.backing()==before && grid.backing().data()==pointer && AllocationFault::live()==live,"failed backing preserves initialized accepted state");
      operation();require(grid.enabled() && !grid.backing().empty(),"every failure has corrected same-owner retry");restore();
      require(grid.backing()==before && AllocationFault::live()==live,"all water candidate resources retire exactly");
    }std::cout<<"water owner "<<kind<<" prefixes [0,"<<census<<"); terminal "<<census<<"; every retry\n";
  }}
}
int main(int argc,char** argv){auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"water family");initMemoryManager();initialized=true;
    {Tree assets;FileSystem files;files.mountReadOnly({assets.path.string()});auto* previous=TheFileSystem;TheFileSystem=&files;
    struct RestoreFiles{FileSystem* prior;~RestoreFiles(){TheFileSystem=prior;}} restoreFiles{previous};
    {GlobalData warm;}
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();{const std::string_view family=argv[1];
      if(family=="functional")functional();else if(family=="motion")motion();else if(family=="negative")negative();
      else if(family=="snapshots")snapshots();else if(family=="faults")faults();else require(false,"unknown family");}
      require(AllocationFault::live()==live,"whole original water owner retirement");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole manager retirement");std::cout<<"PASS source water CPU provider; terrain/root pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL water "<<Int(error)<<'\n';}
  catch(XferStatus status){std::cerr<<"FAIL transfer "<<Int(status)<<'\n';}catch(SaveCode status){std::cerr<<"FAIL save "<<Int(status)<<'\n';}
  catch(...){std::cerr<<"FAIL water ownership\n";}AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
