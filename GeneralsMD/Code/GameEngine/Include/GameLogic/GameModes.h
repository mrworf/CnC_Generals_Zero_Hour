// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Lib/BaseType.h"

// Original mode identities: keep the excluded Internet slot, never renumber.
enum {
    GAME_SINGLE_PLAYER,
    GAME_LAN,
    GAME_SKIRMISH,
    GAME_REPLAY,
    GAME_SHELL,
    GAME_INTERNET,
    GAME_NONE
};
inline Bool nativeGameModeSupported(Int mode) noexcept {
    return mode >= GAME_SINGLE_PLAYER && mode <= GAME_NONE && mode != GAME_INTERNET;
}
