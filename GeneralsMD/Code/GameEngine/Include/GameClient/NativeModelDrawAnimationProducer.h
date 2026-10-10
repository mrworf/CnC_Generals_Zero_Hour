// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeModelDrawCurrentQueries.h"
class Display;
// Shared source state owner. Specialized effects/physical callbacks remain
// abstract, never successful substitutes. Concrete source providers are pending.
class NativeModelDrawAnimationProducer : public NativeModelDrawCurrentQueries {
  MEMORY_POOL_GLUE_ABC(NativeModelDrawAnimationProducer)
  using Mode=NativeModelAnimationClock::Mode;
  struct State {
    const ModelConditionInfo* source=nullptr;
    const ModelConditionInfo* pending=nullptr;
    Int animation=-1;
    UnsignedInt pendingDuration=~UnsignedInt(0);
    Bool paused=FALSE;
    Mode savedMode=Mode::Loop;
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
protected:
  void initializeNativeAnimationProducer();
  void advanceNativeConditionAnimation(const Matrix3D&);
  const ModelConditionInfo* nativeSelectedCondition()const noexcept{return m_state.source;}
  const ModelConditionInfo* nativePendingCondition()const noexcept{return m_state.pending;}
  Int nativeSelectedAnimation()const noexcept{return m_state.animation;}
  Bool nativeAnimationPaused()const noexcept{return m_state.paused;}
  UnsignedInt nativePendingDuration()const noexcept{return m_state.pendingDuration;}
public:
  NativeModelDrawAnimationProducer(Thing*,const ModuleData*,NativeW3DAssets&);
  void replaceModelConditionState(const ModelConditionFlags&)override;
  void setPauseAnimation(Bool)override;
  void setAnimationFrame(int)override;
  void setAnimationLoopDuration(UnsignedInt)override;
  void setAnimationCompletionTime(UnsignedInt)override;
};
