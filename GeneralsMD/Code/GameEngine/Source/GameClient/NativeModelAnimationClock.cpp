// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeModelAnimationClock.h"
#include "Common/Errors.h"
#include <cmath>

NativeModelAnimationClock NativeModelAnimationClock::start(std::uint32_t frames,float rate,float frame,
    Mode mode,std::uint32_t syncTime,float previousFrame){
  if(!frames || !std::isfinite(rate) || rate<=0 || !std::isfinite(frame) || !std::isfinite(previousFrame) ||
      static_cast<std::uint32_t>(mode)>static_cast<std::uint32_t>(Mode::OnceBackwards))throw ERROR_BAD_ARG;
  NativeModelAnimationClock out;out.m_frames=frames;out.m_rate=rate;out.m_frame=frame;
  out.m_previous=previousFrame;out.m_mode=mode;out.m_sync=syncTime;
  out.m_direction=mode<Mode::LoopBackwards?1.f:-1.f;return out;
}
void NativeModelAnimationClock::setMultiplier(float value){
  if(!std::isfinite(value))throw ERROR_BAD_ARG;m_multiplier=value;
}
NativeModelAnimationClock NativeModelAnimationClock::advanced(std::uint32_t syncTime)const{
  if(!m_frames)throw ERROR_BAD_ARG;
  auto out=*this;const float last=float(m_frames-1);
  if(m_mode!=Mode::Manual){
    const float elapsed=float(std::uint32_t(syncTime-m_sync));
    const float delta=m_rate*m_multiplier*m_direction*elapsed*.001f;
    out.m_frame+=delta;if(!std::isfinite(delta)||!std::isfinite(out.m_frame))throw ERROR_BAD_ARG;
    switch(m_mode){
      case Mode::Once:if(out.m_frame>=last)out.m_frame=last;break;
      case Mode::Loop:if(out.m_frame>=last)out.m_frame-=last;if(out.m_frame>=last)out.m_frame=0;break;
      case Mode::OnceBackwards:if(out.m_frame<0)out.m_frame=0;break;
      case Mode::LoopBackwards:if(out.m_frame<0)out.m_frame+=last;if(out.m_frame<0)out.m_frame=last;break;
      case Mode::PingPong:
        if(m_direction>=1){if(out.m_frame>=last){out.m_frame=last*2-out.m_frame;if(out.m_frame>=last)out.m_frame=last;out.m_direction=-m_direction;}}
        else if(out.m_frame<0){out.m_frame=-out.m_frame;if(out.m_frame>=last)out.m_frame=0;out.m_direction=-m_direction;}
        break;
      case Mode::Manual:break;
    }
  }
  out.m_previous=out.m_frame==m_frame?m_previous:m_frame;out.m_sync=syncTime;return out;
}
bool NativeModelAnimationClock::complete()const noexcept{
  return m_frames && ((m_mode==Mode::Once && m_frame==float(m_frames-1)) ||
    (m_mode==Mode::OnceBackwards && m_frame==0));
}
