// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeModelDrawAnimationProducer.h"
#include "GameClient/NativeModelDrawConditionBatch.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/Display.h"
#include "GameClient/Drawable.h"
#include "Common/GlobalData.h"
#include "Common/NativeSourceMath.h"
#include "GameClient/NativeParticleSystemAdmission.h"
#include "GameClient/ParticleSys.h"
#include <cmath>
#include <limits>

namespace {
NativeW3DPose::Matrix raw(const Matrix3D& value){NativeW3DPose::Matrix out;
  for(unsigned n=0;n<3;++n)for(unsigned k=0;k<4;++k){
    if(!std::isfinite(value[n][k]))throw ERROR_BAD_ARG;
    out[4*n+k]=value[n][k];
  }return out;
}
bool has(Int flags,ACBits bit){return (flags&(1<<bit))!=0;}
void applySubobjects(NativeW3DSubobjects& target,const std::vector<ModelConditionInfo::HideShowSubObjInfo>& rules){
  // Validate the complete list before a retained current model is changed.
  for(const auto& rule:rules)(void)target.find(rule.subObjName.str());
  for(const auto& rule:rules){const NativeW3DSubobjects::Rule value{rule.subObjName.str(),bool(rule.hide)};target.apply(std::span(&value,1));}
}
}
struct NativeModelDrawAnimationProducer::ParticleCohort {
  struct Tracker {ParticleSystemID id;std::size_t bone;};
  ParticleSystemManager* owner;
  std::vector<Tracker> trackers;
  explicit ParticleCohort(ParticleSystemManager& value):owner(&value){}
  ~ParticleCohort()noexcept{
    if(trackers.empty())return;
    if(TheParticleSystemManager!=owner)std::terminate();
    for(const auto& tracker:trackers)if(auto* system=owner->findParticleSystem(tracker.id))system->destroy();
  }
};
NativeModelDrawAnimationProducer::NativeModelDrawAnimationProducer(Thing* thing,const ModuleData* data,NativeW3DAssets& assets)
  :NativeModelDrawCurrentQueries(thing,data,assets){}
NativeModelDrawAnimationProducer::~NativeModelDrawAnimationProducer()=default;
UnsignedInt NativeModelDrawAnimationProducer::syncTime()const{
  requireClientOwner();if(!m_display || TheDisplay!=m_display)throw ERROR_BAD_ARG;
  return m_display->getAnimationSyncTime();
}
bool NativeModelDrawAnimationProducer::complete()const noexcept{
  return !m_current || m_current->binding.model->kind!=NativeW3DAssets::Prototype::Kind::Hlod ||
    (m_current->clock && m_current->clock->complete());
}
Real NativeModelDrawAnimationProducer::fraction(const State& state,const Current* current){
  if(!state.source || !(state.source->m_flags&W3D_ALL_MAINTAIN_FRAME_FLAGS) ||
     !current || current->binding.model->kind!=NativeW3DAssets::Prototype::Kind::Hlod || !current->clock)return -1;
  const auto frames=current->binding.animation->motion->frames();
  if(current->frame<0)return 0;
  if(current->frame>=float(frames))return 1;
  if(frames==1)return std::numeric_limits<Real>::quiet_NaN(); // Source0/0 suppresses maintain.
  return current->frame/float(frames-1);
}
NativeModelDrawAnimationProducer::Candidate NativeModelDrawAnimationProducer::cloneCandidate()const{
  Candidate out;out.state=m_state;
  if(m_current){
    auto binding=NativeW3DAssets::Binding{m_current->binding.model,m_current->binding.animation,m_current->binding.pose.clone()};
    out.current=std::make_unique<Current>(Current{std::move(binding),m_current->world,m_current->treeScale,
      m_current->frame,m_current->bounds,m_current->objectScale,m_current->clock,m_current->subobjects.clone()});
  }
  if(m_validated)out.validated=std::make_unique<ModelConditionInfo>(*m_validated);
  return out;
}
void NativeModelDrawAnimationProducer::selectAnimation(Candidate& candidate,const ModelConditionInfo* previous,
                                                       Real previousFraction,UnsignedInt now)const{
  auto& state=candidate.state;const auto& source=*state.source;
  if(source.m_animations.empty()){state.animation=-1;return;}
  const auto count=source.m_animations.size();if(count>std::size_t(INT32_MAX))throw ERROR_BAD_ARG;
  if(count==1)state.animation=0;
  else if(previous==state.source){const auto avoid=state.animation;
    do {state.animation=GameClientRandomValue(0,Int(count-1));}while(state.animation==avoid);
  }else state.animation=GameClientRandomValue(0,Int(count-1));
  const auto clip=nativeAssets().animation(source.m_animations[state.animation].getName().str());
  if(!candidate.current)return;
  const auto frames=clip->motion->frames();if(frames>std::uint32_t(INT32_MAX))throw ERROR_BAD_ARG;
  auto mode=static_cast<Mode>(std::uint32_t(source.m_mode));
  Int frame=mode==Mode::OnceBackwards || mode==Mode::LoopBackwards?Int(frames-1):0;
  if(has(source.m_flags,RANDOMIZE_START_FRAME))frame=GameClientRandomValue(0,Int(frames-1));
  else if(has(source.m_flags,START_FRAME_FIRST))frame=0;
  else if(has(source.m_flags,START_FRAME_LAST))frame=Int(frames-1);
  else if(previous && previous!=state.source && (source.m_flags&previous->m_flags&W3D_ALL_MAINTAIN_FRAME_FLAGS) && previousFraction>=0)
    frame=REAL_TO_INT(previousFraction*float(frames)-1);
  // Source selects RANDOMSTART for any existing render object, even when its
  // leaf Set_Animation ignores playback. Only HLOD consumes the rate draw.
  if(candidate.current->binding.model->kind!=NativeW3DAssets::Prototype::Kind::Hlod)return;
  auto& current=*candidate.current;
  auto clock=NativeModelAnimationClock::start(frames,float(clip->motion->frameRate()),float(frame),mode,now,current.frame);
  current.binding.pose.evaluateRuntime(raw(current.world),clip->motion.get(),float(frame),current.treeScale);
  clock.setMultiplier(GameClientRandomValueReal(source.m_animMinSpeedFactor,source.m_animMaxSpeedFactor));
  current.binding.animation=clip;current.clock=clock;current.frame=float(frame);
}
NativeModelDrawAnimationProducer::Candidate NativeModelDrawAnimationProducer::prepareState(
    const ModelConditionInfo* requested,UnsignedInt now,bool skipTransition)const{
  if(!requested)throw ERROR_BAD_ARG;
  const auto* selected=requested;const ModelConditionInfo* pending=nullptr;
  if(!skipTransition && m_state.source && requested!=m_state.source &&
      m_state.source->m_transitionKey!=NAMEKEY_INVALID && requested->m_transitionKey!=NAMEKEY_INVALID){
    const auto& transitions=nativeModelDrawData().m_transitionMap;
    auto found=transitions.find(buildTransitionSig(m_state.source->m_transitionKey,requested->m_transitionKey));
    if(found!=transitions.end()){pending=requested;selected=&found->second;}
  }
  const auto model=selected->m_modelName.isEmpty() || selected->m_modelName.isNone()?
    NativeW3DAssets::Model{}:nativeAssets().model(selected->m_modelName.str());
  ModelConditionInfo querySource=*selected;
  // Leaf Set_Animation is a source no-op rather than hierarchy binding.
  if(!model || !model->hierarchy)querySource.m_animations.clear();
  auto query=querySource.prepareNativeQueries(nativeAssets(),getDrawable()->getScale(),
    nativeModelDrawData().m_extraPublicBones,TheGlobalData->m_standardPublicBones);
  bool recreate=!m_state.source || selected->m_modelName!=m_state.source->m_modelName || !m_validated;
  if(!recreate)for(unsigned n=0;n<MAX_TURRETS;++n)
    if(query.condition->m_turrets[n].m_turretAngleNameKey!=m_validated->m_turrets[n].m_turretAngleNameKey ||
       query.condition->m_turrets[n].m_turretPitchNameKey!=m_validated->m_turrets[n].m_turretPitchNameKey)recreate=true;
  Candidate out;
  if(!recreate)out=cloneCandidate();
  else if(query.binding.model){
    const Matrix3D world=m_current?m_current->world:*getDrawable()->getTransformMatrix();
    const auto root=raw(world); // Also stage first leaf roots before publication.
    const Real scale=getDrawable()->getScale(),treeScale=std::abs(scale-1)>.01f?scale:1;
    auto bounds=NativeW3DModelBounds::decode(query.binding.model,treeScale);
    auto subobjects=NativeW3DSubobjects::admit(query.binding.model);
    if(getDrawable()->isDrawableEffectivelyHidden())subobjects.setHidden(true);
    query.binding.animation.reset(); // Pristine first clip is not the selected current clip.
    if(query.binding.model->hierarchy)query.binding.pose.evaluateRuntime(root,nullptr,0,treeScale);
    out.current=std::make_unique<Current>(Current{std::move(query.binding),world,treeScale,0,std::move(bounds),1,{},std::move(subobjects)});
  }
  out.validated=std::move(query.condition);out.state=m_state;
  if(recreate)out.state.paused=FALSE; // Source nukeCurrentRender lifecycle.
  out.state.source=selected;out.state.pending=pending;out.state.pendingDuration=~UnsignedInt(0);
  // Every caller admits a real model state, including a deferred target. Idle
  // animation reselection uses cloneCandidate/selectAnimation instead.
  out.state.particles.reset();out.state.recalcParticles=!getDrawable()->testDrawableStatus(DRAWABLE_STATUS_NO_STATE_PARTICLES);
  if(out.current){applySubobjects(out.current->subobjects,selected->m_hideShowVec);applySubobjects(out.current->subobjects,out.state.subobjectOverrides);}
  selectAnimation(out,m_state.source,fraction(m_state,m_current.get()),now);
  return out;
}
void NativeModelDrawAnimationProducer::publish(Candidate&& candidate)noexcept{
  NativeModelDrawConditionBatch::retainPrevious(*this);
  m_current=std::move(candidate.current);m_validated=std::move(candidate.validated);m_state=std::move(candidate.state);
}
void NativeModelDrawAnimationProducer::showSubObject(const AsciiString& name,Bool show){
  requireClientOwner();NativeModelDrawConditionBatch::validate(*this);
  if(name.isEmpty())return; // Source empty notification is a no-op.
  if(name.getLength()>255)throw ERROR_BAD_ARG;
  auto overrides=m_state.subobjectOverrides;bool found=false;
  for(auto& rule:overrides)if(rule.subObjName.compareNoCase(name)==0){rule.hide=!show;found=true;}
  if(!found)overrides.push_back({name,!show});
  m_state.subobjectOverrides=std::move(overrides); // Queue only, source applies later.
}
void NativeModelDrawAnimationProducer::updateSubObjects(){
  requireClientPhase();NativeModelDrawConditionBatch::validate(*this);
  if(!m_current||m_state.subobjectOverrides.empty())return;
  if(NativeModelDrawConditionBatch::needsCopy(*this)){
    auto candidate=cloneCandidate();applySubobjects(candidate.current->subobjects,candidate.state.subobjectOverrides);publish(std::move(candidate));
  }else applySubobjects(m_current->subobjects,m_state.subobjectOverrides);
}
std::size_t NativeModelDrawAnimationProducer::nativeStateParticleCount()const noexcept{
  return m_state.particles?m_state.particles->trackers.size():0;
}
UnsignedInt NativeModelDrawAnimationProducer::nativeStateParticleID(std::size_t index)const{
  if(index>=nativeStateParticleCount())throw ERROR_BAD_ARG;
  return UnsignedInt(m_state.particles->trackers[index].id);
}
void NativeModelDrawAnimationProducer::recalcNativeStateParticles(){
  requireClientPhase();NativeModelDrawConditionBatch::validate(*this);
  if(!m_state.recalcParticles)return;
  if(!m_state.source||getDrawable()->testDrawableStatus(DRAWABLE_STATUS_NO_STATE_PARTICLES)||m_state.source->m_particleSysBones.empty()){
    m_state.recalcParticles=FALSE;return;
  }
  if(!TheParticleSystemManager)throw ERROR_BAD_ARG;
  NativeModelDrawConditionBatch::enrollParticles(*TheParticleSystemManager);
  NativeParticleSystemAdmission admission(*TheParticleSystemManager);
  auto candidate=std::make_shared<ParticleCohort>(*TheParticleSystemManager);
  std::optional<NativeW3DPose> pose;
  if(m_current&&m_current->binding.model->hierarchy){
    pose.emplace(m_current->binding.pose.clone());Matrix3D root(true);root.Scale(getDrawable()->getScale());
    pose->evaluateRuntime(raw(root),m_current->binding.animation?m_current->binding.animation->motion.get():nullptr,m_current->frame,m_current->treeScale);
  }
  for(const auto& source:m_state.source->m_particleSysBones){
    auto* system=TheParticleSystemManager->createParticleSystem(source.particleSystemTemplate);
    if(!system)continue;
    const auto bone=pose?pose->sourceBoneIndex(source.boneName.str()):0;
    Coord3D position{0,0,0};Real rotation=0;
    if(bone){const auto& value=pose->bone(bone).transform;Matrix3D transform;
      for(unsigned n=0;n<3;++n)for(unsigned k=0;k<4;++k)transform[n][k]=value[4*n+k];
      const auto translation=transform.Get_Translation();position={translation.X,translation.Y,translation.Z};rotation=nativeSourceYaw(transform);
    }
    system->setPosition(&position);system->rotateLocalTransformZ(rotation);
    system->attachToDrawable(getDrawable());system->setSaveable(FALSE);
    if(getDrawable()->isDrawableEffectivelyHidden()||m_state.fullyObscured)system->stop();
    candidate->trackers.push_back({system->getSystemID(),bone});
  }
  m_state.particles=std::move(candidate);m_state.recalcParticles=FALSE;
  admission.commit();
}
Bool NativeModelDrawAnimationProducer::updateBonesForClientParticleSystems(){
  requireClientPhase();NativeModelDrawConditionBatch::validate(*this);
  if(!m_state.particles||!m_current||!m_state.source)return TRUE;
  auto& cohort=*m_state.particles;
  if(TheParticleSystemManager!=cohort.owner)throw ERROR_BAD_ARG;
  NativeModelDrawConditionBatch::enrollParticles(*cohort.owner);
  NativeParticleSystemAdmission admission(*cohort.owner);
  for(const auto& tracker:cohort.trackers){
    auto* system=cohort.owner->findParticleSystem(tracker.id);
    if(!system||!tracker.bone)continue;
    const auto& value=m_current->binding.pose.bone(tracker.bone).transform;Matrix3D transform;
    for(unsigned n=0;n<3;++n)for(unsigned k=0;k<4;++k)transform[n][k]=value[4*n+k];
    const auto translation=transform.Get_Translation();const Coord3D position{translation.X,translation.Y,translation.Z};
    system->setPosition(&position);system->rotateLocalTransformZ(nativeSourceYaw(transform));
    system->setLocalTransform(&transform);system->setSkipParentXfrm(TRUE);
  }
  admission.commit();return TRUE;
}
void NativeModelDrawAnimationProducer::applyParticleVisibility(Bool fullyObscured){
  requireClientOwner();NativeModelDrawConditionBatch::validate(*this);
  if(!m_state.particles){m_state.fullyObscured=fullyObscured;return;}
  auto& cohort=*m_state.particles;if(TheParticleSystemManager!=cohort.owner)throw ERROR_BAD_ARG;
  NativeModelDrawConditionBatch::enrollParticles(*cohort.owner);
  NativeParticleSystemAdmission admission(*cohort.owner);
  const bool hidden=getDrawable()->isDrawableEffectivelyHidden()||fullyObscured;
  for(const auto& tracker:cohort.trackers)if(auto* system=cohort.owner->findParticleSystem(tracker.id)){
    if(hidden)system->stop();else system->start();
  }
  m_state.fullyObscured=fullyObscured;admission.commit();
}
void NativeModelDrawAnimationProducer::setHidden(Bool hidden){
  requireClientOwner();NativeModelDrawConditionBatch::validate(*this);
  Candidate candidate;const bool copy=m_current&&NativeModelDrawConditionBatch::needsCopy(*this);
  if(copy){candidate=cloneCandidate();candidate.current->subobjects.setHidden(hidden);}
  applyParticleVisibility(m_state.fullyObscured);
  if(copy)publish(std::move(candidate));else if(m_current)m_current->subobjects.setHidden(hidden);
}
void NativeModelDrawAnimationProducer::setFullyObscuredByShroud(Bool value){applyParticleVisibility(value);}
void NativeModelDrawAnimationProducer::initializeNativeAnimationProducer(){
  requireClientOwner();if(m_display || m_state.source || !TheDisplay)throw ERROR_BAD_ARG;
  const auto now=TheDisplay->getAnimationSyncTime();
  const auto* initial=nativeModelDrawData().findBestInfoUncached(ModelConditionFlags{});
  NameKeyTransaction keys(*TheNameKeyGenerator);NativeW3DAssets::Transaction assets(nativeAssets());GameClientRandomTransaction random;
  auto candidate=prepareState(initial,now);publish(std::move(candidate));m_display=TheDisplay;
  assets.commit();keys.commit();random.commit();
}
void NativeModelDrawAnimationProducer::replaceModelConditionState(const ModelConditionFlags& conditions){
  NativeModelDrawConditionBatch::validate(*this);
  const auto now=syncTime();const auto* requested=nativeModelDrawData().findBestInfoUncached(conditions);
  if(!requested)return;
  if(m_state.source && ((requested==m_state.source&&!m_state.pending)||requested==m_state.pending))return;
  if(m_state.source && requested!=m_state.source && requested->m_allowToFinishKey!=NAMEKEY_INVALID &&
      requested->m_allowToFinishKey==m_state.source->m_transitionKey && !complete()){
    m_state.pending=requested;m_state.pendingDuration=~UnsignedInt(0);return;
  }
  NameKeyTransaction keys(*TheNameKeyGenerator);NativeW3DAssets::Transaction assets(nativeAssets());GameClientRandomTransaction random;
  auto candidate=prepareState(requested,now);publish(std::move(candidate));
  assets.commit();keys.commit();random.commit();
}
void NativeModelDrawAnimationProducer::setPauseAnimation(Bool paused){
  const auto now=syncTime();if(m_state.paused==paused)return;
  if(NativeModelDrawConditionBatch::needsCopy(*this)&&m_current&&m_current->clock){
    auto candidate=cloneCandidate();auto& state=candidate.state;auto& current=*candidate.current;
    auto clock=NativeModelAnimationClock::start(current.binding.animation->motion->frames(),
      float(current.binding.animation->motion->frameRate()),current.frame,paused?Mode::Manual:state.savedMode,now,current.frame);
    if(paused)state.savedMode=current.clock->mode();current.clock=clock;state.paused=paused;
    publish(std::move(candidate));return;
  }
  if(m_current && m_current->clock && m_current->binding.model->kind==NativeW3DAssets::Prototype::Kind::Hlod){
    auto clock=NativeModelAnimationClock::start(m_current->binding.animation->motion->frames(),
      float(m_current->binding.animation->motion->frameRate()),m_current->frame,paused?Mode::Manual:m_state.savedMode,now,m_current->frame);
    if(paused)m_state.savedMode=m_current->clock->mode();
    m_current->clock=clock;
  }
  m_state.paused=paused;
}
void NativeModelDrawAnimationProducer::setAnimationFrame(int frame){
  const auto now=syncTime();if(!m_current || m_state.animation<0)return;
  const auto clip=nativeAssets().animation(m_state.source->m_animations[m_state.animation].getName().str());
  if(m_current->binding.model->kind!=NativeW3DAssets::Prototype::Kind::Hlod)return;
  auto clock=NativeModelAnimationClock::start(clip->motion->frames(),float(clip->motion->frameRate()),float(frame),Mode::Manual,now,m_current->frame);
  if(NativeModelDrawConditionBatch::needsCopy(*this)){
    auto candidate=cloneCandidate();auto& current=*candidate.current;
    current.binding.pose.evaluateRuntime(raw(current.world),clip->motion.get(),float(frame),current.treeScale);
    current.binding.animation=clip;current.frame=float(frame);current.clock=clock;publish(std::move(candidate));return;
  }
  m_current->binding.pose.evaluateRuntime(raw(m_current->world),clip->motion.get(),float(frame),m_current->treeScale);
  m_current->binding.animation=clip;m_current->frame=float(frame);m_current->clock=clock;
}
void NativeModelDrawAnimationProducer::duration(Current* current,UnsignedInt frames){
  if(!current || !current->clock || current->binding.model->kind!=NativeW3DAssets::Prototype::Kind::Hlod)return;
  const Real desired=std::ceil(float(frames)*MSEC_PER_LOGICFRAME_REAL);
  if(desired>0){
    const Real natural=float(current->binding.animation->motion->frames())*1000.f/float(current->binding.animation->motion->frameRate());
    current->clock->setMultiplier(natural/desired);
  }
}
Real NativeModelDrawAnimationProducer::naturalDuration(const ModelConditionInfo* source)const{
  const auto clip=nativeAssets().animation(source->m_animations.front().getName().str());
  const auto result=float(clip->motion->frames())*1000.f/float(clip->motion->frameRate());
  if(!std::isfinite(result)||result<=0)throw ERROR_BAD_ARG;
  return result;
}
void NativeModelDrawAnimationProducer::setAnimationLoopDuration(UnsignedInt frames){
  (void)syncTime();
  if(NativeModelDrawConditionBatch::needsCopy(*this)&&m_current&&m_current->clock){
    auto candidate=cloneCandidate();duration(candidate.current.get(),frames);candidate.state.pendingDuration=~UnsignedInt(0);
    publish(std::move(candidate));return;
  }
  duration(m_current.get(),frames);m_state.pendingDuration=~UnsignedInt(0);
}
void NativeModelDrawAnimationProducer::setAnimationCompletionTime(UnsignedInt frames){
  (void)syncTime();
  if(m_state.source && m_state.source->m_transitionSig!=NO_TRANSITION && !m_state.source->m_animations.empty() &&
     m_state.pending && m_state.pending->m_transitionSig==NO_TRANSITION && !m_state.pending->m_animations.empty()){
    NativeW3DAssets::Transaction admission(nativeAssets());
    const Real a=naturalDuration(m_state.source),b=naturalDuration(m_state.pending);
    const auto split=REAL_TO_INT_FLOOR((float(frames)*a)/(a+b));
    if(split<0 || std::uint32_t(split)>frames)throw ERROR_BAD_ARG;
    if(NativeModelDrawConditionBatch::needsCopy(*this)&&m_current&&m_current->clock){
      auto candidate=cloneCandidate();duration(candidate.current.get(),UnsignedInt(split));candidate.state.pendingDuration=frames-UnsignedInt(split);
      publish(std::move(candidate));
    }else {duration(m_current.get(),UnsignedInt(split));m_state.pendingDuration=frames-UnsignedInt(split);}
    admission.commit();
  }else setAnimationLoopDuration(frames);
}
void NativeModelDrawAnimationProducer::advanceRetained(const Matrix3D& world,UnsignedInt now){
  if(NativeModelDrawConditionBatch::needsCopy(*this)&&m_current){
    auto candidate=cloneCandidate();auto& current=*candidate.current;
    if(current.clock){auto clock=current.clock->advanced(now);
      current.binding.pose.evaluateRuntime(raw(world),current.binding.animation->motion.get(),clock.frame(),current.treeScale);
      current.frame=clock.frame();current.clock=clock;
    }else if(current.binding.model->hierarchy)
      current.binding.pose.evaluateRuntime(raw(world),current.binding.animation?current.binding.animation->motion.get():nullptr,current.frame,current.treeScale);
    current.world=world;publish(std::move(candidate));return;
  }
  if(m_current&&!m_current->clock)updateNativeCurrentPose(world,m_current->frame);
  else advanceNativeCurrentPose(world,now);
}
void NativeModelDrawAnimationProducer::advanceNativeConditionAnimation(const Matrix3D& world){
  requireClientPhase();const auto now=syncTime();const auto root=raw(world);
  if(!complete()){
    advanceRetained(world,now);
    return;
  }
  const bool restart=m_state.source && m_state.animation>=0 &&
    (m_state.source->m_animations[m_state.animation].isIdleAnim() || has(m_state.source->m_flags,RESTART_ANIM_WHEN_COMPLETE));
  if(!m_state.pending && !restart){
    advanceRetained(world,now);
    return;
  }
  NameKeyTransaction keys(*TheNameKeyGenerator);NativeW3DAssets::Transaction assets(nativeAssets());GameClientRandomTransaction random;
  Candidate candidate;
  if(m_state.pending){
    const auto pendingDuration=m_state.pendingDuration;
    candidate=prepareState(m_state.pending,now,true);
    if(pendingDuration!=~UnsignedInt(0))duration(candidate.current.get(),pendingDuration);
    if(candidate.state.animation>=0 &&
       (candidate.state.source->m_animations[candidate.state.animation].isIdleAnim() ||
        has(candidate.state.source->m_flags,RESTART_ANIM_WHEN_COMPLETE)))
      selectAnimation(candidate,candidate.state.source,-1,now);
  }else {candidate=cloneCandidate();selectAnimation(candidate,m_state.source,-1,now);}
  if(candidate.current && candidate.current->clock){
    auto clock=candidate.current->clock->advanced(now);
    candidate.current->binding.pose.evaluateRuntime(root,candidate.current->binding.animation->motion.get(),clock.frame(),candidate.current->treeScale);
    candidate.current->clock=clock;candidate.current->frame=clock.frame();candidate.current->world=world;
  }else if(candidate.current){
    if(candidate.current->binding.model->hierarchy)
      candidate.current->binding.pose.evaluateRuntime(root,nullptr,candidate.current->frame,candidate.current->treeScale);
    candidate.current->world=world;
  }
  publish(std::move(candidate));assets.commit();keys.commit();random.commit();
}
