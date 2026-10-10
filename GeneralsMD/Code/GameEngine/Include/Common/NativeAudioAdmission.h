// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <memory>
class AudioManager;
class AudioEventRTS;
// Main-owner admission savepoint. A committed child retains its outer boundary.
// Queue processing/ownership transfer happens only after all admissions retire.
class NativeAudioAdmission final {
  struct Impl;
  std::unique_ptr<Impl> m_state;
  static thread_local Impl* current;
  static void retireCaller(const AudioEventRTS&) noexcept;
public:
  explicit NativeAudioAdmission(AudioManager&);
  ~NativeAudioAdmission() noexcept;
  NativeAudioAdmission(const NativeAudioAdmission&)=delete;
  NativeAudioAdmission& operator=(const NativeAudioAdmission&)=delete;
  void commit() noexcept;
  static void touch(const AudioEventRTS&);
  static void requireConsumption(AudioManager&);
};
