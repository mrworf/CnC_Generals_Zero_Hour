// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/Diplomacy.h"

// Main-thread, borrowed presentation callback. reset requests full replay;
// otherwise the only new entry is entries.back(). Throwing may partially alter
// presentation but never commits model state; next notification fully replays.
using NativeDiplomacyBriefingView=void(*)(const BriefingList&,Bool reset,void*);
class NativeDiplomacyBriefing {
public:
  NativeDiplomacyBriefing()=default;
  NativeDiplomacyBriefing(const NativeDiplomacyBriefing&)=delete;
  NativeDiplomacyBriefing& operator=(const NativeDiplomacyBriefing&)=delete;
  BriefingList* entries() noexcept {return &m_entries;}
  void update(AsciiString text,Bool clear);
  void attach(NativeDiplomacyBriefingView,void*);
  Bool detach() noexcept;
  Bool attached() const noexcept {return m_view!=nullptr;}
private:
  BriefingList m_entries;
  NativeDiplomacyBriefingView m_view=nullptr;
  void* m_context=nullptr;
  Bool m_notifying=FALSE,m_dirty=FALSE;
  void notify(const BriefingList&,Bool);
};
NativeDiplomacyBriefing& originalDiplomacyBriefing();
