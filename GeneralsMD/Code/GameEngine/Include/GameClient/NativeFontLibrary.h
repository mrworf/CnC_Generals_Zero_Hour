// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/GameFont.h"
#include <cstdint>
#include <memory>
#include <vector>

// Simulation/client-thread CPU font ownership. Stock public APIs only; no host
// font installation, proprietary distribution or borrowed glyph-slot buffers.
class NativeFontLibrary final:public FontLibrary {
public:
  using AcquisitionObserver=void (*)(void*);
  struct Glyph {
    Int advance=0, width=0, height=0, left=0, top=0;
    Int lineHeight=0, ascent=0;
    Bool alternate=FALSE, local=FALSE;
    std::vector<std::uint8_t> coverage;
  };
  explicit NativeFontLibrary(AcquisitionObserver observer=nullptr,void* context=nullptr);
  ~NativeFontLibrary() override;
  NativeFontLibrary(const NativeFontLibrary&)=delete;
  NativeFontLibrary& operator=(const NativeFontLibrary&)=delete;
  void init() override;
  void reset() override;
  bool initialized() const noexcept;
  std::size_t localFaceCount() const noexcept;
  bool owns(const GameFont* font) const noexcept;
  Int extraOverlap(const GameFont* font) const;
  Glyph readGlyph(const GameFont* font,WideChar character);
private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
  AcquisitionObserver m_observer;
  void* m_context;
  Bool loadFontData(GameFont* font) override;
  void releaseFontData(GameFont* font) override;
};
