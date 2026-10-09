// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/MapUtil.h"
#include "GameNetwork/NetworkDefs.h"
#include <array>
#include <string_view>
class FileSystem;

struct NativeMapPreviewControl {
  Bool present=FALSE;
  ICoord2D size{};
};
struct NativeMapPreviewStart {
  Bool visible=FALSE;
  ICoord2D position{};
};

// CPU placement shared by the original GUI entry points. No retained metadata,
// image or window pointers; a rejected candidate leaves accepted state intact.
class NativeMapPreviewLayout {
public:
  void rebuild(const MapMetaData&,Int width,Int height,
      const std::array<NativeMapPreviewControl,MAX_SLOTS>&);
  const auto& starts() const noexcept {return m_starts;}
  const TechAndSupplyImages& markers() const noexcept {return m_markers;}
  void publishMarkers(TechAndSupplyImages& output) noexcept {
    output.m_supplyPosList.swap(m_markers.m_supplyPosList);
    output.m_techPosList.swap(m_markers.m_techPosList);
  }
private:
  std::array<NativeMapPreviewStart,MAX_SLOTS> m_starts{};
  TechAndSupplyImages m_markers;
};

Bool nativeMapDrawPositions(Int x,Int y,Int width,Int height,Region3D extent,
    ICoord2D& ul,ICoord2D& lr) noexcept;
// Values are original slot indices, or -1; duplicate destinations retain the
// original last-slot-wins order. Presence belongs to the destination, not slot.
std::array<Int,MAX_SLOTS> nativeMapPreviewLabels(const GameInfo&,Int players,
    Bool loadScreen,const std::array<NativeMapPreviewControl,MAX_SLOTS>&);
// Copy only into the captured protected user namespace, streamed atomically.
// Input/output owners retire on every failure. Exceptions do not publish a file.
void nativeCopyMapPreview(FileSystem&,const NativeUserStorage&,
    const AsciiString& input,std::string_view destination);
