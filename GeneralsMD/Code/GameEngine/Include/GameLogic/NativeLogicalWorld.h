// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameLogic/NativeTerrainHeightMap.h"

// Actual logical WorldHeightMap owner callbacks, without a render-device owner.
// Process-global objects/sides/triggers retain their original runtime lifetime.
// This owner holds logical height metadata only; visual terrain is a later phase.
class NativeLogicalWorld {
  NativeTerrainHeightMap m_height;
public:
  using Admission = void (*)(const NativeTerrainHeightMap& candidateHeight, void* context);
  void load(ChunkInputStream& stream);
  // Synchronous CPU continuation while original world rollback remains live.
  // The caller must retain its own staged owners until this method succeeds.
  void load(ChunkInputStream& stream, Admission admission, void* context);
  const NativeTerrainHeightMap& height() const noexcept { return m_height; }
};
