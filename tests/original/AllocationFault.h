// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
namespace AllocationFault {
void arm(std::size_t ordinal) noexcept;
void disarm() noexcept;
std::size_t live() noexcept;
bool triggered() noexcept;
// Complete generated-operation census when armed above its total allocation
// demand. Counts attempted standard ownership acquisitions, not live units.
std::size_t attempts() noexcept;
}
