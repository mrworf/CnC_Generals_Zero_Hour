// SPDX-License-Identifier: GPL-3.0-or-later
// Actual shared layout/map/slot/copy owners. Physical GUI execution is pending.
#include "AllocationFault.h"
#include "Common/GameMemory.h"
#include "Common/INI.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/NativeUserStorage.h"
#include "GameClient/NativeMapPreviewLayout.h"
#include "GameNetwork/GameInfo.h"
#include "Common/MultiplayerSettings.h"
#include <cmath>
#include <cerrno>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
namespace {
void require(bool value,const char* label){if(!value)throw std::runtime_error(label);}
bool equal(ICoord2D a,ICoord2D b){return a.x==b.x && a.y==b.y;}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode){rejected=true;}
  require(rejected,"malformed preview rejects");}
Region3D extent(Real width,Real height){Region3D result{};result.hi.x=width;result.hi.y=height;return result;}
MapMetaData map(){MapMetaData result;result.m_extent=extent(200,200);result.m_numPlayers=2;result.m_isMultiplayer=TRUE;
  result.m_waypoints.emplace("Player_1_Start",Coord3D{100,100,0});
  result.m_waypoints.emplace("Player_2_Start",Coord3D{102,98,0});
  result.m_supplyPositions={{0,0,0},{200,200,0}};result.m_techPositions={{100,100,0}};return result;}
auto controls(){std::array<NativeMapPreviewControl,MAX_SLOTS> result{};
  for(auto& item:result)item={TRUE,{20,20}};return result;}
// Independent original-source formula for admitted inputs, retaining Real
// intermediates and integer truncation. Never feed malformed values to it.
void oracle(Int x,Int y,Int width,Int height,Region3D e,ICoord2D& ul,ICoord2D& lr){
  Real rw=e.width()/Real(width),rh=e.height()/Real(height);
  if(rw>=rh){ul={0,Int((Real(height)-e.height()/rw)/2)};lr={Int(e.width()/rw),height-ul.y};}
  else{ul={Int((Real(width)-e.width()/rh)/2),0};lr={width-ul.x,Int(e.height()/rh)};}
  ul.x+=x;ul.y+=y;lr.x+=x;lr.y+=y;
}
void geometry(){
  for(Int width:{1,7,100,257})for(Int height:{1,9,97,300})
    for(auto e:{extent(100,100),extent(125,250),extent(500,75),extent(13.7f,19.3f)}){
      ICoord2D expectedUL,expectedLR,ul{},lr{};oracle(29,-17,width,height,e,expectedUL,expectedLR);
      findDrawPositions(29,-17,width,height,e,&ul,&lr);
      require(equal(ul,expectedUL) && equal(lr,expectedLR),"source aspect fit and truncation");
    }
  auto metadata=map();auto c=controls();NativeMapPreviewLayout layout;layout.rebuild(metadata,200,200,c);
  require(equal(layout.starts()[0].position,{90,90}) && equal(layout.starts()[1].position,{111,111}),
    "ordered collision in common local coordinates");
  require(layout.markers().m_supplyPosList.size()==2 && equal(layout.markers().m_supplyPosList.front(),{193,-7}) &&
      equal(layout.markers().m_supplyPosList.back(),{-7,193}) && equal(layout.markers().m_techPosList.front(),{93,93}),
      "inverted Y, half marker offsets, original reverse lists");
  c[0].present=FALSE;layout.rebuild(metadata,200,200,c);
  require(!layout.starts()[0].visible && equal(layout.starts()[1].position,{92,92}),"sparse controls do not collide");
  c[1].size={10,30};layout.rebuild(metadata,200,200,c);
  const ICoord2D expected{Int((Real(102)/200)*200-5),Int((1-Real(98)/200)*200-15)};
  require(equal(layout.starts()[1].position,expected),"per-control dimensions retain Real truncation");
  metadata.m_isMultiplayer=FALSE;layout.rebuild(metadata,200,200,c);
  for(const auto& start:layout.starts())require(!start.visible,"solo map has no multiplayer start markers");
  TechAndSupplyImages output;layout.publishMarkers(output);require(output.m_supplyPosList.size()==2 &&
      layout.markers().m_supplyPosList.empty(),"no-allocation marker owner transfer");
}
void negative(){
  auto metadata=map();auto c=controls();NativeMapPreviewLayout layout;layout.rebuild(metadata,200,200,c);
  auto preserved=[&]{require(equal(layout.starts()[1].position,{111,111}) && layout.markers().m_supplyPosList.size()==2,
      "rejected complete candidate preserves admitted placement");};
  for(Int value:{-1,0}){
    rejects([&]{layout.rebuild(metadata,value,200,c);});preserved();
    rejects([&]{layout.rebuild(metadata,200,value,c);});preserved();
  }
  for(Int count:{-1,MAX_SLOTS+1,std::numeric_limits<Int>::max()}){
    metadata.m_numPlayers=count;rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();
  }metadata.m_numPlayers=2;
  for(Real value:{0.0f,-1.0f,std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::quiet_NaN()}){
    metadata.m_extent.hi.x=value;rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();
  }metadata.m_extent.hi.x=200;
  metadata.m_waypoints.erase("Player_2_Start");rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();
  metadata.m_waypoints.emplace("Player_2_Start",Coord3D{102,98,0});
  metadata.m_techPositions.push_back({std::numeric_limits<Real>::infinity(),0,0});
  rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();metadata.m_techPositions.pop_back();
  c[1].size.x=-1;rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();c[1].size.x=20;
  ICoord2D ul{17,29},lr{31,43};
  require(!nativeMapDrawPositions(std::numeric_limits<Int>::max(),0,200,200,metadata.m_extent,ul,lr) &&
      equal(ul,{17,29}) && equal(lr,{31,43}),"origin overflow leaves both outputs unchanged");
  rejects([&]{findDrawPositions(0,0,200,200,metadata.m_extent,nullptr,&lr);});
  rejects([&]{findDrawPositions(0,0,200,200,metadata.m_extent,&ul,&ul);});
  metadata.m_extent.lo.x=-std::numeric_limits<Real>::max();metadata.m_extent.hi.x=std::numeric_limits<Real>::max();
  rejects([&]{layout.rebuild(metadata,200,200,c);});preserved();metadata.m_extent=extent(200,200);
  layout.rebuild(metadata,200,200,c);preserved();
}
struct Setup:GameInfo{
  std::array<GameSlot,MAX_SLOTS> slots;
  Setup(){for(Int i=0;i<MAX_SLOTS;++i)setSlotPointer(i,&slots[i]);}
};
void labels(){
  FileSystem files;auto* priorFiles=TheFileSystem;
  struct FilesPublication{FileSystem* prior;~FilesPublication(){TheFileSystem=prior;}} restoreFiles{priorFiles};
  TheFileSystem=&files;
  auto* previous=TheWritableGlobalData;
  struct Publication{GlobalData* prior;~Publication(){TheWritableGlobalData=prior;}} restore{previous};
  GlobalData data;
  TheWritableGlobalData=&data;
  Setup game;auto c=controls();c[0].present=FALSE;
  // Slot zero has no control, but its chosen destination does: the original
  // source-slot check must not suppress that player's label.
  game.slots[0].setPlayerTemplate(0);game.slots[0].setStartPos(1);
  game.slots[1].setPlayerTemplate(0);game.slots[1].setStartPos(0);
  auto result=nativeMapPreviewLabels(game,2,FALSE,c);
  require(result[1]==0 && result[0]==-1,"destination pointer presence, not source-slot presence");
  game.slots[2].setPlayerTemplate(0);game.slots[2].setStartPos(1);
  require(nativeMapPreviewLabels(game,2,FALSE,c)[1]==2,"source last-slot-wins order");
  require(nativeMapPreviewLabels(game,2,TRUE,c)[1]==2,"actual apparent-position method when no random policy");
  game.slots[2].setPlayerTemplate(PLAYERTEMPLATE_OBSERVER);
  require(nativeMapPreviewLabels(game,2,FALSE,c)[1]==0,"observers excluded by original template predicate");
  for(Int index:{-1,MAX_SLOTS,std::numeric_limits<Int>::max()}){
    game.slots[0].setStartPos(index);require(nativeMapPreviewLabels(game,2,FALSE,c)[1]==-1,"invalid destination safely ignored");
  }
  rejects([&]{nativeMapPreviewLabels(game,MAX_SLOTS+1,FALSE,c);});
  // Actual source random/team/observer policy, including missing publications.
  game.slots[0].setState(SLOT_PLAYER,UnicodeString(L"local"),1);
  game.slots[0].setPlayerTemplate(0);game.slots[0].setStartPos(0);game.slots[0].saveOffOriginalInfo();
  game.slots[0].setStartPos(1);
  MultiplayerSettings settings;
  auto* previousSettings=TheMultiplayerSettings;auto* previousGame=TheGameInfo;
  struct Views{MultiplayerSettings* settings;GameInfo* game;~Views(){TheMultiplayerSettings=settings;TheGameInfo=game;}}
      views{previousSettings,previousGame};
  TheMultiplayerSettings=&settings;TheGameInfo=nullptr;
  require(game.slots[0].getApparentStartPos()==0 && !isSlotLocalAlly(&game.slots[0]),
      "unpublished game uses original hidden random position without dereferencing it");
  TheGameInfo=&game;game.setInGame();game.setLocalIP(999);
  require(!isSlotLocalAlly(&game.slots[0]) && game.slots[0].getApparentStartPos()==0,
      "missing local slot is checked before original slot lookup");
  game.setLocalIP(1);require(isSlotLocalAlly(&game.slots[0]) && game.slots[0].getApparentStartPos()==1,
      "local player sees actual selected position");
  game.slots[1].setPlayerTemplate(0);game.slots[1].setStartPos(0);game.slots[1].saveOffOriginalInfo();game.slots[1].setStartPos(1);
  require(!isSlotLocalAlly(&game.slots[1]) && game.slots[1].getApparentStartPos()==0,"opponent random selection stays hidden");
  game.slots[0].setTeamNumber(2);game.slots[1].setTeamNumber(2);
  require(isSlotLocalAlly(&game.slots[1]) && game.slots[1].getApparentStartPos()==1,"team sees actual position");
  game.slots[1].setTeamNumber(3);game.slots[0].setPlayerTemplate(PLAYERTEMPLATE_OBSERVER);game.slots[0].saveOffOriginalInfo();
  require(isSlotLocalAlly(&game.slots[1]) && game.slots[1].getApparentStartPos()==1,"original observer sees all positions");
  GameSlot detached;require(!isSlotLocalAlly(&detached) && !isSlotLocalAlly(nullptr),"absent borrowed slot identity is not allied");
}
void layoutFaults(){
  auto metadata=map();auto c=controls();std::size_t census=0;
  {NativeMapPreviewLayout discovery;AllocationFault::arm(SIZE_MAX);
    try{discovery.rebuild(metadata,200,200,c);}catch(...){AllocationFault::disarm();throw;}
    census=AllocationFault::attempts();AllocationFault::disarm();}
  require(census>0 && census<128,"complete bounded layout manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){
    const auto outside=AllocationFault::live();
    {NativeMapPreviewLayout layout;layout.rebuild(metadata,200,200,c);const auto live=AllocationFault::live();
      bool failed=false;AllocationFault::arm(ordinal);
      try{layout.rebuild(metadata,400,400,c);}catch(const std::bad_alloc&){failed=true;}
      AllocationFault::disarm();require(failed==(ordinal<census) && failed==AllocationFault::triggered() &&
          (failed || AllocationFault::attempts()==census),"every layout failure and exact terminal");
      if(failed){require(AllocationFault::live()==live && equal(layout.starts()[0].position,{90,90}) &&
          layout.markers().m_supplyPosList.size()==2,"whole offside layout rollback");layout.rebuild(metadata,400,400,c);}
      require(equal(layout.starts()[0].position,{190,190}),"same-owner corrected retry");
    }require(AllocationFault::live()==outside,"layout whole-owner retirement");
  }std::cout<<"layout ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
unsigned descriptors(){DIR* directory=::opendir("/proc/self/fd");require(directory,"descriptor census");
  unsigned count=0;while(::readdir(directory))++count;::closedir(directory);return count;}
struct IO:NativeStorageIO{
  std::size_t ordinal=SIZE_MAX,count=0;bool triggered=false;
  bool fail(){const bool hit=count++==ordinal;triggered|=hit;return hit;}
  void arm(std::size_t value){ordinal=value;count=0;triggered=false;}
  int openFile(int directory,const char* name,int flags,unsigned mode)override{
    if(fail()){errno=EIO;return -1;}return NativeStorageIO::openFile(directory,name,flags,mode);}
  Int writeFile(int descriptor,const void* bytes,Int count)override{
    if(fail()){errno=EIO;return -1;}return NativeStorageIO::writeFile(descriptor,bytes,count);}
  int sync(int descriptor)override{if(fail()){errno=EIO;return -1;}return NativeStorageIO::sync(descriptor);}
  void preparePublish()override{if(fail())throw NativeStorageError();}
};
struct Storage {
  std::filesystem::path root;
  FileSystem files;IO io;std::unique_ptr<NativeUserStorage> user;
  std::string payload=std::string(150000,'G');AsciiString input{"Maps/Generated/Generated.tga"};
  Storage(){char path[]="/tmp/zh-preview-XXXXXX";auto* selected=::mkdtemp(path);require(selected,"generated root");root=selected;
    std::filesystem::create_directories(root/"assets/Maps/Generated");
    std::ofstream output(root/"assets/Maps/Generated/Generated.tga",std::ios::binary);output<<payload;output.close();
    files.mountReadOnly({(root/"assets").string()});
    user=std::make_unique<NativeUserStorage>(NativeUserPaths{(root/"data").string(),(root/"cache").string()},files,&io);
  }
  ~Storage(){user.reset();std::error_code ignored;std::filesystem::remove_all(root,ignored);}
  void copy(){nativeCopyMapPreview(files,*user,input,"MapPreviews/generated.tga");}
  void prior(){io.arm(SIZE_MAX);auto output=user->beginWrite(NativeUserArea::Data,"MapPreviews/generated.tga");
    output->write("prior",5);output->commit();}
  std::string read(){auto bytes=user->readFile(NativeUserArea::Data,"MapPreviews/generated.tga",200000);
    require(bool(bytes),"preview published");return std::string(bytes->begin(),bytes->end());}
  void integrity(){std::ifstream source(root/"assets/Maps/Generated/Generated.tga",std::ios::binary);
    std::string actual{std::istreambuf_iterator<char>(source),{}};require(actual==payload,"generated supplied data remains unchanged");
    for(const auto& entry:std::filesystem::directory_iterator(root/"data/MapPreviews"))
      require(entry.path().filename()=="generated.tga","no abandoned temporary candidate");}
};
void copy(){Storage context;context.copy();require(context.read()==context.payload,"streamed whole preview publication");
  context.integrity();const auto fds=descriptors();
  rejects([&]{nativeCopyMapPreview(context.files,*context.user,"missing.tga","MapPreviews/generated.tga");});
  bool rejected=false;try{nativeCopyMapPreview(context.files,*context.user,context.input,"../escape");}catch(const NativeStorageError&){rejected=true;}
  require(rejected && descriptors()==fds && context.read()==context.payload,"bad destination/missing input retain owners and published bytes");
}
void copyFaults(bool allocations){
  std::size_t census=0;
  {Storage discovery;discovery.prior();discovery.io.arm(SIZE_MAX);
    if(allocations)AllocationFault::arm(SIZE_MAX);
    try{discovery.copy();}catch(...){AllocationFault::disarm();throw;}
    census=allocations?AllocationFault::attempts():discovery.io.count;AllocationFault::disarm();}
  require(census>0 && census<128,"bounded preview copy manifest");
  for(std::size_t ordinal=0;ordinal<=census;++ordinal){
    const auto outside=AllocationFault::live();const auto externalFD=descriptors();
    {Storage context;context.prior();const auto live=AllocationFault::live();const auto fds=descriptors();
      context.io.arm(allocations?SIZE_MAX:ordinal);if(allocations)AllocationFault::arm(ordinal);
      bool failed=false;try{context.copy();}catch(const std::bad_alloc&){require(allocations,"unexpected allocation failure");failed=true;}
        catch(const NativeStorageError&){require(!allocations,"unexpected storage failure");failed=true;}
      const auto attempts=allocations?AllocationFault::attempts():context.io.count;
      const bool triggered=allocations?AllocationFault::triggered():context.io.triggered;
      AllocationFault::disarm();context.io.arm(SIZE_MAX);
      require(triggered==(ordinal<census) && (ordinal<census || attempts==census),"every copy fault ordinal and exact terminal");
      if(allocations)require(failed==(ordinal<census),"allocation manifest exhausts every failure/retry");
      require(descriptors()==fds && AllocationFault::live()==live,"failed/successful copy retires acquired units exactly");
      require(context.read()==(failed?"prior":context.payload),"prepublication rollback or irreversible complete publication");
      if(failed)context.copy();require(context.read()==context.payload,"same-owner full copy retry");context.integrity();
    }require(AllocationFault::live()==outside && descriptors()==externalFD,"complete storage/file owners retire exactly");
  }std::cout<<"copy allocations "<<allocations<<" ordinals [0,"<<census<<"); terminal "<<census<<'\n';
}
}
int main(int argc,char** argv){bool initialized=false;
  try{require(argc==2,"preview family required");initMemoryManager();initialized=true;
    // Warm pooled source File and namespace ownership before fault baselines.
    {Storage warm;warm.copy();}const std::string family=argv[1];
    if(family=="labels")labels(); // Admit source pool metadata before owner baselines.
    for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
      if(family=="geometry")geometry();else if(family=="negative")negative();else if(family=="labels")labels();
      else if(family=="layout-faults")layoutFaults();else if(family=="copy")copy();
      else if(family=="copy-allocation-faults")copyFaults(TRUE);else if(family=="copy-storage-faults")copyFaults(FALSE);
      else throw std::runtime_error("unknown preview family");require(AllocationFault::live()==live,"repeated preview owners retire");}
    shutdownMemoryManager();initialized=false;std::cout<<"PASS original map preview CPU/storage; physical GUI pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode){std::cerr<<"FAIL original preview admission\n";}
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
