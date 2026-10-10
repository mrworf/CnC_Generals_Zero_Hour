// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/NativeModelDrawQueries.h"
#include "GameClient/NativeModelAnimationClock.h"

class GameClient;

// Current-client pose is not the condition-selected pristine logic cache.
// Concrete source animation/presentation producers remain abstract.
class NativeModelDrawCurrentQueries : public NativeModelDrawQueries {
  MEMORY_POOL_GLUE_ABC(NativeModelDrawCurrentQueries)
  struct Current {
    NativeW3DAssets::Binding binding;
    Matrix3D world;
    Real treeScale;
    Real frame;
    std::optional<NativeModelAnimationClock> clock;
  };
  GameClient* const m_client;
  std::unique_ptr<Current> m_current;
  void requireClientOwner() const;
  void requireClientPhase() const;
protected:
  void replaceNativeCurrentModel(std::string_view model,std::string_view animation,
                                const Matrix3D& world,Real frame);
  void updateNativeCurrentPose(const Matrix3D& world,Real frame);
  void replaceNativeTimedModel(std::string_view model,std::string_view animation,const Matrix3D& world,
                              Real frame,NativeModelAnimationClock::Mode,std::uint32_t syncTime);
  void advanceNativeCurrentPose(const Matrix3D& world,std::uint32_t syncTime);
  void setNativeAnimationMultiplier(Real);
  const NativeModelAnimationClock* nativeAnimationClock()const noexcept{return m_current&&m_current->clock?&*m_current->clock:nullptr;}
  const NativeW3DAssets::Binding* nativeCurrentBinding()const noexcept{return m_current?&m_current->binding:nullptr;}
public:
  NativeModelDrawCurrentQueries(Thing*,const ModuleData*,NativeW3DAssets&);
  Bool clientOnly_getRenderObjBoneTransform(const AsciiString&,Matrix3D*)const override;
  Int getCurrentBonePositions(const char*,Int,Coord3D*,Matrix3D*,Int)const override;
  Bool getCurrentWorldspaceClientBonePositions(const char*,Matrix3D&)const override;
};
