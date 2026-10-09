// SPDX-License-Identifier: GPL-3.0-or-later
// Original skirmish/load-screen placement, independent of GameSpy menu code.
#include "PreRTS.h"
#include "GameClient/NativeMapPreviewLayout.h"
#include "GameNetwork/GameInfo.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameText.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/Image.h"

TechAndSupplyImages TheSupplyAndTechImageLocations;
namespace {
auto controls(GameWindow* windows[]) {
  std::array<NativeMapPreviewControl,MAX_SLOTS> result{};
  if(windows)for(Int i=0;i<MAX_SLOTS;++i)if(windows[i]){
    result[i].present=TRUE;windows[i]->winGetSize(&result[i].size.x,&result[i].size.y);
  }
  return result;
}
void withdraw(GameWindow* windows[],GameWindow* mapWindow) {
  TheSupplyAndTechImageLocations.m_supplyPosList.clear();
  TheSupplyAndTechImageLocations.m_techPosList.clear();
  if(mapWindow){
    mapWindow->winSetUserData(nullptr);
    const auto* unknown=TheMappedImageCollection?TheMappedImageCollection->findImageByName("UnknownMap"):nullptr;
    mapWindow->winSetEnabledImage(0,unknown);
    if(unknown)mapWindow->winSetStatus(WIN_STATUS_IMAGE);else mapWindow->winClearStatus(WIN_STATUS_IMAGE);
  }
  if(windows)for(Int i=0;i<MAX_SLOTS;++i)if(windows[i])windows[i]->winHide(TRUE);
}
}

void positionAdditionalImages(MapMetaData* metadata,GameWindow* mapWindow,Bool /*force*/) {
  if(!metadata || !mapWindow || mapWindow->winIsHidden()){
    TheSupplyAndTechImageLocations.m_supplyPosList.clear();
    TheSupplyAndTechImageLocations.m_techPosList.clear();return;
  }
  ICoord2D size;mapWindow->winGetSize(&size.x,&size.y);
  NativeMapPreviewLayout candidate;candidate.rebuild(*metadata,size.x,size.y,{});
  candidate.publishMarkers(TheSupplyAndTechImageLocations);
}

void positionStartSpots(AsciiString mapName,GameWindow* windows[],GameWindow* mapWindow) {
  const auto* metadata=TheMapCache?TheMapCache->findMap(mapName):nullptr;
  if(!metadata || !mapWindow){withdraw(windows,mapWindow);return;}
  ICoord2D size;mapWindow->winGetSize(&size.x,&size.y);
  NativeMapPreviewLayout candidate;candidate.rebuild(*metadata,size.x,size.y,controls(windows));
  const Image* image=getMapPreviewImage(mapName);
  if(!image && TheMappedImageCollection)image=TheMappedImageCollection->findImageByName("UnknownMap");
  mapWindow->winSetUserData(const_cast<MapMetaData*>(metadata));
  mapWindow->winSetEnabledImage(0,image);
  if(image)mapWindow->winSetStatus(WIN_STATUS_IMAGE);else mapWindow->winClearStatus(WIN_STATUS_IMAGE);
  if(mapWindow->winIsHidden()){
    TheSupplyAndTechImageLocations.m_supplyPosList.clear();TheSupplyAndTechImageLocations.m_techPosList.clear();
  }else candidate.publishMarkers(TheSupplyAndTechImageLocations);
  if(windows)for(Int i=0;i<MAX_SLOTS;++i)if(windows[i]){
    const auto& start=candidate.starts()[i];
    if(start.visible)windows[i]->winSetPosition(start.position.x,start.position.y);
    windows[i]->winHide(!start.visible);
  }
}

void positionStartSpots(GameInfo* game,GameWindow* windows[],GameWindow* mapWindow) {
  if(!game){withdraw(windows,mapWindow);return;}
  AsciiString map=game->getMap();
  if(!game->isGameInProgress()){
    const Int local=game->getLocalSlotNum();
    const auto* slot=local>=0 && local<MAX_SLOTS?game->getConstSlot(local):nullptr;
    if(!slot || !slot->hasMap())map.clear();
  }
  positionStartSpots(map,windows,mapWindow);
}

void updateMapStartSpots(GameInfo* game,GameWindow* windows[],Bool loadScreen) {
  const auto* metadata=game && TheMapCache?TheMapCache->findMap(game->getMap()):nullptr;
  if(!metadata){withdraw(windows,nullptr);return;}
  if(!windows)return;
  if(!TheGameText)throw ERROR_BAD_ARG;
  const auto labels=nativeMapPreviewLabels(*game,metadata->m_numPlayers,loadScreen,controls(windows));
  // Stage every fallible label/tooltip before changing any original gadget.
  std::array<UnicodeString,MAX_SLOTS> text,tooltip;
  for(Int i=0;i<MAX_SLOTS;++i)if(windows[i]){
    if(!loadScreen)tooltip[i]=TheGameText->fetch("TOOLTIP:StartPosition");
    if(labels[i]>=0){
      AsciiString key;key.format("NUMBER:%d",labels[i]+1);text[i]=TheGameText->fetch(key);
      if(!loadScreen)tooltip[i].format(TheGameText->fetch("TOOLTIP:StartPositionN"),labels[i]+1);
    }
  }
  for(Int i=0;i<MAX_SLOTS;++i)if(windows[i]){
    GadgetButtonSetText(windows[i],text[i]);if(!loadScreen)windows[i]->winSetTooltip(tooltip[i]);
  }
}
