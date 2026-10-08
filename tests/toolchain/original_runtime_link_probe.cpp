// SPDX-License-Identifier: GPL-3.0-or-later
// Link-only original startup dependency diagnostic. Never execute as gameplay
// or register with CTest: this is not the native entry point or simulation.
#include "Common/GameEngine.h"
static void originalRuntimeRoot(GameEngine& engine) {
    engine.GameEngine::init(0,nullptr);
}
int main() {
    auto* volatile retain=&originalRuntimeRoot;
    return retain==nullptr;
}
