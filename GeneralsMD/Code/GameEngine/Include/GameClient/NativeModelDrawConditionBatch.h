// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/DrawModule.h"
#include <memory>
class NativeModelDrawAnimationProducer;
class ParticleSystemManager;

// Enroll from raw lists, never a dirty getter. Nested notifications share the
// original namespace/assets/client cursor and retain exact prior module owners.
class NativeModelDrawConditionBatch final {
  struct Impl;
  std::unique_ptr<Impl> m_owned;
  Impl* m_root=nullptr;
  static thread_local Impl* current;
public:
  NativeModelDrawConditionBatch(const DrawModule* const*,ModelConditionFlags* flags=nullptr,Bool* dirty=nullptr);
  ~NativeModelDrawConditionBatch() noexcept;
  NativeModelDrawConditionBatch(const NativeModelDrawConditionBatch&)=delete;
  NativeModelDrawConditionBatch& operator=(const NativeModelDrawConditionBatch&)=delete;
  void commit()noexcept;
  static void validate(const NativeModelDrawAnimationProducer&);
  static bool needsCopy(const NativeModelDrawAnimationProducer&);
  static void retainPrevious(NativeModelDrawAnimationProducer&)noexcept;
  static void enrollParticles(ParticleSystemManager&);
};
