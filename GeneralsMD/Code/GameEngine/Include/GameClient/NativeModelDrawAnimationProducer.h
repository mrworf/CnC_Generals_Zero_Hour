// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeModelDrawCurrentQueries.h"
class Display;
class NativeModelDrawConditionBatch;
// Shared source state owner. Specialized effects/physical callbacks remain
// abstract, never successful substitutes. Concrete source providers are pending.
class NativeModelDrawAnimationProducer : public NativeModelDrawCurrentQueries {
  MEMORY_POOL_GLUE_ABC(NativeModelDrawAnimationProducer)
  friend class NativeModelDrawConditionBatch;
  using Mode=NativeModelAnimationClock::Mode;
  struct ParticleCohort;
  struct State {
    const ModelConditionInfo* source=nullptr;
    const ModelConditionInfo* pending=nullptr;
    Int animation=-1;
    UnsignedInt pendingDuration=~UnsignedInt(0);
    Bool paused=FALSE;
    Mode savedMode=Mode::Loop;
    std::shared_ptr<ParticleCohort> particles;
    Bool recalcParticles=FALSE;
    Bool fullyObscured=FALSE;
    std::vector<ModelConditionInfo::HideShowSubObjInfo> subobjectOverrides;
  };
  struct Candidate {
    std::unique_ptr<Current> current;
    std::unique_ptr<ModelConditionInfo> validated;
    State state;
  };
  State m_state;
  std::unique_ptr<ModelConditionInfo> m_validated;
  Display* m_display=nullptr;
  Candidate cloneCandidate()const;
  Candidate prepareState(const ModelConditionInfo*,UnsignedInt,bool restart=false)const;
  void selectAnimation(Candidate&,const ModelConditionInfo*,Real,UnsignedInt)const;
  void publish(Candidate&&)noexcept;
  UnsignedInt syncTime()const;
  bool complete()const noexcept;
  static Real fraction(const State&,const Current*);
  static void duration(Current*,UnsignedInt);
  Real naturalDuration(const ModelConditionInfo*)const;
  void advanceRetained(const Matrix3D&,UnsignedInt);
  void applyParticleVisibility(Bool);
protected:
  void initializeNativeAnimationProducer();
  void advanceNativeConditionAnimation(const Matrix3D&);
  void recalcNativeStateParticles(); // Complete client callback; full model tick is separate.
  std::size_t nativeStateParticleCount()const noexcept;
  UnsignedInt nativeStateParticleID(std::size_t)const;
  const ModelConditionInfo* nativeSelectedCondition()const noexcept{return m_state.source;}
  const ModelConditionInfo* nativePendingCondition()const noexcept{return m_state.pending;}
  Int nativeSelectedAnimation()const noexcept{return m_state.animation;}
  Bool nativeAnimationPaused()const noexcept{return m_state.paused;}
  std::size_t nativeSubobjectOverrideCount()const noexcept{return m_state.subobjectOverrides.size();}
  UnsignedInt nativePendingDuration()const noexcept{return m_state.pendingDuration;}
public:
  NativeModelDrawAnimationProducer(Thing*,const ModuleData*,NativeW3DAssets&);
  void replaceModelConditionState(const ModelConditionFlags&)override;
  void setPauseAnimation(Bool)override;
  void setAnimationFrame(int)override;
  void setAnimationLoopDuration(UnsignedInt)override;
  void setAnimationCompletionTime(UnsignedInt)override;
  Bool updateBonesForClientParticleSystems()override;
  void setHidden(Bool)override;
  void setFullyObscuredByShroud(Bool)override;
  void showSubObject(const AsciiString&,Bool)override;
  void updateSubObjects()override;
};
