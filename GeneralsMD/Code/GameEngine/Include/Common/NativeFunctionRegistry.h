// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindow.h"
#include "GameClient/WindowLayout.h"
#include <array>
#include <span>
#include <variant>

// Actual source prototypes, not object-pointer erasure. System/Input share one
// prototype; Init/Update/Shutdown likewise share one layout prototype.
class NativeWindowCallback {
  using Value = std::variant<std::monostate, GameWinSystemFunc,
      GameWinTooltipFunc, GameWinDrawFunc, WindowLayoutInitFunc>;
  Value m_value;
public:
  constexpr NativeWindowCallback() noexcept = default;
  constexpr NativeWindowCallback(std::nullptr_t) noexcept {}
  constexpr NativeWindowCallback(GameWinSystemFunc f) noexcept : m_value(f) {}
  constexpr NativeWindowCallback(GameWinTooltipFunc f) noexcept : m_value(f) {}
  constexpr NativeWindowCallback(GameWinDrawFunc f) noexcept : m_value(f) {}
  constexpr NativeWindowCallback(WindowLayoutInitFunc f) noexcept : m_value(f) {}
  template<class T> T get() const noexcept {
    const auto* value=std::get_if<T>(&m_value);
    return value ? *value : nullptr;
  }
  explicit operator bool() const noexcept {
    return std::visit([](auto value) {
      if constexpr (std::is_same_v<decltype(value),std::monostate>) return false;
      else return value != nullptr;
    },m_value);
  }
  bool operator==(const NativeWindowCallback&) const noexcept = default;
};
enum class NativeFunctionTableIndex : Int {
  TABLE_ANY = -1,
  TABLE_GAME_WIN_SYSTEM = 0,
  TABLE_GAME_WIN_INPUT,
  TABLE_GAME_WIN_TOOLTIP,
  TABLE_GAME_WIN_DEVICEDRAW,
  TABLE_GAME_WIN_DRAW,
  TABLE_WIN_LAYOUT_INIT,
  TABLE_WIN_LAYOUT_DEVICEINIT,
  TABLE_WIN_LAYOUT_UPDATE,
  TABLE_WIN_LAYOUT_SHUTDOWN,
  MAX_FUNCTION_TABLES
};
struct NativeFunctionEntry {
  NameKeyType key;
  const char* name;
  NativeWindowCallback func;
};
struct NativeFunctionTable {
  std::span<NativeFunctionEntry> entries;
  NativeFunctionTableIndex index;
};
class NativeFunctionRegistry {
  std::array<std::span<NativeFunctionEntry>,9> m_tables{};
public:
  // Complete transaction; includes the original sentinel in each bounded span.
  // Names may be interned monotonically, but no entry keys/table links publish
  // until every supplied table is valid and all keys are prepared.
  void loadTables(std::span<const NativeFunctionTable> tables, NameKeyGenerator& names);
  std::span<NativeFunctionEntry> table(NativeFunctionTableIndex index) const noexcept;
  NativeWindowCallback find(NameKeyType key,NativeFunctionTableIndex index) const noexcept;
  Bool validate() const noexcept;
};
