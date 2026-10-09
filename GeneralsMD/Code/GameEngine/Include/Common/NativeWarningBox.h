// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/OSDisplay.h"
#include <string>
class GameTextInterface;
struct NativeWarningRequest {
  std::string title,message;
  UnsignedInt buttons,options;
};
// Synchronous main-thread borrowed presenter. Request strings are owned through
// the call; neither presenter nor context is retained after returning/throwing.
using NativeWarningPresenter=OSDisplayButtonType(*)(const NativeWarningRequest&,void*);
OSDisplayButtonType nativeDisplayWarning(GameTextInterface*,const AsciiString& title,
    const AsciiString& message,UnsignedInt buttons,UnsignedInt options,
    NativeWarningPresenter,void*);
// Source audio's retry is an explicit affirmative choice, never dialog failure.
inline Bool nativeMusicRetryRequested(OSDisplayButtonType result) noexcept {return result==OSDBT_OK;}
