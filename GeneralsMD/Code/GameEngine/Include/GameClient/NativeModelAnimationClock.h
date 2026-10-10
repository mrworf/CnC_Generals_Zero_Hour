// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

// Original single-motion animobj.cpp timing, separate from clip/pose ownership.
// A caller stages this value and commits it only after pose admission succeeds.
class NativeModelAnimationClock final {
public:
  enum class Mode : std::uint32_t { Manual=0, Loop, Once, PingPong, LoopBackwards, OnceBackwards };
  static NativeModelAnimationClock start(std::uint32_t frames,float rate,float frame,
      Mode mode,std::uint32_t syncTime,float previousFrame=0);
  NativeModelAnimationClock advanced(std::uint32_t syncTime) const;
  void setMultiplier(float);
  float frame()const noexcept{return m_frame;}
  float previousFrame()const noexcept{return m_previous;}
  float direction()const noexcept{return m_direction;}
  float multiplier()const noexcept{return m_multiplier;}
  Mode mode()const noexcept{return m_mode;}
  bool complete()const noexcept;
private:
  std::uint32_t m_frames=0,m_sync=0;
  Mode m_mode=Mode::Manual;
  float m_rate=0,m_frame=0,m_previous=0,m_multiplier=1,m_direction=1;
};
