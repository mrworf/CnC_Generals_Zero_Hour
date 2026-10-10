// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeModelDrawCurrentQueries.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "WWMath/obbox.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
NativeW3DPose::Matrix raw(const Matrix3D& matrix){NativeW3DPose::Matrix out;
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col){
    const auto value=matrix[row][col];if(!std::isfinite(value))throw ERROR_BAD_ARG;out[row*4+col]=value;
  }return out;
}
Matrix3D matrix(const NativeW3DPose::Matrix& values){Matrix3D out;
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<4;++col)out[row][col]=values[row*4+col];return out;
}
Matrix3D orthogonalInverse(const Matrix3D& world){
  // Original matrix3d.cpp::Get_Orthogonal_Inverse: transpose R, then -R'T.
  // That out-of-line legacy object also contains platform assembly; use the
  // already-shared inline math without importing its unported object file.
  Matrix3D inverse(true);
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)inverse[row][col]=world[col][row];
  Vector3 translation;Matrix3D::Rotate_Vector(inverse,world.Get_Translation(),&translation);
  inverse.Set_Translation(-translation);return inverse;
}
void nameInput(const char* name){if(!name || std::strlen(name)>255)throw ERROR_BAD_ARG;}
void evaluate(NativeW3DAssets::Binding& binding,const NativeW3DPose::Matrix& root,Real frame,Real scale){
  if(!std::isfinite(frame))throw ERROR_BAD_ARG;
  if(binding.model->hierarchy)binding.pose.evaluateRuntime(root,binding.animation?binding.animation->motion.get():nullptr,frame,scale);
}
}
NativeModelDrawCurrentQueries::NativeModelDrawCurrentQueries(Thing* thing,const ModuleData* data,NativeW3DAssets& assets)
  :NativeModelDrawQueries(thing,data,assets),m_client(TheGameClient){if(!m_client)throw ERROR_BAD_ARG;}
NativeModelDrawCurrentQueries::~NativeModelDrawCurrentQueries()=default;
void NativeModelDrawCurrentQueries::requireClientOwner()const{
  requireNativeParents();
  if(TheGameClient!=m_client)throw ERROR_BAD_ARG;
}
void NativeModelDrawCurrentQueries::requireClientPhase()const{
  requireClientOwner();if(TheGameLogic && TheGameLogic->isInGameLogicUpdate())throw ERROR_BAD_ARG;
}
void NativeModelDrawCurrentQueries::replaceNativeCurrentModel(std::string_view model,std::string_view animation,
                                                            const Matrix3D& world,Real frame){
  // Original Drawable condition notifications can originate in GameLogic.
  // They publish client-owned state; they do not read live client poses into it.
  requireClientOwner();const auto root=raw(world);if(!std::isfinite(frame))throw ERROR_BAD_ARG;
  if(model.empty()){m_current.reset();return;}
  const Real scale=getDrawable()->getScale();if(!std::isfinite(scale))throw ERROR_BAD_ARG;
  const Real treeScale=std::abs(scale-1)>.01f?scale:1;
  NativeW3DAssets::Transaction admission(nativeAssets());
  auto binding=nativeAssets().bind(model,animation,false,root,treeScale);
  evaluate(binding,root,frame,treeScale);
  auto bounds=NativeW3DModelBounds::decode(binding.model,treeScale);
  auto subobjects=NativeW3DSubobjects::admit(binding.model);
  auto candidate=std::make_unique<Current>(Current{std::move(binding),world,treeScale,frame,std::move(bounds),1,{},std::move(subobjects)});
  m_current=std::move(candidate);admission.commit();
}
void NativeModelDrawCurrentQueries::updateNativeCurrentPose(const Matrix3D& world,Real frame){
  requireClientPhase();const auto root=raw(world);if(!std::isfinite(frame))throw ERROR_BAD_ARG;
  if(!m_current)return;
  evaluate(m_current->binding,root,frame,m_current->treeScale);
  m_current->world=world; // Whole pose commits before its corresponding root.
  m_current->frame=frame;
  m_current->clock.reset(); // Explicit frame publication becomes manual state.
}
void NativeModelDrawCurrentQueries::replaceNativeTimedModel(std::string_view model,std::string_view animation,
    const Matrix3D& world,Real frame,NativeModelAnimationClock::Mode mode,std::uint32_t syncTime){
  requireClientOwner();const auto root=raw(world);
  const Real scale=getDrawable()->getScale();if(!std::isfinite(scale))throw ERROR_BAD_ARG;
  const Real treeScale=std::abs(scale-1)>.01f?scale:1;
  NativeW3DAssets::Transaction admission(nativeAssets());
  auto binding=nativeAssets().bind(model,animation,false,root,treeScale);
  if(!binding.animation)throw ERROR_BAD_ARG;
  auto clock=NativeModelAnimationClock::start(binding.animation->motion->frames(),
    float(binding.animation->motion->frameRate()),frame,mode,syncTime,m_current?m_current->frame:0);
  evaluate(binding,root,frame,treeScale);
  auto bounds=NativeW3DModelBounds::decode(binding.model,treeScale);
  auto subobjects=NativeW3DSubobjects::admit(binding.model);
  auto candidate=std::make_unique<Current>(Current{std::move(binding),world,treeScale,frame,std::move(bounds),1,clock,std::move(subobjects)});
  m_current=std::move(candidate);admission.commit();
}
void NativeModelDrawCurrentQueries::advanceNativeCurrentPose(const Matrix3D& world,std::uint32_t syncTime){
  requireClientPhase();const auto root=raw(world);if(!m_current || !m_current->clock)return;
  auto clock=m_current->clock->advanced(syncTime);
  evaluate(m_current->binding,root,clock.frame(),m_current->treeScale);
  m_current->world=world;m_current->frame=clock.frame();m_current->clock=clock;
}
void NativeModelDrawCurrentQueries::setNativeAnimationMultiplier(Real value){
  requireClientOwner();if(!std::isfinite(value))throw ERROR_BAD_ARG;
  if(m_current && m_current->clock)m_current->clock->setMultiplier(value);
}
void NativeModelDrawCurrentQueries::setNativeCurrentObjectScale(Real value){
  requireClientOwner();if(!std::isfinite(value)||value<0)throw ERROR_BAD_ARG;
  if(m_current)m_current->objectScale=value;
}
Bool NativeModelDrawCurrentQueries::clientOnly_getRenderObjInfo(Coord3D* position,Real* radius,Matrix3D* transform)const{
  requireClientPhase();if(!position||!radius||!transform)throw ERROR_BAD_ARG;if(!m_current)return FALSE;
  const auto world=raw(m_current->world);const auto r=m_current->bounds.worldSphereRadius(m_current->binding.pose,world,m_current->objectScale);
  const auto p=m_current->world.Get_Translation();*position={p.X,p.Y,p.Z};*radius=r;*transform=m_current->world;return TRUE;
}
Bool NativeModelDrawCurrentQueries::clientOnly_getRenderObjBoundBox(OBBoxClass* output)const{
  requireClientPhase();if(!output)throw ERROR_BAD_ARG;if(!m_current)return FALSE;
  const auto world=raw(m_current->world);const auto box=m_current->bounds.objectBox(m_current->binding.pose,world);
  // Source OBBox::Transform preserves extents and multiplies the identity basis.
  OBBoxClass result;
  result.Extent=Vector3(box.extent[0],box.extent[1],box.extent[2]);
  Matrix3D::Transform_Vector(m_current->world,Vector3(box.center[0],box.center[1],box.center[2]),&result.Center);
  for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)result.Basis[row][col]=m_current->world[row][col];
  for(unsigned n=0;n<3;++n){if(!std::isfinite(result.Center[n])||!std::isfinite(result.Extent[n]))throw ERROR_BAD_ARG;
    for(unsigned k=0;k<3;++k)if(!std::isfinite(result.Basis[n][k]))throw ERROR_BAD_ARG;}
  *output=result;return TRUE;
}
Bool NativeModelDrawCurrentQueries::clientOnly_getRenderObjBoneTransform(const AsciiString& name,Matrix3D* output)const{
  requireClientPhase();if(!output)throw ERROR_BAD_ARG;nameInput(name.str());
  if(!m_current)return FALSE;
  const auto index=m_current->binding.pose.sourceBoneIndex(name.str());
  if(!index){output->Make_Identity();return FALSE;}
  *output=matrix(m_current->binding.pose.bone(index).transform);return TRUE;
}
Bool NativeModelDrawCurrentQueries::getCurrentWorldspaceClientBonePositions(const char* name,Matrix3D& output)const{
  requireClientPhase();nameInput(name);if(!m_current)return FALSE;
  const auto index=m_current->binding.pose.sourceBoneIndex(name);if(!index)return FALSE;
  output=matrix(m_current->binding.pose.bone(index).transform);return TRUE;
}
Int NativeModelDrawCurrentQueries::getCurrentBonePositions(const char* prefix,Int start,Coord3D* positions,Matrix3D* transforms,Int capacity)const{
  if(capacity<=0)return 0;
  requireClientPhase();nameInput(prefix);if(start<0 || start>99)throw ERROR_BAD_ARG;if(!m_current)return 0;
  Matrix3D inverse=orthogonalInverse(m_current->world);inverse.Scale(getDrawable()->getScale());(void)raw(inverse);
  Matrix3D staged[64];Int count=0;const Int maximum=std::min(capacity,64),end=start==0?0:99;
  const auto length=std::strlen(prefix);char name[258];
  for(Int index=start;index<=end && count<maximum;++index){
    std::memcpy(name,prefix,length);std::size_t used=length;
    if(index){name[used++]=char('0'+index/10);name[used++]=char('0'+index%10);}name[used]='\0';
    const auto bone=m_current->binding.pose.sourceBoneIndex(name);if(!bone)break;
    staged[count]=matrix(m_current->binding.pose.bone(bone).transform);staged[count].preMul(inverse);(void)raw(staged[count]);++count;
  }
  for(Int i=0;i<count;++i){if(transforms)transforms[i]=staged[i];
    if(positions){const auto p=staged[i].Get_Translation();positions[i]={p.X,p.Y,p.Z};}
  }return count;
}
