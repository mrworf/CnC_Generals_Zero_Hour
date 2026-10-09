// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GameCommon.h"
#include "Common/DataChunk.h"
class Waypoint;
class MapObject;
class NativeTerrainHeightMap;

// Actual source pooled Waypoints, staged as one CPU graph. Neighbors are
// borrowed; the linked list owns nodes. Shared operations also serve TerrainLogic.
class NativeWaypointGraph final {
  Waypoint* m_head=nullptr;
public:
  NativeWaypointGraph()=default;
  ~NativeWaypointGraph() noexcept;
  NativeWaypointGraph(const NativeWaypointGraph&)=delete;
  NativeWaypointGraph& operator=(const NativeWaypointGraph&)=delete;
  Waypoint* first() const noexcept { return m_head; }
  void load(ChunkInputStream& stream,MapObject* objects,
            const NativeTerrainHeightMap* initialHeight=nullptr);
  void resnap(const NativeTerrainHeightMap& height);
  void swap(NativeWaypointGraph& other) noexcept;
  void exchange(Waypoint*& externalHead) noexcept;
  static void prepend(Waypoint*& head,MapObject* object,Real height);
  static void link(Waypoint* head,Int source,Int target) noexcept;
  static void clear(Waypoint*& head) noexcept;
  static Bool readLinks(Waypoint* head,DataChunkInput& input);
};
