// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/StatsCollector.h"
#include "Common/NativeUserStorage.h"
#include "Common/GlobalData.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/Money.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#if defined(_DEBUG) || defined(_INTERNAL)
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/GameInfo.h"
#endif
namespace {
const NativeUserStorage& requiredStorage() {
  if(!TheNativeUserStorage) throw ERROR_BAD_ARG;
  return *TheNativeUserStorage;
}
class OriginalStatsSource final:public NativeStatsSource {
  static Player& player() {
    if(!ThePlayerList || !ThePlayerList->getLocalPlayer()) throw ERROR_BAD_ARG;
    return *ThePlayerList->getLocalPlayer();
  }
public:
  UnsignedInt frame() const override {
    if(!TheGameLogic) throw ERROR_BAD_ARG;
    return TheGameLogic->getFrame();
  }
  Int intervalSeconds() const override {
    if(!TheGlobalData) throw ERROR_BAD_ARG;
    return TheGlobalData->m_playStats;
  }
  Int localPlayerIndex() const override {return player().getPlayerIndex();}
  NativeStatsIdentity identity() const override {
    if(!TheGlobalData) throw ERROR_BAD_ARG;
    NativeStatsIdentity result;result.map=TheGlobalData->m_mapName;result.side=player().getSide();
#if defined(_DEBUG) || defined(_INTERNAL)
    result.benchmarkSeconds=TheGlobalData->m_benchmarkTimer;
    if(TheGlobalData->m_saveStats) {
      result.directory=TheGlobalData->m_baseStatsDir;
      if(!result.directory.isEmpty() && !result.directory.endsWith("/") && !result.directory.endsWith("\\"))
        result.directory.concat("/");
      result.directory.concat("Stats/");
      if(TheNetwork && TheLAN) {
        GameInfo* game=TheLAN->GetMyGame();
        if(!game) throw ERROR_BAD_ARG;
        AsciiString names;
        for(Int slot=0;slot<MAX_SLOTS;++slot) {
          const auto* value=game->getSlot(slot);
          if(value && value->isHuman()) {
            AsciiString name;name.format("%ls_",value->getName().str());names.concat(name);
          }
        }
        AsciiString map=result.map;const auto* leaf=map.reverseFind('\\');
        if(leaf) map=leaf+1;
        for(unsigned ordinal=0;ordinal<4;++ordinal) map.removeLastChar();
        result.sessionName.format("%s%s_%d_%d",names.str(),map.str(),game->getSeed(),game->getLocalSlotNum());
      }
    }
#endif
    return result;
  }
  NativeStatsSample sample() const override {
    if(!TheGameLogic || !player().getMoney()) throw ERROR_BAD_ARG;
    NativeStatsSample result;result.money=player().getMoney()->countMoney();
    for(Object* object=TheGameLogic->getFirstObject();object;object=object->getNextObject()) {
      // Original source eligibility and local/other-player partition.
      if(!(object->isKindOf(KINDOF_INFANTRY) || object->isKindOf(KINDOF_VEHICLE)) ||
          object->isNeutralControlled() || object->getControllingPlayer()->getSide().compare("Civilian")==0)
        continue;
      if(object->getControllingPlayer()->isLocalPlayer()) ++result.playerUnits;
      else ++result.aiUnits;
    }
    return result;
  }
};
OriginalStatsSource& originalSource() {static OriginalStatsSource source;return source;}
}
StatsCollector::StatsCollector():StatsCollector(requiredStorage(),originalSource()) {}
