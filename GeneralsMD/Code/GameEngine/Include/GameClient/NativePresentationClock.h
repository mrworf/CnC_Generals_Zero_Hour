// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

// W3DDisplay's fixed-step accumulated time and separately published WW3D time.
// Value staging permits a fallible fast-camera step before owner publication.
class NativePresentationClock final {
public:
  struct Step {bool cameraOnly;};
  explicit NativePresentationClock(std::uint32_t accumulated=0,std::uint32_t published=0)
    :m_accumulated(accumulated),m_published(published){}
  Step advance(std::uint32_t clientFrame,std::uint32_t milliseconds,bool frozen,bool fast) noexcept {
    frozen=frozen || m_lastFrame==clientFrame;m_lastFrame=clientFrame;
    if(!frozen)m_accumulated+=milliseconds;
    const bool cameraOnly=!frozen && fast;
    if(!cameraOnly)m_published=m_accumulated;
    return {cameraOnly};
  }
  std::uint32_t accumulated()const noexcept{return m_accumulated;}
  std::uint32_t published()const noexcept{return m_published;}
private:
  std::uint32_t m_accumulated,m_published,m_lastFrame=UINT32_MAX;
};
