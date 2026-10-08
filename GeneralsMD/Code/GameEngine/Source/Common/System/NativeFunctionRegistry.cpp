// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/NativeFunctionRegistry.h"
#include <limits>
#include <vector>
namespace {
using Index=NativeFunctionTableIndex;
bool valid(Index index) noexcept { return static_cast<Int>(index)>=0 && static_cast<Int>(index)<9; }
bool family(const NativeWindowCallback& callback,Index index) noexcept {
  if (!callback) return true; // source permits explicitly absent entries
  switch(index) {
    case Index::TABLE_GAME_WIN_SYSTEM: case Index::TABLE_GAME_WIN_INPUT:
      return callback.get<GameWinSystemFunc>()!=nullptr;
    case Index::TABLE_GAME_WIN_TOOLTIP: return callback.get<GameWinTooltipFunc>()!=nullptr;
    case Index::TABLE_GAME_WIN_DEVICEDRAW: case Index::TABLE_GAME_WIN_DRAW:
      return callback.get<GameWinDrawFunc>()!=nullptr;
    case Index::TABLE_WIN_LAYOUT_INIT: case Index::TABLE_WIN_LAYOUT_DEVICEINIT:
    case Index::TABLE_WIN_LAYOUT_UPDATE: case Index::TABLE_WIN_LAYOUT_SHUTDOWN:
      return callback.get<WindowLayoutInitFunc>()!=nullptr;
    default: return false;
  }
}
}
void NativeFunctionRegistry::loadTables(std::span<const NativeFunctionTable> tables,NameKeyGenerator& names) {
  std::array<bool,9> selected{}; std::size_t total=0;
  // Validate the whole bounded batch before interning names or reserving storage.
  for (const auto& input:tables) {
    if (!valid(input.index) || selected[static_cast<Int>(input.index)] || input.entries.empty()) throw ERROR_BAD_ARG;
    selected[static_cast<Int>(input.index)]=true;
    const auto& sentinel=input.entries.back();
    if (sentinel.name || sentinel.func || sentinel.key!=NAMEKEY_INVALID) throw ERROR_BAD_ARG;
    for (const auto& entry:input.entries.first(input.entries.size()-1))
      if (!entry.name || !*entry.name || !family(entry.func,input.index)) throw ERROR_BAD_ARG;
    if (input.entries.size()-1 > std::numeric_limits<std::size_t>::max()-total) throw ERROR_BAD_ARG;
    total+=input.entries.size()-1;
  }
  std::vector<NameKeyType> keys; keys.reserve(total);
  for (const auto& input:tables)
    for (const auto& entry:input.entries.first(input.entries.size()-1))
      keys.push_back(names.nameToKey(entry.name));
  std::size_t cursor=0;
  for (const auto& input:tables) {
    for (auto& entry:input.entries.first(input.entries.size()-1)) entry.key=keys[cursor++];
    m_tables[static_cast<Int>(input.index)]=input.entries;
  }
}
std::span<NativeFunctionEntry> NativeFunctionRegistry::table(Index index) const noexcept {
  if (!valid(index)) return {};
  return m_tables[static_cast<Int>(index)];
}
NativeWindowCallback NativeFunctionRegistry::find(NameKeyType key,Index index) const noexcept {
  if (key<=NAMEKEY_INVALID || key>NAMEKEY_MAX) return {};
  const auto search=[&](std::span<NativeFunctionEntry> entries) {
    for (const auto& entry:entries) if (entry.key==key) return entry.func;
    return NativeWindowCallback{};
  };
  if (index==Index::TABLE_ANY) {
    for (auto entries:m_tables) if (auto callback=search(entries)) return callback;
    return {};
  }
  return search(table(index));
}
Bool NativeFunctionRegistry::validate() const noexcept {
  bool unique=true;
  for (auto source:m_tables) for (const auto& entry:source) if (entry.func)
    for (auto target:m_tables) for (const auto& other:target)
      if (&entry!=&other && entry.func==other.func) unique=false;
  return unique;
}
