// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/DisplayString.h"
#include "GameClient/NativeFontLibrary.h"
#include <vector>

// Original display-string interface with owned CPU geometry. The string and
// library remain client-thread owners; no borrowed FreeType slot is retained.
class NativeDisplayString final:public DisplayString {
  MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NativeDisplayString,"NativeDisplayString")
public:
  struct Placement {
    Int x=0,y=0;
    NativeFontLibrary::Glyph glyph;
    Bool hotkey=FALSE;
  };
  explicit NativeDisplayString(NativeFontLibrary& fonts);
  void setText(UnicodeString text) override;
  void setFont(GameFont* font) override;
  void appendChar(WideChar character) override;
  void removeLastChar() override;
  void notifyTextChanged() override;
  void reset() override;
  void setWordWrap(Int width) override;
  void setWordWrapCentered(Bool centered) override;
  void setUseHotkey(Bool enabled,Color color) override;
  void setClipRegion(IRegion2D* region) override;
  void getSize(Int* width,Int* height) override;
  Int getWidth(Int position=-1) override;
  void draw(Int x,Int y,Color color,Color shadow) override;
  void draw(Int x,Int y,Color color,Color shadow,Int dx,Int dy) override;
  const std::vector<Placement>& placements() const noexcept { return m_layout.placements; }
private:
  struct Layout {
    Int width=0,height=0;
    std::vector<Int> advances;
    std::vector<Placement> placements;
  };
  Layout prepare(const UnicodeString& text,GameFont* font,Int wrap,Bool centered,Bool hotkeys);
  void adopt(UnicodeString text,GameFont* font,Int wrap,Bool centered,Bool hotkeys,Color hotkeyColor);
  NativeFontLibrary& m_fonts;
  Layout m_layout;
  Int m_wrap=0;
  Bool m_centered=FALSE,m_hotkeys=FALSE;
  Color m_hotkeyColor=0;
  IRegion2D m_clip{};
};
