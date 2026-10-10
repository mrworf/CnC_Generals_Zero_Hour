// SPDX-License-Identifier: GPL-3.0-or-later
// Source camera choreography only. No physical output/root acceptance is claimed.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "ScriptCameraFixture.h"
#include "NativeTerrainDeviceFixture.h"
#include "GameClient/NativeCamera.h"
#include "GameClient/NativeCameraShaker.h"
#include "GameClient/NativeCameraView.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/NativeServiceOwners.h"
#include "GameClient/ScriptedView.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/TerrainLogic.h"
#include "WWMath/matrix3d.h"
#include <array>
#include <cmath>
#include <cstring>
#include <cstdlib>
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
struct SpatialCamera:NativeCameraView {
  const NativeCamera& spatial=m_spatialCamera;
  NativeCamera::State state(){return {m_pos,m_cameraOffset,m_groundLevel,m_zoom,m_angle,m_pitchAngle,getFXPitch(),
    m_FOV,m_width,m_height,m_originX,m_originY,FALSE};}
  void refresh(){synchronizeCamera();}
  Drawable* pickDrawable(const ICoord2D*,Bool,PickType)override{throw ERROR_BAD_ARG;}
  Int iterateDrawablesInRegion(IRegion2D*,Bool(*)(Drawable*,void*),void*)override{throw ERROR_BAD_ARG;}
  void drawView()override{throw ERROR_BAD_ARG;}
  void updateView()override{throw ERROR_BAD_ARG;} // Full frame/lock lifecycle is not this fixture.
  void shake(const Coord3D*,CameraShakeType)override{throw ERROR_BAD_ARG;}
  void Add_Camera_Shake(const Coord3D&,float,float,float)override{throw ERROR_BAD_ARG;}
  void cameraEnableSlaveMode(const AsciiString&,const AsciiString&)override{throw ERROR_BAD_ARG;}
  void cameraDisableSlaveMode()override{throw ERROR_BAD_ARG;}
  void forceCameraConstraintRecalc()override{throw ERROR_BAD_ARG;}
  Coord3D position()const{return m_pos;}
  Coord2D scrollAmount()const{return m_scrollAmount;}
};
struct HeightInput:ChunkInputStream {
  std::string bytes;UnsignedInt position=0;
  HeightInput(){auto put=[&](UnsignedInt n,unsigned width=4){for(unsigned i=0;i<width;++i)bytes.push_back(char(n>>(8*i)));};
    bytes="CkMp";put(1);put(13,1);bytes+="HeightMapData";put(1);
    put(1);put(2,2);put(12+400);put(20);put(20);put(400);bytes.append(400,char(32));
  }
  Int read(void* output,Int count)override{if(count<0 || (!output && count))throw ERROR_BAD_ARG;
    const auto n=std::min<std::size_t>(count,bytes.size()-position);if(n)std::memcpy(output,bytes.data()+position,n);position+=n;return n;}
  UnsignedInt tell()override{return position;}
  Bool absoluteSeek(UnsignedInt n)override{if(n>bytes.size())return FALSE;position=n;return TRUE;}
  Bool eof()override{return position==bytes.size();}
};
void spatial(){
  SpatialCamera camera;camera.init();camera.setZoom(1);camera.setOrigin(17,23);camera.refresh();
  require(close(camera.get3DCameraPosition().x,0) && close(camera.get3DCameraPosition().y,-290) &&
    close(camera.get3DCameraPosition().z,300),"actual script camera source transformed eye");
  const Coord3D target{0,0,10};ICoord2D screen;
  require(camera.worldToScreenTriReturn(&target,&screen)==View::WTS_INSIDE_FRUSTUM && screen.x==337 && screen.y==263,"source look-at projects to viewport center with origin");
  const auto plane=camera.spatial.atZ(screen,10);require(close(plane.x,0) && close(plane.y,0) && plane.z==10,"project/unproject source center plane");
  const Coord3D offscreen{500,0,10};require(camera.worldToScreenTriReturn(&offscreen,&screen)==View::WTS_OUTSIDE_FRUSTUM,"offscreen is valid projection, not invalid");
  const auto behind=camera.get3DCameraPosition();require(camera.worldToScreenTriReturn(&behind,&screen)==View::WTS_INVALID && screen.x==0 && screen.y==0,"eye/near exclusion zeros invalid projection");
  NativeCamera::State state=camera.state();state.offset={0,0,300};state.zoom=1;state.lookAt={0,0,0};
  NativeCamera vertical;vertical.configure(state);const Coord3D nearPoint{0,0,290},farPoint{0,0,-900},beyond{0,0,-901};
  require(vertical.project(&nearPoint,&screen)==View::WTS_INSIDE_FRUSTUM && vertical.project(&farPoint,&screen)==View::WTS_INSIDE_FRUSTUM &&
    vertical.project(&beyond,&screen)==View::WTS_INVALID,"source inclusive near/far depths versus invalid beyond");
  state.zoom=1.3f;vertical.configure(state);const Coord3D expanded{0,0,-2000};require(vertical.project(&expanded,&screen)==View::WTS_INSIDE_FRUSTUM,"source zoom extends far clip");
  NativeCamera effects;auto effect=camera.state();effect.originX=0;effect.originY=0;effect.fxPitch=.5f;
  effects.configure(effect);const Coord3D raisedTarget{0,0,155};
  require(effects.project(&raisedTarget,&screen)==View::WTS_INSIDE_FRUSTUM && screen.x==320 && screen.y==240,"source FX pitch below one raises target, not eye");
  effect.fxPitch=2;effects.configure(effect);require(close(effects.eye().y,-145) && close(effects.eye().z,300),"source FX pitch above one divides horizontal eye displacement");
  effect.fxPitch=1;effect.pitch=.2f;effects.configure(effect);
  require(close(effects.eye().y,(-300*std::cos(.2f)-300*std::sin(.2f))*(1-10.0f/300)) &&
    close(effects.eye().z,(-300*std::sin(.2f)+300*std::cos(.2f))*(1-10.0f/300)+10),"source X pitch rotates Y/Z before ground translation");
  effect.pitch=0;effect.width=601;effect.height=451;effect.originX=101;effect.originY=-23;effects.configure(effect);
  const auto logicalCenter=effects.centerRay();const auto roundedCenter=effects.ray({401,202});
  require(logicalCenter[1].x!=roundedCenter[1].x,"source logical center is not a floored odd-viewport pixel");
  for(const ICoord2D sample : {ICoord2D{111,0},ICoord2D{200,100},ICoord2D{650,350}}){
    const auto plane=effects.atZ(sample,10);require(effects.project(&plane,&screen)!=View::WTS_INVALID &&
      std::abs(screen.x-sample.x)<=1 && std::abs(screen.y-sample.y)<=1,"source normalized pixel/plane inverse within Real/truncation quantization");}
  camera.setAngle(PI/2);camera.refresh();require(close(camera.get3DCameraPosition().x,290) && close(camera.get3DCameraPosition().y,0),"pitch-then-yaw transform order");
  camera.setAngle(0);camera.setZoom(1);
  const Coord3D destination{100,100,0};camera.moveCameraTo(&destination,99,0,FALSE,0,0);
  for(unsigned i=0;i<4;++i){camera.updateCameraMovements(33);camera.refresh();}
  require(camera.isCameraMovementFinished() && close(camera.get3DCameraPosition().x,100) && close(camera.get3DCameraPosition().y,-200),"actual original choreography updates real eye and sampled ground");
  NativeTerrainHeightMap map;HeightInput input;map.loadHeightData(input,NativeTerrainHeightMap::Purpose::HeightChunkBacking);
  const auto ray=camera.spatial.ray({337,263});Coord3D contact;
  require(map.intersectSegment(ray[0],ray[1],&contact) && close(contact.z,20) && close(contact.x,100),"scripted camera ray reaches real generated display mesh");
  DeviceFixture visual;NativeTerrainVisual::TerrainState backing;backing.height.swap(map);visual.exchangeTerrainState(backing);
  auto* previous=TheTerrainVisual;TheTerrainVisual=&visual;
  struct Restore{TerrainVisual* previous;~Restore(){TheTerrainVisual=previous;}}restore{previous};
  const ICoord2D center{337,263};camera.screenToTerrain(&center,&contact);
  require(close(contact.x,100) && close(contact.z,20),"production View virtual ray composes real visual backing and original empty bridge list");
  const Coord3D elevated{100,100,100};camera.lookAt(&elevated);camera.refresh();
  require(close(camera.state().lookAt.x,100) && close(camera.state().lookAt.y,180),"production elevated look-at uses source center ray and real display mesh");
}
void spatialNegative(){
  NativeCamera camera;NativeCamera::State valid;valid.offset={0,-300,300};camera.configure(valid);const Coord3D accepted=camera.eye();
  for(unsigned field=0;field<8;++field){auto bad=valid;
    if(field==0)bad.width=0;else if(field==1)bad.height=-1;else if(field==2)bad.zoom=0;else if(field==3)bad.offset={0,0,0};
    else if(field==4)bad.fieldOfView=PI;else if(field==5)bad.angle=std::numeric_limits<Real>::quiet_NaN();
    else if(field==6)bad.lookAt.x=std::numeric_limits<Real>::infinity();else bad.ground=std::numeric_limits<Real>::quiet_NaN();
    rejects([&]{camera.configure(bad);});require(camera.eye().x==accepted.x && camera.eye().y==accepted.y && camera.eye().z==accepted.z,"rejected candidate preserves accepted camera");
    camera.configure(valid);
  }
  auto horizon=valid;horizon.fxPitch=0;camera.configure(horizon);rejects([&]{camera.atZ({320,240},10);});camera.configure(valid);
  auto coincident=valid;coincident.ground=300;camera.configure(coincident);
  require(camera.pose().backward.x==-1 && camera.pose().up.z==1,"normal camera preserves original coincident Look_At fallback");
  const auto coplanar=camera.atZ({320,240},300);require(coplanar.x==camera.eye().x && coplanar.y==camera.eye().y && coplanar.z==300,
    "coplanar source ray returns its plane-origin point instead of original NaN arithmetic");camera.configure(valid);
  ICoord2D output{19,21};require(camera.project(nullptr,&output)==View::WTS_INVALID && output.x==19 && output.y==21,"null projection preserves source optional-output contract");
  const Coord3D nan{std::numeric_limits<Real>::quiet_NaN(),0,0};require(camera.project(&nan,&output)==View::WTS_INVALID && output.x==0 && output.y==0,"defined malformed projection invalid");
  const auto extreme=camera.ray({std::numeric_limits<Int>::min(),std::numeric_limits<Int>::max()});
  require(std::isfinite(extreme[1].x) && std::isfinite(extreme[1].y) && std::isfinite(extreme[1].z),"full-width pixel subtraction cannot overflow");
  NativeCamera empty;rejects([&]{empty.eye();});rejects([&]{empty.ray({0,0});});
  SpatialCamera view;view.init();const auto acceptedEye=view.get3DCameraPosition();
  rejects([&]{view.setHeight(0);});rejects([&]{view.setFieldOfView(std::numeric_limits<Real>::quiet_NaN());});
  rejects([&]{view.setWidth(100);}); // Missing legitimate Display width parent.
  rejects([&]{view.setGuardBandBias(nullptr);});
  require(view.get3DCameraPosition().x==acceptedEye.x && view.get3DCameraPosition().y==acceptedEye.y &&
    view.get3DCameraPosition().z==acceptedEye.z,"production View rejection preserves real accepted eye");
  Coord3D result{19,21,23};const ICoord2D center{320,240};
  rejects([&]{view.screenToTerrain(&center,&result);});
  require(result.x==19 && result.y==21 && result.z==23,"missing terrain parent cannot fabricate or publish a contact");
  rejects([&]{view.screenToWorldAtZ(nullptr,&result,0);});
  rejects([&]{view.screenToWorldAtZ(&center,&result,std::numeric_limits<Real>::infinity());});
  require(result.x==19 && result.y==21 && result.z==23,"malformed plane requests preserve caller output");
  view.screenToWorldAtZ(&center,&result,10);require(std::isfinite(result.x) && result.z==10,"production plane corrected retry");
}
void spatialOwnership(){
  NativeCamera camera;NativeCamera::State state;state.offset={0,-300,300};
  AllocationFault::arm(0);
  try{for(unsigned i=0;i<100;++i){state.lookAt={Real(i),Real(i),0};camera.configure(state);const auto ray=camera.ray({320,240});
    ICoord2D screen;camera.project(&ray[1],&screen);camera.atZ({320,240},10);}}
  catch(...){AllocationFault::disarm();throw;}
  const auto count=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(count==0 && !hit,"complete native camera acquisition terminal0 across repeated lifetimes/queries");
}
void navigation(Parents& parents){
  SpatialCamera camera;camera.init();camera.setZoom(1);
  const Real scale=500.0f*std::tan(25*PI/180)/640;
  Coord2D delta{4,-3};camera.scrollBy(&delta);
  require(close(camera.position().x,4*scale) && close(camera.position().y,3*scale/std::sqrt(2.0f)) && camera.position().z==0,
    "production source view-plane scroll scale and XY-only adoption");
  require(camera.scrollAmount().x==4 && camera.scrollAmount().y==-3 &&
    close(camera.get3DCameraPosition().x,camera.position().x),"scroll delta and accepted real eye published together");
  camera.setAngle(PI/2);const auto before=camera.position();delta={1,0};camera.scrollBy(&delta);
  require(close(camera.position().x,before.x) && close(camera.position().y,before.y+scale),"source rotated screen X follows camera right, not world X");
  camera.setAngle(0);camera.setHeight(1001);camera.setOrigin(101,-23);
  const auto portrait=camera.position();delta={0,2};camera.scrollBy(&delta);
  require(camera.position().x==portrait.x && camera.position().y==portrait.y && camera.scrollAmount().y==2,
    "source integer aspect zero in portrait viewport still records scroll");
  camera.setHeight(480);camera.rotateCamera(.25f,99,0,0);camera.zoomCamera(.8f,99,0,0);camera.pitchCamera(.7f,99,0,0);
  delta={1,1};camera.scrollBy(&delta);camera.updateCameraMovements(33);
  require(camera.getAngle()==0 && !camera.isCameraMovementFinished() && camera.getZoom()<1 && camera.getFXPitch()<1,
    "scroll cancels rotation only and retains independent zoom/pitch");
  camera.updateCameraMovements(33);camera.updateCameraMovements(33);
  require(camera.isCameraMovementFinished() && close(camera.getZoom(),.8f) && close(camera.getFXPitch(),.7f),"retained activities reach original terminal");
  camera.setZoom(1);const Coord3D destination{100,100,0};camera.moveCameraTo(&destination,99,0,FALSE,0,0);
  delta={1,0};camera.scrollBy(&delta);for(unsigned i=0;i<4;++i)camera.updateCameraMovements(33);
  require(camera.isCameraMovementFinished() && close(camera.position().x,100) && close(camera.position().y,100),"scroll does not cancel original waypoint path");
  camera.setSnapMode(View::LOCK_TETHER,2);require(camera.isCameraMovementFinished() && camera.updateCameraMovements(0),
    "scripted lock work and source movement-finished predicates are distinct");
  camera.setCameraLock(static_cast<ObjectID>(17));require(camera.getCameraLock()==static_cast<ObjectID>(17) && !camera.updateCameraMovements(0),"actual lock resets scripted-lock flag");
  parents.data->m_disableCameraMovement=TRUE;camera.setSnapMode(View::LOCK_FOLLOW,0);
  camera.setCameraLock(static_cast<ObjectID>(18));require(camera.getCameraLock()==static_cast<ObjectID>(17) && camera.updateCameraMovements(0),"disabled lock admission preserves accepted lock and scripted flag");
  camera.setCameraLock(INVALID_ID);require(camera.getCameraLock()==INVALID_ID && !camera.updateCameraMovements(0),"disabled motion still allows lock withdrawal");
  parents.data->m_disableCameraMovement=FALSE;
  AllocationFault::arm(0);try{for(unsigned i=0;i<100;++i)camera.scrollBy(&delta);}
  catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const auto hit=AllocationFault::triggered();AllocationFault::disarm();
  require(attempts==0 && !hit,"complete repeated navigation acquisition terminal0");
}
void navigationNegative(){
  SpatialCamera camera;camera.init();camera.setZoom(1);camera.rotateCamera(.25f,99,0,0);
  const auto position=camera.position(),eye=camera.get3DCameraPosition();const auto scroll=camera.scrollAmount();
  for(Coord2D bad:{Coord2D{std::numeric_limits<Real>::quiet_NaN(),0},Coord2D{0,std::numeric_limits<Real>::infinity()},
    Coord2D{std::numeric_limits<Real>::max(),0},Coord2D{0,std::numeric_limits<Real>::max()}}){
    rejects([&]{camera.scrollBy(&bad);});require(camera.position().x==position.x && camera.position().y==position.y &&
      camera.get3DCameraPosition().x==eye.x && camera.get3DCameraPosition().y==eye.y && camera.scrollAmount().x==scroll.x &&
      camera.scrollAmount().y==scroll.y && !camera.isCameraMovementFinished(),"malformed scroll preserves complete accepted navigation/choreography");}
  camera.scrollBy(nullptr);Coord2D zero{};camera.scrollBy(&zero);require(!camera.isCameraMovementFinished(),"optional/zero scroll does not cancel motion");
  rejects([&]{camera.setSnapMode(View::LOCK_TETHER,std::numeric_limits<Real>::quiet_NaN());});
  rejects([&]{camera.setSnapMode(View::LOCK_FOLLOW,-1);});
  camera.updateCameraMovements(33);require(camera.getAngle()!=0,"failed navigation did not cancel original rotation");
  Coord2D retry{1,0};camera.scrollBy(&retry);require(camera.isCameraMovementFinished() && camera.scrollAmount().x==1,"corrected same-owner scroll retry adopts and cancels rotation");
  camera.setCameraLock(static_cast<ObjectID>(19));const auto* previous=TheGlobalData;TheWritableGlobalData=nullptr;
  try{rejects([&]{camera.setCameraLock(INVALID_ID);});}catch(...){TheWritableGlobalData=const_cast<GlobalData*>(previous);throw;}
  TheWritableGlobalData=const_cast<GlobalData*>(previous);require(camera.getCameraLock()==static_cast<ObjectID>(19),"missing lock parent preserves accepted ID");
  camera.setCameraLock(INVALID_ID);
}
void poses(){
  NativeCamera::State state;state.originX=17;state.originY=23;
  NativeCamera camera;const Coord3D eye{40,50,300},target{40,50,0};
  const auto pose=NativeCamera::lookAtPose(eye,target);camera.configurePose(state,pose);
  ICoord2D pixel;require(camera.project(&target,&pixel)==View::WTS_INSIDE_FRUSTUM && pixel.x==337 && pixel.y==263,
    "explicit source chase pose projects its target to the absolute viewport center");
  const auto ray=camera.ray(pixel),center=camera.centerRay();
  require(close(ray[0].z,300) && close(ray[1].z,-900) && close(center[1].z,-900),"explicit pose uses normalized far-depth rays");
  const auto ground=camera.atZ(pixel,0);require(close(ground.x,40) && close(ground.y,50),"chase plane projection preserves actual pose eye");
  NativeCamera::Pose identity;identity.eye=eye;
  const auto x=NativeCamera::rotateLocal(identity,{PI/2,0,0});
  const auto y=NativeCamera::rotateLocal(identity,{0,PI/2,0});
  const auto z=NativeCamera::rotateLocal(identity,{0,0,PI/2});
  require(close(x.up.z,1) && close(x.backward.y,-1) && close(y.right.z,-1) && close(y.backward.x,1) &&
    close(z.right.y,1) && close(z.up.x,-1),"local X/Y/Z shake rotations retain original postmultiply signs");
  const auto combined=NativeCamera::rotateLocal(identity,{PI/2,PI/2,PI/2});
  require(close(combined.right.z,1) && close(combined.up.y,-1) && close(combined.backward.x,1),
    "combined shake preserves ordered local X then Y then Z rather than world-axis rotations");
  Matrix3D source(1);source.Rotate_X(.13f);source.Rotate_Y(-.27f);source.Rotate_Z(.41f);
  const auto ordered=NativeCamera::rotateLocal(identity,{.13f,-.27f,.41f});
  const auto sourceX=source.Get_X_Vector(),sourceY=source.Get_Y_Vector(),sourceZ=source.Get_Z_Vector();
  require(close(ordered.right.x,sourceX.X) && close(ordered.right.y,sourceX.Y) && close(ordered.right.z,sourceX.Z) &&
    close(ordered.up.x,sourceY.X) && close(ordered.up.y,sourceY.Y) && close(ordered.up.z,sourceY.Z) &&
    close(ordered.backward.x,sourceZ.X) && close(ordered.backward.y,sourceZ.Y) && close(ordered.backward.z,sourceZ.Z),
    "all nine combined pose columns match actual original Matrix3D rotation methods");
  auto authored=identity;authored.right={2,0,0};authored.up={.25f,3,0};authored.backward={0,0,4};
  camera.configurePose(state,authored);const Coord3D point{41,50,200};
  const Int expectedX=state.originX+Int((Real(state.width)*(Real(.5/(25*std::tan(double(state.fieldOfView)/2)))+1))/2);
  require(camera.project(&point,&pixel)==View::WTS_INSIDE_FRUSTUM && pixel.x==expectedX,
    "finite authored scale and shear use actual general inverse rather than normalization or transpose projection");
  const auto authoredPose=camera.pose();require(authoredPose.right.x==2 && authoredPose.up.x==.25f && authoredPose.up.y==3 && authoredPose.backward.z==4,
    "explicit source bone pose round trip retains every raw column");
  const auto normalized=camera.centerRay();require(close(normalized[1].z,-900),"scaled bone pose still uses normalized source pick-ray length");
  const auto displacement=camera.viewPlaneDisplacement({20,0});
  require(close(displacement.x,Real(2*20*std::tan(double(state.fieldOfView)/2)/state.width)*2),"device plane displacement preserves authored axis scale");
  state.drawEntireTerrain=TRUE;camera.configurePose(state,identity);require(close(camera.centerRay()[1].z,-11700),"explicit pose retains source extended clip range");
  AllocationFault::arm(0);try{for(unsigned n=0;n<100;++n){camera.configurePose(state,NativeCamera::rotateLocal(pose,{.01f,.02f,.03f}));
      camera.project(&target,&pixel);camera.ray(pixel);camera.centerRay();camera.atZ(pixel,0);camera.pose();}}
  catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
  require(attempts==0 && !hit,"explicit chase/shake/slave spatial operations have exact allocation terminal0");
}
void posesNegative(){
  NativeCamera::State state;NativeCamera camera;NativeCamera::Pose pose;pose.eye={10,20,300};camera.configurePose(state,pose);
  const auto accepted=camera.pose();const auto end=camera.centerRay()[1];
  for(unsigned failure=0;failure<5;++failure){auto bad=pose;
    if(failure==0)bad.eye.x=std::numeric_limits<Real>::quiet_NaN();
    if(failure==1)bad.right.x=std::numeric_limits<Real>::infinity();
    if(failure==2)bad.up=bad.right;
    if(failure==3)bad.backward={};
    if(failure==4)bad.eye.z=std::numeric_limits<Real>::infinity();
    rejects([&]{camera.configurePose(state,bad);});
    require(camera.eye().x==accepted.eye.x && camera.eye().z==accepted.eye.z && camera.centerRay()[1].z==end.z,
      "malformed bone/chase pose rejection preserves complete accepted spatial backing");
    camera.configurePose(state,pose);require(camera.pose().right.x==1,"each rejection permits corrected same-owner pose retry");
  }
  auto badState=state;badState.width=0;rejects([&]{camera.configurePose(badState,pose);});
  badState=state;badState.fieldOfView=PI;rejects([&]{camera.configurePose(badState,pose);});
  const auto coincident=NativeCamera::lookAtPose(pose.eye,pose.eye);
  require(coincident.right.y==-1 && coincident.up.z==1 && coincident.backward.x==-1,
    "source-defined coincident Look_At preserves default +X facing, not invented rejection");
  rejects([&]{NativeCamera::rotateLocal(pose,{0,std::numeric_limits<Real>::quiet_NaN(),0});});
  NativeCamera empty;rejects([&]{empty.pose();});
  Region3D extent{{0,0,0},{500,500,20}};
  rejects([&]{empty.constraintRegion(extent,10,FALSE);});
  auto malformed=extent;malformed.lo.x=501;rejects([&]{camera.constraintRegion(malformed,10,FALSE);});
  malformed=extent;malformed.hi.y=std::numeric_limits<Real>::quiet_NaN();rejects([&]{camera.constraintRegion(malformed,10,FALSE);});
  rejects([&]{camera.constraintRegion(extent,std::numeric_limits<Real>::infinity(),FALSE);});
  Region2D badBounds{{0,0},{std::numeric_limits<Real>::quiet_NaN(),10}};
  rejects([&]{NativeCamera::clipPosition({0,0,0},badBounds);});
  auto hugeOrigin=state;hugeOrigin.originX=std::numeric_limits<Int>::max();camera.configurePose(hugeOrigin,pose);
  rejects([&]{camera.constraintRegion(extent,10,FALSE);});
  camera.configurePose(state,pose);camera.constraintRegion(extent,10,FALSE);
  require(camera.eye().z==accepted.eye.z,"invalid construction/rotation/state does not mutate existing owner");
}
void shaker(){
  // Independent original-formula oracle and RNG cardinality. One source
  // record, not a replacement implementation injected into a camera provider.
  const auto random=[](Real lo,Real hi){return (Real(std::rand()&0xFFF)/Real(0xFFF))*(hi-lo)+lo;};
  constexpr Real dt=1.0f/30;
  const Real minOmega=DEG_TO_RADF(12.5f*360),maxOmega=DEG_TO_RADF(15.0f*360),endOmega=DEG_TO_RADF(360);
  std::srand(123);Real omega[3],phase[3];
  for(auto& value:omega)value=random(minOmega,maxOmega);for(auto& value:phase)value=random(0,DEG_TO_RADF(360));
  Real expected[3]{};const Real axis[3]={DEG_TO_RADF(7.5f),DEG_TO_RADF(15),DEG_TO_RADF(5)};
  const Real intensity=(10*PI/180.0f)*(1-dt/2);
  for(unsigned n=0;n<3;++n){const Real frequency=omega[n]+(endOmega-omega[n])*dt;
    expected[n]+=axis[n]*intensity*std::sin(frequency*dt+phase[n]);
    for(auto& value:expected)value+=random(-intensity*.5f,intensity*.5f);}
  const int expectedNext=std::rand();
  std::srand(123);NativeCameraShaker effects;effects.add({0,0,0},100,2,10);effects.timestep(dt);
  const auto actual=effects.angles({0,0,0});
  require(close(actual.x,expected[0]) && close(actual.y,expected[1]) && close(actual.z,expected[2]) && std::rand()==expectedNext,
    "owned C3 shaker uses original six admission/nine secondary draws and sinusoid/degree/time formulas");
  std::srand(17);const int first=std::rand();std::srand(17);const auto outside=effects.angles({1000,0,0});
  require(outside.x==0 && outside.y==0 && outside.z==0 && std::rand()==first,"outside-radius query neither contributes angles nor consumes visual RNG");
  AllocationFault::arm(0);try{effects.timestep(2);}catch(...){AllocationFault::disarm();throw;}
  const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
  require(attempts==0 && !hit && !effects.active() && effects.count()==0,"expiry retires records without fallible temporary-list acquisition");
  effects.add({0,0,0},100,.1f,1);effects.add({0,0,0},100,.2f,1);effects.timestep(.1f);
  require(effects.count()==1,"exact duration endpoint retires only the expired record");effects.timestep(.1f);
  require(!effects.active(),"whole C3 cohort expires exactly");
}
void shakerOwnership(){
  for(unsigned ordinal=0;ordinal<=1;++ordinal){const auto live=AllocationFault::live();
    {NativeCameraShaker effects;std::srand(31);const int first=std::rand();std::srand(31);
      AllocationFault::arm(ordinal);bool failed=false;try{effects.add({0,0,0},100,1,10);}catch(const std::bad_alloc&){failed=true;}
      const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
      require(failed==(ordinal==0) && hit==failed && attempts==1,"exact C3 admission allocation failure/terminal pair");
      if(failed)require(!effects.active() && AllocationFault::live()==live && std::rand()==first,
        "failed admission preserves owner and consumes no visual random draws");
      std::srand(31);effects.add({0,0,0},100,1,10);require(effects.active(),"each admission failure has a successful same-owner retry");}
    require(AllocationFault::live()==live,"whole root-owned C3 capacity retires exactly");
  }
  NativeCameraShaker effects;effects.add({0,0,0},100,1,10);
  for(unsigned invalid=0;invalid<6;++invalid){Real radius=100,duration=1,power=10;Coord3D point{};
    if(invalid==0)radius=0;if(invalid==1)duration=-1;if(invalid==2)power=-1;
    if(invalid==3)point.x=std::numeric_limits<Real>::quiet_NaN();
    if(invalid==4)radius=std::numeric_limits<Real>::max();if(invalid==5)duration=std::numeric_limits<Real>::max();
    const auto count=effects.count();std::srand(29);const int first=std::rand();std::srand(29);
    rejects([&]{effects.add(point,radius,duration,power);});
    require(effects.count()==count && std::rand()==first,"malformed C3 admission preserves existing cohort and visual RNG");
    effects.add({0,0,0},100,1,10);require(effects.count()==count+1,"corrected C3 admission retry preserves prior records");
  }
  const auto count=effects.count();rejects([&]{effects.timestep(-1);});rejects([&]{effects.angles({0,std::numeric_limits<Real>::infinity(),0});});
  require(effects.count()==count,"invalid timestep/eye query preserves active cohort");
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
      // Warm the identical complete spatial/parser cohort, not just its old
      // camera/waypoint parents. Name metadata retires with the whole manager.
      spatial();
      for(unsigned repeat=0;repeat<3;++repeat){const auto live=AllocationFault::live();const auto views=View::liveViewCount();
        const std::string_view family=argv[1];if(family=="modes")modes(parents);else if(family=="paths")paths(parents);else if(family=="negative")negative();else if(family=="ownership")ownership();
        else if(family=="spatial")spatial();else if(family=="spatial-negative")spatialNegative();else if(family=="spatial-ownership")spatialOwnership();
        else if(family=="navigation")navigation(parents);else if(family=="navigation-negative")navigationNegative();
        else if(family=="poses")poses();else if(family=="poses-negative")posesNegative();
        else if(family=="shaker")shaker();else if(family=="shaker-ownership")shakerOwnership();else require(false,"unknown family");
        require(AllocationFault::live()==live && View::liveViewCount()==views && TheMemoryPoolFactory->findMemoryPool("Waypoint")->getUsedBlockCount()==0,"repeated camera/View registration and actual waypoint owners retire exactly");}}
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole memory manager retirement");
    std::cout<<"PASS original camera choreography; native UI/root/output still pending\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}catch(ErrorCode error){std::cerr<<"FAIL camera "<<Int(error)<<'\n';}
  if(initialized)shutdownMemoryManager();return 1;
}
