// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "GameClient/GameClient.h"

// Original logical client, with explicitly absent physical output. This is not
// an interactive renderer and must never be selected for player-facing mode.
class NativeHeadlessGameClient : public GameClient {
public:
  NativeHeadlessGameClient():GameClient(TRUE){}
  Drawable* friend_createDrawable(const ThingTemplate*,DrawableStatus) override;
  void createRayEffectByTemplate(const Coord3D*,const Coord3D*,const ThingTemplate*) override;
  void addScorch(const Coord3D*,Real,Scorches) override {}
  void setTeamColor(Int,Int,Int) override {}
  void adjustLOD(Int) override {}
  void notifyTerrainObjectMoved(Object*) override {}
private:
  Display* createGameDisplay() override {throw ERROR_BAD_ARG;}
  InGameUI* createInGameUI() override {throw ERROR_BAD_ARG;}
  GameWindowManager* createWindowManager() override {throw ERROR_BAD_ARG;}
  FontLibrary* createFontLibrary() override {throw ERROR_BAD_ARG;}
  DisplayStringManager* createDisplayStringManager() override {throw ERROR_BAD_ARG;}
  VideoPlayerInterface* createVideoPlayer() override {throw ERROR_BAD_ARG;}
  IMEManagerInterface* createIMEManager() override {throw ERROR_BAD_ARG;}
  TerrainVisual* createTerrainVisual() override {throw ERROR_BAD_ARG;}
  Keyboard* createKeyboard() override {throw ERROR_BAD_ARG;}
  Mouse* createMouse() override {throw ERROR_BAD_ARG;}
  SnowManager* createSnowManager() override {throw ERROR_BAD_ARG;}
  void setFrameRate(Real) override {}
};
