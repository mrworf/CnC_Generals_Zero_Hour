// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Electronic Arts Inc.; original GameLogic floating-point service.
// Native extraction preserves the active nearest-rounding/single-precision path.
#include "GameLogic/FPUControl.h"
#include "Common/Errors.h"
#include <cfenv>
#include <fpu_control.h>
#if !defined(__x86_64__)
#error "The supported native simulation target is x86-64 Linux"
#endif
void setFPMode() {
    if(std::fesetenv(FE_DFL_ENV)!=0 || std::fesetround(FE_TONEAREST)!=0)throw ERROR_BAD_ARG;
    fpu_control_t control;_FPU_GETCW(control);
    control=fpu_control_t((control&~(_FPU_EXTENDED|_FPU_RC_ZERO))|_FPU_SINGLE|_FPU_RC_NEAREST);
    _FPU_SETCW(control);
}
