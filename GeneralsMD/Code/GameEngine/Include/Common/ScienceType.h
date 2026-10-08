// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
// Original dynamic science IDs use defined signed 32-bit representation.
enum ScienceType : std::int32_t {SCIENCE_INVALID=-1};
static_assert(sizeof(ScienceType)==4 && alignof(ScienceType)==alignof(std::int32_t));
