// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <memory>
class ParticleSystemManager;
class ParticleSystemTemplate;

// Source-owner savepoint, including systems committed by nested creators.
// Candidate cleanup is immediate; accepted destroy() remains deferred.
class NativeParticleSystemAdmission final {
  struct Impl;
  std::unique_ptr<Impl> m_state;
  static thread_local Impl* current;
public:
  explicit NativeParticleSystemAdmission(ParticleSystemManager&);
  ~NativeParticleSystemAdmission() noexcept;
  NativeParticleSystemAdmission(const NativeParticleSystemAdmission&)=delete;
  NativeParticleSystemAdmission& operator=(const NativeParticleSystemAdmission&)=delete;
  void commit() noexcept;
  static const ParticleSystemTemplate* slave(const ParticleSystemTemplate&);
};
