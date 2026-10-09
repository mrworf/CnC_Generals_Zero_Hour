// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/GlobalData.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/NativePresentationState.h"

// Original script entry points. Shared source state must not depend on linking
// the obsolete Windows renderer. Nonnull devices implement the real operation.
void oversizeTheTerrain(Int amount) {
  if(TheTerrainVisual)TheTerrainVisual->oversizeTerrain(amount);
}
void doSkyBoxSet(Bool startDraw) {
  if(TheWritableGlobalData)TheWritableGlobalData->m_drawSkyBox=startDraw;
}
