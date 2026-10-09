// SPDX-License-Identifier: GPL-3.0-or-later
// Source camera choreography only. No physical output/root acceptance is claimed.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "ScriptCameraFixture.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/NativeServiceOwners.h"
#include "GameClient/ScriptedView.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/TerrainLogic.h"
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
bool close(Real lhs,Real rhs){return std::abs(lhs-rhs)<0.0001f;}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(ErrorCode e){rejected=e==ERROR_BAD_ARG;}require(rejected,"rejected camera input");}
using Camera=ScriptCameraFixture;
struct Parents {
  FileSystem files;TerrainLogic terrain;
  std::unique_ptr<GlobalData> data;
  FileSystem* previousFiles=TheFileSystem;
  TerrainLogic* previousTerrain=TheTerrainLogic;
  GlobalData* previousData=TheWritableGlobalData;
  Parents(){TheFileSystem=&files;TheTerrainLogic=&terrain;
    try{data=std::make_unique<GlobalData>();TheWritableGlobalData=data.get();
      data->m_cameraHeight=300;data->m_cameraPitch=45;data->m_cameraYaw=0;
      data->m_minCameraHeight=20;data->m_maxCameraHeight=500;
      data->m_disableCameraMovement=FALSE;
    }catch(...){TheFileSystem=previousFiles;TheTerrainLogic=previousTerrain;throw;}}
  ~Parents(){data.reset();TheWritableGlobalData=previousData;TheFileSystem=previousFiles;TheTerrainLogic=previousTerrain;}
};
struct Points {
  std::array<Waypoint*,25> values{};
  ~Points(){for(auto* value:values)if(value)value->deleteInstance();}
  void add(unsigned i,const Coord3D& location){
    values[i]=newInstance(Waypoint)(static_cast<WaypointID>(i+1),AsciiString::TheEmptyString,&location,
      AsciiString::TheEmptyString,AsciiString::TheEmptyString,AsciiString::TheEmptyString,FALSE);
    if(i)values[i-1]->addLink(values[i]);
  }
};
void modes(Parents& parents){
  Camera camera;camera.init();const Coord3D origin{};camera.lookAt(&origin);
  camera.rotateCamera(0.25f,66,0,0);camera.zoomCamera(0.6f,99,0,0);camera.pitchCamera(0.4f,99,0,0);
  camera.cameraModFreezeTime();camera.cameraModFinalTimeMultiplier(3);
  require(!camera.isCameraMovementFinished() && camera.isTimeFrozen(),"all source activities gate logic freeze");
  require(camera.updateCameraMovements(33),"source client update advances camera");
  require(close(camera.getAngle(),PI/4) && close(camera.getZoom(),1.3f+(0.6f-1.3f)/3),"original interpolation and quantization");
  camera.updateCameraMovements(33);
  require(!camera.isTimeFrozen() && !camera.isCameraMovementFinished() && close(camera.getAngle(),PI/2),"rotation clears freeze while longer zoom/pitch remain active");
  camera.updateCameraMovements(33);
  require(camera.isCameraMovementFinished() && close(camera.getZoom(),0.6f) && close(camera.getFXPitch(),0.4f) && camera.getTimeMultiplier()==3,"all endpoints and multiplier");
  require(!camera.updateCameraMovements(33),"idle camera does not invent work");
  camera.zoomCamera(1,33,0,0);camera.cameraModFreezeTime();camera.updateCameraMovements(33);
  require(camera.isCameraMovementFinished() && camera.isTimeFrozen(),"zoom alone does not clear source freeze request");
  camera.rotateCamera(0,33,0,0);camera.updateCameraMovements(33);
  require(!camera.isTimeFrozen(),"rotation retirement clears retained freeze");
  camera.setFrameLengthMilliseconds(10);camera.rotateCamera(0.25f,20,0,0);
  camera.updateCameraMovements(500);require(!camera.isCameraMovementFinished(),"rotation counts updates, not elapsed path time");
  camera.updateCameraMovements(0);require(camera.isCameraMovementFinished(),"configured frame quantization exact terminal");
  camera.rotateCamera(0.25f,20,0,0);camera.cameraModFreezeTime();parents.data->m_disableCameraMovement=TRUE;
  const Real angle=camera.getAngle();camera.updateCameraMovements(10);camera.updateCameraMovements(10);
  require(camera.isCameraMovementFinished() && !camera.isTimeFrozen() && camera.getAngle()==angle,"disabled motion preserves position and retires timing/freeze");
  parents.data->m_disableCameraMovement=FALSE;
  camera.rotateCamera(1,100,0,0);
  for(auto mode:{FM_VIEW_MB_IN_AND_OUT_ALPHA,FM_VIEW_MB_IN_AND_OUT_SATURATE,FM_VIEW_MB_IN_ALPHA,FM_VIEW_MB_OUT_ALPHA,FM_VIEW_MB_IN_SATURATE,FM_VIEW_MB_OUT_SATURATE}){
    camera.filter(FT_VIEW_MOTION_BLUR_FILTER,mode);require(camera.isCameraMovementFinished(),"six original motion blur exceptions");}
  camera.filter(FT_VIEW_DEFAULT,FM_VIEW_DEFAULT);require(!camera.isCameraMovementFinished(),"other filters retain movement predicate");
  camera.setAngle(0);require(camera.isCameraMovementFinished(),"manual angle cancels coupled motion");
  const Coord3D east{100,0,0};camera.rotateCameraTowardPosition(&east,20,0,0,FALSE);
  camera.updateCameraMovements(10);camera.updateCameraMovements(10);
  require(close(camera.getAngle(),-PI/2),"source ninety-degree facing convention");
  camera.setAngle(0);camera.rotateCameraTowardPosition(&east,20,0,0,TRUE);
  camera.updateCameraMovements(10);camera.updateCameraMovements(10);
  require(close(camera.getAngle(),3*PI/2),"source reverse rotation keeps exact unnormalized terminal angle");
  camera.setZoom(1);camera.zoomCamera(0.4f,60,20,20);camera.updateCameraMovements(10);
  const ParabolicEase ease(20.0f/60,20.0f/60);
  require(close(camera.getZoom(),1+(0.4f-1)*ease(1.0f/6)),"original parabolic easing, not linear replacement");
  for(unsigned n=0;n<5;++n)camera.updateCameraMovements(10);
  camera.setFrameLengthMilliseconds(1);camera.setTimeMultiplier(std::numeric_limits<Int>::min());
  camera.rotateCamera(0,1,0,0);camera.cameraModFinalTimeMultiplier(std::numeric_limits<Int>::max());camera.updateCameraMovements(1);
  require(camera.isCameraMovementFinished() && camera.getTimeMultiplier()==std::numeric_limits<Int>::max(),"defined full-width multiplier interpolation");
}
void paths(Parents& parents){
  Camera camera;camera.init();const Coord3D origin{},target{120,0,0};camera.lookAt(&origin);
  camera.moveCameraTo(&target,99,999,FALSE,0,0);camera.cameraModFreezeTime();
  camera.updateCameraMovements(33);require(close(camera.position().x,40),"single target source quadratic interpolation");
  camera.updateCameraMovements(33);require(close(camera.position().x,80),"source path second interpolation");
  camera.updateCameraMovements(33);
  require(!camera.isCameraMovementFinished() && camera.isCameraMovementAtWaypointAlongPath() && !camera.isCameraMovementAtWaypointAlongPath(),"arrival consumed on query before final retirement");
  camera.updateCameraMovements(33);require(camera.isCameraMovementFinished() && !camera.isTimeFrozen() && camera.position().x==120,"next update retires path and establishes endpoint");
  camera.lookAt(&origin);camera.moveCameraTo(&target,1,0,FALSE,0,0);
  require(!camera.isCameraMovementFinished() && !camera.isCameraMovementAtWaypointAlongPath(),"instant move keeps source pending path");
  camera.updateCameraMovements(33);require(camera.isCameraMovementFinished() && camera.position().x==120,"instant move retires on later update");
  camera.lookAt(&origin);Points points;for(unsigned i=0;i<25;++i)points.add(i,{Real((i+1)*20),0,0});
  camera.moveCameraAlongWaypointPath(points.values[0],240,66,TRUE,0,0);
  camera.cameraModRollingAverage(3);camera.cameraModFinalTimeMultiplier(4);
  camera.updateCameraMovements(10);require(camera.position().x==0,"shutter advances time without positioning");
  for(unsigned i=0;i<30;++i)camera.updateCameraMovements(10);
  require(camera.isCameraMovementFinished() && camera.position().x==480,"maximum padded path retains first 24 linked targets safely");
  camera.lookAt(&origin);camera.moveCameraAlongWaypointPath(nullptr,100,0,TRUE,0,0);
  require(camera.isCameraMovementFinished() && camera.position().x==0,"empty initialized path is idle");
  points.values[24]->addLink(points.values[0]);camera.moveCameraAlongWaypointPath(points.values[0],240,0,FALSE,0,0);
  camera.updateCameraMovements(241);require(camera.isCameraMovementFinished() && camera.position().x==480,"noncoincident linked cycle terminates at original point limit");
  camera.lookAt(&origin);
  camera.moveCameraTo(&origin,30,0,TRUE,0,0);camera.updateCameraMovements(30);camera.updateCameraMovements(1);
  require(camera.isCameraMovementFinished() && std::isfinite(camera.getAngle()),"coincident oriented path has defined endpoint angle");
  camera.moveCameraTo(&target,30,0,FALSE,0,0);camera.cameraModFreezeTime();parents.data->m_disableCameraMovement=TRUE;
  camera.updateCameraMovements(30);require(!camera.isCameraMovementFinished(),"disabled path exact duration is still active");
  camera.updateCameraMovements(1);require(camera.isCameraMovementFinished() && !camera.isTimeFrozen() && camera.position().x==0,"disabled path retires only past duration");
  parents.data->m_disableCameraMovement=FALSE;
}
void negative(){
  Camera camera;camera.init();const Coord3D origin{},target{100,0,0};camera.lookAt(&origin);
  camera.moveCameraTo(&target,99,0,FALSE,0,0);camera.cameraModFreezeTime();
  const Real nan=std::numeric_limits<Real>::quiet_NaN();const Coord3D bad{nan,0,0},elevated{10,10,50};
  rejects([&]{camera.moveCameraTo(&bad,99,0,FALSE,0,0);});rejects([&]{camera.moveCameraTo(nullptr,99,0,FALSE,0,0);});
  rejects([&]{camera.lookAt(&elevated);});rejects([&]{camera.setFrameLengthMilliseconds(0);});
  rejects([&]{camera.updateCameraMovements(-1);});rejects([&]{camera.rotateCamera(nan,99,0,0);});
  rejects([&]{camera.rotateCameraTowardObject(INVALID_ID,99,0,0,0);});
  Points cycle;cycle.add(0,origin);cycle.add(1,origin);cycle.values[1]->addLink(cycle.values[0]);
  rejects([&]{camera.moveCameraAlongWaypointPath(cycle.values[0],99,0,FALSE,0,0);});
  require(camera.isTimeFrozen() && !camera.isCameraMovementFinished() && camera.position().x==0,"invalid input preserves accepted camera movement");
  camera.updateCameraMovements(33);camera.updateCameraMovements(33);camera.updateCameraMovements(33);camera.updateCameraMovements(33);
  require(camera.isCameraMovementFinished() && camera.position().x==100,"accepted movement completes after rejections");
  camera.moveCameraTo(&target,99,0,FALSE,0,0);camera.updateCameraMovements(100);
  require(camera.isCameraMovementFinished(),"corrected same owner retry");
}
void ownership(){
  require(!TheTacticalView,"camera fixture requires no foreign tactical publication");
  NativeServiceOwners<1> owners;
  auto acquire=[&]{return owners.create(TheTacticalView,[]{
    auto camera=std::make_unique<Camera>();camera->init();return camera.release();});};
  AllocationFault::arm(SIZE_MAX);acquire();const auto count=AllocationFault::attempts();AllocationFault::disarm();owners.retire(TheTacticalView);
  require(count==1,"exact camera owner allocation terminal");
  for(std::size_t ordinal=0;ordinal<=count;++ordinal){const auto live=AllocationFault::live();bool failed=false;
    AllocationFault::arm(ordinal);try{acquire();}catch(const std::bad_alloc&){failed=true;}
    const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<count),"complete camera acquisition failure/terminal pair");
    if(failed)require(!TheTacticalView && !owners.owns(TheTacticalView) && AllocationFault::live()==live,"failed camera never publishes or retains ownership");
    else{auto* accepted=TheTacticalView;rejects(acquire);require(TheTacticalView==accepted,"duplicate rejects without replacing accepted camera");owners.retire(TheTacticalView);}
    auto* retry=acquire();require(retry==TheTacticalView && owners.owns(TheTacticalView),"corrected camera retry has legitimate publication");
    owners.retire(TheTacticalView);require(!TheTacticalView && AllocationFault::live()==live,"whole camera owner retirement");
  }
  Camera camera;camera.init();const Coord3D target{100,0,0};
  AllocationFault::arm(0);camera.moveCameraTo(&target,99,0,TRUE,0,0);camera.cameraModFreezeTime();
  for(unsigned n=0;n<4;++n)camera.updateCameraMovements(33);
  const auto choreographyCount=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
  require(choreographyCount==0 && !hit && camera.isCameraMovementFinished(),"zero-allocation choreography exact terminal without substituted allocator");
  std::cout<<"camera owner prefixes [0,"<<count<<"); terminal "<<count<<"; every retry; choreography terminal 0\n";
}
}
int main(int argc,char** argv){const auto initial=AllocationFault::live();bool initialized=false;
  try{require(argc==2,"camera test family");initMemoryManager();initialized=true;
    {Parents parents;{Camera warm;warm.init();Points points;points.add(0,{20,0,0});}
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();
        const std::string_view family=argv[1];if(family=="modes")modes(parents);else if(family=="paths")paths(parents);else if(family=="negative")negative();else if(family=="ownership")ownership();else require(false,"unknown family");
        require(AllocationFault::live()==live && TheMemoryPoolFactory->findMemoryPool("Waypoint")->getUsedBlockCount()==0,"repeated camera and actual waypoint owners retire exactly");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole memory manager retirement");
    std::cout<<"PASS original camera choreography; native UI/root/output still pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL camera "<<Int(error)<<'\n';}
  if(initialized)shutdownMemoryManager();return 1;
}
