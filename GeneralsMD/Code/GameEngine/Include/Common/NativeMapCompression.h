// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <memory>
struct NativeMapData {
  std::unique_ptr<char[]> bytes;
  std::int32_t size = 0;
};
// Source-compatible raw/EAR/EAB/EAH/ZL1..9 admission. NOX remains
// explicitly unsupported pending their bounded native port, never raw fallback.
NativeMapData decodeNativeMapData(NativeMapData input);
