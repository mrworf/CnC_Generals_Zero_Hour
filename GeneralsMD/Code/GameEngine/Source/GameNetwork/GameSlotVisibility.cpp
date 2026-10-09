// SPDX-License-Identifier: GPL-3.0-or-later
// Original scalar visibility/alliance policy; translated names remain separate.
#include "PreRTS.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/GameInfo.h"
#include "Common/MultiplayerSettings.h"
Bool isSlotLocalAlly(const GameSlot* slot){
  if(!slot || !TheGameInfo)return FALSE;
  Int slotIndex=-1;
  for(Int i=0;i<MAX_SLOTS;++i)if(TheGameInfo->getConstSlot(i)==slot){slotIndex=i;break;}
  const Int localIndex=TheGameInfo->getLocalSlotNum();
  if(slotIndex<0 || localIndex<0 || localIndex>=MAX_SLOTS)return FALSE;
  const auto* localSlot=TheGameInfo->getConstSlot(localIndex);if(!localSlot)return FALSE;
  if(slotIndex==localIndex)return TRUE;
  if(slot->getTeamNumber()==localSlot->getTeamNumber() && slot->getTeamNumber()>=0)return TRUE;
  if(localSlot->getOriginalPlayerTemplate()==PLAYERTEMPLATE_OBSERVER)return TRUE;
  return FALSE;
}
Int GameSlot::getApparentPlayerTemplate() const {
  if(TheMultiplayerSettings && TheMultiplayerSettings->showRandomPlayerTemplate() && !isSlotLocalAlly(this))
    return m_origPlayerTemplate;
  return m_playerTemplate;
}
Int GameSlot::getApparentColor() const {
  if(TheMultiplayerSettings && m_origPlayerTemplate==PLAYERTEMPLATE_OBSERVER)
    return TheMultiplayerSettings->getColor(PLAYERTEMPLATE_OBSERVER)->getColor();
  if(TheMultiplayerSettings && TheMultiplayerSettings->showRandomColor() && !isSlotLocalAlly(this))return m_origColor;
  return m_color;
}
Int GameSlot::getApparentStartPos() const {
  if(TheMultiplayerSettings && TheMultiplayerSettings->showRandomStartPos() && !isSlotLocalAlly(this))return m_origStartPos;
  return m_startPos;
}
