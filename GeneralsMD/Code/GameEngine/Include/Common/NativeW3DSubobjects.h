// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeW3DAssets.h"
#include <span>

// Mutable per-instance presentation state. Immutable admitted prototypes are
// borrowed by identity, never edited. Bone pose visibility is a separate input.
class NativeW3DSubobjects final {
public:
  struct Rule {std::string_view name;bool hidden;};
  struct Child {NativeW3DAssets::Model model;std::uint32_t bone;bool hidden=false;};
  static NativeW3DSubobjects admit(const NativeW3DAssets::Model&);
  NativeW3DSubobjects clone()const{return *this;}
  void swap(NativeW3DSubobjects&)noexcept;
  std::span<const Child> children()const noexcept{return m_children;}
  std::optional<std::size_t> find(std::string_view)const;
  // Whole rule-list validation precedes any mutation. Missing names are source
  // no-ops; malformed names reject without retaining an applied prefix.
  void apply(std::span<const Rule>);
  bool setFirstOnBone(std::uint32_t,bool);
  void setHidden(bool value)noexcept{m_hidden=value;}
  bool hidden()const noexcept{return m_hidden;}
private:
  NativeW3DAssets::Model m_model;
  std::vector<Child> m_children;
  bool m_hidden=false;
  bool descendant(std::uint32_t,std::uint32_t)const noexcept;
};
