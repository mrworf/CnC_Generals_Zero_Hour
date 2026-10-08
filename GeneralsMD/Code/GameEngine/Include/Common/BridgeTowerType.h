// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"
// Original source bridge tower identities shared by map data and terrain roads.
enum BridgeTowerType : UnsignedInt {
    BRIDGE_TOWER_FROM_LEFT = 0,
    BRIDGE_TOWER_FROM_RIGHT,
    BRIDGE_TOWER_TO_LEFT,
    BRIDGE_TOWER_TO_RIGHT,
    BRIDGE_MAX_TOWERS
};
