// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "GameClient/NativeDiplomacyBriefing.h"
#include <algorithm>

void NativeDiplomacyBriefing::notify(const BriefingList& entries,Bool reset){
  if(!m_view)return;
  m_notifying=TRUE;
  struct Restore{Bool& flag;~Restore(){flag=FALSE;}} restore{m_notifying};
  m_view(entries,reset,m_context);
}
void NativeDiplomacyBriefing::attach(NativeDiplomacyBriefingView view,void* context){
  if(!view || m_notifying)throw ERROR_BAD_ARG;
  if(m_view){if(m_view==view && m_context==context)return;throw ERROR_BAD_ARG;}
  // Capture publication only after complete initial synchronization. No member
  // points into the prospective view if its first callback rejects.
  m_notifying=TRUE;
  struct Restore{Bool& flag;~Restore(){flag=FALSE;}} restore{m_notifying};
  view(m_entries,TRUE,context);
  m_view=view;m_context=context;m_dirty=FALSE;
}
Bool NativeDiplomacyBriefing::detach() noexcept {
  if(m_notifying)return FALSE;
  m_view=nullptr;m_context=nullptr;m_dirty=FALSE;return TRUE;
}
void NativeDiplomacyBriefing::update(AsciiString text,Bool clear){
  if(m_notifying)throw ERROR_BAD_ARG;
  const Bool unchanged=!clear && (text.isEmpty() || std::find(m_entries.begin(),m_entries.end(),text)!=m_entries.end());
  if(unchanged){
    if(m_dirty){notify(m_entries,TRUE);m_dirty=FALSE;}
    return;
  }
  BriefingList candidate;
  if(!clear)candidate=m_entries;
  if(!text.isEmpty())candidate.push_back(std::move(text));
  if(m_view){const Bool reset=clear || m_dirty;m_dirty=TRUE;notify(candidate,reset);m_dirty=FALSE;}
  m_entries.swap(candidate);
}
NativeDiplomacyBriefing& originalDiplomacyBriefing(){static NativeDiplomacyBriefing owner;return owner;}
BriefingList* GetBriefingTextList(){return originalDiplomacyBriefing().entries();}
void UpdateDiplomacyBriefingText(AsciiString text,Bool clear){originalDiplomacyBriefing().update(std::move(text),clear);}
