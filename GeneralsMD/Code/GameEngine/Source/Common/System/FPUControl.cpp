/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

#include "PreRTS.h"
#include "GameLogic/FPUControl.h"

// Extracted without behavioral redesign from GameLogic/System/GameLogic.cpp.
void setFPMode( void )
{
  // Despite the stale historical CHOP comment, the shipped body selects
  // nearest rounding and 24-bit x87 precision.
#if defined(_WIN32)
  _fpreset();
  UnsignedInt curVal = _statusfp();
  UnsignedInt newVal = curVal;
  newVal = (newVal & ~_MCW_RC) | (_RC_NEAR & _MCW_RC);
  newVal = (newVal & ~_MCW_PC) | (_PC_24 & _MCW_PC);
  _controlfp(newVal, _MCW_PC | _MCW_RC);
#elif defined(__x86_64__)
  std::feclearexcept(FE_ALL_EXCEPT);
  std::fesetround(FE_TONEAREST);
  unsigned short control = 0;
  __asm__ volatile("fnstcw %0" : "=m"(control));
  control = static_cast<unsigned short>(control & ~0x0f00U);
  __asm__ volatile("fldcw %0" : : "m"(control));
#else
#error "M26 original FPU control supports x86-64 Linux only"
#endif
}
