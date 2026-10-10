// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc. Original Drawable logic-query adaptation.
#include "PreRTS.h"
#include "GameClient/NativeModelDrawQueries.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/FPUControl.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include <optional>
#include <algorithm>
#include <cmath>
#include <cstring>

NativeModelQueryTransaction::NativeModelQueryTransaction(NativeW3DAssets& owner)
  :m_keys(*TheNameKeyGenerator),m_admission(owner),assets(owner){
  if(current)throw ERROR_BAD_ARG;
  current=this;
}
NativeModelQueryTransaction::~NativeModelQueryTransaction() noexcept {
  if(!m_committed)for(auto it=m_previous.rbegin();it!=m_previous.rend();++it){
    it->module->m_cache=std::move(it->cache);
    it->module->m_attachValid=it->attachValid;
    it->module->m_attachOffset=it->offset;
  }
  current=nullptr;
}
void NativeModelQueryTransaction::touch(const NativeModelDrawQueries& module){
  for(const auto& prior:m_previous)if(prior.module==&module)return;
  m_previous.push_back({&module,module.m_cache,module.m_attachValid,module.m_attachOffset});
}
void NativeModelQueryTransaction::commit() noexcept {m_admission.commit();m_keys.commit();m_committed=true;}
thread_local NativeModelQueryTransaction* NativeModelQueryTransaction::current=nullptr;

NativeModelDrawQueryDispatch::NativeModelDrawQueryDispatch(const Drawable& draw){
  const DrawModule* const* raw=const_cast<Drawable&>(draw).getDrawModulesNonDirty();
  NativeW3DAssets* owner=nullptr;
  for(auto modules=raw;*modules;++modules){
    if(const auto* native=dynamic_cast<const NativeModelDrawQueries*>(*modules)){
      native->requireLogicPhase();m_active=true;
      if(owner&&owner!=&native->m_assets)throw ERROR_BAD_ARG;
      owner=&native->m_assets;
    }
  }
  if(owner){
      m_conditions.emplace(raw);
      if(!NativeModelQueryTransaction::current)m_transaction.emplace(*owner);
      else if(&NativeModelQueryTransaction::current->assets!=owner)throw ERROR_BAD_ARG;
  }
  (void)draw.getDrawModules(); // Dirty publication participates in this root.
}

namespace {
Thing* drawableThing(Thing* thing){if(!thing || !dynamic_cast<Drawable*>(thing))throw ERROR_BAD_ARG;return thing;}
const W3DModelDrawModuleData& definition(const ModuleData* data){
  auto* draw=data?data->getAsW3DModelDrawModuleData():nullptr;if(!draw)throw ERROR_BAD_ARG;return *draw;
}
void begin(std::optional<NativeModelQueryTransaction>& transaction,NativeW3DAssets& assets){
  if(!NativeModelQueryTransaction::current)transaction.emplace(assets);
  else if(&NativeModelQueryTransaction::current->assets!=&assets)throw ERROR_BAD_ARG;
}
void publish(std::optional<NativeModelQueryTransaction>& transaction) noexcept {if(transaction)transaction->commit();}
void prefixInput(const char* prefix,Int start){
  if(!prefix || std::strlen(prefix)>255 || start<0 || start>99)throw ERROR_BAD_ARG;
}
}

NativeModelDrawQueries::NativeModelDrawQueries(Thing* thing,const ModuleData* data,NativeW3DAssets& assets)
  :DrawModule(drawableThing(thing),data),m_assets(assets),m_data(definition(data)),m_names(TheNameKeyGenerator),m_thread(std::this_thread::get_id()){
  if(!m_names || !TheGlobalData || &assets.files()!=TheFileSystem)throw ERROR_BAD_ARG;
}
NativeModelDrawQueries::~NativeModelDrawQueries()=default;

void NativeModelDrawQueries::requireNativeParents() const {
  // Reject workers before inspecting mutable simulation publications.
  if(std::this_thread::get_id()!=m_thread)throw ERROR_BAD_ARG;
  if(TheNameKeyGenerator!=m_names || !TheGlobalData || &m_assets.files()!=TheFileSystem)throw ERROR_BAD_ARG;
}
void NativeModelDrawQueries::requireLogicPhase() const {
  requireNativeParents();
  if(!((TheGameLogic && TheGameLogic->isInGameLogicUpdate()) || (TheGameState && TheGameState->isInLoadGame())))throw ERROR_BAD_ARG;
  setFPMode();
}
const ModelConditionInfo* NativeModelDrawQueries::prepare(const ModelConditionFlags& conditions) const {
  const auto* selected=m_data.findBestInfoUncached(conditions);
  if(!selected)return nullptr;
  const Real scale=getDrawable()->getScale();
  if(!std::isfinite(scale))throw ERROR_BAD_ARG;
  if(m_cache && m_cache->source==selected && m_cache->configuration==TheGlobalData && m_cache->scale==scale)return m_cache->query.condition.get();
  auto query=selected->prepareNativeQueries(m_assets,scale,m_data.m_extraPublicBones,TheGlobalData->m_standardPublicBones);
  auto candidate=std::make_shared<Cache>(Cache{selected,TheGlobalData,scale,std::move(query)});
  NativeModelQueryTransaction::current->touch(*this);
  m_cache=std::move(candidate);m_attachValid=FALSE;m_attachOffset.Set(0,0,0);
  return m_cache->query.condition.get();
}

Int NativeModelDrawQueries::getPristineBonePositionsForConditionState(const ModelConditionFlags& conditions,
    const char* prefix,Int start,Coord3D* positions,Matrix3D* transforms,Int capacity) const {
  if(capacity<=0)return 0;
  prefixInput(prefix,start);requireLogicPhase();
  std::optional<NativeModelQueryTransaction> transaction;begin(transaction,m_assets);
  const auto* data=prepare(conditions);if(!data){publish(transaction);m_lastPristineFallback=FALSE;return 0;}
  const Int maximum=std::min(capacity,64),end=start==0?0:99;
  Matrix3D staged[64];Int count=0;bool missing=false;
  char name[258];const auto prefixLength=std::strlen(prefix);
  for(Int index=start;index<=end && count<maximum;++index){
    std::memcpy(name,prefix,prefixLength);std::size_t length=prefixLength;
    if(index){name[length++]=char('0'+index/10);name[length++]=char('0'+index%10);}name[length]='\0';
    for(std::size_t i=0;i<length;++i)if(name[i]>='A' && name[i]<='Z')name[i]+=char('a'-'A');
    const auto* matrix=data->findPristineBone(NAMEKEY(name),nullptr);
    if(!matrix){
      const auto* actor=getDrawable()->getObject();
      if(actor)staged[count]=*actor->getTransformMatrix();else staged[count].Make_Identity();
      missing=true;break;
    }
    staged[count++]=*matrix;
  }
  publish(transaction);
  for(Int index=0;index<count;++index){
    if(transforms)transforms[index]=staged[index];
    if(positions){const auto point=staged[index].Get_Translation();positions[index]={point.X,point.Y,point.Z};}
  }
  // Original pristine sweep writes a defined fallback transform on its first
  // miss but does not count it, or publish an accompanying position.
  if(missing && transforms)transforms[count]=staged[count];
  m_lastPristineFallback=missing;
  return count;
}

Bool NativeModelDrawQueries::getProjectileLaunchOffset(const ModelConditionFlags& conditions,
    WeaponSlotType slot,Int barrel,Matrix3D* launch,WhichTurretType turret,Coord3D* rotation,Coord3D* pitch) const {
  if(static_cast<UnsignedInt>(slot)>=WEAPONSLOT_COUNT || turret<TURRET_INVALID || turret>=MAX_TURRETS)throw ERROR_BAD_ARG;
  requireLogicPhase();std::optional<NativeModelQueryTransaction> transaction;begin(transaction,m_assets);
  const auto* data=prepare(conditions);if(!data){publish(transaction);m_lastProjectileOutput=FALSE;return FALSE;}
  // Attachment recursion may replace this module's one-entry cache with the
  // Drawable's different condition. Keep the selected candidate alive locally.
  const auto retained=m_cache;
  Matrix3D stagedLaunch(true);Coord3D stagedRotation{0,0,0},stagedPitch{0,0,0};
  const auto& barrels=data->m_weaponBarrelInfoVec[slot];const bool valid=launch && !barrels.empty();
  if(valid){if(barrel<0 || std::size_t(barrel)>=barrels.size())barrel=0;
    stagedLaunch=barrels[barrel].m_projectileOffsetMtx;
    if(turret!=TURRET_INVALID){stagedLaunch.Pre_Rotate_Z(data->m_turrets[turret].m_turretArtAngle);stagedLaunch.Pre_Rotate_Y(-data->m_turrets[turret].m_turretArtPitch);}
  }
  if(turret!=TURRET_INVALID){
    if(!m_attachValid){Vector3 offset(0,0,0);
      if(m_data.m_attachToDrawableBone.isNotEmpty()){
        Matrix3D matrix(true);
        if(getDrawable()->getPristineBonePositions(m_data.m_attachToDrawableBone.str(),0,nullptr,&matrix,1)==1)offset=matrix.Get_Translation();
      }
      NativeModelQueryTransaction::current->touch(*this);m_attachOffset=offset;m_attachValid=TRUE;
    }
    const auto& info=data->m_turrets[turret];
    if(rotation){data->findPristineBonePos(info.m_turretAngleNameKey,stagedRotation);stagedRotation.x+=m_attachOffset.X;stagedRotation.y+=m_attachOffset.Y;stagedRotation.z+=m_attachOffset.Z;}
    if(pitch){data->findPristineBonePos(info.m_turretPitchNameKey,stagedPitch);stagedPitch.x+=m_attachOffset.X;stagedPitch.y+=m_attachOffset.Y;stagedPitch.z+=m_attachOffset.Z;}
  }
  publish(transaction);if(valid)*launch=stagedLaunch;if(rotation)*rotation=stagedRotation;if(pitch)*pitch=stagedPitch;
  m_lastProjectileOutput=TRUE;return valid;
}
