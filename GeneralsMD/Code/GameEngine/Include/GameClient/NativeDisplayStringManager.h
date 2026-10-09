// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/DisplayStringManager.h"
#include "GameClient/NativeDisplayString.h"
#include <array>

class NativeDisplayStringManager final:public DisplayStringManager {
public:
#ifdef KRIS_BRUTAL_HACK_FOR_AIRCRAFT_CARRIER_DEBUGGING
  static constexpr Int groupCount=20;
#else
  static constexpr Int groupCount=10;
#endif
  NativeDisplayStringManager()=default;
  explicit NativeDisplayStringManager(NativeFontLibrary& fonts):m_fonts(&fonts) {}
  ~NativeDisplayStringManager() override;
  void reset() override;
  void postProcessLoad() override;
  DisplayString* newDisplayString() override;
  void freeDisplayString(DisplayString* string) override;
  DisplayString* getGroupNumeralString(Int numeral) override;
  DisplayString* getFormationLetterString() override { return m_formation; }
  std::size_t count() const noexcept;
private:
  NativeFontLibrary& fonts() const;
  NativeFontLibrary* m_fonts=nullptr;
  std::array<DisplayString*,groupCount> m_groups{};
  DisplayString* m_formation=nullptr;
};
