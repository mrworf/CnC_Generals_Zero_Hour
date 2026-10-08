// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
namespace AllocationFault {
void arm(std::size_t ordinal) noexcept;
void disarm() noexcept;
std::size_t live() noexcept;
bool triggered() noexcept;
}
