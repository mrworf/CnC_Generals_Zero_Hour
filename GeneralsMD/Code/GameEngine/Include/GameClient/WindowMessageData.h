// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
// Synchronous in-process window callbacks carry both scalar bits and borrowed
// addresses. This is not the serialization width of any save/network field.
using WindowMsgData = std::uintptr_t;
